/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2021 SMT, Dexus, Alone Coder, deathsoft, djdron, scor, 3dEyes

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

#ifndef _HAIKU_SURFACE_VIEW_H
#define _HAIKU_SURFACE_VIEW_H

#include <stdio.h>

#include <AppKit.h>
#include <KernelKit.h>
#include <InterfaceKit.h>
#include <Cursor.h>
#include <game/WindowScreen.h>

#include "haiku_filters.h"

namespace xPlatform
{

const uint32 kRepaintEvent = 'RPNT';

class SurfaceView : public BView 
{
 public:
							SurfaceView(BRect rect, int width, int height);
							~SurfaceView();

		virtual void		MouseDown(BPoint point);
		virtual void		MouseUp(BPoint point);
		virtual void		MouseMoved(BPoint p, uint32 transit, const BMessage *message);
		virtual void		MessageReceived(BMessage *msg);
		virtual	void		AttachedToWindow();
		virtual void		Draw(BRect r);

		void				LockBuffer() { fBufferBitmap->Lock(); }
		void				UnlockBuffer() { fBufferBitmap->Unlock(); }
		uint32*				GetBuffer() { return (uint32*)fBufferBitmap->Bits(); }
		uint32				GetBufferSize() { return fBufferBitmap->BitsLength(); }

		void				EnableBilinear(bool enabled) {fBilinear = enabled; }
		void				EnableXBR(bool enabled) {fXBRFiltering = enabled; }
		void				EnableSmartBorder(bool enabled) {fSmartBorder = enabled; }
		void				EnableMouseGrab(bool enabled);

		int					Width() { return fPixelBufferWidth; }
		int					Height() { return fPixelBufferHeight; }

 private:
		void				ResizeOffscreenBitmap(int width, int height);
 		void 				Repaint();

 		BMessageRunner*		fTimerMessageRunner;

 		BView*				fBufferOffscreenView;
		BBitmap*			fBufferOffscreenBitmap;
		BBitmap*			fBufferBitmap;
		BBitmap*			fBufferBitmap2x;
		BBitmap*			fBufferBitmap3x;
		BBitmap*			fBufferBitmap4x;

 		int					fPixelBufferWidth;
 		int					fPixelBufferHeight;

 		bool				fBilinear;
 		bool				fXBRFiltering;
 		bool				fSmartBorder;
 		bool				fMouseGrab;

 		uint32				fLastMouseButtons;

		xbr_data*			fXbrData;
		xbr_params 			fXbrParams;
};

}
#endif
