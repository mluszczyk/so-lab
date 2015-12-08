#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

#include "czytelnia.h"

const char *MESSAGE = "--- Czytelnia z wait i signal ---";

char napis[200];

void *czytelnik(void *arg) {
	while (true) {
		czytelnik_chce_wejsc();
		// usleep(100);
		printf("%s\n", napis);
		czytelnik_wychodzi();
	}
	return NULL;
}

void *pisarz(void *arg) {
	while (true) {
		pisarz_chce_wejsc();
		for (int i = 0; i < 1; ++i) {
			int l = rand();  // wymaga nagłówka stdlib.h
			snprintf(napis, 200, "Liczba wypisana 6 razy %d %d  %d %d %d %d", l, l, l, l, l, l);
		}
		pisarz_wychodzi();
	}
	return NULL;
}

int main(int argc, char *argv[]) {
	puts(MESSAGE);

	inicjalizacja();

	pthread_t threads[10];
	for (int i = 0; i < 10; ++i) {
		pthread_create(&threads[i], NULL, (i % 2)? pisarz : czytelnik, NULL);
	}

	for (int i = 0; i < 10; ++i) {
		pthread_join(threads[i], NULL);
	}

	deinicjalizacja();

	return 0;
}
