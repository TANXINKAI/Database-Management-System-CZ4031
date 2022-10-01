#pragma once
#include <cstddef>
#include <stdexcept>
#include "math.h"
#include "MovieInfo.h"
#include "BlockManager.h"

enum ENUM_STORAGE_SCALE {
	ENUM_STORAGE_SCALE_BYTE = 0,
	ENUM_STORAGE_SCALE_KILOBYTE = 10,
	ENUM_STORAGE_SCALE_MEGABYTE = 20,
	ENUM_STORAGE_SCALE_GIGABYTE = 30,
};

class Storage
{
public:
	bool verbose = false;
	BlockManager blockManager;

	Storage(size_t blockSize, size_t storageSize);
	Storage(size_t blockSize, size_t storageSize, bool ignoreOrphanedBytes);
	~Storage();
	void ctor(size_t blockSize, size_t storageSize, bool ignoreOrphanedBytes);
	size_t getUsedStorageSize();
	double getStorageSize(ENUM_STORAGE_SCALE type);
	size_t getBlockSize(){
		return blockSize;
	}

	unsigned char * insertMovieInfo(MovieInfo mi);
	MovieInfo getMovieInfoAt(int block, int offset);

private:
	size_t blockSize;
	size_t storageSize;

	unsigned char * createBlock();
	unsigned char* getNextAvailableSpaceInBlock(unsigned char * block, unsigned int sizePerRecord);
};