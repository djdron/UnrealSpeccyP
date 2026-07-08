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

package app.usp

import android.os.Bundle
import android.view.MenuItem
import android.view.View

import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat
import androidx.fragment.app.FragmentActivity
import androidx.viewpager2.widget.ViewPager2
import com.google.android.material.tabs.TabLayout
import com.google.android.material.tabs.TabLayoutMediator

class FileOpen : FragmentActivity() {

	private lateinit var viewPager: ViewPager2

	companion object {
		private var active_tab = 0
	}

	override fun onCreate(savedInstanceState: Bundle?) {
		super.onCreate(savedInstanceState)
		setContentView(R.layout.file_open)

		val rootLayout = findViewById<View>(android.R.id.content)
		ViewCompat.setOnApplyWindowInsetsListener(rootLayout) { view, insets ->
			val systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars())
			view.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom)
			insets
		}

		val tabLayout = findViewById<TabLayout>(R.id.tabLayout)
		viewPager = findViewById(R.id.viewPager)

		val tabsAdapter = FileTabsAdapter(this)
		viewPager.adapter = tabsAdapter

		TabLayoutMediator(tabLayout, viewPager) { tab, position ->
			when (position) {
				0 -> tab.setText(R.string.file_system)
				1 -> tab.setText(R.string.vtrdos)
				2 -> tab.setText(R.string.wos)
				3 -> tab.setText(R.string.rzx)
				4 -> tab.setText(R.string.bbb)
			}
		}.attach()

		viewPager.setCurrentItem(active_tab, false)
	}

	override fun onDestroy() {
		active_tab = viewPager.currentItem
		super.onDestroy()
	}

	override fun onOptionsItemSelected(item: MenuItem): Boolean {
		if (item.itemId == android.R.id.home) {
			onBackPressedDispatcher.onBackPressed()
			return true
		}
		return super.onOptionsItemSelected(item)
	}
}
