#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include "source/lista.h"

#define MIN(a,b) ((a) < (b) ? a : b)
#define TAMNOMEMAX 100
#define QTD_MUTEX 3
#define QTD_BARRIERS 2

int exitFlag;
int threadsOcupadas;
pthread_mutex_t lock[QTD_MUTEX];
//Barreira0 trava as threads em uma determinada região até que todas apotem para o primeiro elemento da lista vendedor
pthread_barrier_t barreira[QTD_BARRIERS];

//Estrutura da oferta de compra e venda
typedef struct {
    int  quantidade;
    int  regQuantidade;
    char nome[30];
} info;

//Estrutura dos argumentos passados aos corretores
struct argumentos_struct {
    int  * nroThread;
    char * nomeArq;
};
typedef struct argumentos_struct argThread;
argThread * newArgThread(int nroThread, char * nomeArq) {
    argThread * aux;
    aux = (argThread *) malloc(sizeof(argThread));
    aux -> nroThread  = (int *) malloc(sizeof(int));
    *(aux -> nroThread)  = nroThread;

    aux -> nomeArq    = malloc(sizeof(char) * strlen(nomeArq));
    memcpy(aux->nomeArq, nomeArq, sizeof(char) * strlen(nomeArq));
    return aux;
}

char * nomeArquivo;
LDDE * vendedor;

int quantidadeNode(NoLDDE * tmpNo) {
    return (*(info *)tmpNo->dados).quantidade;
}

char * nomeNode(NoLDDE * tmpNo) {
    return (*(info *)tmpNo->dados).nome;
}

int quantidadeCompra(NoLDDE * ptrVendedor, NoLDDE * ptrOferta) {
    int qtdVenda  = quantidadeNode(ptrVendedor);
    int qtdCompra = quantidadeNode(ptrOferta);

    return MIN(qtdVenda, qtdCompra);
}

void * corretor(void *argumentos){
	LDDE * listaOfertas = listaCriar(sizeof(info));
    int quantidadeComprada;
	argThread * args = (argThread *) argumentos;

	if(listaOfertas != NULL) {
        //Leitura do arquivo do corretor
        info *reg = (info *) malloc(sizeof(reg));

        FILE *ptr = fopen(args->nomeArq, "r");
        if(ptr){
        	while((fscanf(ptr,"%s %d\n", reg->nome, &reg->quantidade)) != EOF){
        		reg->regQuantidade = reg->quantidade;
				listaInserir(listaOfertas, reg);
        	}
        	fclose(ptr);
        }
        //local de disputa pela variavel thread ocupadas
        pthread_mutex_lock(&lock[0]);
        threadsOcupadas = threadsOcupadas-1;
        printf("[%s] leu arquivo\n", args->nomeArq);
        NoLDDE * temp = listaOfertas->inicioLista;
        pthread_mutex_unlock(&lock[0]);

        //local de verificar ofertas de vendas e realizar a compra
        NoLDDE *ptrOferta;
        NoLDDE *ptrVendedor;
        //Thread Espera vendedor popular lista
        while(1) {
            ptrVendedor = vendedor->inicioLista;
            if(ptrVendedor != NULL) break;
        }
        printf("[%s] PtrVendedor deixou de ser NULL\n", args->nomeArq);
        pthread_barrier_wait(&barreira[0]);

        while(1) {
            ptrOferta = listaOfertas->inicioLista;
            while(ptrOferta != NULL) {
                if(strcmp(nomeNode(ptrVendedor), nomeNode(ptrOferta)) == 0) {
                    pthread_mutex_lock(&lock[1]);
                    if(quantidadeNode(ptrVendedor) > 0 && quantidadeNode(ptrOferta) > 0) {
                        int qtdCompra = quantidadeCompra(ptrVendedor, ptrOferta);
                        printf("[%s] Comprando %d de %s\n", args->nomeArq, qtdCompra, nomeNode(ptrVendedor));
                        (*(info *)ptrVendedor->dados).quantidade -= qtdCompra;
                        (*(info *)ptrOferta->dados).quantidade   -= qtdCompra;
                    }
                    pthread_mutex_unlock(&lock[1]);
                }
                ptrOferta = ptrOferta->prox;
            }

            if(ptrVendedor == vendedor->fimLista && exitFlag) {
                //ponteiro chegou ao fim da lista e não há mais itens para serem inseridos
                //Barreira[1] faz com que as threads comecem a impimir só quando
                //a main terminou de inserir itens na lista vendedor
                pthread_barrier_wait(&barreira[1]);

                //lock[2] faz com que seja printado apenas a informação de uma
                //Thread por vez.
                pthread_mutex_lock(&lock[2]);
                printf("[%s] ptrVendedor chegou ao fim\n", args->nomeArq);
                ptrOferta = listaOfertas->inicioLista;
                while(ptrOferta != NULL) {
                    printf("%s %d\n", nomeNode(ptrOferta), quantidadeNode(ptrOferta));
                    ptrOferta = ptrOferta->prox;
                }
                pthread_mutex_unlock(&lock[2]);
                pthread_exit(NULL);
            } else if(ptrVendedor == vendedor->fimLista && !exitFlag)
                //ponteiro cheogu ao fim da lista e ainda há itens a serem
                //inseridos, ele só continua esperando
                continue;
            else {
                //ponteiro não chegou ao fim da lista então ele pode avançar
                ptrVendedor = ptrVendedor->prox;
            }
        }
    	destroi(&listaOfertas);
    }else{
    	printf("errro ao criar lista\n");
    }

	pthread_exit(NULL);
}

