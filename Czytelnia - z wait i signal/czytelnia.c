/*
 * czytelnia.c
 *
 *  Created on: 17 lis 2015
 *      Author: michal
 */
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include "czytelnia.h"


#define SYSCHK(PRED) \
  if(!(PRED)) { \
    printf("SYSCHK fail at line %d\n", __LINE__); \
    perror(#PRED); \
    exit(-1); \
}


#define OUT_DEBUG // printf("Wychodzę z %s\n", __func__);
#define IN_DEBUG // printf("Wchodzę do %s\n", __func__);

pthread_mutex_t mutex;
int ile_pisarzy_czeka = 0, ile_czytelnikow_czeka = 0;
pthread_cond_t cond_pisarz, cond_czytelnik;
bool pisarz_w_srodku = false;
int ile_czytelnikow_w_srodku = 0;
int prio_czytelnicy = 0, prio_pisarze = 0;

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

	ile_czytelnikow_czeka += 1;
	while (pisarz_w_srodku || prio_pisarze > 0 ||
			(ile_pisarzy_czeka > 0 && prio_czytelnicy == 0))
		SYSCHK(pthread_cond_wait(&cond_czytelnik, &mutex) == 0);

	ile_czytelnikow_czeka -= 1;

	if (prio_czytelnicy > 0)
		prio_czytelnicy -= 1;

	ile_czytelnikow_w_srodku += 1;

	OUT_DEBUG;
	assert(ile_czytelnikow_w_srodku && !pisarz_w_srodku);
	SYSCHK(pthread_mutex_unlock(&mutex) == 0);
}
void czytelnik_wychodzi() {
	pthread_mutex_lock(&mutex);
	IN_DEBUG;

	assert(!pisarz_w_srodku);

	ile_czytelnikow_w_srodku--;

	if (ile_czytelnikow_w_srodku == 0 && ile_pisarzy_czeka > 0
			&& prio_czytelnicy == 0) {
		prio_pisarze = 1;
		SYSCHK(pthread_cond_signal(&cond_pisarz) == 0);
	}

	OUT_DEBUG;
	pthread_mutex_unlock(&mutex);
}
void pisarz_chce_wejsc() {
	pthread_mutex_lock(&mutex);
	IN_DEBUG;

	ile_pisarzy_czeka += 1;

	while (pisarz_w_srodku || ile_czytelnikow_w_srodku > 0 ||
			prio_czytelnicy > 0) {
		SYSCHK(pthread_cond_wait(&cond_pisarz, &mutex) == 0);
	}

	if (prio_pisarze > 0)
		prio_pisarze -= 1;

	ile_pisarzy_czeka--;
	pisarz_w_srodku = true;

	OUT_DEBUG;
	assert(pisarz_w_srodku && ile_czytelnikow_w_srodku == 0);
	pthread_mutex_unlock(&mutex);
}
void pisarz_wychodzi() {
	pthread_mutex_lock(&mutex);
	IN_DEBUG;

	assert(ile_czytelnikow_w_srodku == 0 && pisarz_w_srodku);

	pisarz_w_srodku = false;

	printf("Pisarz wychodzi, czeka %d czytelników\n", ile_czytelnikow_czeka);
	if (ile_czytelnikow_czeka) {
		prio_czytelnicy = ile_czytelnikow_czeka;
		for (int i = 0; i < ile_czytelnikow_czeka; ++i)
			pthread_cond_signal(&cond_czytelnik);
	} else if (ile_pisarzy_czeka > 0) {
		prio_pisarze = 1;
		pthread_cond_signal(&cond_pisarz);
	}

	OUT_DEBUG;
	pthread_mutex_unlock(&mutex);
}
