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
#include "source/lista.h"
#include "source/utils.h"

#define MIN(a,b) ((a) < (b) ? a : b)
#define TAMNOMEMAX 100

//Inicialização das variáveis de controle
FILE * output;
int exitFlag;
int tamNomeArquivo = 0;
char * nomeArquivo;
LDDE * vendedor;
pthread_mutex_t lock;
pthread_barrier_t barrier;

/*------------------------------------------------------------------------------
    info
    Estrutura de oferta de compra e venda
------------------------------------------------------------------------------*/
struct info{
    unsigned int  quantidade;
    unsigned int  regQuantidade;
    char nome[30];
};
typedef struct info info;

/*------------------------------------------------------------------------------
    argumentos_struct
    estrutura dos argumentos passados para thread
------------------------------------------------------------------------------*/
struct argumentos_struct {
    int  * nroThread;
    char * nomeArq;
};
typedef struct argumentos_struct argThread;

/*------------------------------------------------------------------------------
    retThread
    estrutura referente ao retorno das threads
------------------------------------------------------------------------------*/
struct retorno_thread_struct {
    int nroThread;
    NoLDDE * ptrOferta;
};
typedef struct retorno_thread_struct retThread;

/*------------------------------------------------------------------------------
    argThread
    Função que retorna ponteiro para struct do argumento passado para
    thread
------------------------------------------------------------------------------*/
argThread * newArgThread(int nroThread, char * nomeArq);

/*------------------------------------------------------------------------------
    file_exist
    Verifica a existencia de um arquivo
    Créditos: StackOverflow
------------------------------------------------------------------------------*/
int file_exist (char *filename);

/*------------------------------------------------------------------------------
    exitError
    Sair do programa com erro printando uma mensagem
------------------------------------------------------------------------------*/
void exitError(char * msg);

/*------------------------------------------------------------------------------
    checkEntry
    Verifica se a entrada do usuário é válida
------------------------------------------------------------------------------*/
int checkEntry(int argc, char ** argv);

/*------------------------------------------------------------------------------
    quantidadeNode
    retorna a quantidade do nó
------------------------------------------------------------------------------*/
int quantidadeNode(NoLDDE * tmpNo);

/*------------------------------------------------------------------------------
    quantidadeInicialNode
    retorna a quantidade inicial do nó
------------------------------------------------------------------------------*/
int quantidadeInicialNode(NoLDDE * tmpNo);

/*------------------------------------------------------------------------------
    nomeNode
    retorna nome do nó
------------------------------------------------------------------------------*/
char * nomeNode(NoLDDE * tmpNo);

/*------------------------------------------------------------------------------
    quantidadeCompra
    retorna a quantidade que deve ser comprada, passando o nó do vendedor
    e o nó da oferta por meio de ponteiros de lista
------------------------------------------------------------------------------*/
int quantidadeCompra(NoLDDE * ptrVendedor, NoLDDE * ptrOferta);

/*------------------------------------------------------------------------------
    corretor
    Cria uma lista com ofertas de compra
    Consulta ofertas de vendas
    Faz compra dentros dos limites de oferta e demanda
------------------------------------------------------------------------------*/
void * corretor(void * argumentos);

