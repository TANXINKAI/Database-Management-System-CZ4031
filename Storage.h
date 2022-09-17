#pragma once
#include <cstddef>
#include <stdexcept>
#include "math.h"
#include "MovieInfo.h"

enum ENUM_STORAGE_SCALE {
	ENUM_STORAGE_SCALE_BYTE = 0,
	ENUM_STORAGE_SCALE_KILOBYTE = 10,
	ENUM_STORAGE_SCALE_MEGABYTE = 20,
	ENUM_STORAGE_SCALE_GIGABYTE = 30,
};

class Storage
{
public:

	Storage(size_t blockSize, size_t storageSize);
	Storage(size_t blockSize, size_t storageSize, bool ignoreOrphanedBytes);
	~Storage();
	void ctor(size_t blockSize, size_t storageSize, bool ignoreOrphanedBytes);
	size_t getUsedStorageSize();
	double getStorageSize(ENUM_STORAGE_SCALE type);

	intptr_t * insertMovieInfo(MovieInfo mi);
	MovieInfo getMovieInfoAt(int block, int offset);

private:
	size_t blockSize;
	size_t storageSize;
	int blockCount;
	intptr_t* blocks;
	intptr_t* memSpace;
	intptr_t* dataPtr;

	bool verbose = false;

	intptr_t * createBlock();
	intptr_t * getNextAvailableSpaceInBlock(unsigned char * block, unsigned int sizePerRecord);


	void freeAllBlocks();
};