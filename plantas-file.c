//Calculadora de plantinhas do Stardew Valley, usando um arquivo como output. 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define DURACAOESTACAO 28
#define STRINGSIZE 50
#define QNTARQUIVO 4

// Esses enums são a única coisa que me impediu de enlouquecer escrevendo isso. 
// é bom poder escrever vegetal ao invés de 1. 
typedef enum {primavera, verao, outono, difere} estacoes;
typedef enum {vegetal, fruta, outro} tipos;
typedef enum {no_machine, keg, jar} maquinas;
typedef enum {no_product, geleia, picles, suco, vinho} produtos;
typedef enum {false, true} bool;

typedef enum {screen, files, delete, help} systemModes; 



const char *NomeEstacoes[] = {"Primavera", "Verão", "Outono", "Multi-estação"};
const char *NomeTipo[] = {"Vegetal", "Fruta", "Outro"};
const char *NomeMaquina[] = {"Não processável", "Keg", "Preserving Jar"}; 
const char *NomeProdutos[] = {"Não processável", "Geleia", "Picles", "Suco", "Vinho"};
const char *NomeBoolean[] = {"Falso", "Verdadeiro"}; 

const char *NomeMode[] = {"Apenas visualização", "Criação de arquivo", "Remover arquivos", "Ajuda"};

const int VegetableBreakpoint = 220, FruitBreakpoint = 50;

// Função para limpar a tela, independente de sistema operacional
void clearscr() {
    setbuf (stdin, NULL); // Também, como sempre executo ela depois de um scanf, adicionei um setbuf. Isso ajudou muito. 
    #ifdef _WIN32
        system("cls");
    #elif defined(unix) || defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
        system("clear");
    #else
        #error "OS not supported."
    #endif
}

void stringformat(char *string) {
    char *pch = strstr(string, "\n");
    if(pch != NULL) {
        strncpy(pch, "\0", 1);
    }
}

typedef struct {
    float dia;
    int colheita, valor, bruto, liquido, percentual; 
} ganhos;

typedef struct {
    bool multiColheitas, multiEstacao;
} plant_inf; 

typedef struct {
    maquinas maquina;
    produtos produto;
    ganhos lucro;
} artesanal;

typedef struct {
    int colheitaIntervalo, crescimentoTotal;
    float yield;
}   multi_colheitas;

typedef struct {
    // Inserido pelo usuário
    char nome[STRINGSIZE];
    int custoSemente, crescimento, estacoesQnt;
    float yield;
    plant_inf info;
    estacoes estacao;
    tipos tipo; 
    // Atribuído pelo programa
    ganhos lucro;
    int colheitas;
    // Estruturas de dados para alocação dinâmica
    multi_colheitas *multiColheita;
    artesanal *produtoArtesanal;
} plantacoes;

// Isso precisa de otimização. A struct está enorme. Posso dividir ela?
/* 
Minhas funções atuais necessitam de informações dessa struct. Não faz sentido torná-las
mais enxutas pois isso tornaria necessário criar ainda mais funções. 

Poderia também adicionar uma parte de lucro considerando o produto já processado, mas
isso seriam ainda mais variáveis. Pensando em criar mais structs (tal como lucro) pra
deixar tudo ainda mais organizado. Está péssimo como está. 
Desde os estágios iniciais disso, pretendia criar uma segunda struct com informações
referentes a apenas plantas de colheita múltipla, no entanto, como já disse, minhas
funções atuais dependem dessas informações, mesmo que sejam exclusivas de plantas de multipla colheita. 

Portanto, tudo o que fiz foi mandar o programa assumir esses valores durante a execução, tal
como *1 colheita*, *1 estação*. Isso funciona, mas não sei se é o ideal. E não vejo como
alterar isso sem criar mais 7 funções, o que definitivamente não quero fazer. 
*/ 

// Isso é experimental enquanto eu não souber um jeito melhor de printar o nome do ENUM, então isso serve pra copiar o nome pra uma string. 
// A função disso vai ser desagradavelmente longa, com alguns gigantescos switch-cases. 

