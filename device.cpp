#include "std.h"

#include "device.h"

//=============================================================================
//	eDevices::eDevices
//-----------------------------------------------------------------------------
eDevices::eDevices() : amount(0)
{
	memset(items, NULL, MAX_AMOUNT);
}
//=============================================================================
//	eDevices::~eDevices
//-----------------------------------------------------------------------------
eDevices::~eDevices()
{
	for(int i = 0; i < amount; ++i)
	{
		delete items[i].dev;
	}
}
//=============================================================================
//	eDevices::Reset
//-----------------------------------------------------------------------------
void eDevices::Reset()
{
	for(int i = 0; i < amount; ++i)
	{
		items[i].dev->Reset();
	}
}
//=============================================================================
//	eDevices::Register
//-----------------------------------------------------------------------------
void eDevices::Add(eDevice* d, int id)
{
	assert(amount <= MAX_AMOUNT);
	items[amount].id	= id;
	items[amount].dev	= d;
	items[amount].dev->Init();
	++amount;
}
//=============================================================================
//	eDevices::Item
//-----------------------------------------------------------------------------
eDevice* eDevices::Item(int id)
{
	for(int i = 0; i < amount; ++i)
	{
		if(items[i].id == id)
			return items[i].dev;
	}
	return NULL;
}
//=============================================================================
//	eDevices::IoRead
//-----------------------------------------------------------------------------
byte eDevices::IoRead(word port) const
{
	byte v = 0xff;
	for(int i = 0; i < amount; ++i)
	{
		items[i].dev->IoRead(port, &v);
	}
	return v;
}
//=============================================================================
//	eDevices::IoWrite
//-----------------------------------------------------------------------------
void eDevices::IoWrite(word port, byte v)
{
	for(int i = 0; i < amount; ++i)
	{
		items[i].dev->IoWrite(port, v);
	}
}
