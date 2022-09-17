// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include "Storage.h"


using namespace std;
using namespace std::chrono;


//Initialize storage instance on the heap
//Bitshift 2 with scale of data storage
Storage* storage = new Storage(200, 100 * (2 << ENUM_STORAGE_SCALE_MEGABYTE));
void parseData();

int main()
{

	parseData();
	cout << "Done";
}

void parseData() {
	fstream file;
	file.open("D:\\Downloads\\data.tsv", ios::in);
	if (file.is_open()) {   //checking whether the file is open
		string line;
		bool firstLine = true;
		while (getline(file, line)) {  //read data from file object and put it into string.
			if (firstLine)
				firstLine = false;
			else {
				unsigned char tconst[10] = {};
				char rating[3] = {};
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
							rating[i-offset] = line[i];
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
			}
		}
		file.close();
	}
	else {
		throw exception("Unable to open file for reading");
	}
}