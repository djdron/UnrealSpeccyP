/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2020 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

package app.usp.fs;

import java.io.File;
import java.util.List;

abstract class FileSelectorSource
{
	class Item
	{
		Item(FileSelectorSource _source, final String _name) { source = _source; name = _name; }

		FileSelectorSource source;

		String name;
		String desc;
		String url;
	};
	enum GetItemsResult { OK, FAIL, UNABLE_CONNECT, INVALID_INFO, CANCELED, NEED_STORAGE_PERMISSION }
	abstract public GetItemsResult GetItems(final File path, List<Item> items, FileSelector.Progress progress);
	enum ApplyResult { OK, FAIL, UNABLE_CONNECT1, UNABLE_CONNECT2, INVALID_INFO, NOT_AVAILABLE, UNSUPPORTED_FORMAT, CANCELED }
	abstract public ApplyResult ApplyItem(Item item, FileSelector.Progress progress);
}
