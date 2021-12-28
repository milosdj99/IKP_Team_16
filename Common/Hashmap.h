#pragma once


#define SIZE 50

typedef struct Data_m {
	int key;
	int value;
} Data;

int GetHashValue(int key);
bool HashInsert(int key, int value, Data** HashArray);
void HashUpdate(int key, int value, Data** HashArray);

Data* HashSearch(int key, Data** HashArray);

bool HashDelete(int key, Data** HashArray);