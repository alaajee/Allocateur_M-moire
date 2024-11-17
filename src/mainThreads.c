#include <sys/resource.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>

#include "mem.h"
#include "moniteurProdCons.h"

#define NOM_THREADS 16

Moniteur procCons;

void *produire(void *arg){
    while(true){
        int indice = rand() % 20 + 1;
        int taille_base = pow(2, indice);
        int taille = pow(2, indice) - pow(2, indice-1);
        unsigned long num = (rand() % taille) + taille_base;
        void *ptr = emalloc(num);
        struct message msg = {ptr};
        printf("On a réservé l'adresse mémoire: %p pour une taille: %ld\n", ptr, num);
        deposer(&procCons, msg);
    }
}

void *consomme(void *arg){
    while(true){
        struct message msg;
        retire(&procCons, &msg);
        efree(msg.ptr);
        printf("On a liberé l'adresse mémoire: %p\n", msg.ptr);
    } 
}

int main(){

    // Initialiser le moniteur
    initialiserMoniteur(&procCons);

    pthread_t threads[NOM_THREADS];
    int ids[NOM_THREADS];

    struct rlimit limit;
    limit.rlim_cur = 0.3;  // 1 seconde
    limit.rlim_max = 0.3;  // 1 seconde

    // Limiter le temps d'exécution à une seconde
    if (setrlimit(RLIMIT_CPU, &limit) != 0) {
        perror("Error en strlimit");
        exit(EXIT_FAILURE);
    }

    // Créer NOM_THREADS threads
    for(int i = 0; i < NOM_THREADS; i++){
        ids[i] = i;
        if(i % 2 == 0){
            if(pthread_create(&threads[i], NULL, produire, (void*) &ids[i]) != 0){
                fprintf(stderr, "Erreur lors de la création du thread %d\n", i);
                exit(EXIT_FAILURE);
            }
        }else{
            if(pthread_create(&threads[i], NULL, consomme, (void*) &ids[i]) != 0){
                fprintf(stderr, "Erreur lors de la création du thread %d\n", i);
                exit(EXIT_FAILURE);
            }
        }
    }

    // Attendre tous les threads
    for (int i = 0; i < NOM_THREADS; ++i) {
        pthread_join(threads[i], NULL);
    }

    // detruire moniteur
    detruireMoniteur(&procCons);

    return 0;
}