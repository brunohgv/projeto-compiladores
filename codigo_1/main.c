#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../util.h"

/*Essa macro "aloca" uma celula, retornando o seu endereco.*/
#define alloc( ) ({\
    if(H.ind==100000){\
        printf("Heap cheia! Executar garbage collection\n");\
        exit(1);\
    }\
    &H.heap[H.ind++];\
})

/*Essa macro gera uma celula '@'.*/
#define gerarRaiz( ) ({\
    Celula *Gn=NULL;\
    Gn=alloc( );\
    Gn->ch='@';\
    Gn->esq=Gn->dir=NULL;\
    Gn;\
})

/*Essa macro concatena dois grafos, criando uma nova celula '@', se necessario.*/
#define concatenarGrafos(Ga, Gb) ({\
    Celula *Gf=Ga;\
    if(Ga->esq==NULL) Ga->esq=Gb;\
    else if (Ga->dir==NULL) Ga->dir=Gb;\
    else{\
        Gf=alloc( );\
        Gf->esq=Ga;\
        Gf->dir=Gb;\
        Gf->ch='@';\
    }\
    Gf;\
})

#define inserirElemento(G, e) ({\
    Celula *ne=NULL, *ng=G;\
    ne=alloc( );\
    ne->esq=ne->dir=NULL;\
    ne->ch=e;\
    if(G->esq==NULL) G->esq=ne;\
    else if (G->dir==NULL) G->dir=ne;\
    else{\
        ng=alloc( );\
        ng->ch='@';\
        ng->esq=G;\
        ng->dir=ne;\
    }\
    ng;\
})

#define localizarOperador(G, A) ({\
    Celula *aux=G;\
    while(aux->esq!=NULL) aux=aux->esq;\
    A->operador=aux;\
})

#define localizarArgumentos(G, A) ({\
    A->a=A->raiz=G;\
    while(A->a->esq!=NULL && A->a->esq!=A->operador){\
        if(A->b==NULL){A->b=A->a;}\
        else if(A->c==NULL){A->c=A->b; A->b=A->a;}\
        else {A->d=A->c; A->c=A->b; A->b=A->a;}\
        A->a=A->a->esq;\
    }\
    if(A->numeroArgumentos==1 && A->b!=NULL) {A->raiz=A->b;}\
    else if(A->numeroArgumentos<=3 && A->c!=NULL) {A->raiz=A->c;}\
    else if(A->numeroArgumentos==4 && A->d!=NULL) {A->raiz=A->d;}\
    if(A->a!=NULL) A->a=A->a->dir;\
    if(A->b!=NULL) A->b=A->b->dir;\
    if(A->c!=NULL) A->c=A->c->dir;\
    if(A->d!=NULL) A->d=A->d->dir;\
})

#define aplicarReducaoK(A) ({\
    A->raiz->esq=A->a;\
})

#define aplicarReducaoI(A) ({\
    A->raiz->esq=A->a;\
})

#define aplicarReducaoS(A) ({\
    Celula *ns=NULL;\
    ns=alloc( );\
    ns->ch='@';\
    ns->esq=A->a;\
    ns->dir=A->c;\
    A->raiz->esq=ns;\
    ns=alloc( );\
    ns->ch='@';\
    ns->esq=A->b;\
    ns->dir=A->c;\
    A->raiz->dir=ns;\
})

#define aplicarReducaoB(A) ({\
    Celula *ns=NULL;\
    ns=alloc( );\
    ns->ch='@';\
    ns->esq=A->b;\
    ns->dir=A->c;\
    A->raiz->esq=A->a;\
    A->raiz->dir=ns;\
})

#define aplicarReducaoC(A) ({\
    Celula *ns=NULL;\
    ns=alloc( );\
    ns->ch='@';\
    ns->esq=A->a;\
    ns->dir=A->c;\
    A->raiz->esq=ns;\
    A->raiz->dir=A->b;\
})

