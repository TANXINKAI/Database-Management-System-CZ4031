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
BplusTree tree = NULL;
void buildIndex();
void experiment1();
void experiment2();
void experiment3();
void experiment4();
void experiment5();
void testTree();
void testTree2();
void testTree3();
void sampleRetrieve();
void parseData(int limit);

int main()
{
	if(true){ //Set to true to run testTree() only
		storage->verbose = true;
		testTree3();
		// testTree2();
		sampleRetrieve();
		return 0;
	}



	// parseData(0);
	// buildIndex();
	// experiment1();
	// experiment2();
	// experiment3();
	// experiment4();
	// experiment5();
}

void buildIndex() {
	std::cout << "Building index (B+ Tree)" << endl;
	auto timeStart = high_resolution_clock::now();
	tree = BplusTree(storage->blockManager.getMovieInfoPerBlock());
	tree.storage = storage;
	int dataCount = 0;
	for (int i = 0; i < storage->blockManager.getBlockCount(); i++) {
		for (int j = 0; j < tree.getTreeOrder() + 1; j++) {
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
		<< "\nMem in Tree (testing): " << to_string(tree.count_memory(root)) << " MB"
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
		firstChildContent = firstChildContent.append(to_string(firstChildKeys[i]) + "(" + to_string(firstChild->getBlocks()[i]) + "," + to_string(firstChild->getOffsets()[i]) + ") ");
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
	tree.remove(1000);
	// tree.remove(33858);
}

void testTree() {
	BplusTree node(3);
	node.insert(5, 0, 0);
	node.insert(15, 0, 1);
	node.insert(25, 0, 2);
	node.insert(35, 0, 3);
	node.insert(45, 0, 4);
	node.insert(55, 0, 0);
	node.insert(65, 0, 1);
	node.insert(75, 0, 2);
	node.insert(85, 0, 3);
	node.insert(95, 0, 4);
	node.insert(105, 0, 0);
	node.insert(115, 0, 1);
	node.insert(125, 0, 2);
	node.insert(135, 0, 3);
	node.insert(145, 0, 4);
	node.insert(155, 0, 0);
	node.insert(165, 0, 1);
	node.insert(175, 0, 2);
	node.insert(185, 0, 3);
	node.insert(195, 0, 4);
	node.insert(205, 0, 3);

	node.insert(28, 0, 4);
	node.insert(20, 0, 3);
	node.insert(18, 0, 3);
	node.display(node.getRoot());
	std::cout << endl << endl;
	// node.insert(40, 0, 3);
	
	node.leafNodedisplay(node.getRoot());
	// node.leafNodedisplaylimited(node.getRoot(), 3);
	// std::cout << endl << endl;

	
	// node.display(node.getRoot());
	// std::cout << endl << endl;
	// int counts;
	// counts = node.count_nodes(node.getRoot());
	// std::cout << "Number of Nodes: " << counts << "\n";
	

	// node.search(15);
	// node.remove(15);
	
}

void testTree2() {
	BplusTree node(3);
	node.insert(1, 0, 0);
	node.insert(4, 0, 1);
	node.insert(7, 0, 2);
	node.insert(10, 0, 4);
	node.insert(17, 0, 3);
	node.insert(19, 0, 4);

	node.insert(25, 0, 2);
	node.insert(28, 0, 3);
	node.insert(31, 0, 4);


	std::cout << "display before insertion" << endl;
	node.display(node.getRoot());
	std::cout << endl << endl;

	
	node.insert(20, 0, 0);
	

	std::cout << "display after insertion 20" << endl;
	node.display(node.getRoot());
	std::cout << node.getRoot();
	std::cout << endl << endl;
	node.leafNodedisplay(node.getRoot());


	node.insert(21, 0, 1);
	std::cout << "display after insertion 21" << endl;
	node.display(node.getRoot());
	std::cout << node.getRoot();
	std::cout << endl << endl;
	// node.leafNodedisplay(node.getRoot());

}

void testTree3() {
	BplusTree node(3);
	
	int t_case = 4; 
	
	switch(t_case){
		case 1: 
			node.insert(1, 0, 0);
			node.insert(4, 0, 1);
			node.insert(5, 0, 2);
			node.insert(7, 0, 4);
			node.insert(17, 0, 3);
			node.insert(19, 0, 4);
			node.insert(20, 0, 2);
			node.insert(21, 0, 3);
			node.insert(25, 0, 0);
			node.insert(31, 0, 4);
			
			std::cout << "display before del" << endl;
			node.display(node.getRoot());
			std::cout << endl << endl;

			std::cout << "display after deletion 5" << endl;
			node.remove(5);
			node.display(node.getRoot());
			std::cout << endl << endl;
			std::cout << "display after deletion 17" << endl;
			node.remove(17);
			node.display(node.getRoot());					
			break;
		case 2:
			node.insert(1, 0, 0);
			node.insert(4, 0, 1);
			node.insert(7, 0, 4);
			node.insert(10, 0, 4);
			node.insert(17, 0, 3);
			node.insert(19, 0, 4);
			node.insert(20, 0, 2);
			node.insert(21, 0, 3);
			node.insert(25, 0, 0);
			node.insert(31, 0, 4);
			
			std::cout << "display before del" << endl;
			node.display(node.getRoot());
			std::cout << endl << endl;

			std::cout << "display after deletion 4" << endl;
			node.remove(4);
			node.display(node.getRoot());
			std::cout << endl << endl;
			node.leafNodedisplay(node.getRoot());

			break;
		case 3:
			node.insert(1, 0, 0);
			node.insert(4, 0, 1);
			node.insert(7, 0, 4);
			node.insert(10, 0, 4);
			node.insert(20, 0, 2);
			node.insert(21, 0, 3);
			node.insert(25, 0, 0);
			node.insert(31, 0, 4);
			
			std::cout << "display before del" << endl;
			node.display(node.getRoot());
			std::cout << endl << endl;

			std::cout << "display after deletion 4" << endl;
			node.remove(4);
			node.display(node.getRoot());
			std::cout << endl << endl;
			// node.leafNodedisplay(node.getRoot());

			std::cout << "display after deletion 25" << endl;
			node.remove(25);
			node.display(node.getRoot());
			std::cout << endl << endl;

			std::cout << "display after deletion 20" << endl;
			node.remove(20);
			node.display(node.getRoot());
			std::cout << endl << endl;

			std::cout << "display after deletion 31" << endl;
			node.remove(31);
			node.display(node.getRoot());
			std::cout << endl << endl;
			node.leafNodedisplay(node.getRoot());
			break;

			case 4:
			node.insert(1, 0, 0);
			node.insert(4, 0, 1);
			node.insert(7, 0, 2);
			node.insert(10, 0, 4);
			node.insert(17, 0, 3);
			node.insert(19, 0, 4);
			node.insert(20, 0, 0);
			node.insert(21, 0, 1);
			node.insert(25, 0, 2);
			node.insert(28, 0, 3);
			node.insert(31, 0, 4);


			std::cout << "display before del" << endl;
			node.display(node.getRoot());
			std::cout << endl << endl;
			
			std::cout << "display after deletion 5 (fail) & 7" << endl;
			node.remove(5);
			node.remove(7);
			node.display(node.getRoot());
			std::cout << endl << endl;
	
			std::cout << "display after deletion 17 & 21" << endl;
			node.remove(17);
			node.remove(21);
			node.display(node.getRoot());
			std::cout << endl << endl;
			
			
			// Bug in deletion of 1, merge and deletion of root node.
			std::cout << "display after deletion 1 " << endl;
			node.remove(1);
			node.display(node.getRoot());
			std::cout << endl << endl;
			// node.leafNodedisplay(node.getRoot());
			
			
			std::cout << "display after deletion 25" << endl;
			node.remove(25);
			node.display(node.getRoot());
			std::cout << endl << endl;



			std::cout << "display after deletion 28 & 31" << endl;
			node.remove(28);
			node.remove(31);
			node.display(node.getRoot());
			std::cout << "display after deletion 4 & 19" << endl;
			node.remove(4);
			node.remove(19);
			node.display(node.getRoot());
			std::cout << endl << endl;
			std::cout << "display after deletion 10" << endl;
			node.remove(10);
			node.display(node.getRoot());
			std::cout << endl << endl;
			
			break;
	}
	
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
	file.open("C:\\Users\\You\\Downloads\\data.tsv", ios::in);
	
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