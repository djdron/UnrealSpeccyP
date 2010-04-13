#ifndef	__DIALOGS_H__
#define	__DIALOGS_H__

#include "controls.h"
#include "../platform/platform.h"

#pragma once

namespace xUi
{

//*****************************************************************************
//	eFileOpenDialog
//-----------------------------------------------------------------------------
class eFileOpenDialog : public eDialog
{
	typedef eDialog eInherited;
public:
	eFileOpenDialog(const char* _path) : list(NULL), selected(NULL)
	{
		strcpy(path, _path);
		memset(folders, 0, sizeof(folders));
	}
	virtual void Init();
	const char* Selected() { return selected; }
	virtual void OnKey(char key);
protected:
	void OnChangePath();
	enum { BACKGROUND_COLOR = 0x01000000 };
protected:
	char path[256];
	eList* list;
	bool folders[256];
	const char* selected;
};

class eManager
{
public:
	eManager(const char* _path) : dialog(NULL), key('\0'), keypress_timer(0)
	{
		strcpy(path, _path);
	}
	~eManager() { SAFE_DELETE(dialog); }
	void Init()
	{
		xRender::_CreateFont(6, 6, "res/spxtrm4f.fnt");
	}
	dword* VideoData() const { return dialog ? xRender::Screen() : NULL; }
	bool Focused() const { return dialog; }
	void Update()
	{
		if(key)
		{
			if(!keypress_timer || keypress_timer > KEY_REPEAT_DELAY)
			{
				SAFE_CALL(dialog)->OnKey(key);
			}
			++keypress_timer;
		}
		SAFE_CALL(dialog)->Update();
		if(dialog && dialog->Selected())
		{
			xPlatform::Handler()->OnOpenFile(dialog->Selected());
			SAFE_DELETE(dialog);
		}
	}
	void OnKey(char _key)
	{
		if(_key == key)
			return;
		key = _key;
		if(!key)
			keypress_timer = 0;
		if(key == '`')
		{
			if(!dialog)
			{
				dialog = new eFileOpenDialog(path);
				dialog->Init();
			}
			else
			{
				SAFE_DELETE(dialog);
			}
		}
	}
	enum { KEY_REPEAT_DELAY = 10 };
protected:
	char path[256];
	xUi::eFileOpenDialog* dialog;
	char key;
	int keypress_timer;
};

}
//namespace xUi

#endif//__DIALOGS_H__