int qnt_colheitas (int crescimento, int colheitaIntervalo, int estacoesQnt);
float yield_calculator (int colheitas, float yield);
int processados (int valor, tipos tipo, maquinas *maquina, produtos *produto);
int maquina_keg (int valor, tipos tipo); 
int maquina_jar (int valor); 
float lucroCalc(int custoSemente, int crescimento, int colheitas, float yield, ganhos *lucro);
float gold_per_day (int custoSemente, int colheitas, int valor, float yield, int crescimento);

void printPlanta(plantacoes planta);
void printLucro(ganhos lucro, bool multiColheitas, bool sementeValor);
void fprintPlanta(plantacoes planta, FILE *text);
void fprintLucro(ganhos lucro, bool multiColheitas, FILE *text, bool sementeValor);

systemModes modeSelector(char modeInput, int argc);
void atribuidorArquivo (FILE *arquivo[QNTARQUIVO], char path[]);
void removedordeArquivos (char path[]);

int main (int argc, char *input[]) {
    clearscr();
    plantacoes planta;
    systemModes modo;
    char stop, *a;
    int countstop = 0;
    if (argc <= 1) {
        modo = modeSelector('h', argc);
    } else {
        modo = modeSelector(*input[1], argc);
    }
    
    if (argc >= 3) {
        a = input[2];
        countstop = atoi(a);
    }

    char path[] = "/home/user/plantas/files/";
    FILE *arquivos[QNTARQUIVO];

    if (modo == files) {
        atribuidorArquivo(arquivos, path);  
    } else if (modo == delete) {
        removedordeArquivos(path);
        exit(0);
    }

    for (int count = 0; stop != 'n'; count++) {

        if (argc >= 3 && count >= countstop) {
            break;
        }

        setbuf (stdin, NULL);
        printf ("\n\nDigite o nome da planta:  ");
        scanf ("%s", planta.nome);
        stringformat(planta.nome);
        clearscr();

        printf ("Sua planta produz múltiplas colheitas?");
        printf ("\n[1] Não");
        printf ("\n[2] Sim");
        printf ("\nEntrada:  ");
        scanf ("%d", &planta.info.multiColheitas);
        planta.info.multiColheitas--;
        if (planta.info.multiColheitas == true) {
            planta.multiColheita = (multi_colheitas *) malloc (sizeof(multi_colheitas));
        }
        clearscr();

        printf ("Insira o preço da semente:  ");
        scanf("%d", &planta.custoSemente);
        clearscr();
 
        printf ("Insira o preço final da planta:  ");
        scanf("%d", &planta.lucro.valor);
        clearscr();

        printf ("Insira a quantidade de dias de crescimento da planta:  ");
        scanf("%d", &planta.crescimento);
        clearscr();

        // Isso só executa com multi_colheitas true. Else 0 tempo entre colheitas.
        if (planta.info.multiColheitas) {
            printf ("Insira a quantidade de dias entre cada colheita:  ");
            scanf ("%d", &planta.multiColheita->colheitaIntervalo);
            clearscr();
        }

        printf ("Defina quantas plantas são deixadas por colheita:  ");
        scanf ("%f", &planta.yield);
        clearscr();

        printf ("Defina se a planta é um vegetal, uma fruta ou outro:  ");
        printf ("\n[1] Vegetal"); 
        printf ("\n[2] Fruta"); 
        printf ("\n[3] Outro");
        printf ("\nInserido:  ");
        scanf ("%d", &planta.tipo);
        planta.tipo--;
        if (planta.tipo != outro) {
            planta.produtoArtesanal = (artesanal *) malloc (sizeof(artesanal));
        }
        clearscr();

        printf ("Defina se a planta cresce em mais de uma estação:  ");
        printf ("\n[1] Estação única");
        printf ("\n[2] Múltiplas estações");
        printf ("\nInput:  ");
        scanf ("%d", &planta.info.multiEstacao);
        planta.info.multiEstacao--;
        clearscr();

        /* Define a estação. Ainda pensando se implemento algo pra que a pessoa
        possa inserir as estações da planta. Quem sabe eu uso um true/false pra cada estação. 
        Como isso é grande, o ideal seria criar um struct pra isso e só gerar ele nesse if. 
        Talvez eu implemente. */
        if (planta.info.multiEstacao) {
            printf ("Insira a quantidade de estações em que a planta cresce:  ");
            scanf ("%d", &planta.estacoesQnt);
            planta.estacao = difere;
        } else {
            planta.estacoesQnt = 1;
            printf ("Defina a estação do ano da planta:  ");
            printf ("\n[1] Primavera"); 
            printf ("\n[2] Verão"); 
            printf ("\n[3] Outono");
            printf ("\nEntrada:  ");
            scanf ("%d", &planta. estacao);
            planta.estacao--;
        }
        clearscr();

        // Teoricamente eu até posso executar a função pq o tempo entre colheitas seria 0 no else. 
        // Mas deixa esse if aqui. Mais seguro. 
        if (planta.info.multiColheitas) {
            planta.colheitas = qnt_colheitas(planta.crescimento, planta.multiColheita->colheitaIntervalo, planta.estacoesQnt);
            planta.multiColheita->crescimentoTotal = ((planta.colheitas-1)*planta.multiColheita->colheitaIntervalo) + planta.crescimento;
            planta.multiColheita->yield = yield_calculator(planta.colheitas, planta.yield);
            planta.lucro.dia = lucroCalc(planta.custoSemente, planta.multiColheita->crescimentoTotal, planta.colheitas, planta.yield, &planta.lucro);
        } else {
            planta.colheitas = 1;
            planta.lucro.dia = lucroCalc(planta.custoSemente, planta.crescimento, planta.colheitas, planta.yield, &planta.lucro);
        }
    
        if (planta.tipo != outro) {
            planta.produtoArtesanal->lucro.valor = processados(planta.lucro.valor, planta.tipo, &planta.produtoArtesanal->maquina, &planta.produtoArtesanal->produto);
            if (planta.info.multiColheitas) {
                planta.produtoArtesanal->lucro.dia = lucroCalc(planta.custoSemente, planta.multiColheita->crescimentoTotal, planta.colheitas, planta.yield, &planta.produtoArtesanal->lucro);
            } else {
                planta.produtoArtesanal->lucro.dia = lucroCalc(planta.custoSemente, planta.crescimento, planta.colheitas, planta.yield, &planta.produtoArtesanal->lucro);
            }
        }
        
        // printf ("\n\tNome:  %s\n\tTipo:  %s\n\tEstação:  %s\n\tMultiestação:  %s\n\tQuantidade de estações:  %d\n\tcustoSemente da semente:  %d\n\tValor:  %d\n\tPossui múltiplas colheitas:  %s\n\tColheitas:  %d\n\tYield:  %.2f\n\tYield total:  %.2f\n\tTempo de crescimento:  %d\n\tDias totais de crescimento:  %d\n\tDias de crescimento entre cada colheita:  %d\n\tMáquina a ser usada:  %s\n\tProduto resultante:  %s\n\tcustoSemente após processada:  %d\n\tGanho por colheita:  %d\n\tGanho líquido:  %d\n\tGanho bruto:  %d\n\tDinheiro por dia:  %.2f\n\tDinheiro por dia processada:  %.2f", planta.nome, NomeTipo[planta.tipo], NomeEstacoes[planta.estacao], NomeBoolean[planta.info.multiEstacao], planta.estacoesQnt, planta.custoSemente, planta.lucro.valor, NomeBoolean[planta.info.multiColheitas], planta.colheitas, planta.yield, planta.multiColheita->yield, planta.crescimento, planta.multiColheita->crescimentoTotal, planta.multiColheita->colheitaIntervalo, NomeMaquina[planta.produtoArtesanal->maquina], NomeProdutos[planta.produto], planta.produtoArtesanal->lucro.valor, planta.lucro.colheita, planta.lucro.liquido, planta.lucro.bruto, planta.lucro.dia, planta.produtoArtesanal->lucro.dia);
        printPlanta(planta);

        if (modo == files) {
            fprintPlanta(planta, arquivos[planta.estacao]);
            for (int count = QNTARQUIVO; count < QNTARQUIVO; count++) {
                fclose(arquivos[count]);
            }
        }

        if (planta.tipo != outro) {
            free (planta.produtoArtesanal);
        }

        if(planta.info.multiColheitas) {
            free (planta.multiColheita);
        }

        if (argc >= 3) {
            continue;
        }

        // só pra garantir o loop. me pergunto como eu poderia fazer pro loop ser um true/false
        // definido pelo usuário.
        printf ("\n\nDeseja inserir uma nova informação? [y/n]");
        printf ("\nEntrada:  ");
        stop = tolower(getchar());

        clearscr();
    }
    printf ("\nAll is well.");
    return 0; 
}