/*------------------------------------------------------------------------------
    main
------------------------------------------------------------------------------*/
int main(int argc, char** argv) {
    checkEntry(argc, argv);
    system("tput reset");
    printf("[ Main ] Entrada ok!\n");
    exitFlag = 0;
	int qtdThreads; //Quantidade de threads
	int i;
    output = fopen("Ouput", "wa");
    vendedor = listaCriar(sizeof(info));

    if(pthread_mutex_init(&lock, NULL) != 0) {
            fprintf(stderr, "Erro na criação do Mutex, amigo se chegou aqui você é um campeão");
            exit(EXIT_FAILURE);
    }

	//Argumentos passados pela linha de comando
    qtdThreads = atoi(argv[1]);
    pthread_t threadCorretor[qtdThreads];
    pthread_barrier_init(&barrier, NULL, qtdThreads+1);

	//Cria threads corretores
	for(i = 0; i < qtdThreads; i++){
        char nomeArq[20];
        snprintf(nomeArq, sizeof(nomeArq), "%s-%d", argv[2], i+1);
        argThread * args = newArgThread(i, nomeArq);
		pthread_create(&threadCorretor[i], NULL, corretor, (void *) args);
    }

    //Barreira espera que todas as threads inicializem, para assim a main possa
    //começar a popular a lista de vendas
    pthread_barrier_wait(&barrier);

    //Vendedor insere ofertas
	FILE * ptr = fopen(argv[2],"r");
	if(vendedor != NULL) {
        if(ptr){
        	info * reg = (info *) malloc(sizeof(reg));
        	while((fscanf(ptr,"%s %d\n", reg->nome, &reg->quantidade)) != EOF){
        		if(reg->nome[0] == '#'){
                    msleep(reg->quantidade);
        		}else{
                    reg->regQuantidade = reg->quantidade;
                    printf("[ Main ] Inserindo %s %d\n", reg->nome, reg->quantidade);
                    listaInserir(vendedor, reg);
        		}
        	}
        	fclose(ptr);
        }
    }

    printf("[ Main ] Terminou de inserir\n");
    exitFlag = 1;

    //Imprime o conteúdo de cada thread em um arquivo Out
    for(i = 0; i < qtdThreads; i++){
        void * retornoThread;
        retThread ret;
		pthread_join(threadCorretor[i], &retornoThread);
        ret = *((retThread *) retornoThread);

        if(ret.ptrOferta != NULL) {
            fprintf(output, "Thread %d - Portfolio de itens:\n", ret.nroThread+1);
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

    pthread_barrier_destroy(&barrier);
    pthread_mutex_destroy(&lock);
	destroi(&vendedor);
    fclose(output);
    return 0;
}

void * corretor(void * argumentos){
	LDDE * listaOfertas = listaCriar(sizeof(info));
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

        //Barreira espera todos os corretores terminarem de lerem seus arquivos
        printf("[%s] Leu arquivo\n", args->nomeArq);
        pthread_barrier_wait(&barrier);

        NoLDDE *  ptrOferta;
        NoLDDE ** ptrVendedor;

        //Barreira espera o vendedor colocar algo na lista
        ptrVendedor = &(vendedor->inicioLista);

        int cont = 0;
        //Corretor verifica disponibilidade e faz a compra
        while(1) {
            if(*ptrVendedor != NULL) {
                ptrOferta = listaOfertas->inicioLista;
                while(ptrOferta != NULL) {
                    if(strcmp(nomeNode(*ptrVendedor), nomeNode(ptrOferta)) == 0) {
                        pthread_mutex_lock(&lock);
                        if(quantidadeNode(*ptrVendedor) > 0 && quantidadeNode(ptrOferta) > 0) {
                            int qtdCompra = quantidadeCompra(*ptrVendedor, ptrOferta);
                            (*(info *)((*ptrVendedor)->dados)).quantidade -= qtdCompra;
                            (*(info *)ptrOferta->dados).quantidade        -= qtdCompra;
                            printf("[%s] Comprando %d %s\n", args->nomeArq, qtdCompra, nomeNode(ptrOferta));
                        }
                        pthread_mutex_unlock(&lock);
                    }
                    ptrOferta = ptrOferta->prox;
                }

                if(*ptrVendedor == (vendedor->fimLista) && exitFlag) {
                    retThread * ret = (retThread *) malloc(sizeof(retThread));
                    ret->nroThread = *(args->nroThread);
                    ret->ptrOferta = listaOfertas->inicioLista;
                    printf("[%s] Chegou ao fim\n", args->nomeArq);
                    sched_yield();
                    pthread_exit(ret);
                } else if(*ptrVendedor == vendedor->fimLista && !exitFlag) {
                    //ponteiro cheogu ao fim da lista e ainda há itens a serem
                    //inseridos, ele só continua esperando
                    //printf("[%s] lol\n", args->nomeArq);
                    continue;
                } else {
                    //ponteiro não chegou ao fim da lista então ele pode avançar
                    cont++;
                    ptrVendedor = &((*ptrVendedor)->prox);
                }
            }
        }
    }else{
    	printf("erro ao criar lista\n");
    }

    return NULL;
}

int quantidadeCompra(NoLDDE * ptrVendedor, NoLDDE * ptrOferta) {
    int qtdVenda  = quantidadeNode(ptrVendedor);
    int qtdCompra = quantidadeNode(ptrOferta);

    return MIN(qtdVenda, qtdCompra);
}

char * nomeNode(NoLDDE * tmpNo) {
    return (*(info *)tmpNo->dados).nome;
}

int quantidadeInicialNode(NoLDDE * tmpNo) {
    return (*(info *)tmpNo->dados).regQuantidade;
}

int quantidadeNode(NoLDDE * tmpNo) {
    return (*(info *)tmpNo->dados).quantidade;
}

int checkEntry(int argc, char ** argv) {
    if(argc == 1)
        exitError(strdup("[ Main ] Não há argumentos suficientes"));

    if(strcmp(argv[1], "help") == 0 || strcmp(argv[1], "Help") == 0) {
        printf("Necessário uso de 2 argumentos\n");
        printf("Arg1 = quantidade de threads\n");
        printf("Arg2 = nome do arquivo de leitura\n");
        exit(EXIT_SUCCESS);
    }

    if(argc != 3)
        exitError(strdup("[ Main ] Entrada inválida\n"));

    int qtdt = atoi(argv[1]);

    if(!file_exist(argv[2])) {
        char msgErro[100];
        snprintf(msgErro, sizeof(msgErro), "[ Main ] Arquivo não encontrado (%s)", argv[2]);
        exitError(msgErro);
    }

    int i;
    for(i = 0; i < qtdt; i++) {
        char nomeArq[20];
        snprintf(nomeArq, sizeof(nomeArq), "%s-%d", argv[2], i+1);
        if(!file_exist(nomeArq)) {
            char  msgErro[100];
            snprintf(msgErro, sizeof(msgErro), "[ Main ] Arquivo não encontrado (%s)", nomeArq);
            exitError(msgErro);
        }
    }

    return 0;
}

void exitError(char * msg) {
    printf("[ ERRO ]%s\n", msg);
    printf("[ ERRO ] 'Help' para informações\n");
    exit(EXIT_FAILURE);
}

int file_exist (char *filename) {
  struct stat   buffer;
  return (stat (filename, &buffer) == 0);
}

argThread * newArgThread(int nroThread, char * nomeArq) {
    argThread * aux;
    aux = (argThread *) malloc(sizeof(argThread));
    aux->nroThread  = (int *) malloc(sizeof(int));
    *(aux->nroThread)  = nroThread;

    aux->nomeArq    = (char *) malloc(sizeof(char) * strlen(nomeArq));
    memcpy(aux->nomeArq, nomeArq, sizeof(char) * strlen(nomeArq));

    return aux;
}
