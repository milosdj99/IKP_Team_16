#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "RingBuffer.h"
#include "Hashmap.h"




Operation BufferGet(RingBuffer* buffer) {
	
	int index = buffer->head;
	buffer->head = (buffer->head + 1) % RING_SIZE;
	return buffer->data[index];
}
void BufferPut(RingBuffer* buffer, Operation new_operation) {

	buffer->data[buffer->tail] = new_operation;
	buffer->tail = (buffer->tail + 1) % RING_SIZE;
}
