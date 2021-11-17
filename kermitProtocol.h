#ifndef __KERMITPROTOCOL__
#define __KERMITPROTOCOL__

#define MARCA_INICIO 126  // marcador de inicio
#define CLIENT 1        // endereço cliente
#define SERVER 2        // endereço servidor
#define ACK 8           // tipo do ACK
#define NACK 9          // tipo do NACK
#define LINE_NUMBER 10  // tipo linha inicial e final
#define LS_CONTENT 11   // tipo conteúdo ls
#define FILE_CONTENT 12 // tipo conteúdo arquivo
#define EOT 13          // tipo do EOT
#define ERROR 15        // tipo error
#define TAM_PACKAGE 19  // tamanho máximo do pacote, em bytes
#define TAM_DATA 15     // tamanho máximo do campo de dados, em bytes
#define TIMEOUT 5       // timeout em segundos

// Buffer kermit package
struct kermitBit
{
  unsigned char header[3];     // possui 24 bits
  unsigned char data[TAM_DATA+1];     // 15 bytes de dados + 1 byte de paridade
};

// Human readable kermit package
typedef struct kermitHuman
{
  unsigned int inicio;
  unsigned int dest;
  unsigned int orig;
  unsigned int tam;
  unsigned int seq;
  unsigned int tipo;
  unsigned int par;
  unsigned char *data;
} kermitHuman;

// Inicia o pacote
void iniciaPackage(kermitHuman *package);

// Reseta o pacote e libera espaço reservado na memória
void resetPackage(kermitHuman *package);

// Prepara e envia o pacote
int sendPackage(kermitHuman *package, int soquete);

// Recebe o pacote
// Retorna 0 em sucesso
// Retorna -1 em caso de erro no recebimento
// Retorna 1 caso detecte erro na paridade
// Retorna 2 em timeout
int receivePackage(kermitHuman *package, int destEsp, int soquete);

// Gera paridade
unsigned char geraPar(struct kermitBit *packageBit, int tam);

// Envia mensagem de acknowledge
void sendACK(int dest, int orig, int *seq, int soquete);

// Envia mensagem de NOT acknowledge
void sendNACK(int dest, int orig, int *seq, int soquete);

// Envia mensagem de error
void sendError(int dest, int orig, int *seq, int tipo, int error, int soquete);

// Verifica se o pacote tem o tipo esperado
int ehPack(kermitHuman *package, int tipo);

// Imprime mensagem de erro
void printError(kermitHuman *package);

// Incrementa sequencia
void incrementaSeq(int *seq);

void enviaEOT(int dest, int orig, int *seq, int soquete);

void enviarmensagemfacil(int soquete,unsigned int destino,unsigned int origem,unsigned int tam,unsigned int *seq,int tipo,char *data);

#endif