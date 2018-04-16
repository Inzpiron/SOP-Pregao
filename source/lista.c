#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "lista.h"

//Criar lista
LDDE * listaCriar(unsigned long tamInfo) {
    LDDE *desc = (LDDE*) malloc(sizeof(LDDE));

    if(!desc) {
        exit(EXIT_FAILURE);
    } else {
        desc->inicioLista = NULL;
        desc->fimLista    = NULL;
        desc->tamInfo     = tamInfo;
    }

    return desc;
}

int listaInserir(LDDE *p, void *novo) {
    NoLDDE *temp;
    int ret = 0;

    if((temp = (NoLDDE*) malloc(sizeof(NoLDDE))) != NULL) {
        if((temp->dados = (void*) malloc(p->tamInfo)) != NULL) {
            memcpy(temp->dados, novo, p->tamInfo);
            temp->prox = NULL;
            if(p->inicioLista == NULL && p->fimLista == NULL) {
                temp->ant = NULL;
                p->fimLista    = temp;
                p->inicioLista = temp;
            } else if(p->inicioLista == p->fimLista) {
                temp->ant  = p->inicioLista;
                temp->prox = NULL;
                p->fimLista = temp;
                p->inicioLista->prox = p->fimLista;
            } else {
                temp->ant   = p->fimLista;
                temp->prox  = NULL;
                p->fimLista->prox = temp;
                p->fimLista = temp;
            }
            ret = 1;
        }
        else
            free(temp);
    }
    return ret;
}

int listaBuscaInicio(LDDE *p, void *reg) {
    int ret = 0;
    if(p->inicioLista != NULL) {
        memcpy(reg, p->inicioLista->dados, p->tamInfo);
        ret = 1;
    }
    return ret;
}

int listaBuscaFim(LDDE *p, void *reg) {
    int ret = 0;
    if(p->fimLista != NULL) {
        memcpy(reg, p->fimLista->dados, p->tamInfo);
        ret = 1;
    }
    return ret;
}

int listaRemover(LDDE * p, NoLDDE * node) {
    if(p->inicioLista == p->fimLista && p->inicioLista != NULL){ // Lista com 1 elemento
        if(p->inicioLista == node) { // node para excluir = node inicio lista
            free(p->inicioLista);
            p->inicioLista = NULL;
            p->fimLista    = NULL;
            return 1;
        }
        return 0;
    } else{ // lista com mais de 1 elemento
        NoLDDE * aux;
        if(p->inicioLista == node) { //node para excluir  = node inicio lista
            aux = p->inicioLista;
            p->inicioLista = p->inicioLista->prox;
            p->inicioLista->ant = NULL;
            free(aux);
            return 1;
        } else if(p->fimLista == node) { // node para excluir = node fim lista
            aux = p->fimLista;
            p->fimLista = p->fimLista->ant;
            p->fimLista->prox = NULL;
            free(aux);
            return 1;
        } else { // lista com mais de 2 elementos, e node para excluir != node fim lista e node inicio lista
            NoLDDE * temp = p->inicioLista;
            while(temp != NULL) {
                if(temp == node) {
                    aux = node;
                    node->prox->ant = node->ant;
                    node->ant->prox = node->prox;
                    free(aux);
                    return 1;
                }
                temp = temp->prox;
            }
        }
    }

    return 0;
}

void destroi(LDDE **pp) {
    free(*pp);
    (*pp) = NULL;
}
