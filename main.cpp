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
Storage* storage = new Storage(200, 100 * (1 << ENUM_STORAGE_SCALE_MEGABYTE),true);
BplusTree tree = NULL;
bool verbose = false;
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
	try{
	if(false){ //Set to true to run testTree() only
		storage->verbose = true;
		testTree();
		sampleRetrieve();
		std::cout << "Press ENTER Key To Exit" << endl;
		cin.get();
		return 0;
	}
	parseData(0);
	buildIndex();
	experiment1();
	experiment2();
	experiment3();
	experiment4();
	experiment5();

	free(storage);

	std::cout << endl << endl << "================== 500MB STORAGE ==================" << endl;
	storage = new Storage(500, 100 * (1 << ENUM_STORAGE_SCALE_MEGABYTE),true);
	tree = NULL;
	parseData(0);
	buildIndex();
	experiment1();
	experiment2();
	experiment3();
	experiment4();
	experiment5();
	}
	catch(exception &message){
					std::cout << message.what() << endl;
	}

	std::cout << endl << endl << "Press ENTER Key To Exit" << endl;
	cin.get();
}

void buildIndex() {
	std::cout << "Building index (B+ Tree)" << endl;
	Node tmp;
	int order = tmp.getOrderWorstCase(storage->getBlockSize());
	std::cout << "B+ Tree Order (N): " << to_string(order) << endl;
	auto timeStart = high_resolution_clock::now();
	tree = BplusTree(order);
	tree.storage = storage;
	int dataCount = 0;
	for (int i = 0; i < storage->blockManager.getBlockCount(); i++) {
		for (int j = 0; j < storage->blockManager.getMovieInfoPerBlock(); j++) {
			MovieInfo mi;
			try {
				mi = storage->getMovieInfoAt(i, j);
				tree.insert(mi.getVotes(), i, j);
				dataCount++;
			}
			catch (exception& message) {
				//Can safely ignore, only exception thrown is when record is not found lol when trying to access empty block + offset
				if (storage->verbose)
					std::cout << message.what() << endl;
			}
			if (dataCount % 50000 == 0 && verbose)
				std::cout << to_string(dataCount) << " records indexed." << endl;

		}
	}
	auto timeEnd = high_resolution_clock::now();
	std::cout << "B+ tree build completed in " << to_string(duration_cast<milliseconds>(timeEnd - timeStart).count()) << " milliseconds. " 
	<< to_string(dataCount) << " records indexed." << endl << endl;
}

void experiment1() {
	double sizeMB = ((double)storage->getUsedStorageSize() / (double)(1 << ENUM_STORAGE_SCALE_MEGABYTE));
	Node* root = tree.getRoot();
	double sizeMBTree = ((double)tree.count_memory(root) / (double)(1 << ENUM_STORAGE_SCALE_MEGABYTE));
	std::cout << endl << "(Experiment 1)" << endl << "Blocks Created: " << to_string(storage->blockManager.getBlockCount())
		<< "\nStorage size: " << to_string(sizeMB) << " MB" << "\nMemory Used By B+ Tree: " << to_string(sizeMBTree) << " MB"
		<< "\nTotal database size: " << to_string(sizeMBTree+sizeMB) << "MB\n\n";
}

void experiment2() {
	Node* root = tree.getRoot();
	double sizeMB = ((double)tree.count_memory(root) / (double)(1 << ENUM_STORAGE_SCALE_MEGABYTE));
	std::cout << endl << "(Experiment 2)" << endl << "Tree Parameter n: " << to_string(tree.getTreeOrder())
		<< "\nNumber of Nodes in Tree: " << to_string(tree.count_nodes(root))
		<< "\nTree Height: " << to_string(tree.getHeight()) << "\n\n";
		
	string rootNodeContent = "Root Node Keys: ";
	int* rootKeys = root->getKeys();
	for (int i = 0; i < root->getSize(); i++) {
		rootNodeContent = rootNodeContent.append(to_string(rootKeys[i]) + " ");
	}
	Node* firstChild = root->getPointers()[0];
	string firstChildContent = "First Child Keys: ";
	int* firstChildKeys = firstChild->getKeys();
	for (int i = 0; i < firstChild->getSize(); i++) {
		firstChildContent = firstChildContent.append(to_string(firstChildKeys[i]) + (verbose ? "(" + to_string(firstChild->getBlocks()[i]) + "," + to_string(firstChild->getOffsets()[i]) + ") " : " "));
	}
	std::cout << rootNodeContent << endl << firstChildContent << "\n\n";

	//tree.display(root);
}

void experiment3() {
	std::cout << endl << "(Experiment 3)" << endl;
	tree.search(500);
}

void experiment4() {
	std::cout << endl << "(Experiment 4)" << endl;
	tree.rangequery(30000, 40000);
}

