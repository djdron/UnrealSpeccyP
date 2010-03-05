#include "../std.h"
#include "device.h"

//=============================================================================
//	eDevices::eDevices
//-----------------------------------------------------------------------------
eDevices::eDevices()
{
	memset(items, 0, sizeof(items));
}
//=============================================================================
//	eDevices::~eDevices
//-----------------------------------------------------------------------------
eDevices::~eDevices()
{
	for(int i = 0; i < D_COUNT; ++i)
	{
		SAFE_DELETE(items[i]);
	}
}
//=============================================================================
//	eDevices::Reset
//-----------------------------------------------------------------------------
void eDevices::Reset()
{
	for(int i = 0; i < D_COUNT; ++i)
	{
		SAFE_CALL(items[i])->Reset();
	}
}
//=============================================================================
//	eDevices::FrameStart
//-----------------------------------------------------------------------------
void eDevices::FrameStart()
{
	for(int i = 0; i < D_COUNT; ++i)
	{
		SAFE_CALL(items[i])->FrameStart();
	}
}
//=============================================================================
//	eDevices::FrameUpdate
//-----------------------------------------------------------------------------
void eDevices::FrameUpdate()
{
	for(int i = 0; i < D_COUNT; ++i)
	{
		SAFE_CALL(items[i])->FrameUpdate();
	}
}
//=============================================================================
//	eDevices::FrameEnd
//-----------------------------------------------------------------------------
void eDevices::FrameEnd(dword tacts)
{
	for(int i = 0; i < D_COUNT; ++i)
	{
		SAFE_CALL(items[i])->FrameEnd(tacts);
	}
}
//=============================================================================
//	eDevices::_Add
//-----------------------------------------------------------------------------
void eDevices::_Add(eDeviceId id, eDevice* d)
{
	assert(d && !items[id]);
	d->Init();
	items[id] = d;
}
//=============================================================================
//	eDevices::IoRead
//-----------------------------------------------------------------------------
byte eDevices::IoRead(word port, int tact) const
{
	byte v = 0xff;
	for(int i = 0; i < D_COUNT; ++i)
	{
		SAFE_CALL(items[i])->IoRead(port, &v, tact);
	}
	return v;
}
//=============================================================================
//	eDevices::IoWrite
//-----------------------------------------------------------------------------
void eDevices::IoWrite(word port, byte v, int tact)
{
	for(int i = 0; i < D_COUNT; ++i)
	{
		SAFE_CALL(items[i])->IoWrite(port, v, tact);
	}
}
