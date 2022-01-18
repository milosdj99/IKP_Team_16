#pragma once


#include "RingBuffer.h"


Data_for_send** NewHashArray();


int GetHashValue(int key);

void HashInsertOrUpdate(int key, Data_for_send value, Data_for_send** HashArray, int* array_of_sizes);

Data_for_send* HashSearch(int key, Data_for_send** HashArray);

bool HashDelete(int key, Data_for_send** HashArray);




