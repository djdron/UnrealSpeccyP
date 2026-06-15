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

package app.usp;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentActivity;
import androidx.viewpager2.adapter.FragmentStateAdapter;

import app.usp.fs.FileSelectorFS;
import app.usp.fs.FileSelectorRZX;
import app.usp.fs.FileSelectorVtrdos;
import app.usp.fs.FileSelectorWOS;
import app.usp.fs.FileSelectorBBB;

public class FileTabsAdapter extends FragmentStateAdapter {

	public FileTabsAdapter(@NonNull FragmentActivity fragmentActivity) {
		super(fragmentActivity);
	}
	@NonNull
	@Override
	public Fragment createFragment(int position)
	{
		switch(position)
		{
			case 1: return new FileSelectorVtrdos();
			case 2: return new FileSelectorWOS();
			case 3: return new FileSelectorRZX();
			case 4: return new FileSelectorBBB();
			default: return new FileSelectorFS();
		}
	}
	@Override
	public int getItemCount() {
		return 5;
	}
}
