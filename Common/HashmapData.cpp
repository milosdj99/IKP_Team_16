#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <malloc.h>
//#include "pch.h"
#include "HashmapData.h"
#include <iostream>




int* NewHashArray() {

	int* array = (int*)malloc(100*sizeof(int));

	for (int i = 0; i < 100; i++) {
		array[i] = 0;
	}

	return array;
}

int GetHashValue(int key, int size) {


	int hash = key % 100;

	return hash;

}

void HashInsertOrUpdate(int key, int value, int* HashArray) {

	int size = sizeof(*HashArray) / sizeof(int);

	int hash = GetHashValue(key, size);

	
	HashArray[hash] = value;
		
	
}



int HashSearch(int key, int* HashArray) {
	
	int size = sizeof(*HashArray) / sizeof(int);

	int hash = GetHashValue(key, size);

	return HashArray[hash];
}


bool HashDelete(int key, int* HashArray) {

	if (HashSearch(key, HashArray) == 0) {

		return false;
	}
	else {

		int size = sizeof(*HashArray) / sizeof(int);

		int hash = GetHashValue(key, size);

		HashArray[hash] = 0;

		return true;
	}
}


