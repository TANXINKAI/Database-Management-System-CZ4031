#pragma once
#include <iostream>
#include <cstring>

struct MovieInfo
{
public:

	MovieInfo() {}
	MovieInfo(unsigned char _tconst[], double _rating, int _votes) {
		for (int i = 0; i < 10; i++)
			tconst[i] = _tconst[i];
		rating = (unsigned int)(_rating * 10);
		votes = _votes;
	}

	int getSerializedLength() {
		return 15;
	}

	unsigned char* getTConst() {
		return tconst;
	}

	int getVotes() {
		return votes;
	}

	double getRating() {
		return double(rating) / 10.0;
	}

	unsigned char* serialize() {
		unsigned char *output = (unsigned char*)malloc(getSerializedLength());
		memcpy(output, &tconst, 10);
		memcpy(output +10, &rating, 1);
		memcpy(output + 11, &votes, 4);
		return output;
	}

	void deserialize(MovieInfo* mi, unsigned char* data) {
		memcpy(&mi->tconst, data, 10);
		memcpy(&mi->rating, data + 10, 1);
		memcpy(&mi->votes, data + 11, 4);
	}

private:
	unsigned char tconst[10];
	unsigned char rating;
	int votes;
};

