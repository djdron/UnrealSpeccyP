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
	void Init();
	void Reset();
	void Add(eDevice* d, int id);
	eDevice* Item(int id);
	byte IoRead(dword port) const;
	void IoWrite(dword port, byte v);

	enum { AMOUNT = 10 };
public:
	struct eItem
	{
		int id;
		eDevice* dev;
	};
	eItem items[AMOUNT];
	int last;
};

extern eDevices devices;

#endif//__DEVICE_H__
