#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "pch.h"
#include "RingBuffer.h"





Data_for_send BufferGet(RingBuffer* buffer) {
	
	int index = buffer->head;
	buffer->head = (buffer->head + 1) % RING_SIZE;
	return buffer->data[index];
}


void BufferPut(RingBuffer* buffer, Data_for_send* new_operation) {

	buffer->data[buffer->tail] = *new_operation;
	buffer->tail = (buffer->tail + 1) % RING_SIZE;
}
