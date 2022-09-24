// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include "Storage.h"
#include "BTreeModel.h"

using namespace std;
using namespace std::chrono;


//Initialize storage instance on the heap
//Bitshift 1 with scale of data storage
Storage* storage = new Storage(200, 100 * (1 << ENUM_STORAGE_SCALE_MEGABYTE));
BPTree tree = NULL;
void buildIndex();
void experiment1();
void experiment2();
void experiment3();
void experiment4();
void experiment5();
void testTree();
void sampleRetrieve();
void parseData(int limit);

int main()
{
		storage->verbose = true;
	if(true){ //Set to true to run testTree() only
		storage->verbose = true;
		testTree();
		//sampleRetrieve();
		cin.get();
		return 0;
	}
	parseData(50);
	buildIndex();
	experiment1();
	experiment2();
	experiment3();
	experiment4();
	experiment5();
	exit(0);
}

void buildIndex() {
	std::cout << "Building index (B+ Tree)" << endl;
	auto timeStart = high_resolution_clock::now();
	tree = BPTree(storage->blockManager.getMovieInfoPerBlock());
	tree.storage = storage;
	int dataCount = 0;
	for (int i = 0; i < storage->blockManager.getBlockCount(); i++) {
		for (int j = 0; j < tree.getTreeOrder() + 1; j++) {
			try {
				tree.insert(storage->getAddressAt(i,j));
				dataCount++;
			}
			catch (exception& message) {
				//Can safely ignore, only exception thrown is when record is not found lol when trying to access empty block + offset
				if (storage->verbose)
					std::cout << message.what() << endl;
			}
			if (dataCount % 50000 == 0)
				std::cout << to_string(dataCount) << " records indexed." << endl;
		}
	}
	auto timeEnd = high_resolution_clock::now();
	std::cout << "B+ tree build completed in " << to_string(duration_cast<milliseconds>(timeEnd - timeStart).count()) << " milliseconds" << endl << endl;

}

void experiment1() {
	double sizeMB = ((double)storage->getUsedStorageSize() / (double)(1 << ENUM_STORAGE_SCALE_MEGABYTE));
	std::cout << endl << "(Experiment 1)" << endl << "Blocks Created: " << to_string(storage->blockManager.getBlockCount())
		<< "\nDatabase size: " << to_string(sizeMB) << " MB\n\n";
}

void experiment2() {
	Node* root = tree.getRoot();
	std::cout << endl << "(Experiment 2)" << endl << "Tree Parameter n: " << to_string(tree.getTreeOrder())
		<< "\nNumber of Nodes in Tree: " << to_string(tree.count_nodes(root))
		<< "\nTree Height: " << to_string(tree.getHeight()) << "\n\n";

	string rootNodeContent = "Root Node Keys: ";
	

	//tree.display(root);
}

void experiment3() {
	std::cout << endl << "(Experiment 3)" << endl;
	tree.search(33858);
}

void experiment4() {
	std::cout << endl << "(Experiment 4)" << endl;
	tree.rangequery(30000, 40000);
}

void experiment5() {
	std::cout << endl << "(Experiment 5)" << endl;
	tree.remove(1000);
}

void testTree() {
	BPTree node(3);
	for(int i=0;i<25;i++){
		unsigned char tconst[10] = {'a','b','c','d','e','f','g','h','i',to_string(i)[0]};
		MovieInfo mi(tconst, i, 5+(i*10));
		std::cout << "Inserting for " << to_string((i * 10)+ 5) << endl;
		storage->insertMovieInfo(mi);
	}
	for (int i = 0; i < storage->blockManager.getBlockCount(); i++) {
		for (int j = 0; j < storage->blockManager.getMovieInfoPerBlock(); j++) {
			try {
				std::cout << "Indexing address " << std::hex << storage->getAddressAt(i,j) << endl;
				node.insert(storage->getAddressAt(i,j));
				node.display(node.getRoot());
			}
			catch (exception& message) {
				//Can safely ignore, only exception thrown is when record is not found lol when trying to access empty block + offset
				if (storage->verbose)
					std::cout << message.what() << endl;
			}
		}
	}

	node.display(node.getRoot());
	std::cout << endl << endl;
	int counts;
	counts = node.count_nodes(node.getRoot());
	std::cout << "Number of Nodes: " << counts << "\n";

	node.search(15);
	node.remove(15);

}

void sampleRetrieve() {
	int blockNumber = 20;
	int recordNumber = 8;
	MovieInfo mi = storage->getMovieInfoAt(blockNumber, recordNumber);
	std::cout << "The data located at Block " << to_string(blockNumber) << ", Record " << to_string(recordNumber)
		<< " [line " << to_string(storage->blockManager.getMovieInfoPerBlock() * blockNumber + recordNumber + 1) << " of file] is (tconst: "
		<< mi.getTConst() << ", rating: " << to_string(mi.getRating()) << ", numVotes: " << to_string(mi.getVotes()) << ")" << endl;
}

void parseData(int limit) {
	std::cout << "Begin data parse/insert" << endl;
	auto timeStart = high_resolution_clock::now();
	int dataCount = 0;
	fstream file;
	file.open("C:\\Users\\woodmon122\\Desktop\\NTU\\Y3S1\\CZ4031\\trunk\\data.tsv", ios::in);
	//file.open("C:\\Users\\austi\\source\\repos\\cz4031\\cz4031\\data.tsv", ios::in);
	if (file.is_open()) {
		string line;
		bool firstLine = true;
		while (getline(file, line)) {
			if (limit != 0 && dataCount == limit)
				break;
			if (firstLine)
				firstLine = false;
			else {
				unsigned char tconst[10] = {};
				char rating[4] = {};
				char* votes = nullptr;
				int offset = 0;
				int counter = 0;
				for (size_t i = 0; i < line.length(); i++) {
					if (line[i] == '\t') {
						offset = i + 1;
						counter++;
					}
					else {
						switch (counter) {
						case 0:
							tconst[i] = line[i];
							break;
						case 1:
							rating[i - offset] = line[i];
							break;
						case 2:
							votes = (char*)&line[i];
							i = line.length();
							break;
						}
					}
				}
				MovieInfo mi(tconst, stod(rating), stoi(votes));
				storage->insertMovieInfo(mi);
				dataCount++;
			}
			if (dataCount % 50000 == 0 && dataCount != 0)
				std::cout << to_string(dataCount) << " records inserted." << endl;
		}
		file.close();
		auto timeEnd = high_resolution_clock::now();
		if(dataCount < 50000)
			std::cout << to_string(dataCount) << " records inserted." << endl;
		std::cout << "Data insertion of " << to_string(dataCount) << " records completed in " << to_string(duration_cast<milliseconds>(timeEnd - timeStart).count()) << " milliseconds" << endl << endl;

	}
	else {
		throw runtime_error("Unable to open file for reading");
	}
}