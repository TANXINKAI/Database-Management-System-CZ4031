#include "Storage.h"
#include "math.h"
#include <iostream>
#include <string>

Storage::Storage(size_t blockSize, size_t storageSize)
{
	this->ctor(blockSize, storageSize, false);
}

Storage::Storage(size_t blockSize, size_t storageSize, bool ignoreOrphanedBytes)
{
	this->ctor(blockSize, storageSize, ignoreOrphanedBytes);
}

void Storage::ctor(size_t blockSize, size_t storageSize, bool ignoreOrphanedBytes) {
	if (!ignoreOrphanedBytes && storageSize % blockSize != 0)
		throw std::invalid_argument("Storage size is not a multiple of blocksize (orphaned bytes).\nUse overload Storage(size_t,size_t,bool) to ignore");
	this->blockSize = blockSize;
	this->storageSize = storageSize;
	this->blockCount = 0;
	this->blocks = nullptr;
}

Storage::~Storage()
{
	freeAllBlocks();
}

size_t Storage::getUsedStorageSize() {
	size_t total = 0;

	// Total size of blocks allocated
	total += (this->blockCount * this->blockSize);

	// Total size of block index
	total += (sizeof(intptr_t) * blockCount);


	//TODO: Total size of B+ tree?

	return total;
}

double Storage::getStorageSize(ENUM_STORAGE_SCALE type) {
	double retVal = this->storageSize / (2 << type);
	return retVal;
}

intptr_t* Storage::createBlock() {
	if (this->blockCount + 1 > (this->storageSize / this->blockSize))
		throw std::exception("Unable to create new blocks as block capacity has been reached");


	//TODO: Add some form of consideration/calculation that includes the size of B+ tree index. To check for space for index AND block
	if (this->storageSize - this->getUsedStorageSize() < 2*blockSize)
		throw std::exception("Unable to create new data as disk capacity has surpassed threshold");

	this->blockCount++;
	intptr_t* block = (intptr_t*)malloc(this->blockSize);
	memset(block, 0, this->blockSize);

	if (blocks == nullptr)
		this->blocks = (intptr_t*)malloc(sizeof(intptr_t *));
	else {
		intptr_t* ptr = (intptr_t*)realloc(this->blocks, sizeof(intptr_t *) * blockCount);
		if (!ptr)
			throw std::exception("Unable to reallocate space for block index array");
		this->blocks = ptr;
	}
	this->blocks[this->blockCount - 1] = (intptr_t)block;

	if (this->verbose) {
		std::cout << "Created new block at " << std::hex << (intptr_t)block << ".\nTotal block count: " << std::to_string(blockCount) << "\nBlock index address: " << std::hex << this->blocks << "\n";
	}

	return block;
}

intptr_t* Storage::getNextAvailableSpaceInBlock(unsigned char* block, unsigned int sizePerRecord) {
	for (int recordOffset = 0; recordOffset < floor(this->blockSize / sizePerRecord); recordOffset++) {
		bool empty = true;
		int offset = 0;
		for (int i = 0; i < sizePerRecord; i++) {
			if (block[(recordOffset * sizePerRecord) + i] != 0)
			{
				empty = false;
				break;
			}
		}

		if (empty)
			return (intptr_t*)&block[(recordOffset * sizePerRecord)];
	}

	//No space in this block
	return nullptr;
}

MovieInfo Storage::getMovieInfoAt(int block, int offset) {
	if (block >= this->blockCount || block < 0) 
		throw std::exception("Block does not exist");

	unsigned char* targetBlock = (unsigned char*)this->blocks[block];
	MovieInfo mi;
	unsigned char* data = &targetBlock[offset * mi.getSerializedLength()];
	mi.deserialize(&mi, data);
	return mi;
}

intptr_t* Storage::insertMovieInfo(MovieInfo mi) {
	unsigned char* availableSpace = nullptr;

	//Loop through all existing blocks and find available space
	if (this->blockCount != 0)
		for (int i = 0; i < this->blockCount; i++) {
			availableSpace = (unsigned char *)this->getNextAvailableSpaceInBlock((unsigned char*)this->blocks[i], mi.getSerializedLength());
			if (availableSpace != nullptr)
				break;
		}

	//No available space available (No existing blocks with allowance for new record)
	if (availableSpace == nullptr)
		availableSpace = (unsigned char*)this->createBlock();

	//Write data onto available space
	unsigned char* data = mi.serialize();
	for (int i = 0; i < mi.getSerializedLength(); i++) {
		availableSpace[i] = data[i];
	}
	free(data);

	if (this->verbose) {
		std::cout << "Wrote " << std::to_string(mi.getSerializedLength()) << " bytes to address at " << std::hex << (intptr_t)availableSpace << "\n";
	}

	return (intptr_t *)availableSpace;
}


void Storage::freeAllBlocks() {
	for (int i = 0; i < this->blockCount; i++) {
		free((void *)this->blocks[i]);
	}
	free(this->blocks);
}