systemModes modeSelector(char modeInput, int argc) {
    printf ("Modo selecionado pelo usuário:  ");
    switch(modeInput) {
        case 'p':
            printf("%s", NomeMode[screen]);
            return screen; 

        case 'f':
            printf("%s", NomeMode[files]); 
            return files;     

        case 'r':
            printf("%s", NomeMode[delete]);
            return delete;

        default:
            printf("Tela de ajuda");
            printf("\nModos:  \n\t%s [p]: Somente exibe as informações na tela\n\t%s [f]:  Cria arquivos organizados com cada entrada\n\t%s [r]:  Deleta os arquivos do programa\n\t%s [h]: Exibe essa tela de ajuda", NomeMode[screen], NomeMode[files], NomeMode[delete], NomeMode[help]);
            if (argc < 3) {
                printf ("\n\nDica:\n\tSe você incluir um número como segundo argumento, pode definir a quantidade de execuções do programa.");
            }
            exit (0);
    }
}


// Calcula quantas colheitas a planta permite. Só é executada em multi_colheita true.
int qnt_colheitas (int crescimento, int colheitaIntervalo, int estacoesQnt) {
    int colheita = 1, dias_de_colheita = DURACAOESTACAO*estacoesQnt; 
    for (dias_de_colheita -= crescimento; dias_de_colheita >= colheitaIntervalo; dias_de_colheita -= colheitaIntervalo) {
        colheita++;
    }
    return colheita;
    /* tenho duas ideias pra lidar com esse problema. 
    Usar um for e ir subtraindo como eu fiz, ou subtrair o tempo de crescimento e dps
    dividir. Meu problema com dividir é que eu não teria como garantir que o programa
    arredonde pra baixo, mas não tenho certeza se tem como fazer isso. */
}

