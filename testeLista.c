#include <stdio.h>
#include "source/lista.h"

int main() {
    LDDE * lista = listaCriar(sizeof(int));
    int vet[] = {0, 1, 2, 3, 4};
    listaInserir(lista, &vet[0]);
    listaInserir(lista, &vet[1]);
    listaInserir(lista, &vet[2]);
    listaInserir(lista, &vet[3]);
    listaInserir(lista, &vet[4]);
    listaRemover(lista, lista->inicioLista->prox->prox);
    //listaRemover(lista, lista->inicioLista);
    //listaRemover(lista, lista->inicioLista);
    //listaRemover(lista, lista->inicioLista);
    //listaRemover(lista, lista->inicioLista);

    if(lista->inicioLista != NULL) {
        printf("%d - %d\n", *(int *)lista->inicioLista->dados, *(int *)lista->fimLista->dados);
        printf("\nOrdem elementos lista:\n");
        NoLDDE * node = lista->inicioLista;
        if(node == NULL)
            printf("lol");
        for(;;) {
            printf("%d\n", *(int *)node->dados);
            if(node->prox == NULL)
                break;

            node = node->prox;
        }

        printf("\nOrdem inversa\n");
        NoLDDE * node0 = lista->fimLista;
        for(;;) {
            printf("%d\n", *(int *) node0->dados);
            if(node0->ant == NULL)
                break;

            node0 = node0->ant;
        }
    }
}
