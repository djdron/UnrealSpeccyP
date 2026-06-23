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

import android.app.ProgressDialog
import android.content.DialogInterface
import android.os.Bundle
import android.widget.TextView
import android.widget.Toast
import android.os.AsyncTask
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
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
			UpdateAsync(getState().current_path, getState().last_name, LongUpdateTitle()).execute()
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
				UpdateAsync(parent, "/" + getState().current_path.name, LongUpdateTitle()).execute()
			}
		} else {
			if (f.startsWith("/")) {
				UpdateAsync(File(getState().current_path.path + f), "", LongUpdateTitle()).execute()
			} else {
				getState().last_name = f
				ApplyAsync(getItems()[position]).execute()
			}
		}
	}

	abstract inner class FSSProgressDialog(
		private val res_title: Int,
		private val res_message: Int
	) : Progress, DialogInterface.OnCancelListener {

		private var pd: ProgressDialog? = null
		private var time_last: Long = 0
		var value_last: Int = 0
		var canceled: Boolean = false

		fun Create() {
			pd = CreateProgress()
			pd?.show()
			time_last = System.nanoTime()
		}

		fun Destroy() {
			pd?.dismiss()
		}

		fun Update(value: Int, value_max: Int) {
			if (pd == null) return
			if (Canceled()) return
			val time = System.nanoTime()
			if (time - time_last < 0.5 * 1e9) return // update each 0.5 sec
			time_last = time
			if (pd!!.isIndeterminate && (value - value_last) * 3 < value_max) {
				pd?.dismiss()
				pd = CreateProgress()
				pd?.isIndeterminate = false
				pd?.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL)
				pd?.show()
			}
			if (!pd!!.isIndeterminate) {
				pd?.max = value_max
				pd?.progress = value
			}
			value_last = value
		}

		private fun CreateProgress(): ProgressDialog {
			val pd = ProgressDialog(context)
			pd.setTitle(getString(res_title))
			pd.setMessage(getString(res_message))
			pd.setOnCancelListener(this)
			pd.setCancelable(true)
			pd.setCanceledOnTouchOutside(false)
			return pd
		}

		override fun Canceled(): Boolean {
			return canceled
		}

		override fun onCancel(di: DialogInterface) {
			canceled = true
			pd?.dismiss()
			pd = ProgressDialog(context)
			pd?.setTitle(getString(res_title))
			pd?.setMessage(getString(R.string.canceling))
			pd?.setCancelable(false)
			pd?.show()
		}
	}

	private inner class ApplyAsync(var item: FileSelectorSource.Item) :
		AsyncTask<Void?, Int?, FileSelectorSource.ApplyResult>() {

		private val progress: FSSProgressDialog = object : FSSProgressDialog(
			R.string.accessing_web,
			R.string.downloading_image
		) {
			override fun OnProgress(current: Int?, max: Int?) {
				publishProgress(current, max)
			}
		}

		override fun onPreExecute() {
			async_task = true
			if (LongUpdate(getState().current_path)) {
				progress.Create()
			}
		}

		override fun doInBackground(vararg args: Void?): FileSelectorSource.ApplyResult {
			return item.source.ApplyItem(item, progress)
		}

		override fun onProgressUpdate(vararg values: Int?) {
			progress.Update(values[0]!!, values[1]!!)
		}

		override fun onPostExecute(r: FileSelectorSource.ApplyResult) {
			progress.Destroy()
			var e: String? = null
			when(r) {
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
			if (r == FileSelectorSource.ApplyResult.OK) {
				activity?.finish()
			}
		}
	}

	private inner class UpdateAsync(
		private val path: File,
		private val select_after_update: String,
		res_title: Int
	) : AsyncTask<Void?, Int?, FileSelectorSource.GetItemsResult>() {

		private val progress: FSSProgressDialog = object : FSSProgressDialog(
			res_title,
			R.string.gathering_list
		) {
			override fun OnProgress(current: Int?, max: Int?) {
				publishProgress(current, max)
			}
		}
		private val items: MutableList<FileSelectorSource.Item> = ArrayList()

		override fun onPreExecute() {
			async_task = true
			if (LongUpdate(path)) {
				progress.Create()
			}
		}

		override fun doInBackground(vararg args: Void?): FileSelectorSource.GetItemsResult {
			for (s in sources) {
				val r = s.GetItems(path, items, progress)
				if (r != FileSelectorSource.GetItemsResult.OK) return r
			}
			return FileSelectorSource.GetItemsResult.OK
		}

		override fun onProgressUpdate(vararg values: Int?) {
			progress.Update(values[0]!!, values[1]!!)
		}

		override fun onPostExecute(r: FileSelectorSource.GetItemsResult) {
			progress.Destroy()
			var e: String? = null
			when(r) {
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