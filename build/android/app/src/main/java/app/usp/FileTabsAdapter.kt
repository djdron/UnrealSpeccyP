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

import androidx.fragment.app.Fragment
import androidx.fragment.app.FragmentActivity
import androidx.viewpager2.adapter.FragmentStateAdapter

import app.usp.fs.FileSelectorFS
import app.usp.fs.FileSelectorRZX
import app.usp.fs.FileSelectorVtrdos
import app.usp.fs.FileSelectorWOS
import app.usp.fs.FileSelectorBBB

class FileTabsAdapter(fragmentActivity: FragmentActivity) : FragmentStateAdapter(fragmentActivity) {

	override fun createFragment(position: Int): Fragment {
		return when (position) {
			1 -> FileSelectorVtrdos()
			2 -> FileSelectorWOS()
			3 -> FileSelectorRZX()
			4 -> FileSelectorBBB()
			else -> FileSelectorFS()
		}
	}

	override fun getItemCount(): Int {
		return 5
	}
}
