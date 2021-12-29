#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <malloc.h>
#include "pch.h"
#include "Hashmap.h"



int* NewHashArray() {

	int* array = (int*)malloc(100* sizeof(int));

	return array;
}

int GetHashValue(int key) {

	return key % 100;

}

bool HashInsert(int key, int value, int* HashArray) {

	int hash = GetHashValue(key);

	if (HashArray[hash] != 0) {
		return false;

	}
	else {
		HashArray[hash] = value;
		return true;
	}

}

void HashUpdate(int key, int value, int* HashArray) {

	int hash = GetHashValue(key);
	
	HashArray[hash] = value;
		
}

int HashSearch(int key, int* HashArray) {

	int hash = GetHashValue(key);

	return HashArray[hash];
}


bool HashDelete(int key, int* HashArray) {

	if (HashSearch(key, HashArray) == 0) {

		return false;
	}
	else {

		int hash = GetHashValue(key);

		HashArray[hash] = 0;

		return true;
	}
}


