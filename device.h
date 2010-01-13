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
	virtual bool IoRead(word port, byte* v) { *v = 0xff; return true; }
	virtual void IoWrite(word port, byte v)	{}
	virtual bool Read(word addr, byte* v) { *v = 0xff; return true; }
	virtual void Write(word addr, byte v) {}
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
	byte IoRead(word port) const;
	void IoWrite(word port, byte v);
	byte Read(word addr) const;
	void Write(word addr, byte v);

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
