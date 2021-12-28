#pragma once


#define RING_SIZE 50

enum Operation_type { ADD, UPDATE, DELETE };

typedef struct Operation_m {

	Operation_type type;
	Data data;

} Operation;

typedef struct RingBuffer_m {

	unsigned int tail;
	unsigned int head;
	Operation data[RING_SIZE];

} RingBuffer;


Operation BufferGet(RingBuffer* buffer);


void BufferPut(RingBuffer* buffer, Operation new_operation);