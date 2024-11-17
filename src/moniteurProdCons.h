#ifndef MONITEUR_PROC_CONS_H
#define MONITEUR_PROC_CONS_H

#include <pthread.h>
#define N 5

struct message{
    void* ptr;
}; 
struct message tampon[N];

typedef struct{
    int n;
    int T;
    int Q;
    pthread_cond_t cProd;
    pthread_cond_t cCons;
    pthread_mutex_t m;
} Moniteur;

void initialiserMoniteur(Moniteur *prodCons);
void detruireMoniteur(Moniteur *prodCons);

void deposer(Moniteur *prodCons, struct message msg);
void retire(Moniteur *prodCons, struct message *msg);

#include "moniteurProdCons.c"

#endif 
