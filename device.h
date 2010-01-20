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
	virtual void IoRead(word port, byte* v) {}
	virtual void IoWrite(word port, byte v)	{}
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
	byte IoRead(word port) const;
	void IoWrite(word port, byte v);

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

#endif//__DEVICE_H__
