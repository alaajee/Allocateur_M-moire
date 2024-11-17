#include "moniteurProdCons.h"

void initialiserMoniteur(Moniteur *prodCons){
    prodCons->n = 0;
    prodCons->T = 0;
    prodCons->Q = 0;
    pthread_mutex_init(&(prodCons->m), NULL);
    pthread_cond_init(&(prodCons->cProd), NULL);
    pthread_cond_init(&(prodCons->cCons), NULL);
}

void detruireMoniteur(Moniteur *prodCons){
    pthread_mutex_destroy(&(prodCons->m));
    pthread_cond_destroy(&(prodCons->cProd));
    pthread_cond_destroy(&(prodCons->cCons));
}

void deposer(Moniteur *prodCons, struct message msg){
    pthread_mutex_lock(&(prodCons->m));
    while(prodCons->n == N){
        pthread_cond_wait(&(prodCons->cProd), &(prodCons->m));
    }

    // dépôt du message
    tampon[prodCons->Q].ptr = msg.ptr;
    printf("Adresse mémoire %p stockée à l'indice %d\n", tampon[prodCons->Q].ptr, prodCons->Q);
    prodCons->Q = (prodCons->Q + 1) % N;

    prodCons->n = prodCons->n + 1;
    pthread_cond_signal(&(prodCons->cCons));    

    pthread_mutex_unlock(&(prodCons->m));
}

void retire(Moniteur *prodCons, struct message *msg){
    pthread_mutex_lock(&(prodCons->m));
    while(prodCons->n == 0){
        pthread_cond_wait(&(prodCons->cCons), &(prodCons->m));
    }

    // retrait du message
    msg->ptr = tampon[prodCons->T].ptr;
    printf("Adresse mémoire %p retirée de l'indice %d\n", msg->ptr, prodCons->T);
    prodCons->T = (prodCons->T + 1) % N;

    prodCons->n = prodCons->n - 1;
    pthread_cond_signal(&(prodCons->cProd)); 

    pthread_mutex_unlock(&(prodCons->m));
}

