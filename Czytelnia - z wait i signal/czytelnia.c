/*
 * czytelnia.c
 *
 *  Created on: 17 lis 2015
 *      Author: michal
 */
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include "czytelnia.h"

#define OUT_DEBUG printf("Wychodzę z %s\n", __func__);
#define IN_DEBUG printf("Wchodzę do %s\n", __func__);

pthread_mutex_t mutex;
int ile_pisarzy_czeka = 0, ile_czytelnikow_czeka = 0;
pthread_cond_t cond_pisarz, cond_czytelnik;
bool pisarz_w_srodku = false;
int ile_czytelnikow_w_srodku = 0;
bool wpuszczamy_czytelnikow = false;

void inicjalizacja() {
	pthread_mutex_init(&mutex, NULL);
	puts("Zainicjalizowano Monitor");
	pthread_cond_init(&cond_pisarz, NULL);
	pthread_cond_init(&cond_czytelnik, NULL);
}
void deinicjalizacja() {
	pthread_cond_destroy(&cond_pisarz);
	pthread_cond_destroy(&cond_czytelnik);
	pthread_mutex_destroy(&mutex);
	puts("Zniszczono Monitor");
}
void czytelnik_chce_wejsc() {
	pthread_mutex_lock(&mutex);
	IN_DEBUG;

	// sam próbuje wejśc
	if (pisarz_w_srodku || ile_pisarzy_czeka) {
		// staje w kolejce
		ile_czytelnikow_czeka += 1;
		while (!wpuszczamy_czytelnikow) {
			printf("Wybudzony czytelnik, pisarz w środku: %d, czelnikow w srodku: %d"
					", wpuszczamy czytelnikow: %d\n",
					(int)pisarz_w_srodku, (int)ile_czytelnikow_w_srodku,
					(int)wpuszczamy_czytelnikow);
			assert(!wpuszczamy_czytelnikow || !pisarz_w_srodku);

			pthread_cond_wait(&cond_czytelnik, &mutex);
		}

		assert(!pisarz_w_srodku);

		ile_czytelnikow_czeka -= 1;
		ile_czytelnikow_w_srodku += 1;

		// ktos go skutecznie wybudził

		if (ile_czytelnikow_czeka == 0) {
			wpuszczamy_czytelnikow = false;
		} else {
			printf("Wpuszczam kolejnego czytelnika\n");
			pthread_cond_signal(&cond_czytelnik);
		}
	} else {
		// nie ma żadnych oczekujący pisarzy ani pisarzy w środku, można wejśc
		ile_czytelnikow_w_srodku += 1;
	}

	OUT_DEBUG;
	assert(ile_czytelnikow_w_srodku && !pisarz_w_srodku);
	pthread_mutex_unlock(&mutex);
}
void czytelnik_wychodzi() {
	pthread_mutex_lock(&mutex);
	IN_DEBUG;

	assert(!pisarz_w_srodku);

	ile_czytelnikow_w_srodku--;

	if (ile_czytelnikow_w_srodku == 0) {
		if (ile_pisarzy_czeka) {
			pthread_cond_signal(&cond_pisarz);
		} else if (ile_czytelnikow_czeka > 0) {
			wpuszczamy_czytelnikow = true;
			printf("Wpuszczam pierwszego czytelnika wychodząc\n");
			pthread_cond_signal(&cond_czytelnik);
		}
	}

	OUT_DEBUG;
	pthread_mutex_unlock(&mutex);
}
void pisarz_chce_wejsc() {
	pthread_mutex_lock(&mutex);
	IN_DEBUG;

	assert(!wpuszczamy_czytelnikow);

	if (pisarz_w_srodku || ile_czytelnikow_w_srodku > 0) {
		ile_pisarzy_czeka++;
		while (pisarz_w_srodku || ile_czytelnikow_w_srodku > 0) {
			printf("Wybudzony pisarz, pisarz w środku: %d, czelnikow w srodku: %d\n",
					(int)pisarz_w_srodku, (int)ile_czytelnikow_w_srodku);
			pthread_cond_wait(&cond_pisarz, &mutex);
		}
		ile_pisarzy_czeka--;
		pisarz_w_srodku = true;
	} else {
		pisarz_w_srodku = true;
	}

	OUT_DEBUG;
	assert(pisarz_w_srodku && ile_czytelnikow_w_srodku == 0);
	pthread_mutex_unlock(&mutex);
}
void pisarz_wychodzi() {
	pthread_mutex_lock(&mutex);
	IN_DEBUG;
	pisarz_w_srodku = false;

	printf("Pisarz wychodzi, czeka %d czytelników\n", ile_czytelnikow_czeka);
	if (ile_czytelnikow_czeka) {
		wpuszczamy_czytelnikow = true;
		printf("Wpuszczam pierwszego czytelnika wychodząc (pisarz)\n");
		pthread_cond_signal(&cond_czytelnik);
	} else {
		pthread_cond_signal(&cond_pisarz);
	}

	OUT_DEBUG;
	pthread_mutex_unlock(&mutex);
}
