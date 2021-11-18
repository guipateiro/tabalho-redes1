#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>
#include "kermitProtocol.h"

// Inicia o pacote
void iniciaPackage(kermitHuman *package)
{
  package->inicio = -1;
  package->dest = -1;
  package->orig = -1;
  package->tam = -1;
  package->seq = -1;
  package->tipo = -1;
  package->par = -1;
  package->data = NULL;
}

// Reseta o pacote e libera espaço reservado na memória
void resetPackage(kermitHuman *package)
{  
  free(package->data);
  package->inicio = -1;
  package->dest = -1;
  package->orig = -1;
  package->tam = -1;
  package->seq = -1;
  package->tipo = -1;
  package->par = -1;
  package->data = NULL;
}

// Prepara e envia o buffer
int sendPackage(kermitHuman *package, int soquete)
{
  unsigned char buffer[TAM_PACKAGE];
  memset(buffer, 0, TAM_PACKAGE);
  struct kermitBit packageBit;

  // insere marcador de início do pacote
  packageBit.header[0] = (unsigned char) package->inicio;

  // insere destino, origem e tamanho no 2° byte
  packageBit.header[1] = (unsigned char) ((package->dest << 6) | (package->orig << 4) | package->tam);

  // insere sequencia e tipo no 3° byte
  packageBit.header[2] = (unsigned char) ((package->seq << 4) | package->tipo);

  if( (package->tam > 0) )
  {
    package->data[package->tam] = '\0';
    memcpy(packageBit.data, package->data, package->tam);
  }

  packageBit.data[package->tam] = (unsigned char) geraPar(&packageBit, package->tam);
  package->par = (unsigned char) packageBit.data[package->tam];

  memcpy(buffer, (unsigned char *) packageBit.header, 3);
  memcpy(buffer+3, (unsigned char *) packageBit.data, package->tam+1);

  if( write(soquete, buffer, TAM_PACKAGE) == -1 )
  {
    fprintf(stderr, "Erro no envio: %s\n", strerror(errno));
    return(-1);
  }

  // organiza file descriptor para timeout
  struct pollfd fd;
  fd.fd = soquete;
  fd.events = POLLIN;

  if( poll(&fd, 1, 5) )
    read(soquete, buffer, TAM_PACKAGE);

  //#ifdef DEBUG
  printf("\nENVIADO\n");
  printf("dest: %d, orig: %d, tam: %d\n", package->dest, package->orig, package->tam);
  printf("seq: %d, tipo: %d\n", package->seq, package->tipo);
  printf("data: %s, par: %d\n", package->data, package->par);
  printf("FIM ENVIADO\n");
  //#endif

  return 1;
}

// Recebe o pacote
// Retorna 0 em sucesso
// Retorna -1 em caso de erro no recebimento
// Retorna 1 caso detecte erro na paridade
// Retorna 2 em timeout
int receivePackage(kermitHuman *package, int destEsp, int soquete)
{  
  unsigned char buffer[TAM_PACKAGE];
  memset(buffer, 0, TAM_PACKAGE);

  // organiza file descriptor para timeout
  struct pollfd fd;
  fd.fd = soquete;
  fd.events = POLLIN;
  
  // espera algum pacote, caso demore mais que TIMEOUT segundos, retorna 2
  int retorno = poll(&fd, 1, TIMEOUT*1000);
  if( retorno == 0 )
    return 2;
  else if( retorno < 0 )
    return(-1);

  int tamBuffer = read(soquete, buffer, TAM_PACKAGE);
  if( tamBuffer == -1 )
  {
    fprintf(stderr, "Erro no recebimento do pacote: %s\n", strerror(errno));
    return(-1);
  }  

  struct kermitBit *packageBit = (struct kermitBit *) buffer;

  // coleta os 1° byte, onde está o marcador de inicio
  package->inicio = (unsigned char) (packageBit->header[0]); 
  // coleta os dois bits mais significativos do 2° byte, onde está o end. destino
  package->dest = (unsigned char) (packageBit->header[1] & 0xc0) >> 6; 

  if( (package->dest != destEsp) || (package->inicio != MARCA_INICIO) )
    return receivePackage(package, destEsp, soquete);

  // coleta os 3° e 4° bits mais significativos do 2° byte, onde está o end. origem
  package->orig = (unsigned char) (packageBit->header[1] & 0x30) >> 4;
  // coleta os 4 bits menos significativos do 2° byte, onde está o tamanho
  package->tam = (unsigned char) (packageBit->header[1] & 0x0F);

  // coleta os 4 bits mais significativos do 3° byte, onde está a sequencia
  package->seq = (unsigned char) (packageBit->header[2] & 0xF0) >> 4;
  // coleta os 4 bits menos significativos do 3° byte, onde está o tipo
  package->tipo = (unsigned char) packageBit->header[2] & 0x0F;

  // se tem tamanho > 0, aloca espaço para guardar dados
  package->data = NULL;
  if( package->tam > 0 ){
    package->data = (unsigned char *) malloc(package->tam);
    memcpy(package->data, packageBit->data, package->tam);
    package->data[package->tam] = '\0';
  }

  package->par = (unsigned char) packageBit->data[package->tam];

  //#ifdef DEBUG
  if(package->dest != 0){
    printf("\nRECEBIDO\n");
    printf("dest: %d, orig: %d, tam: %d\n", package->dest, package->orig, package->tam);
    printf("seq: %d, tipo: %d\n", package->seq, package->tipo);
    printf("data: %s, par: %d\n", package->data, package->par);
    printf("FIM RECEBIDO\n");
  }
  //#endif

  if( geraPar(packageBit, package->tam) != package->par )
    return 1;

 if( poll(&fd, 1, 1) )
    read(soquete, buffer, TAM_PACKAGE);

  return 0;
}

