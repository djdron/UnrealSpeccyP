#ifndef	__SNAPSHOT_H__
#define	__SNAPSHOT_H__

#pragma once

class eSnapshot
{
public:
	eSnapshot(const char* path) { Load(path); }
protected:
	void Load(const char* path);
};

#endif//__SNAPSHOT_H__
