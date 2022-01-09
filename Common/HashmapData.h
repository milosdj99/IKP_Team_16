#pragma once


#define SIZE 50


int* NewHashArray();

int GetHashValue(int key, int size);

void HashInsertOrUpdate(int key, int value, int* HashArray);

int HashSearch(int key, int* HashArray);

bool HashDelete(int key, int* HashArray);