#pragma once


#define SIZE 50


int* NewHashArray();

int GetHashValue(int key);

bool HashInsert(int key, int value, int* HashArray);

void HashUpdate(int key, int value, int* HashArray);

int HashSearch(int key, int* HashArray);

bool HashDelete(int key, int* HashArray);