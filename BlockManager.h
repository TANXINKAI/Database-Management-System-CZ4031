#pragma once
#include "MovieInfo.h"
#include "math.h"
#include <iostream>
#include <string>
#include <cstring>

class BlockManager {
public:
	BlockManager() {};
	BlockManager(bool verbose, size_t blockSize) { this->verbose = verbose; this->blockSize = blockSize; }
	void parse(BlockManager* parser, unsigned char* data, size_t blockSize) {
		parser->fullFlag = data[0];

		unsigned char* avail = (unsigned char*)malloc(sizeof(int));
		memcpy(avail, data + 1, 4);
		parser->freeRecordOffset = *(int*)avail;
		parser->blockSize = blockSize;
		free(avail);

		this->blockData = data;
	}
	unsigned char* createBlock() {

		unsigned char* block = (unsigned char*)malloc(this->blockSize);
		memset(block, 0, this->blockSize);

		int recordOffset = 0;
		memcpy(block + 1, &recordOffset, sizeof(int));

		blockCount++;

		if (blocks == nullptr)
			this->blocks = (intptr_t*)malloc(sizeof(intptr_t *));
		else {
			intptr_t* ptr = (intptr_t*)realloc(this->blocks, sizeof(intptr_t*) * blockCount);
			if (!ptr)
				throw std::runtime_error("Unable to reallocate space for block index array");
			this->blocks = ptr;
		}
		this->blocks[this->blockCount - 1] = (intptr_t)block;

		if (true) {
			std::cout << "Created new block at " << std::hex << (intptr_t)block << ".\nTotal block count: " << std::to_string(blockCount) << "\nBlock index address: " << std::hex << this->blocks << "\n";
		}

		return block;
	}

	MovieInfo getMovieInfoAt(int offset) {
		MovieInfo mi;
		if (offset >= this->freeRecordOffset)
			throw std::runtime_error("Record does not exist");
		unsigned char* data = &this->blockData[offset * mi.getSerializedLength() + HEADER_LENGTH];
		mi.deserialize(&mi, data);
		return mi;
	}

	unsigned char* insertMovieInfoAt(MovieInfo mi) {
		int sizePerRecord = mi.getSerializedLength();
		if (!this->isFull()) {
			if (this->freeRecordOffset < floor((this->blockSize - HEADER_LENGTH) / sizePerRecord)) {
				unsigned char* data = mi.serialize();
				for (int i = 0; i < sizePerRecord; i++) {
					this->blockData[HEADER_LENGTH + (sizePerRecord * this->freeRecordOffset) + i] = data[i];
				}
				free(data);
				this->freeRecordOffset++;

				memcpy(this->blockData + 1, &freeRecordOffset, sizeof(int));

				if (this->freeRecordOffset == floor((this->blockSize - HEADER_LENGTH) / sizePerRecord))
					this->blockData[0] = 1;
				return &this->blockData[HEADER_LENGTH + (sizePerRecord * (this->freeRecordOffset))];
			}
		}
		return nullptr;
	}

	intptr_t getAddressOfRecordAt(int offset) {
		MovieInfo mi;
		int sizePerRecord = mi.getSerializedLength();
		intptr_t ptr = (intptr_t)&this->blockData[(offset * sizePerRecord) + HEADER_LENGTH];
		return ptr;
	}

	unsigned char* getRecordSpaceAt(int offset) {
		MovieInfo mi;
		int sizePerRecord = mi.getSerializedLength();
		unsigned char* ptr = nullptr;
		bool empty = true;
		for (int i = 0; i < mi.getSerializedLength(); i++) {
			if (this->blockData[(this->freeRecordOffset * sizePerRecord) + i + HEADER_LENGTH] != 0)
			{
				empty = false;
				break;
			}
		}

		if (empty)
			ptr = (unsigned char*)&this->blockData[(this->freeRecordOffset * sizePerRecord) + HEADER_LENGTH];
		return ptr;
	}

	int getMovieInfoPerBlock() {
		MovieInfo mi;
		return floor((this->blockSize - HEADER_LENGTH) / mi.getSerializedLength());
	}

	int getRecordsCount() {
		return this->freeRecordOffset;
	}

	bool isFull() {
		return this->fullFlag;
	}


	void freeAllBlocks() {
		for (int i = 0; i < this->blockCount; i++) {
			free((void *)this->blocks[i]);
		}
		free(this->blocks);
	}

	int getBlockCount() {
		return this->blockCount;
	}

	intptr_t getBlock(int block) {
		return this->blocks[block];
	}

private:
	int HEADER_LENGTH = 5;
	bool fullFlag = 0;
	int freeRecordOffset = 0;
	unsigned char* blockData;
	bool verbose = false;

	size_t blockSize;
	intptr_t* blocks = nullptr;
	int blockCount = 0;
};