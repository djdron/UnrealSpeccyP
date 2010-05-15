/*
 Portable ZX-Spectrum emulator.
 Copyright (C) 2001-2010 SMT, Dexus, Alone Coder, deathsoft, djdron, scor
 
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

#include "../../std.h"

#ifdef _MAC

#undef self

#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include "../io.h"

static struct eBundleInit
{
	eBundleInit()
	{
		CFBundleRef bundle = CFBundleGetMainBundle();
		if(!bundle)
			return;
		CFURLRef bundle_url = CFBundleCopyBundleURL(bundle);
		if(!bundle_url)
			return;
		FSRef res_path_ref;
		bool ok = CFURLGetFSRef(bundle_url, &res_path_ref);
		if(!ok)
			return;
		char res_path[xIo::MAX_PATH_LEN];
		OSStatus s = FSRefMakePath(&res_path_ref, (UInt8*)res_path, xIo::MAX_PATH_LEN);
		if(s)
			return;
		strcat(res_path, "/Contents/Resources/");
		xIo::SetResourcePath(res_path);
	}
} bi;

#endif//_MAC
