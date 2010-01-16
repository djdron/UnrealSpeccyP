#ifndef	__DEVICE_H__
#define	__DEVICE_H__

#pragma once

//*****************************************************************************
//	eDevice
//-----------------------------------------------------------------------------
class eDevice
{
public:
	virtual void Init() {}
	virtual void Reset() {}
	virtual void Update() {}
	virtual bool IoRead(dword port, byte* v) { *v = 0xff; return true; }
	virtual void IoWrite(dword port, byte v)	{}
};

//*****************************************************************************
//	eDevices
//-----------------------------------------------------------------------------
class eDevices
{
public:
	eDevices();
	~eDevices();
	void Reset();
	void Add(eDevice* d, int id);
	eDevice* Item(int id);
	byte IoRead(dword port) const;
	void IoWrite(dword port, byte v);

protected:
	enum { MAX_AMOUNT = 10 };

	struct eItem
	{
		int id;
		eDevice* dev;
	};
	eItem items[MAX_AMOUNT];
	int amount;
};

extern eDevices devices;

#endif//__DEVICE_H__