void imprimirPort(LDDE *lista){
	NoLDDE * temp1 = lista->inicioLista;
	NoLDDE * temp2 = lista->inicioLista;
	int demanda = 0;
	int compra = 0;

	while(temp1!=NULL){
		temp1 = temp1->prox;
		temp2 = lista->inicioLista;
		demanda = (*(info *)temp2->dados).regQuantidade;
		while(temp2!=NULL){
			if((strcmp(nomeNode(temp1), nomeNode(temp2)) == 0)){
				compra +=  demanda - (*(info *)temp2->dados).quantidade;
			}
			temp2 = temp2->prox;
		}
		//printf("print aqui nome, demanda, compra\n", );
	}
}


int main(int argc, char** argv) {
    system("tput reset");
    exitFlag = 0;
	int qtdThreads; //Quantidade de threads
	int i;
    vendedor = listaCriar(sizeof(info));

    for(i = 0; i < QTD_MUTEX; i++) {
        if(pthread_mutex_init(&lock[i], NULL) != 0) {
            fprintf(stderr, "Erro na criação do Mutex, amigo se chegou aqui você é um campeão");
            exit(EXIT_FAILURE);
        }
    }

	//Argumentos passados pela linha de comando
    qtdThreads = atoi(argv[1]);
    threadsOcupadas = qtdThreads;
    pthread_t threadCorretor[qtdThreads];
    pthread_barrier_init(&barreira[0], NULL, qtdThreads);
    pthread_barrier_init(&barreira[1], NULL, qtdThreads+1);

	//Cria threads corretores
	for(i = 0; i < qtdThreads; i++){
        char nomeArq[20];
        snprintf(nomeArq, sizeof(nomeArq), "%s-%d", argv[2], i+1);
        argThread * args = newArgThread(i, nomeArq);
		pthread_create(&threadCorretor[i], NULL, corretor, (void *) args);
    }

    while(1) {
        if(threadsOcupadas == 0)
            break;
    }

    printf("[ Main ] Todos os arquivos foram lidos\n\n");

	FILE * ptr = fopen(argv[2],"r");
	if(vendedor != NULL) {
        if(ptr){
        	info * reg = (info *) malloc(sizeof(reg));
        	while((fscanf(ptr,"%s %d\n", reg->nome, &reg->quantidade)) != EOF){
        		if(reg->nome[0] == '#'){
        			//usleep(reg->quantidade*1000);
                    sleep(1);
        		}else{
                    listaInserir(vendedor, reg);
                    printf("[ Main ] Inserindo: %s %d\n", nomeNode(vendedor->fimLista), quantidadeNode(vendedor->fimLista));
        		}
        	}
        	fclose(ptr);
        }
    }
    exitFlag = 1;
    printf("[ Main ] Fim inserção lista vendedor\n");
    //Barreira[1] espera todas as threads chegarem ao fim, assim temos o controle
    //e o momento em que todas já terminaram de fazer manipulações na lista vendedor
    //MOTIVO: Nem todas as threads podem estar apontando para o fim da lista quando
    //a main já tiver parado de popula-la. Então botei essa barreira pra ter certeza
    //que todas já estão no fim para poder printar o conteudo.
    pthread_barrier_wait(&barreira[1]);


    for(i = 0; i<qtdThreads; i++){
		pthread_join(threadCorretor[i], NULL);
	}

	destroi(&vendedor);
    return 0;
}
