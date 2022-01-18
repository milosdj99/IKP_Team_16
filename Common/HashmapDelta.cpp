#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <malloc.h>
//#include "pch.h"
#include "HashmapDelta.h"
#include <iostream>




Data_for_send** NewHashArray() {

	Data_for_send** array = (Data_for_send**)malloc(100 * sizeof(Data_for_send));

	for (int i = 0; i < 100; i++) {
		array[i] = (Data_for_send*)malloc(sizeof(Data_for_send));
	}

	return array;
}

int GetHashValue(int key) {


	int hash = key % 100;

	return hash;

}

void HashInsertOrUpdate(int key, Data_for_send value, Data_for_send** HashArray, int* array_of_sizes) {



	int hash = GetHashValue(key);

	int trenutni_br_podataka = array_of_sizes[hash];


	HashArray[hash][trenutni_br_podataka] = value;


}



Data_for_send* HashSearch(int key, Data_for_send** HashArray) {

	int hash = GetHashValue(key);

	return HashArray[hash];
}



bool HashDelete(int key, Data_for_send** HashArray) {

	if (HashSearch(key, HashArray) == NULL) {

		return false;
	}
	else {

		int hash = GetHashValue(key);

		HashArray[hash] = NULL;

		return true;
	}
}


