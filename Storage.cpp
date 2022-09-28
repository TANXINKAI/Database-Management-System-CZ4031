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
	this->blockManager = BlockManager(this->verbose, blockSize);
}

Storage::~Storage()
{
	this->blockManager.freeAllBlocks();
}

size_t Storage::getUsedStorageSize() {
	size_t total = 0;

	// Total size of blocks allocated
	total += (this->blockManager.getBlockCount() * this->blockSize);

	// Total size of block index
	total += (sizeof(intptr_t) * this->blockManager.getBlockCount());


	//TODO: Total size of B+ tree?

	return total;
}

double Storage::getStorageSize(ENUM_STORAGE_SCALE type) {
	double retVal = this->storageSize / (1 << type);
	return retVal;
}

unsigned char* Storage::createBlock() {
	if (this->blockManager.getBlockCount() + 1 > (this->storageSize / this->blockSize))
		throw std::runtime_error("Unable to create new blocks as block capacity has been reached");


	//TODO: Add some form of consideration/calculation that includes the size of B+ tree index. To check for space for index AND block
	if (this->storageSize - this->getUsedStorageSize() < 2*blockSize)
		throw std::runtime_error("Unable to create new data as disk capacity has surpassed threshold");

	return this->blockManager.createBlock();
}

unsigned char* Storage::getNextAvailableSpaceInBlock(unsigned char* block, unsigned int sizePerRecord) {
	this->blockManager.parse(&this->blockManager, block, this->blockSize);

	if (this->blockManager.isFull())
		return nullptr;

	int recordOffset = this->blockManager.getRecordsCount();

	return this->blockManager.getRecordSpaceAt(recordOffset);
}

MovieInfo Storage::getMovieInfoAt(int block, int offset) {
	if (block >= this->blockManager.getBlockCount() || block < 0)
		throw std::runtime_error("Block does not exist");

	unsigned char* targetBlock = (unsigned char*)this->blockManager.getBlock(block);
	this->blockManager.parse(&this->blockManager, targetBlock, this->blockSize);

	return this->blockManager.getMovieInfoAt(offset);
}

unsigned char* Storage::insertMovieInfo(MovieInfo mi) {
	unsigned char* availableSpace = nullptr;
	int blockCount = this->blockManager.getBlockCount();
	//Loop through all existing blocks and find available space
	if (blockCount != 0) {
		availableSpace = (unsigned char*)this->blockManager.getBlock(blockCount - 1);

		this->blockManager.parse(&this->blockManager, availableSpace, this->blockSize);
		if (this->blockManager.isFull())
			availableSpace = nullptr;
	}
	//No available space available (No existing blocks with allowance for new record)
	if (availableSpace == nullptr){
		availableSpace = (unsigned char*)this->createBlock();
		this->blockManager.parse(&this->blockManager, availableSpace, this->blockSize);
	}

	unsigned char* addressWritten = this->blockManager.insertMovieInfoAt(mi);

	if (this->verbose) {
		if (addressWritten)
			std::cout << "Wrote " << std::to_string(mi.getSerializedLength()) << " bytes to address at " << std::hex << (intptr_t)addressWritten << "\n";
	}

	return availableSpace;
}