// Calcula a produção total da planta. Sempre executada já que atribuo colheita a 1 se for o caso.
float yield_calculator (int colheitas, float yield) {
    return colheitas*yield;
    // isso nem precisava ser uma função
    // ao menos eu poderia botar isso num if se quisesse. tanto faz. 
}

// Atribui as informações referentes ao processamento da planta e chama as funções de cálculo corretas. 
int processados(int valor, tipos tipo, maquinas *maquina, produtos *produto) {
    // Informações úteis: 
    /*
    Vegetais tem que se tornar suco se valerem mais de 220
    Frutas tem que se tornar vinho se valerem mais de 50
    Caso contrário, viram respectivamente picles e geleia.

    Outra forma de abordar esses ifs seria mandar executar as funções do keg e da jar
    duas vezes, e comparar os valores. Mas usar esses números é mais fácil. 
    E, estou usando constantes pra isso. por que sim.  
    */
    int planta_processada;
    if (tipo == vegetal) {
        if (valor > VegetableBreakpoint) {
            planta_processada = maquina_keg(valor, tipo);
            *maquina = keg;
            *produto = suco;
        } else {
            planta_processada = maquina_jar(valor);
            *maquina = jar;
            *produto = picles;
        }
    } else if (tipo == fruta) {
        if (valor > FruitBreakpoint) {
            planta_processada = maquina_keg(valor, tipo);
            *maquina = keg;
            *produto = vinho;
        } else {
            planta_processada = maquina_jar(valor);
            *maquina = jar;
            *produto = geleia;
        }
    } else {
        planta_processada = 0;
    }
    return planta_processada;
    // por mais que pareça estranha, essa função executa perfeitamente. 
    // posso manter assim.
}

