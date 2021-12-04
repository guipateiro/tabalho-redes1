#ifndef __PROTOCOLO__
#define __PROTOCOLO__

#define MARCA_INICIO 126  // marcador de inicio
#define CLIENT 1        // endereço cliente
#define SERVER 2        // endereço servidor
#define TAM_PACKAGE 19  // tamanho máximo do pacote, em bytes
#define TAM_DATA 15     // tamanho máximo do campo de dados, em bytes
#define TIMEOUT 3       // timeout em segundos

#define CD 0
#define LS 1
#define VER 2
#define LINHA 3
#define LINHAS 4
#define EDIT 5
#define COMPILAR 6
#define ACK 8
#define NACK 9 
#define LIN 10
#define CONLS 11
#define ARQ 12
#define EOT 13
#define ERRO 15
#define TAM_DIRETORIO 500

// Buffer kermit pacote
struct pacote_binario{
    unsigned char header[3];            // possui 24 bits
    unsigned char data[TAM_DATA+1];     // 15 bytes de dados + 1 byte de paridade
};

// Human readable kermit pacote
typedef struct Pacote_legivel{
    unsigned int inicio;
    unsigned int destino;
    unsigned int origem;
    unsigned int tam;
    unsigned int seq;
    unsigned int tipo;
    unsigned int par;
    unsigned char *data;
} Pacote_legivel;

void iniciapacote(Pacote_legivel *pacote);

void resetpacote(Pacote_legivel *pacote);

int enviapacote(Pacote_legivel *pacote, int soquete);

// Recebe o pacote
// Retorna 0 em sucesso
// Retorna -1 em caso de erro no recebimento
// Retorna 1 caso detecte erro na paridade
// Retorna 2 em timeout
int recebepacote(Pacote_legivel *pacote, int destEsp, int soquete);

unsigned char Paridade(struct pacote_binario *pacoteBinario, int tam);

void enviaACK(int destino, int origem, uint *seq, int soquete);

void enviaNACK(int destino, int origem, uint *seq, int soquete);

void enviaError(int destino, int origem, uint *seq, int tipo, int error, int soquete);

int pacote_tipo(Pacote_legivel *pacote, int tipo);

void printError(Pacote_legivel *pacote);

void incrementaSeq(uint *seq);

void enviaEOT(int destino, int origem, uint *seq, int soquete);

void enviarmensagemfacil(int destino, int origem, int tam, uint *seq, int tipo, char *data, int soquete);

void enviastringfacil(char *retorno, int destino, int origem, uint *seq, int tipo, int soquete);

#endif