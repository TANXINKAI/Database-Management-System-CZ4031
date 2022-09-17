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
void parseData(int limit);

int main()
{
	parseData(0);

	MovieInfo mi = storage->getMovieInfoAt(20, 8);
	cout << "The located at Block 20, Record 8 is (tconst: " << mi.getTConst() << ", rating: " << to_string(mi.getRating()) << ", numVotes: " << to_string(mi.getVotes()) << ")" << endl;
}

void parseData(int limit) {
	cout << "Begin data parse/insert" << endl;
	auto timeStart = high_resolution_clock::now();
	int dataCount = 0;
	fstream file;
	file.open("D:\\Downloads\\data.tsv", ios::in);
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
			dataCount++;

			if (dataCount % 50000 == 0)
				cout << to_string(dataCount) << " records inserted." << endl;
		}
		file.close();
		auto timeEnd = high_resolution_clock::now();
		cout << "Data insertion of " << to_string(dataCount) << " records completed in " << to_string(duration_cast<milliseconds>(timeEnd - timeStart).count()) << " milliseconds" << endl;

	}
	else {
		throw exception("Unable to open file for reading");
	}
}