void atribuidorArquivo (FILE *arquivo[QNTARQUIVO], char path[]) {
    char temppath[STRINGSIZE];
    for (int count = 0; count < QNTARQUIVO; count++) {
        strcpy(temppath, path);
        strcat(temppath, NomeEstacoes[count]);
        strcat(temppath, ".txt");
        arquivo[count] = fopen (temppath, "w"); 
    }
}

void removedordeArquivos (char path[]) {
    char temppath[STRINGSIZE];
    for (int count = 0; count < QNTARQUIVO; count++) {
        strcpy(temppath, path);
        strcat(temppath, NomeEstacoes[count]);
        strcat(temppath, ".txt");
        remove(temppath);
    }
}

// Calcula o preço de suco ou vinho
int maquina_keg(int valor, tipos tipo) {
    int preco_processada;
    if (tipo == fruta) {
        preco_processada = valor*3;
    } else if (tipo == vegetal) {
        preco_processada = valor*2.25;
    }
    return preco_processada;
}

// Calcula o preço de geleia ou picles
int maquina_jar(int valor) {
    return ((valor*2)+50);
}

// Calcula todo o lucro final da planta. Como é uma função, poderia tbm criar uma struct
// "lucro" e usar ela duas vezes pra atribuir os lucros da planta e do produto artesanal.
float lucroCalc(int custoSemente, int crescimento, int colheitas, float yield, ganhos *lucro) {
    lucro->colheita = lucro->valor*yield; 
    lucro->bruto = (lucro->colheita)*colheitas;
    if (custoSemente > 0) {
        lucro->liquido = ((lucro->colheita)*colheitas) - custoSemente;
        lucro->percentual = (lucro->bruto*100)/(custoSemente*2);
    }
    return ((colheitas*(lucro->valor*yield)) - custoSemente) / crescimento; 
}

void printPlanta(plantacoes planta) {
    bool boolSemente = planta.custoSemente > 0;

    printf ("%s", planta.nome);
    printf ("\n\t-> Informações:  ");
    printf ("\n\t\tTipo da planta:  %s\n\t\tCusto da semente:  %d\n\t\tTempo de crescimento:  %d dias\n\t\tMúltiplas colheitas:  %s\n\t\tYield:  %.2f\n\t\tMultiestações:  %s\n\t\tEstação:  %s", NomeTipo[planta.tipo], planta.custoSemente, planta.crescimento, NomeBoolean[planta.info.multiColheitas], planta.yield, NomeBoolean[planta.info.multiEstacao], NomeEstacoes[planta.estacao]);
    if (planta.estacao == difere) {
        printf ("\n\t\tQuantidade de estações:  %d", planta.estacoesQnt);
    }
    if (planta.info.multiColheitas) {
        printf("\n\n\t-> Colheitas:  ");
        printf("\n\t\tQuantidade de colheitas:  %d\n\t\tTempo entre cada colheita:  %d dias\n\t\tYield total da planta:  %.2f\n\t\tTempo de vida total da planta:  %d dias", planta.colheitas, planta.multiColheita->colheitaIntervalo, planta.multiColheita->yield, planta.multiColheita->crescimentoTotal);
    }
    if (planta.tipo != outro) {
        printf ("\n\n\t-> Informações do produto artesanal:  ");
        printf ("\n\t\tValor do produto:  %d\n\t\tTipo do produto:  %s\n\t\tMáquina:  %s", planta.produtoArtesanal->lucro.valor, NomeProdutos[planta.produtoArtesanal->produto], NomeMaquina[planta.produtoArtesanal->maquina]);
    }
    printf ("\n\n\t-> Lucro da planta:  ");
    printLucro(planta.lucro, planta.info.multiColheitas, boolSemente);
    if (planta.tipo != outro) {
        printf ("\n\n\t-> Lucro do produto artesanal:  ");
        printLucro(planta.produtoArtesanal->lucro, planta.info.multiColheitas, boolSemente);
    }
    printf ("\n\n\n");
}