#define aplicarReducaoSl(A) ({\
    Celula *ns=NULL;\
    ns=alloc( );\
    ns->ch='@';\
    ns->esq=A->a;\
    ns->dir=NULL;\
    A->raiz->esq=ns;\
    ns=alloc( );\
    ns->ch='@';\
    ns->esq=A->b;\
    ns->dir=A->d;\
    A->raiz->esq->dir=ns;\
    ns=alloc( );\
    ns->ch='@';\
    ns->esq=A->c;\
    ns->dir=A->d;\
    A->raiz->dir=ns;\
})

#define aplicarReducaoBl(A) ({\
    Celula *ns=NULL;\
    ns=alloc( );\
    ns->ch='@';\
    ns->esq=A->a;\
    ns->dir=A->b;\
    A->raiz->esq=ns;\
    ns=alloc( );\
    ns->ch='@';\
    ns->esq=A->c;\
    ns->dir=A->d;\
    A->raiz->dir=ns;\
})

#define aplicarReducaoCl(A) ({\
    Celula *ns=NULL;\
    ns=alloc( );\
    ns->ch='@';\
    ns->esq=A->a;\
    ns->dir=NULL;\
    A->raiz->esq=ns;\
    ns=alloc( );\
    ns->ch='@';\
    ns->esq=A->b;\
    ns->dir=A->d;\
    A->raiz->esq->dir=ns;\
    A->raiz->dir=A->c;\
})



typedef struct celula{
    char ch;
    struct celula *esq, *dir;
} Celula;



typedef struct argumentos{
    unsigned numeroArgumentos;
    Celula *operador, *a, *b, *c, *d, *raiz;
} Args;



typedef struct heap{
    unsigned ind;
    Celula heap[100000];
} Heap;



/*----------VARIAVEIS GLOBAIS----------*/
Heap H;
/*-------------------------------------*/



Celula* converterVetorGrafo(char* entrada, unsigned* i, unsigned tam, Celula* G){
    Celula *Gn;
    while((*i)<tam){
        if(entrada[*i]=='(') {
            (*i)++;
            Gn=converterVetorGrafo(entrada, i, tam, gerarRaiz( ));
            G=concatenarGrafos(G, Gn);
        } else if(entrada[*i]==')') {
            (*i)++;
            break;
        } else {
            G=inserirElemento(G, entrada[*i]);
            (*i)++;
        }
    }
    return G;
}



