/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2026 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

package app.usp.fs

import java.io.File
import java.util.ArrayList

import android.content.DialogInterface
import android.os.Bundle
import android.widget.TextView
import android.widget.Toast
import android.widget.ProgressBar
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import androidx.appcompat.app.AlertDialog
import androidx.lifecycle.LifecycleOwner
import androidx.lifecycle.lifecycleScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import app.usp.R

abstract class FileSelector : Fragment() {

	interface Progress {
		fun OnProgress(current: Int?, max: Int?)
		fun Canceled(): Boolean
	}

	class State {
		var current_path: File = File("/")
		var last_name: String = ""
		var items: MutableList<FileSelectorSource.Item> = ArrayList()
	}

	abstract fun getState(): State

	fun getItems(): MutableList<FileSelectorSource.Item> {
		return getState().items
	}

	abstract fun LongUpdate(path: File): Boolean
	abstract fun LongUpdateTitle(): Int

	protected var update_on_resume: Boolean = true
	private var async_task: Boolean = false
	protected lateinit var recyclerView: RecyclerView
	protected var adapter: MyListAdapter? = null

	fun PathLevel(path: File): Int {
		var p: File? = path
		var l = 0
		while (p?.parentFile.also { p = it } != null) {
			++l
		}
		return l
	}

	protected var sources: MutableList<FileSelectorSource> = ArrayList()

	override fun onCreateView(
		inflater: LayoutInflater,
		container: ViewGroup?,
		savedInstanceState: Bundle?
	): View? {
		val view = inflater.inflate(R.layout.fragment_list, container, false)
		recyclerView = view.findViewById(R.id.recyclerView)
		recyclerView.layoutManager = LinearLayoutManager(context)
		return view
	}

	override fun onResume() {
		super.onResume()
		if (update_on_resume) {
			Update()
		}
		update_on_resume = true
	}

	fun Update() {
		if (getItems().size > 0) {
			SetItems()
			SelectItem(getState().last_name)
		} else {
			UpdateAsync(context as LifecycleOwner, getState().current_path, getState().last_name, LongUpdateTitle())
		}
	}

	private fun SetItems() {
		adapter = MyListAdapter(ArrayList(getItems()))
		recyclerView.adapter = adapter
	}

	private fun SelectItem(name: String) {
		if (name.isNotEmpty()) {
			var idx = 0
			for (i in getItems()) {
				if (i.name == name) {
					recyclerView.scrollToPosition(idx)
					break
				}
				++idx
			}
		}
	}

	protected fun FileClicked(position: Int) {
		if (async_task) return
		val f = getItems()[position].name
		if (f == "/..") {
			val parent = getState().current_path.parentFile
			if (parent != null) {
				UpdateAsync(context as LifecycleOwner, parent, "/" + getState().current_path.name, LongUpdateTitle())
			}
		} else {
			if (f.startsWith("/")) {
				UpdateAsync(context as LifecycleOwner, File(getState().current_path.path + f), "", LongUpdateTitle())
			} else {
				getState().last_name = f
				ApplyAsync(context as LifecycleOwner, getItems()[position])
			}
		}
	}

	abstract inner class FSSProgressDialog(
		private val res_title: Int,
		private val res_message: Int
	) : Progress, DialogInterface.OnCancelListener {

		private var dialog: AlertDialog? = null
		private var progressBar: ProgressBar? = null
		private var time_last: Long = 0
		var value_last: Int = 0
		var canceled: Boolean = false
		fun Create() {
			val inflater = LayoutInflater.from(context)
			val view = inflater.inflate(R.layout.progress_dialog_layout, null)

			progressBar = view.findViewById(R.id.progress_bar)
			val messageTextView = view.findViewById<TextView>(R.id.progress_message)
			messageTextView?.setText(res_message)

			dialog = AlertDialog.Builder(requireContext())
				.setTitle(res_title)
				.setView(view)
				.setOnCancelListener(this)
				.setCancelable(true)
				.setNegativeButton(android.R.string.cancel) { dialogInterface, _ ->
					dialogInterface.cancel()
				}
				.create()

			dialog?.setCanceledOnTouchOutside(false)
			dialog?.show()
			time_last = System.nanoTime()
		}

		fun Destroy() {
			dialog?.dismiss()
			dialog = null
			progressBar = null
		}

		fun Update(value: Int, value_max: Int) {
			if (dialog == null || Canceled()) return

			val time = System.nanoTime()
			if (time - time_last < 0.5 * 1e9) return
			time_last = time

			progressBar?.let { pb ->
				pb.max = value_max
				pb.progress = value
			}
			value_last = value
		}
		override fun Canceled(): Boolean {
			return canceled
		}
		override fun onCancel(di: DialogInterface) {
			canceled = true
			dialog?.dismiss()

			dialog = AlertDialog.Builder(requireContext())
				.setTitle(res_title)
				.setMessage(getString(R.string.canceling))
				.setCancelable(false)
				.create()
			dialog?.show()
		}
	}