void printLucro(ganhos lucro, bool multiColheitas, bool sementeValor) {
    printf ("\n\t\tValor:  %d", lucro.valor);
    if (multiColheitas) {
        printf ("\n\t\tGanho por colheita:  %d", lucro.colheita);
    }
    printf ("\n\t\tGanho por dia:  %.2f\n\t\tGanho total bruto:  %d", lucro.dia, lucro.bruto);
    if (sementeValor) {
        printf ("\n\t\tGanho total líquido:  %d\n\t\tPercentual de ganho:  %d%%", lucro.liquido, lucro.percentual);
    }
}

void fprintPlanta(plantacoes planta, FILE *text) {
    bool boolSemente = planta.custoSemente > 0;

    fprintf (text, "%s", planta.nome);
    fprintf (text, "\n\t-> Informações:  ");
    fprintf (text, "\n\t\tTipo da planta:  %s\n\t\tcustoSemente da semente:  %d\n\t\tTempo de crescimento:  %d dias\n\t\tMúltiplas colheitas:  %s\n\t\tYield:  %.2f\n\t\tMultiestações:  %s\n\t\tEstação:  %s", NomeTipo[planta.tipo], planta.custoSemente, planta.crescimento, NomeBoolean[planta.info.multiColheitas], planta.yield, NomeBoolean[planta.info.multiEstacao], NomeEstacoes[planta.estacao]);
    if (planta.estacao == difere) {
        fprintf (text, "\n\t\tQuantidade de estações:  %d", planta.estacoesQnt);
    }
    if (planta.info.multiColheitas) {
        fprintf(text, "\n\n\t-> Colheitas:  ");
        fprintf(text, "\n\t\tQuantidade de colheitas:  %d\n\t\tTempo entre cada colheita:  %d dias\n\t\tYield total da planta:  %.2f\n\t\tTempo de vida total da planta:  %d dias", planta.colheitas, planta.multiColheita->colheitaIntervalo, planta.multiColheita->yield, planta.multiColheita->crescimentoTotal);
    }
    if (planta.tipo != outro) {
        fprintf (text, "\n\n\t-> Informações do produto artesanal:  ");
        fprintf (text, "\n\t\tValor do produto:  %d\n\t\tTipo do produto:  %s\n\t\tMáquina:  %s", planta.produtoArtesanal->lucro.valor, NomeProdutos[planta.produtoArtesanal->produto], NomeMaquina[planta.produtoArtesanal->maquina]);
    }
    fprintf (text, "\n\n\t-> Lucro da planta:  ");
    fprintLucro(planta.lucro, planta.info.multiColheitas, text, boolSemente);
    if (planta.tipo != outro) {
        fprintf (text, "\n\n\t-> Lucro do produto artesanal:  ");
        fprintLucro(planta.produtoArtesanal->lucro, planta.info.multiColheitas, text, boolSemente);
    }
    fprintf (text, "\n\n\n");
}

void fprintLucro(ganhos lucro, bool multiColheitas, FILE *text, bool sementeValor) {
    fprintf (text, "\n\t\tValor:  %d", lucro.valor);
    if (multiColheitas) {
        fprintf (text, "\n\t\tGanho por colheita:  %d", lucro.colheita);
    }
    fprintf (text, "\n\t\tGanho por dia:  %.2f\n\t\tGanho total bruto:  %d", lucro.dia, lucro.bruto);
    if (sementeValor) {
        fprintf(text, "\n\t\tGanho total líquido:  %d\n\t\tPercentual de ganho:  %d%%", lucro.liquido, lucro.percentual);
    }
}