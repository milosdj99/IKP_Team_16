#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "Hashmap.h"



Data* NewHashArray() {

	Data* array = (Data*)calloc(50, sizeof(Data));

	return array;
}

int GetHashValue(int key) {

	return key % SIZE;

}

bool HashInsert(int key, int value, Data** HashArray) {

	
	Data* new_data = (Data*)malloc(sizeof(Data));
	new_data->key = key;
	new_data->value = value;

	int hash = GetHashValue(key);

	if (HashArray[hash] != NULL) {
		return false;

	}
	else {
		HashArray[hash] = new_data;
		return true;
	}

}

void HashUpdate(int key, int value, Data** HashArray) {


	Data* new_data = (Data*)malloc(sizeof(Data));
	new_data->key = key;
	new_data->value = value;

	int hash = GetHashValue(key);

	
	HashArray[hash] = new_data;
		

}

Data* HashSearch(int key, Data** HashArray) {

	int hash = GetHashValue(key);

	return HashArray[hash];
}


bool HashDelete(int key, Data** HashArray) {

	if (HashSearch(key, HashArray) == NULL) {

		return false;
	}
	else {

		int hash = GetHashValue(key);

		HashArray[hash] = NULL;
	}
}


