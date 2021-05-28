#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define SIZE (16)

void *buffer[SIZE];
int in = 0, out = 0;

void enqueue0(void *value) {
	buffer[in] = value;
	in++;
	if (in == SIZE) in = 0;
	return;
}

void *dequeue0(void) {
	void *result = buffer[out];
	out++;
	if (out == SIZE) out = 0;
	return result;
}

void enqueue1(void *value) {
	buffer[(in++) % SIZE] = value; /* XXX */
	return;
}

void *dequeue1(void) {
	void *result = buffer[(out++) % SIZE]; /* XXX */
	return result;
}

void enqueue2(void *value) { /* SIZE is 2^k */
	buffer[(in++) & (SIZE - 1)] = value;
	return;
}

void *dequeue2(void) { /* SIZE is 2^k */
	void *result = buffer[(out++) & (SIZE - 1)];
/*
	void *result = ((out++) & (SIZE - 1))[buffer];
*/
	return result;
}

/*
	buffer[i] == *(buffer + i) == *(i + buffer) == i[buffer]
	buffer[1] == *(buffer + 1) == *(1 + buffer) == 1[buffer]
*/

int main(void) {
	memset(buffer, 0, sizeof (buffer) / sizeof (buffer[0]));
	
	return 0;
}

