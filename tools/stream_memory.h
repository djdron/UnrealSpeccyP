#ifndef __STREAM_MEMORY_H__
#define __STREAM_MEMORY_H__

#pragma once

namespace xIo
{

class eStreamMemory
{
public:
	eStreamMemory(const void* _data, size_t _data_size) : data((const byte*)_data), data_size(_data_size), ptr(NULL) {}
	void Open() { ptr = data; }
	int Close() { ptr = NULL; return 0; }
	size_t Size() const { return data_size; }
	size_t Read(void* dst, size_t size)
	{
		if(Pos() + size > data_size)
		{
			size = data_size - Pos();
		}
		memcpy(dst, ptr, size);
		ptr += size;
		return size;
	}
	size_t Pos() const { return ptr - data; }
	enum eSeekMode { S_CUR, S_END, S_SET };
	typedef long int off_t;
	int Seek(off_t offset, eSeekMode mode = S_SET)
	{
		switch(mode)
		{
		case S_CUR:
			{
				const byte* new_ptr = ptr + offset;
				if(new_ptr < data || new_ptr > data + data_size)
					return -1;
				ptr = new_ptr;
				return 0;
			}
		case S_END:
			if(-offset > (off_t)data_size || offset > 0)
				return -1;
			ptr = data + data_size - offset;
			return 0;
		case S_SET:
			if(offset < 0 || offset > (off_t)data_size)
				return -1;
			ptr = data + offset;
			return 0;
		}
		return -1;
	}
protected:
	const byte* data;
	size_t data_size;
	const byte* ptr;
};

}
//namespace xIo

#endif//__STREAM_MEMORY_H__
