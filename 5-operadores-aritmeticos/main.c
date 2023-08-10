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

/*Essa macro procura se a referencia para um operador existe e retorna seu ponteiro*/
#define buscarReferencia(e) ({\
    Celula *ref=NULL;\
    for(int it=0;it<N_REFS;it++) {\
        if(refs[it]->ch==e){\
            ref=refs[it];\
            break;\
        }\
    }\
    ref;\
})

#define inserirElemento(G, e) ({\
    Celula *ne=NULL, *ng=G;\
    ne=buscarReferencia(e);\
    if(ne==NULL) {\
        ne=alloc( );\
        ne->esq=ne->dir=NULL;\
        ne->ch=e;\
    }\
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

#define aplicarReducaoTrue(A) ({\
    A->raiz->esq=A->a;\
})

#define aplicarReducaoFalse(A) ({\
    A->raiz->esq=A->b;\
})

#define aplicarReducaoMaiorQue(A) ({\
    int a=A->a->ch-'0';\
    int b=A->b->ch-'0';\
    if (a > b) {\
        A->raiz->esq=buscarReferencia('T');\
    } else {\
        A->raiz->esq=buscarReferencia('F');\
    }\
})

#define aplicarReducaoMenorQue(A) ({\
    int a = A->a->ch - '0';\
    int b = A->b->ch - '0';\
    if (a < b) {\
        A->raiz->esq=buscarReferencia('T');\
    } else {\
        A->raiz->esq=buscarReferencia('F');\
    }\
})

#define aplicarReducaoIgual(A) ({\
    int a = A->a->ch - '0';\
    int b = A->b->ch - '0';\
    if (a == b) {\
        A->raiz->esq=buscarReferencia('T');\
    } else {\
        A->raiz->esq=buscarReferencia('F');\
    }\
})

#define aplicarReducaoSoma(A) ({\
    int a = A->a->ch - '0';\
    int b = A->b->ch - '0';\
    Celula *ns=NULL;\
    ns=alloc();\
    ns->esq=ns->dir=NULL;\
    ns->ch= (a + b) + '0';\
    A->raiz->esq=ns;\
})

#define aplicarReducaoSubtracao(A) ({\
    int a = A->a->ch - '0';\
    int b = A->b->ch - '0';\
    Celula *ns=NULL;\
    ns=alloc();\
    ns->esq=ns->dir=NULL;\
    ns->ch= (a - b) + '0';\
    A->raiz->esq=ns;\
})

#define aplicarReducaoMultiplicacao(A) ({\
    int a = A->a->ch - '0';\
    int b = A->b->ch - '0';\
    Celula *ns=NULL;\
    ns=alloc();\
    ns->esq=ns->dir=NULL;\
    ns->ch= (a * b) + '0';\
    A->raiz->esq=ns;\
})

#define aplicarReducaoDivisao(A) ({\
    int a = A->a->ch - '0';\
    int b = A->b->ch - '0';\
    Celula *ns=NULL;\
    ns=alloc();\
    ns->esq=ns->dir=NULL;\
    ns->ch= (a / b) + '0';\
    A->raiz->esq=ns;\
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
int N_REFS = 17;
Celula* refs[17];
/*-------------------------------------*/