// Gera paridade
unsigned char geraPar(struct kermitBit *packageBit, int tam){

    unsigned char paridade = packageBit->header[1] ^ packageBit->header[2];
    for (int i = 0; i < tam; i++)  
        paridade = paridade ^ packageBit->data[i];

    return paridade;
}

// Envia mensagem de acknowledge
void sendACK(int dest, int orig, int *seq, int soquete)
{  
  kermitHuman package;

  package.inicio = MARCA_INICIO;
  package.dest = dest;
  package.orig = orig;
  package.tam = 0;
  package.seq = *seq;
  package.tipo = ACK;
  package.par = 0;
  package.data = NULL;
  
  if( sendPackage(&package, soquete) < 0 )
    exit(-1);

  incrementaSeq(seq);
}

// Envia mensagem de NOT acknowledge
void sendNACK(int dest, int orig, int *seq, int soquete)
{  
  kermitHuman package;

  package.inicio = MARCA_INICIO;
  package.dest = dest;
  package.orig = orig;
  package.tam = 0;
  package.seq = *seq;
  package.tipo = NACK;
  package.par = 0;
  package.data = NULL;
  
  if( sendPackage(&package, soquete) < 0 )
    exit(-1);

  incrementaSeq(seq);
}

// Envia mensagem de error
void sendError(int dest, int orig, int *seq, int tipo, int error, int soquete)
{
  kermitHuman package;

  unsigned char error_code = 0;

  if( (error == 13) || (error == 1) )
    error_code = 1;
  else if( ((error == 2) && (tipo == 0)) || (error == 20) )
    error_code = 2;
  else if( error == 2 )
    error_code = 3;
  else if( (error == -1) )
    error_code = 4;

  package.data = malloc(1);
  package.data[0] = (unsigned char) (error_code & 0xff);

  package.inicio = MARCA_INICIO;
  package.dest = dest;
  package.orig = orig;
  package.tam = 1;
  package.seq = *seq;
  package.tipo = ERROR;
  package.par = 0;  

  if( sendPackage(&package, soquete) < 0 )
    exit(-1);

  incrementaSeq(seq);

  free(package.data);
  package.data = NULL;
}

// Verifica se o pacote tem o destino e tipo esperado
int ehPack(kermitHuman *package, int tipo)
{
  if( (package->tipo == tipo) )
    return 1;

  return 0;
}

// Imprime mensagem de erro
void printError(kermitHuman *package)
{
  if( package->data[0] == 1 ){
    printf("Acesso proibido \n");
  } else if( package->data[0] == 2 ){
    printf("Diretório inexistente\n");
  } else if( package->data[0] == 3 ){
    printf("Arquivo inexistente\n");
  } else if( package->data[0] == 4 ){
    printf("Linha inexistente\n");
  } else {
    printf("Ocorreu algum erro!\n");
  }
}

// Incrementa sequencia
void incrementaSeq(int *seq)
{
  if( *seq > 14 )
    *seq = 0;
  else
    (*seq)++;
}

// Envia mensagem de NOT acknowledge
void enviaEOT(int dest, int orig, int *seq, int soquete)
{  
  enviarmensagemfacil(soquete,dest,orig,0,seq,EOT,NULL);
}

void enviarmensagemfacil(int soquete,unsigned int destino,unsigned int origem,unsigned int tam,unsigned int *seq,int tipo,char *data){
  kermitHuman packageSend, packageRec;
    
  packageSend.inicio = MARCA_INICIO;
  packageSend.dest = destino;
  packageSend.orig = origem;
  packageSend.tam = tam;
  packageSend.seq = *seq;
  packageSend.tipo = tipo;
  packageSend.data = malloc(packageSend.tam);
  memcpy(packageSend.data, data, packageSend.tam);

  if( sendPackage(&packageSend, soquete) < 0 )
    exit(-1);

      // quando tipo = ACK, sucesso no comando cd
      // quando tipo = ERROR, houve erro
      iniciaPackage(&packageRec);
      do{
        resetPackage(&packageRec);

        // espera receber pacote
        int retorno = receivePackage(&packageRec, packageSend.orig, soquete);
        if( retorno == -1 ){
          exit(-1);
        } 
        if (retorno > 0)
          printf("em timeout \n");
        // se o retorno for timeout, erro de paridade
        // ou se o pacote for NACK, envia o pacote novamente
        if( (retorno > 0) || ehPack(&packageRec, NACK) ){
            if( sendPackage(&packageSend, soquete) < 0 )
              exit(-1);
        }
      }while( !ehPack(&packageRec, ACK) && !ehPack(&packageRec, ERROR));

  incrementaSeq(seq);

  if(ehPack(&packageRec, ERROR)){
    printError(&packageRec);
  }   

  // Libera memória
  resetPackage(&packageSend);
  resetPackage(&packageRec);
}