#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define TAMNOMEMAX 100

int threadsOcupadas;

//Estrutura da lista
typedef struct noLDDE {
    void *dados;
    struct noLDDE *prox;
    struct noLDDE *ant;
} NoLDDE;

typedef struct LDDE { 
    int tamInfo;
    NoLDDE *lista;
} LDDE;

//Estrutura da oferta de compra e venda 
typedef struct { 
    int quantidade;
    char nome[30];	
} info;

//Estrutura dos argumentos passados aos corretores
struct argumentos_struct {
    int qtdthreads;
    int tamnomeArq;
    char nomeArq[TAMNOMEMAX];
   
};

//Criar lista
int cria(LDDE **pp, int tamInfo)
{   
    int ret = 0;
    LDDE *desc = (LDDE*) malloc(sizeof(LDDE));

    if( !desc ) {
        ret = 0;
    }
    else {	
        desc->lista = NULL;
        desc->tamInfo = tamInfo;
        ret = 1;
    }

    (*pp) = desc;

    return ret;
}
int insereNoFim(LDDE *p, void *novo)
{ 	
    NoLDDE *temp, *aux;
    int ret = 0;

    if( (temp = (NoLDDE*) malloc(sizeof(NoLDDE))) != NULL ) {
        if((temp->dados = (void*) malloc(p->tamInfo)) != NULL ) {
            memcpy(temp->dados, novo, p->tamInfo);
            temp->prox = NULL;
            if(p->lista == NULL) {	
                p->lista = temp;
                temp->ant = NULL;
            }
            else {	
                aux = p->lista;
                while(aux->prox != NULL)
                    aux = aux->prox;
                aux->prox = temp;
                temp->ant = aux;
            }
            ret = 1;
        }
        else {
            free(temp);
        }
    }

    return ret;
}

int buscaNoInicio(LDDE *p, void *reg)
{  
    int ret = 0;

    if(p->lista != NULL) { 	
        memcpy(reg, p->lista->dados, p->tamInfo);
        ret = 1;
    }

    return ret;
}

void destroi(LDDE **pp)
{
    
    free(*pp);
    (*pp) = NULL;
}
LDDE *vendedor; 

void *corretor(void *argumentos){
	LDDE *lista = NULL; 
	struct argumentos_struct *args = (struct argumentos_struct *)argumentos;
	char nomeArq[args->tamnomeArq+100];
	//Concatena nome do arquivo com o id do corretor
	snprintf(nomeArq, sizeof(nomeArq), "%s-%d", args->nomeArq, args->qtdthreads);
	FILE *ptr;
	//Abre o arquivo correspondente do corretor
	ptr = fopen(nomeArq,"r");

	//Cria lista de ofertas
	if( cria(&lista, sizeof(info)) == 1) {     
        printf("lista criada\n");
        info *reg, x;
        reg = &x;
        //Percorre o arquivo
        if(ptr){
        	printf("corretor abriu\n");
        	while( (fscanf(ptr,"%s %d\n", reg->nome, &reg->quantidade))!=EOF ){

				//printf("%s %d\n", reg->nome, reg->quantidade);
				//Insere na lista cada oferta
				insereNoFim(lista,reg);
        	}
        
        	fclose(ptr);
        }

        //local de disputa pela variavel thread ocupadas
        threadsOcupadas = threadsOcupadas-1;
        
        //local de verificar ofertas de vendas e realizar a compra

        //Codigo apenas para testes
        NoLDDE *aux;
        while(1){
        	//Se o vendedor não é nulo, então tem alguma coisa a venda
        	if(vendedor->lista!= NULL){
        		//aux aponta para lista do vendedor
        		aux = vendedor->lista;
        		//enquanto ter coisas a venda
        		 while(aux->prox!=NULL) {
            		//copia o que tem dentro de um bloco da lista
            		memcpy(reg, aux->dados, vendedor->tamInfo);
            		//testa se é maior que 0
            		if(reg->quantidade > 0){
            			printf("pode comprar\n");
            			//consome tudo(apenas para testar)
            			reg->quantidade = 0;
            			//depois de atualizar a quantidade copia para a lista
            			memcpy(aux->dados, reg, vendedor->tamInfo);
            		}
            		aux = aux->prox;
        		}
        		
        		
        	}
    	}
      
        
       
	   
   
    	destroi(&lista);
    }else{
    	printf("errro ao criar lista\n");
    }
	

	pthread_exit(NULL);
}
	 

 
int main(int argc, char** argv)
{
	int qtdthreads; //Quantidade de threads
	
	struct argumentos_struct args;
	int i;
	vendedor = NULL;
	


	//Argumentos passados pela linha de comando	 
    qtdthreads = atoi(argv[1]); 
    int tamArq = strlen(argv[2]);
    threadsOcupadas = qtdthreads;
    pthread_t cor[qtdthreads];
	
	for(i=0; i<tamArq; i++){
		
		args.nomeArq[i] = argv[2][i];
		
	}
	args.nomeArq[tamArq]= '\0';
	
	//Inicializa struct
	args.tamnomeArq = tamArq;
	args.qtdthreads = qtdthreads;


	//Cria threads corretores
	for(i = 0; i<qtdthreads; i++){
		args.qtdthreads = qtdthreads-i;
		 pthread_create(&cor[i], NULL, corretor, (void *)&args); 

	}
	char nomeArq[args.tamnomeArq+1]; //Nome do arquivo
	FILE *ptr;
	//Abre o arquivo para vendas
	snprintf(nomeArq, sizeof(nomeArq), "%s", args.nomeArq);
	ptr = fopen(nomeArq,"r");
	if( cria(&vendedor, sizeof(info)) == 1) {     
        printf("lista para vendas criada\n");
        info *reg, x;
        reg = &x;
        //Percorre o arquivo
        if(ptr){
        	printf("arquivo vendas abriu\n");
        	while( (fscanf(ptr,"%s %d\n", reg->nome, &reg->quantidade))!=EOF ){
        		if(reg->nome[0] == '#'){
        			printf("dorme\n");
        		}else{
        			insereNoFim(vendedor,reg);
        			printf("inseriu\n");
        		}
				//printf("%s %d\n", reg->nome, reg->quantidade);
				//Insere na lista cada oferta
				
        	}
        
        	fclose(ptr);
        }
       
	   
   
   
    }else{
    	printf("errro ao criar lista\n");
    }
	for(i = 0; i<qtdthreads; i++){
		
		 pthread_join(cor[i], NULL);

     	 

	}
	destroi(&vendedor);
    return 0;
}