Celula* avaliarExpressao(Celula *G){
    Args *A=NULL;
    char reduziu;
    unsigned chamadas, reducoesS, reducoesK, reducoesI, reducoesB, reducoesC, reducoesSl, reducoesBl, reducoesCl;

    A=(Args*)calloc(1, sizeof(Args));
    if(A==NULL) {
        printf("Erro na alocacao dinamica!\n");
        exit(1);
    }
    chamadas=0;
    reducoesS=reducoesK=reducoesI=reducoesB=reducoesC=reducoesSl=reducoesBl=reducoesCl=0;
    do{
        A->numeroArgumentos=reduziu=0;
        A->operador=A->a=A->b=A->c=A->d=A->raiz=NULL;
        localizarOperador(G, A);
        chamadas++;                         /*Incrementa o numero de chamadas ao avaliador de expressoes.*/
        switch(A->operador->ch){
            case 'S':
                A->numeroArgumentos=3;      /*Especifica o numero de argumentos necessarios.*/
                localizarArgumentos(G, A);  /*Localiza os ponteiros dos argumentos.*/
                if(A->c!=NULL){             /*Se o ultimo argumento nao foi encontrado, a reducao nao sera realizada.*/
                    aplicarReducaoS(A);     /*Aplica a reducao de grafo.*/
                    reducoesS++;            /*Incrementa o contador de reducoes.*/
                    reduziu=1;              /*Indica que uma reducao foi realizada.*/
                }
                break;
            case 'K':
                A->numeroArgumentos=2;
                localizarArgumentos(G, A);
                if(A->b!=NULL){
                    aplicarReducaoK(A);
                    reducoesK++;
                    reduziu=1;
                }
                break;
            case 'I':
                A->numeroArgumentos=1;
                localizarArgumentos(G, A);
                if(A->a!=NULL){
                    aplicarReducaoI(A);
                    reducoesI++;
                    reduziu=1;
                }
                break;
            case 'B':
                A->numeroArgumentos=3;
                localizarArgumentos(G, A);
                if(A->c!=NULL){
                    aplicarReducaoB(A);
                    reducoesB++;
                    reduziu=1;
                }
                break;
            case 'C':
                A->numeroArgumentos=3;
                localizarArgumentos(G, A);
                if(A->c!=NULL){
                    aplicarReducaoC(A);
                    reducoesC++;
                    reduziu=1;
                }
                break;
            case 's':
                A->numeroArgumentos=4;
                localizarArgumentos(G, A);
                if(A->d!=NULL){
                    aplicarReducaoSl(A);
                    reducoesSl++;
                    reduziu=1;
                }
                break;
            case 'b':
                A->numeroArgumentos=4;
                localizarArgumentos(G, A);
                if(A->d!=NULL){
                    aplicarReducaoBl(A);
                    reducoesBl++;
                    reduziu=1;
                }
                break;
            case 'c':
                A->numeroArgumentos=4;
                localizarArgumentos(G, A);
                if(A->d!=NULL){
                    aplicarReducaoCl(A);
                    reducoesCl++;
                    reduziu=1;
                }
                break;
        }
    }while(reduziu==1);               /*Enquanto reducoes forem realizadas, continue a executar todo o laco.*/
    printf("ESTATISTICAS DE EXECUCAO NA MAQUINA ABSTRATA (MAQUINA DE REDUCAO DE GRAFOS):\n   -CHAMADAS AO AVALIADOR: %u.\n   -REDUCOES:\n\t   S:  %u\n\t   K:  %u\n\t   I:  %u\n\t   B:  %u\n\t   C:  %u\n\t   S': %u\n\t   B': %u\n\t   C': %u\n", chamadas, reducoesS, reducoesK, reducoesI, reducoesB, reducoesC, reducoesSl, reducoesBl, reducoesCl);
    printf("   -NUMERO DE CELULAS CONSUMIDAS: %u\n", H.ind);
    free(A);
}



void imprimirGrafo(Celula* G){
    if(G->esq!=NULL) imprimirGrafo(G->esq);
    if(G->dir!=NULL) imprimirGrafo(G->dir);
    printf("%c", G->ch);
}



int main( ){
    unsigned i=0;
    Celula *G=NULL;

    char entrada[100000];
    readFileToString("../string_ks.txt", entrada);
    /*--------------------INICIALIZACAO DO GRAFO E DA HEAP--------------------*/
    H.ind=0;            /*Inicializa o contador do heap.*/
    clock_t tempoInicio, tempoFim;
    double tempoDeCpu;
    tempoInicio = clock(); /*captura tempo de início da execução.*/
    G=gerarRaiz( );
    G->ch='.';          /*O ponto ('.') representa o inicio do grafo. Entao ele so apresenta elementos a sua esquerda.*/
    G->esq=converterVetorGrafo(entrada, &i, strlen(entrada), gerarRaiz( ));
    /*------------------------------------------------------------------------*/
    avaliarExpressao(G);
    printf("   -GRAFO FINAL: ");
    imprimirGrafo(G->esq);
    printf("\n");
    tempoFim = clock(); /*captura tempo final da execução.*/
    tempoDeCpu = ((double)(tempoFim - tempoInicio)) / CLOCKS_PER_SEC;
    printf("   -TEMPO DE EXECUCAO: %f", tempoDeCpu);
    printf("\n");
    return 0;
}