	private fun ApplyAsync(lifecycleOwner: LifecycleOwner, item: FileSelectorSource.Item) {
		lifecycleOwner.lifecycleScope.launch {
			async_task = true

			val progress = object : FSSProgressDialog(R.string.accessing_web, R.string.downloading_image) {
				override fun OnProgress(current: Int?, max: Int?) {
					if (current != null && max != null) {
						lifecycleOwner.lifecycleScope.launch(Dispatchers.Main) {
							Update(current, max)
						}
					}
				}
			}

			progress.Create()

			val result = withContext(Dispatchers.IO) {
				item.source.ApplyItem(item, progress)
			}

			progress.Destroy()

			var e: String? = null
			when(result) {
				FileSelectorSource.ApplyResult.OK -> activity?.finish()
				FileSelectorSource.ApplyResult.FAIL -> e = getString(R.string.file_select_failed)
				FileSelectorSource.ApplyResult.UNABLE_CONNECT1 -> e = getString(R.string.file_select_unable_connect1)
				FileSelectorSource.ApplyResult.UNABLE_CONNECT2 -> e = getString(R.string.file_select_unable_connect2)
				FileSelectorSource.ApplyResult.INVALID_INFO -> e = getString(R.string.file_select_invalid_info)
				FileSelectorSource.ApplyResult.NOT_AVAILABLE -> e = getString(R.string.file_select_not_available)
				FileSelectorSource.ApplyResult.UNSUPPORTED_FORMAT -> e = getString(R.string.file_select_unsupported_format)
				else -> {}
			}
			if (e != null) {
				val me = String.format(getString(R.string.file_select_open_error), e)
				Toast.makeText(context, me, Toast.LENGTH_LONG).show()
			}
			async_task = false
		}
	}

	private fun UpdateAsync(lifecycleOwner: LifecycleOwner, path: File, select_after_update: String, res_title: Int) {
		lifecycleOwner.lifecycleScope.launch {
			async_task = true
			val progress = object : FSSProgressDialog(res_title, R.string.gathering_list) {
				override fun OnProgress(current: Int?, max: Int?) {
					if (current != null && max != null) {
						lifecycleOwner.lifecycleScope.launch(Dispatchers.Main) {
							Update(current, max)
						}
					}
				}
			}

			if (LongUpdate(path)) progress.Create()

			val items = ArrayList<FileSelectorSource.Item>()
			val result = withContext(Dispatchers.IO) {
				var r = FileSelectorSource.GetItemsResult.OK
				for (s in sources) {
					r = s.GetItems(path, items, progress)
					if (r != FileSelectorSource.GetItemsResult.OK) break
				}
				r
			}

			progress.Destroy()
			var e: String? = null
			when(result) {
				FileSelectorSource.GetItemsResult.FAIL -> e = getString(R.string.file_select_failed)
				FileSelectorSource.GetItemsResult.UNABLE_CONNECT -> e = getString(R.string.file_select_unable_connect1)
				FileSelectorSource.GetItemsResult.INVALID_INFO -> e = getString(R.string.file_select_invalid_info)
				FileSelectorSource.GetItemsResult.OK -> {
					getState().current_path = path
					getState().items = items
					SetItems()
					if(select_after_update.isNotEmpty()) {
						SelectItem(select_after_update)
					}
				}
				else -> {}
			}
			if (e != null) {
				val me = String.format(getString(R.string.file_select_update_error), e)
				Toast.makeText(context, me, Toast.LENGTH_LONG).show()
			}
			async_task = false
		}
	}

	protected inner class MyListAdapter(private val items: List<FileSelectorSource.Item>) :
		RecyclerView.Adapter<MyListAdapter.FileViewHolder>() {

		private val dp_5: Int
		private val dp_10: Int

		init {
			val scale = resources.displayMetrics.density
			dp_5 = (5 * scale + 0.5f).toInt()
			dp_10 = (10 * scale + 0.5f).toInt()
		}

		override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): FileViewHolder {
			val view = LayoutInflater.from(parent.context)
				.inflate(R.layout.file_selector_item, parent, false)
			return FileViewHolder(view)
		}

		override fun onBindViewHolder(holder: FileViewHolder, position: Int) {
			val item = items[position]
			holder.t1.text = item.name
			holder.t2.text = item.desc
			if (item.desc == null) {
				holder.t2.visibility = View.GONE
				holder.itemView.setPadding(dp_10, dp_10, dp_10, dp_10)
			} else {
				holder.t2.visibility = View.VISIBLE
				holder.itemView.setPadding(dp_10, dp_5, dp_10, dp_5)
			}
			holder.itemView.setOnClickListener { FileClicked(position) }
		}

		override fun getItemCount(): Int {
			return items.size
		}

		inner class FileViewHolder(itemView: View) : RecyclerView.ViewHolder(itemView) {
			val t1: TextView = itemView.findViewById(R.id.textLine)
			val t2: TextView = itemView.findViewById(R.id.textLine2)
		}
	}
}