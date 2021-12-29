#pragma once


#define RING_SIZE 50

typedef struct Data_for_send_m {

	int key;
	int value;

} Data_for_send;

typedef struct RingBuffer_m {

	unsigned int tail;
	unsigned int head;
	Data_for_send data[RING_SIZE];

} RingBuffer;


Data_for_send BufferGet(RingBuffer* buffer);


void BufferPut(RingBuffer* buffer, Data_for_send* new_data);