Celula* converterVetorGrafo(char* entrada, unsigned* i, unsigned tam, Celula* G){
    Celula *Gn;
    while((*i)<tam){
        printf("%c", entrada[*i]);
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
    unsigned chamadas, reducoesS, reducoesK, reducoesI, reducoesB, reducoesC, reducoesSl, reducoesBl, reducoesCl, reducoesT, reducoesF, reducoesMaiorQue, reducoesMenorQue, reducoesIgual, reducoesSoma, reducoesSubtracao, reducoesMultiplicacao, reducoesDivisao;

    A=(Args*)calloc(1, sizeof(Args));
    if(A==NULL) {
        printf("Erro na alocacao dinamica!\n");
        exit(1);
    }
    chamadas=0;
    reducoesS=reducoesK=reducoesI=reducoesB=reducoesC=reducoesSl=reducoesBl=reducoesCl=reducoesT=reducoesF=reducoesMaiorQue=reducoesMenorQue=reducoesIgual=reducoesSoma=reducoesSubtracao=reducoesMultiplicacao=reducoesDivisao=0;
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
            case 'T':
                A->numeroArgumentos=2;
                localizarArgumentos(G, A);
                if(A->b!=NULL){
                    aplicarReducaoTrue(A);
                    reducoesT++;
                    reduziu=1;
                }
                break;
            case 'F':
                A->numeroArgumentos=2;
                localizarArgumentos(G, A);
                if(A->b!=NULL){
                    aplicarReducaoFalse(A);
                    reducoesF++;
                    reduziu=1;
                }
                break;
            case '>':
                A->numeroArgumentos=2;
                localizarArgumentos(G, A);
                if(A->b!=NULL){
                    aplicarReducaoMaiorQue(A);
                    reducoesMaiorQue++;
                    reduziu=1;
                }
                break;
            case '<':
                A->numeroArgumentos=2;
                localizarArgumentos(G, A);
                if(A->b!=NULL){
                    aplicarReducaoMenorQue(A);
                    reducoesMenorQue++;
                    reduziu=1;
                }
                break;
            case '=':
                A->numeroArgumentos=2;
                localizarArgumentos(G, A);
                if(A->b!=NULL){
                    aplicarReducaoIgual(A);
                    reducoesIgual++;
                    reduziu=1;
                }
                break;
            case '+':
                A->numeroArgumentos=2;
                localizarArgumentos(G, A);
                if(A->b!=NULL){
                    aplicarReducaoSoma(A);
                    reducoesSoma++;
                    reduziu=1;
                }
                break;
            case '-':
                A->numeroArgumentos=2;
                localizarArgumentos(G, A);
                if(A->b!=NULL){
                    aplicarReducaoSubtracao(A);
                    reducoesSubtracao++;
                    reduziu=1;
                }
                break;
            case '*':
                A->numeroArgumentos=2;
                localizarArgumentos(G, A);
                if(A->b!=NULL){
                    aplicarReducaoMultiplicacao(A);
                    reducoesMultiplicacao++;
                    reduziu=1;
                }
                break;
            case '/':
                A->numeroArgumentos=2;
                localizarArgumentos(G, A);
                if(A->b!=NULL){
                    aplicarReducaoDivisao(A);
                    reducoesDivisao++;
                    reduziu=1;
                }
                break;

        }
    }while(reduziu==1);               /*Enquanto reducoes forem realizadas, continue a executar todo o laco.*/
    printf("ESTATISTICAS DE EXECUCAO NA MAQUINA ABSTRATA (MAQUINA DE REDUCAO DE GRAFOS):\n   -CHAMADAS AO AVALIADOR: %u.\n   -REDUCOES:\n\t   S:  %u\n\t   K:  %u\n\t   I:  %u\n\t   B:  %u\n\t   C:  %u\n\t   S': %u\n\t   B': %u\n\t   C': %u\n\t   T': %u\n\t   F': %u\n\t   >': %u\n\t   <': %u\n\t   =': %u\n\t   +': %u\n\t   -': %u\n\t   *': %u\n\t   /': %u\n", chamadas, reducoesS, reducoesK, reducoesI, reducoesB, reducoesC, reducoesSl, reducoesBl, reducoesCl, reducoesT, reducoesF, reducoesMaiorQue, reducoesMenorQue, reducoesIgual, reducoesSoma, reducoesSubtracao, reducoesMultiplicacao, reducoesDivisao);
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

    char entrada[100000] = "/63";
    // char entrada[100000];
    // readFileToString("../string_turner.txt", entrada);
    /*--------------------INICIALIZAÇÃO DAS REFERENCIAS----------*/

    /*--------------------INICIALIZACAO DO GRAFO E DA HEAP--------------------*/
    H.ind=0;            /*Inicializa o contador do heap.*/
    /*--------------------REGISTRO DOS OPERADORES--------------------*/
    Celula* Sref=alloc();
    Sref->esq=Sref->dir=NULL;
    Sref->ch='S';
    Celula* Kref=alloc();
    Kref->esq=Kref->dir=NULL;
    Kref->ch='K';
    Celula* Iref=alloc();
    Iref->esq=Iref->dir=NULL;
    Iref->ch='I';
    Celula* Bref=alloc();
    Bref->esq=Bref->dir=NULL;
    Bref->ch='B';
    Celula* Cref=alloc();
    Cref->esq=Cref->dir=NULL;
    Cref->ch='C';
    Celula* sref=alloc();
    sref->esq=sref->dir=NULL;
    sref->ch='s';
    Celula* bref=alloc();
    bref->esq=bref->dir=NULL;
    bref->ch='b';
    Celula* cref=alloc();
    cref->esq=cref->dir=NULL;
    cref->ch='c';
    Celula* Tref=alloc();
    Tref->esq=Tref->dir=NULL;
    Tref->ch='T';
    Celula* Fref=alloc();
    Fref->esq=Fref->dir=NULL;
    Fref->ch='F';
    Celula* mtref=alloc();
    mtref->esq=mtref->dir=NULL;
    mtref->ch='>';
    Celula* ltref=alloc();
    ltref->esq=ltref->dir=NULL;
    ltref->ch='<';
    Celula* eqref=alloc();
    eqref->esq=eqref->dir=NULL;
    eqref->ch='=';
    Celula* somaref=alloc();
    somaref->esq=somaref->dir=NULL;
    somaref->ch='+';
    Celula* subtracaoref=alloc();
    subtracaoref->esq=subtracaoref->dir=NULL;
    subtracaoref->ch='-';
    Celula* multiplicacaoref=alloc();
    multiplicacaoref->esq=multiplicacaoref->dir=NULL;
    multiplicacaoref->ch='*';
    Celula* divisaoref=alloc();
    divisaoref->esq=divisaoref->dir=NULL;
    divisaoref->ch='/';


    refs[0] = Sref;
    refs[1] = Kref;
    refs[2] = Iref;
    refs[3] = Bref;
    refs[4] = Cref;
    refs[5] = sref;
    refs[6] = bref;
    refs[7] = cref;
    refs[8] = Tref;
    refs[9] = Fref;
    refs[10] = mtref;
    refs[11] = ltref;
    refs[12] = eqref;
    refs[13] = somaref;
    refs[14] = subtracaoref;
    refs[15] = multiplicacaoref;
    refs[16] = divisaoref;

    /*-------------------REGISTRO DE TEMPO E INICIALIZAÇÃO--------------*/
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
