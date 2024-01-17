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

#include "../platform.h"
#include "../../tools/point.h"
#include "../../ui/ui.h"

#include "haiku_surface.h"

namespace xPlatform
{
static void OpZoomGet(float* sx, float* sy, const ePoint& org_size, const ePoint& size)
{
	*sx = ((float)org_size.x) / size.x;
	*sy = ((float)org_size.y) / size.y;
}

struct eMouseDelta
{
	eMouseDelta() : x(0.0f), y(0.0f) {}
	eMouseDelta(const ePoint& d, float sx, float sy)
	{
		x = sx*d.x;
		y = sy*d.y;
	}
	eMouseDelta& operator+=(const eMouseDelta& d) { x += d.x; y += d.y; return *this; }
	float x;
	float y;
};

static eMouseDelta mouse_delta;

static uint32* memset32(uint32 *buf, uint32 val, size_t count)
{
	while(count--) *buf++ = val;
		return buf;
}

SurfaceView::SurfaceView(BRect rect, int width, int height) :
	BView(rect, "SurfaceView", B_FOLLOW_ALL, B_WILL_DRAW | B_PULSE_NEEDED),
	fBilinear(false),
	fXBRFiltering(false),
	fSmartBorder(false),
	fMouseGrab(false),
	fLastMouseButtons(0),
	fTimerMessageRunner(NULL)
{
	fPixelBufferWidth = width;
	fPixelBufferHeight = height;

	fBufferBitmap = new BBitmap(BRect(0, 0, width - 1, height - 1), B_RGB32);
	width = fPixelBufferWidth * 2;	height = fPixelBufferHeight * 2;
	fBufferBitmap2x = new BBitmap(BRect(0, 0, width - 1, height - 1), B_RGB32);
	width = fPixelBufferWidth * 3;	height = fPixelBufferHeight * 3;
	fBufferBitmap3x = new BBitmap(BRect(0, 0, width - 1, height - 1), B_RGB32);
	width = fPixelBufferWidth * 4;	height = fPixelBufferHeight * 4;
	fBufferBitmap4x = new BBitmap(BRect(0, 0, width - 1, height - 1), B_RGB32);

	fXbrData = (xbr_data*)malloc(sizeof(xbr_data));
	xbr_init_data(fXbrData);

	fXbrParams.data = fXbrData;
	fXbrParams.input = (uint8*)fBufferBitmap->Bits();
	fXbrParams.output = (uint8*)fBufferBitmap2x->Bits();
	fXbrParams.inWidth = fPixelBufferWidth;
	fXbrParams.inHeight = fPixelBufferHeight;
	fXbrParams.inPitch = fBufferBitmap->BytesPerRow();
	fXbrParams.outPitch = fBufferBitmap2x->BytesPerRow();

	fBufferOffscreenView = new BView(Bounds(), "bufferOffscreenView", B_FOLLOW_ALL_SIDES, 0);
	fBufferOffscreenBitmap = new BBitmap(Bounds(), B_RGB32, true);
	fBufferOffscreenBitmap->AddChild(fBufferOffscreenView);
	fBufferOffscreenView->SetDrawingMode(B_OP_COPY);
	SetDrawingMode(B_OP_COPY);
}

SurfaceView::~SurfaceView()
{
	if (fTimerMessageRunner)
		delete fTimerMessageRunner;
	if (fBufferBitmap)
		delete fBufferBitmap;
	if (fBufferBitmap2x)
		delete fBufferBitmap2x;
	if (fBufferBitmap3x)
		delete fBufferBitmap3x;
	if (fBufferBitmap4x)
		delete fBufferBitmap4x;
	if (fBufferOffscreenBitmap)
		delete fBufferOffscreenBitmap;
}

void
SurfaceView::AttachedToWindow()
{
	BMessenger messenger(this, Window());
	BMessage message(kRepaintEvent);
	fTimerMessageRunner = new BMessageRunner(messenger, &message, 1000000 / 50);
}

void
SurfaceView::MessageReceived(BMessage *message)
{
	switch (message->what) {
		case kRepaintEvent:
		{
			Repaint();
			return;
		}
		default:
			break;
	}
	BView::MessageReceived(message);
}

void 
SurfaceView::Draw(BRect rect)
{
	DrawBitmap(fBufferOffscreenBitmap, rect, rect);
}

void 
SurfaceView::MouseMoved(BPoint p, uint32 transit, const BMessage *message)
{
	if (fMouseGrab) {
		ePoint size;
		size.x = Bounds().Width();
		size.y = Bounds().Height();

		float sx, sy;
		OpZoomGet(&sx, &sy, ePoint(xUi::WIDTH, xUi::HEIGHT), size);

		BPoint center(Bounds().Width() / 2, Bounds().Height() / 2);
		BPoint shift = p - center;

		float scale_x = float(xUi::WIDTH) / size.x / sx;
		float scale_y = float(xUi::HEIGHT) / size.y / sy;
		mouse_delta += eMouseDelta(ePoint(shift.x, shift.y), scale_x, scale_y);

		int dx = mouse_delta.x;
		int dy = mouse_delta.y;
		if(dx || dy) {
			mouse_delta.x -= dx;
			mouse_delta.y -= dy;
			Handler()->OnMouse(MA_MOVE, dx, -dy);
			center = ConvertToScreen(center);
			set_mouse_position(center.x, center.y);
		}
	}
}

void
SurfaceView::MouseDown(BPoint p)
{
	if (fMouseGrab) {
		uint32 buttons = Window()->CurrentMessage()->FindInt32("buttons");
		Handler()->OnMouse(MA_BUTTON, buttons & B_PRIMARY_MOUSE_BUTTON ? 0 : 1, 1);
		fLastMouseButtons = buttons;
	}
}

void
SurfaceView::MouseUp(BPoint p)
{	
	if (fMouseGrab) {
		uint32 buttons = 0;
		GetMouse(&p, &buttons, false);
		Handler()->OnMouse(MA_BUTTON,
			((fLastMouseButtons & B_PRIMARY_MOUSE_BUTTON) &&
			!(buttons & B_PRIMARY_MOUSE_BUTTON)) ? 0 : 1, 0);
	}
}

void
SurfaceView::EnableMouseGrab(bool enabled)
{
	fMouseGrab = enabled;
	if (enabled) {
		BPoint center(Bounds().Width() / 2, Bounds().Height() / 2);
		center = ConvertToScreen(center);
		set_mouse_position(center.x, center.y);
	}
}

void 
SurfaceView::Repaint()
{
	ResizeOffscreenBitmap(Bounds().Width(), Bounds().Height());
	fBufferOffscreenBitmap->Lock();

	BRect rect = Bounds();
	BRect bmpRect = fBufferOffscreenBitmap->Bounds();

	float kx = rect.Width() / (float)fPixelBufferWidth;
	float ky = rect.Height() / (float)fPixelBufferHeight;

	BRect dstRect = rect;
	if (kx >= ky) {
		float w = fPixelBufferWidth * ky;
		float h = fPixelBufferHeight * ky;
		dstRect.left = (rect.Width() - w) / 2 - 1;
		dstRect.top = 0;
		dstRect.right = dstRect.left + w + 3;
		dstRect.bottom = h;
	} else {
		float w = fPixelBufferWidth * kx;
		float h = fPixelBufferHeight * kx;
		dstRect.left = 0;
		dstRect.top = (rect.Height() - h) / 2;
		dstRect.right = w;
		dstRect.bottom = dstRect.top + h;
	}

	bool scale1x = bmpRect.Width() - 1 <= xUi::WIDTH && bmpRect.Height() - 1 <= xUi::HEIGHT;

	if (!fSmartBorder)
		memset(fBufferOffscreenBitmap->Bits(), 0, fBufferOffscreenBitmap->BitsLength());

	int multiplyIndex = ceilf(bmpRect.Width() / xUi::WIDTH) - 1;

	if (fXBRFiltering && multiplyIndex > 1) {
		if (multiplyIndex == 2) {
			fXbrParams.output = (uint8*)fBufferBitmap2x->Bits();
			fXbrParams.outPitch = fBufferBitmap2x->BytesPerRow();
			xbr_filter_xbr2x(&fXbrParams);
			fBufferOffscreenView->DrawBitmap(fBufferBitmap2x, fBufferBitmap2x->Bounds(),
				dstRect, (fBilinear && !scale1x) ? B_FILTER_BITMAP_BILINEAR : 0);
		} else if (multiplyIndex == 3) {
			fXbrParams.output = (uint8*)fBufferBitmap3x->Bits();
			fXbrParams.outPitch = fBufferBitmap3x->BytesPerRow();
			xbr_filter_xbr3x(&fXbrParams);
			fBufferOffscreenView->DrawBitmap(fBufferBitmap3x, fBufferBitmap3x->Bounds(),
				dstRect, (fBilinear && !scale1x) ? B_FILTER_BITMAP_BILINEAR : 0);
		} else {
			fXbrParams.output = (uint8*)fBufferBitmap4x->Bits();
			fXbrParams.outPitch = fBufferBitmap4x->BytesPerRow();
			xbr_filter_xbr4x(&fXbrParams);
			fBufferOffscreenView->DrawBitmap(fBufferBitmap4x, fBufferBitmap4x->Bounds(),
				dstRect, (fBilinear && !scale1x) ? B_FILTER_BITMAP_BILINEAR : 0);
		}
	} else {
		fBufferOffscreenView->DrawBitmap(fBufferBitmap, fBufferBitmap->Bounds(),
			dstRect, (fBilinear && !scale1x) ? B_FILTER_BITMAP_BILINEAR : 0);
	}

	if (fSmartBorder) {
		uint32 *bmp = (uint32*)fBufferOffscreenBitmap->Bits();
		uint32 *endBuffer = bmp + (fBufferOffscreenBitmap->BitsLength() / 4);
		int32 rowSize = fBufferOffscreenBitmap->BytesPerRow() / 4;
		if (kx > ky) {
			for (int y = 0; y < bmpRect.Height() + 1; y++) {
				uint32 value = bmp[y * rowSize + (uint32)dstRect.left + 1];
				uint32 *dst = bmp + y * rowSize;
				memset32(dst, value, MIN((uint32)dstRect.left + 1, endBuffer - dst));
				value = bmp[y * rowSize + (uint32)dstRect.right - 1];
				dst = bmp + y * rowSize + (uint32)dstRect.right;
				memset32(dst, value, MIN(rowSize - (uint32)dstRect.right, endBuffer - dst));
			}
		} else {
			uint32 topColor = bmp[int32(dstRect.top + 0.5) * rowSize];
			uint32 bottomColor = bmp[int32(dstRect.bottom - 0.5) * rowSize];
			memset32(bmp, topColor, int32(dstRect.top + 0.5) * rowSize);
			uint32 *dst = bmp + int32(dstRect.bottom + 0.5) * rowSize;
			memset32(dst, bottomColor, MIN(int32(rect.bottom - dstRect.bottom + 1) * rowSize, endBuffer - dst));
		}
	}

	DrawBitmap(fBufferOffscreenBitmap);
	fBufferOffscreenBitmap->Unlock();
}

void
SurfaceView::ResizeOffscreenBitmap(int width, int height)
{
	if (((int)fBufferOffscreenBitmap->Bounds().Width()) == width &&
		((int)fBufferOffscreenBitmap->Bounds().Height()) == height) {
		return;
	}

	fBufferOffscreenBitmap->RemoveChild(fBufferOffscreenView);
	delete fBufferOffscreenBitmap;

	fBufferOffscreenView->ResizeTo(width, height);
	fBufferOffscreenBitmap = new BBitmap(BRect(0, 0, width, height), B_RGB32, true);
	fBufferOffscreenBitmap->AddChild(fBufferOffscreenView);
}

}
