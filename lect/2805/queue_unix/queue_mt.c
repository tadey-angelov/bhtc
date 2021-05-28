#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>

#define SIZE (16)

void *buffer[SIZE];
int in = 0, out = 0;
sem_t countsem, spacesem;
sem_t lock;

void enqueue(void *value) {
	sem_wait(&spacesem);
	
/* XXX BEGIN CRIT. SECT. */
	sem_wait(&lock);
	buffer[(in++) & (SIZE - 1)] = value;
	sem_post(&lock);
/* XXX CRIT. SECT. END */
	
	sem_post(&countsem);
	return;
}

void *dequeue(void) {
	void *result;
	
	sem_wait(&countsem);
	
/* XXX BEGIN CRIT. SECT. */
	sem_wait(&lock);
	result = buffer[(out++) & (SIZE - 1)];
	sem_post(&lock);	
/* XXX CRIT. SECT. END */
	
	sem_post(&spacesem);
	return result;
}

void *prod(void *_arg) {
	unsigned long val = 0ul;
	int j;
	
	while (1) {
		enqueue((void *)val);
		for (j = 0; j < 32; j++, val++)
			enqueue((void *)val);
		sleep(1u);
	}
	
	return NULL;
	
}

void *cons(void *_arg) {
	unsigned long val;
	
	while (1) {
		val = (unsigned long)dequeue();
		printf("%lu\n", val);
		fflush(stdout);
	}
	
	return 0;
}

int main(void) {
	pthread_t p, c;
	
	sem_init(&countsem, 0, 0);
	sem_init(&spacesem, 0, SIZE);
	sem_init(&lock, 0, 1);
	
	pthread_create(&p, NULL, prod, NULL);
	pthread_create(&c, NULL, cons, NULL);
	
	pthread_join(c, NULL);
	pthread_join(p, NULL);
	
	return 0;
}

