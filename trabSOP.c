#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include "source/lista.h"

#define TAMNOMEMAX 100

int threadsOcupadas;
pthread_mutex_t lock;

//Estrutura da oferta de compra e venda
typedef struct {
    int  quantidade;
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

/*
    buscaOfertaCompra
    Funcao que realiza compras
    Tem como entrada
    - ponteiro da lista de ofertas do comprador
    - nome do produto que esta a venda
    - quantidade disponivel para venda

*/

int quantidadeNode(NoLDDE * tmpNo) {
    return (*(info *)tmpNo->dados).quantidade;
}

char * nomeNode(NoLDDE * tmpNo) {
    return (*(info *)tmpNo->dados).nome;
}

int buscaOfertaCompra(LDDE *p, char nome[], int quantDispVendedor){
    NoLDDE *aux;
    aux = p->inicioLista;
    info *reg, x;
    reg = &x;
    int quantidadeComprada;
    //Percorre lista comprador
    while(aux->prox!=NULL) {
        //obtem cada oferta do comprador
        memcpy(reg, aux->dados, p->tamInfo);
        //se quantidade é zero então o comprador está satisfeito
        if(reg->quantidade == 0){
            //printf("satifeito\n");
            return 0;
        }
        //Compara o nome do produto da lista do vendedor com o nome da lista
        // do comprador
        if(memcmp(reg->nome,nome,sizeof(reg->nome))){
            //Se iguais quer dizer que o comprador quer comprar algo com aquele nome
            printf("encontrado\n");
            //Se o vendedor tem uma quantidade maior que o comprador deseja
            //o comprador compra todas daquele produto
            if(quantDispVendedor >= reg->quantidade){
                //comprador fica satisfeito em relação aquele produto
                quantidadeComprada = reg->quantidade;
                reg->quantidade = 0;
                //registra na lista do vendedor o valor atualizado
                memcpy(aux->dados, reg, p->tamInfo);
                //retorna a quatidade comprada
                return quantidadeComprada;
            }else{
                //Se o vendedor tem menos que o comprador precisa
                reg->quantidade = reg->quantidade - quantDispVendedor;
                memcpy(aux->dados, reg, p->tamInfo);
                quantidadeComprada = quantDispVendedor;
                return quantidadeComprada;
            }

        }
        aux = aux->prox;
    }
  return 0;
}

void *corretor(void *argumentos){
	LDDE *listaOfertas = listaCriar(sizeof(info));
    int quantidadeComprada;
	argThread * args = (argThread *) argumentos;

	if(listaOfertas != NULL) {
        //Leitura do arquivo do corretor
        info *reg = (info *) malloc(sizeof(reg));
        FILE *ptr = fopen(args->nomeArq, "r");
        if(ptr){
        	while((fscanf(ptr,"%s %d\n", reg->nome, &reg->quantidade)) != EOF)
				listaInserir(listaOfertas, reg);
        	fclose(ptr);
        }

        //local de disputa pela variavel thread ocupadas
        pthread_mutex_lock(&lock);
        threadsOcupadas = threadsOcupadas-1;
        printf("Corretor %s leu arquivo\n\n", args->nomeArq);
        NoLDDE * temp = listaOfertas->inicioLista;
        pthread_mutex_unlock(&lock);
        //local de verificar ofertas de vendas e realizar a compra
        //Codigo apenas para testes
        NoLDDE *ptrVendedor = vendedor->inicioLista;
        while(1){
            if(ptrVendedor != NULL) {
                NoLDDE * temp = listaOfertas->inicioLista;

                while(temp != NULL) {
                    if(strcmp(nomeNode(ptrVendedor), nomeNode(temp)) == 0){
                        if(quantidadeNode(ptrVendedor) > 0) {
                            pthread_mutex_lock(&lock);
                            printf("Em %s\n", args->nomeArq);
                            printf("    Tentando comprar %s\n\n", nomeNode(ptrVendedor));
                            pthread_mutex_unlock(&lock);
                        }
                    }
                    temp = temp->prox;
                }
                ptrVendedor = ptrVendedor->prox;
            }
    	}
    	destroi(&listaOfertas);
    }else{
    	printf("errro ao criar lista\n");
    }

	pthread_exit(NULL);
}



int main(int argc, char** argv) {
	int qtdThreads; //Quantidade de threads
	int i;
    if(pthread_mutex_init(&lock, NULL) != 0) {
        fprintf(stderr, "Erro na criação do Mutex, amigo se chegou aqui você é um campeão");
        exit(EXIT_FAILURE);
    }

	//Argumentos passados pela linha de comando
    qtdThreads = atoi(argv[1]);
    threadsOcupadas = qtdThreads;
    pthread_t threadCorretor[qtdThreads];

	//Cria threads corretores
	for(i = 0; i < qtdThreads; i++){
        char nomeArq[20];
        snprintf(nomeArq, sizeof(nomeArq), "%s-%d", argv[2], i+1);
        argThread * args = newArgThread(i, nomeArq);
		pthread_create(&threadCorretor[i], NULL, corretor, (void *) args);
    }

    char * arquivoCorretor = argv[2];
	FILE * ptr = fopen("prgA","r");
    vendedor = listaCriar(sizeof(info));
	if(vendedor != NULL) {
        //printf("lista para vendas criada\n");
        //Percorre o arquivo
        if(ptr){
        	info * reg = (info *) malloc(sizeof(reg));
            //fflush(stdout);
        	while((fscanf(ptr,"%s %d\n", reg->nome, &reg->quantidade)) != EOF){
                //fflush(stdout);
                //printf("%s %d\n", reg->nome, reg->quantidade);
        		if(reg->nome[0] == '#'){
        			//printf("dorme\n");
        		}else{
                    //printf("Main\n Inseriu: %s %d\n\n", reg->nome, reg->quantidade);
                    listaInserir(vendedor, reg);
                    printf("Inserindo: %s %d\n\n", nomeNode(vendedor->fimLista), quantidadeNode(vendedor->fimLista));
                    //usleep(1000);
                    //printf("%s %d", reg->nome, *reg->quantidade);
        			//printf("inseriu\n");
        		}
        	}
        	fclose(ptr);
        }
    }

    for(i = 0; i<qtdThreads; i++){
		 pthread_join(threadCorretor[i], NULL);
	}

	destroi(&vendedor);
    return 0;
}
