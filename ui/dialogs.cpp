#include "../std.h"
#include "dialogs.h"

#include <io.h>

namespace xUi
{

//=============================================================================
//	eFileOpenDialog::Init
//-----------------------------------------------------------------------------
void eFileOpenDialog::Init()
{
	background = BACKGROUND_COLOR;
	eRect r(8, 8, 140, xRender::HEIGHT - 8);
	Bound() = r;
	list = new eList;
	list->Bound() = eRect(6, 6, r.Width() - 6, r.Height() - 6);
	Insert(list);
	OnChangePath();
}
//=============================================================================
//	eFileOpenDialog::OnChangePath
//-----------------------------------------------------------------------------
void eFileOpenDialog::OnChangePath()
{
	_finddata_t fd;
	dword handle = _findfirst(path, &fd);
	dword res = handle;
	list->Clear();
	memset(folders, 0, sizeof(folders));
	int i = 0;
	while(res != -1)
	{
		if(strcmp(fd.name, "."))
		{
			folders[i++] = fd.attrib&0x10;
			list->Insert(fd.name);
		}
		res = _findnext(handle, &fd);
	}
	_findclose(handle);
}
static void GetUpLevel(char* path, int level = 1)
{
	for(int i = strlen(path); --i >= 0; )
	{
		if(((path[i] == '\\') || (path[i] == '/')) && !--level)
		{
			path[i + 1] = '\0';
			return;
		}
	}
}
//=============================================================================
//	eFileOpenDialog::OnKey
//-----------------------------------------------------------------------------
void eFileOpenDialog::OnKey(char key)
{
	if(key == 'e')
	{
		if(folders[list->Selector()])
		{
			GetUpLevel(path);
			strcat(path, list->Selected());
			strcat(path, "/*");
			OnChangePath();
			return;
		}
		GetUpLevel(path);
		strcat(path, list->Selected());
		selected = path;
		return;
	}
	eInherited::OnKey(key);
}

}
//namespace xUi