void experiment5() {
	std::cout << endl << "(Experiment 5)" << endl;
	// tree.remove(1000);
	Node* root = tree.getRoot();

	int keys_deleted = 0;
	int tree_nodes = tree.count_nodes(root);
    int original_node_count = tree.count_nodes(root);
	std::cout << "\nNumber of Nodes in Original Tree: " << to_string(tree.count_nodes(root)) <<endl;
	
	while(tree.remove(1000) != -1);


    string rootNodeContent = "Root Node Keys: ";
    int* rootKeys = root->getKeys();
    for (int i = 0; i < root->getSize(); i++) {
        rootNodeContent = rootNodeContent.append(to_string(rootKeys[i]) + " ");
    }

    Node* firstChild = root->getPointers()[0];
    string firstChildContent = "First Child Keys: ";
    int* firstChildKeys = firstChild->getKeys();
    for (int i = 0; i < firstChild->getSize(); i++) {
        firstChildContent = firstChildContent.append(to_string(firstChildKeys[i]) + " ");
    }

    std::cout << "\nNumber of Nodes in Final Tree: " << to_string(tree.count_nodes(root));
    std::cout << "\nNumber of Nodes deleted or merged: " << to_string(original_node_count - tree.count_nodes(root));
	std::cout << "\nHeight of Final Tree: " << to_string(tree.getHeight()) <<endl;
	std::cout << rootNodeContent << endl << firstChildContent << "\n";

}

void testTree() {
	BplusTree node(3);
	node.storage = storage;
	
	unsigned char tconst[10] = {'a','b','c','d','e','f','g','h','i','j'};
	for(int c=0;c<21;c++){
		MovieInfo mi(tconst,0.0,c*10+5);
		storage->insertMovieInfo(mi);
	}
	MovieInfo mi1(tconst,0.0,28);
	storage->insertMovieInfo(mi1);

	MovieInfo mi2(tconst,0.0,20);
	storage->insertMovieInfo(mi2);

	MovieInfo mi3(tconst,0.0,18);
	storage->insertMovieInfo(mi3);

	MovieInfo mi4(tconst,0.0,40);
	storage->insertMovieInfo(mi4);
	
	MovieInfo mi5(tconst,0.0,45);
	storage->insertMovieInfo(mi5);

	MovieInfo mi6(tconst,0.0,45);
	storage->insertMovieInfo(mi6);

	MovieInfo mi7(tconst,0.0,40);
	storage->insertMovieInfo(mi7);

	MovieInfo mi8(tconst,0.0,40);
	storage->insertMovieInfo(mi8);
	
	MovieInfo mi9(tconst,0.0,40);
	storage->insertMovieInfo(mi9);

	MovieInfo mi10(tconst,0.0,40);
	storage->insertMovieInfo(mi10);

	MovieInfo mi11(tconst,0.0,165);
	//storage->insertMovieInfo(mi11);

	MovieInfo mi12(tconst,0.0,165);
	//storage->insertMovieInfo(mi12);

	for (int i = 0; i < storage->blockManager.getBlockCount(); i++) {
		for (int j = 0; j < storage->blockManager.getMovieInfoPerBlock(); j++) {
			MovieInfo mi;
			try {
				mi = storage->getMovieInfoAt(i, j);
				node.insert(mi.getVotes(), i, j);
			}
			catch (exception& message) {
				//Can safely ignore, only exception thrown is when record is not found lol when trying to access empty block + offset
				if (storage->verbose)
					std::cout << message.what() << endl;
			}
		}
	}

	std::cout << endl << endl;
	node.display(node.getRoot());
	std::cout << endl << endl;

	int counts;
	counts = node.count_nodes(node.getRoot());
	std::cout << "Number of Nodes: " << to_string(counts) << "\n";

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

	file.open("/Users/sidhaarth/Desktop/Project 1/data.tsv", ios::in);
	
	// file.open("data.tsv", ios::in);
	if (file.is_open()) {
		string line;
		bool firstLine = true;
		while (getline(file, line)) {
			if (limit != 0 && dataCount == limit)
				break;
			if (firstLine)
				firstLine = false;
			else {
				unsigned char tconst[11] = {};
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
							tconst[10] = '\0';
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
			if (dataCount % 50000 == 0 && dataCount != 0 && verbose)
				std::cout << to_string(dataCount) << " records inserted." << endl;
		}
		file.close();
		auto timeEnd = high_resolution_clock::now();
		if(dataCount < 50000 && verbose)
			std::cout << to_string(dataCount) << " records inserted." << endl;
		std::cout << "Data insertion of " << to_string(dataCount) << " records completed in " << to_string(duration_cast<milliseconds>(timeEnd - timeStart).count()) << " milliseconds" << endl << endl;
	}
	else {
		throw runtime_error("Unable to open file for reading");
	}
}