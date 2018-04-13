#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include "source/lista.h"
#include "source/utils.h"

#define MIN(a,b) ((a) < (b) ? a : b)
#define TAMNOMEMAX 100
#define QTD_MUTEX 2
#define QTD_BARRIERS 1

//Inicialização das variáveis de controle
FILE * output;
int exitFlag;
int threadsOcupadas;
char * nomeArquivo;
LDDE * vendedor;
pthread_mutex_t lock[QTD_MUTEX];
//Barreira0 trava as threads em uma determinada região até que todas apotem para o primeiro elemento da lista vendedor
pthread_barrier_t barreira[QTD_BARRIERS];
//fim

//Estrutura da oferta de compra e venda
typedef struct {
    unsigned int  quantidade;
    unsigned int  regQuantidade;
    char nome[30];
} info;
//fim

//Estrutura dos argumentos passados aos corretores
struct argumentos_struct {
    int  * nroThread;
    char * nomeArq;
};
typedef struct argumentos_struct argThread;

struct retorno_thread_struct {
    int nroThread;
    NoLDDE * ptrOferta;
};
typedef struct retorno_thread_struct retThread;

argThread * newArgThread(int nroThread, char * nomeArq) {
    argThread * aux;
    aux = (argThread *) malloc(sizeof(argThread));
    aux -> nroThread  = (int *) malloc(sizeof(int));
    *(aux -> nroThread)  = nroThread;

    aux -> nomeArq    = malloc(sizeof(char) * strlen(nomeArq));
    memcpy(aux->nomeArq, nomeArq, sizeof(char) * strlen(nomeArq));

    return aux;
}
//fim

int quantidadeNode(NoLDDE * tmpNo) {
    return (*(info *)tmpNo->dados).quantidade;
}

int quantidadeInicialNode(NoLDDE * tmpNo) {
    return (*(info *)tmpNo->dados).regQuantidade;
}

char * nomeNode(NoLDDE * tmpNo) {
    return (*(info *)tmpNo->dados).nome;
}

int quantidadeCompra(NoLDDE * ptrVendedor, NoLDDE * ptrOferta) {
    int qtdVenda  = quantidadeNode(ptrVendedor);
    int qtdCompra = quantidadeNode(ptrOferta);

    return MIN(qtdVenda, qtdCompra);
}

void * corretor(void * argumentos){
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
        pthread_barrier_wait(&barreira[0]);

        //local de verificar ofertas de vendas e realizar a compra
        NoLDDE *ptrOferta;
        NoLDDE *ptrVendedor;
        //Thread Espera vendedor popular lista
        ptrVendedor = vendedor->inicioLista;

        int cont = 0;
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
                printf("[%s] Ponteiro chegou ao fim da lista\n", args->nomeArq);
                retThread * ret = (retThread *) malloc(sizeof(retThread));
                ret->nroThread = *(args->nroThread);
                ret->ptrOferta = listaOfertas->inicioLista;
                pthread_exit(ret);
            } else if(ptrVendedor == vendedor->fimLista && !exitFlag)
                //ponteiro cheogu ao fim da lista e ainda há itens a serem
                //inseridos, ele só continua esperando
                continue;
            else {
                //ponteiro não chegou ao fim da lista então ele pode avançar
                cont++;
                ptrVendedor = ptrVendedor->prox;
            }
        }
    }else{
    	printf("errro ao criar lista\n");
    }
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
    printf("[ Main ] Inicio do programa\n");
    exitFlag = 0;
	int qtdThreads; //Quantidade de threads
	int i;
    output = fopen("Ouput", "wa");
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
    pthread_barrier_init(&barreira[0], NULL, qtdThreads+1);

	//Cria threads corretores
	for(i = 0; i < qtdThreads; i++){
        char nomeArq[20];
        snprintf(nomeArq, sizeof(nomeArq), "%s-%d", argv[2], i+1);
        argThread * args = newArgThread(i, nomeArq);
		pthread_create(&threadCorretor[i], NULL, corretor, (void *) args);
    }

    //Barreira espera que todas as threads inicializem, para assim a main possa
    //começar a popular a lista de vendas

    int check = 0;
	FILE * ptr = fopen(argv[2],"r");
	if(vendedor != NULL) {
        if(ptr){
        	info * reg = (info *) malloc(sizeof(reg));
        	while((fscanf(ptr,"%s %d\n", reg->nome, &reg->quantidade)) != EOF){
        		if(reg->nome[0] == '#'){
        			//usleep(reg->quantidade*1000);
                    msleep(reg->quantidade);
        		}else{
                    reg->regQuantidade = reg->quantidade;
                    listaInserir(vendedor, reg);
                    if(check == 0) {
                        pthread_barrier_wait(&barreira[0]);
                        check = 1;
                    }
                    printf("[ Main ] Inserindo: %s %d\n", nomeNode(vendedor->fimLista), quantidadeNode(vendedor->fimLista));
        		}
        	}
        	fclose(ptr);
        }
    }

    exitFlag = 1;
    printf("[ Main ] Fim inserção lista vendedor\n");

    //Imprime o conteúdo de cada thread
    for(i = 0; i < qtdThreads; i++){
        void * retornoThread;
        retThread ret;
		pthread_join(threadCorretor[i], &retornoThread);
        ret = *((retThread *) retornoThread);

        if(ret.ptrOferta != NULL) {
            fprintf(output, "Thread %d - Portfolio de itens\n", ret.nroThread+1);
            fprintf(output, "Item               Quantidade  Demanda\n");
            while(ret.ptrOferta != NULL) {
                fprintf(output, fmtport, nomeNode(ret.ptrOferta),
                                         quantidadeInicialNode(ret.ptrOferta) - quantidadeNode(ret.ptrOferta),
                                         quantidadeInicialNode(ret.ptrOferta));
                ret.ptrOferta = ret.ptrOferta->prox;
            }
            fprintf(output, "\n");
        } else {
            printf("Pthread retornou NULL\n");
            exit(EXIT_FAILURE);
        }
	}

    //Após as threads terminarem é a vez da main escrever no arquivo output
    NoLDDE * ptrLista = vendedor->inicioLista;
    fprintf(output, "Saldo de itens:\n");
    fprintf(output, "Item               Quantidade  Ofertado\n");
    char * _nome;
    int _quantidade = 0;
    int _ofertado   = 0;
    while(1) {
        _nome        = nomeNode(ptrLista);
        _quantidade += quantidadeNode(ptrLista);
        _ofertado   += quantidadeInicialNode(ptrLista);
        if(ptrLista->prox == NULL || strcmp(nomeNode(ptrLista->prox), _nome) != 0) {
            fprintf(output, fmtsaldo, _nome, _quantidade, _ofertado);
            _quantidade = 0;
            _ofertado   = 0;
        }

        ptrLista = ptrLista->prox;
        if(ptrLista == NULL)
            break;
    }

    printf("[ Main ] fim\n");
	destroi(&vendedor);
    fclose(output);
    return 0;
}
