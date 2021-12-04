#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>
#include "protocolo.h"

// Inicia o pacote
void iniciapacote(Pacote_legivel *pacote){

    pacote->inicio = -1;
    pacote->destino = -1;
    pacote->origem = -1;
    pacote->tam = -1;
    pacote->seq = -1;
    pacote->tipo = -1;
    pacote->par = -1;
    pacote->data = NULL;
}

// Reseta o pacote e libera espaço reservado na memória
void resetpacote(Pacote_legivel *pacote){

    free(pacote->data);
    pacote->inicio = -1;
    pacote->destino = -1;
    pacote->origem = -1;
    pacote->tam = -1;
    pacote->seq = -1;
    pacote->tipo = -1;
    pacote->par = -1;
    pacote->data = NULL;
}

// Prepara e envia o buffer
int enviapacote(Pacote_legivel *pacote, int soquete){

    unsigned char buffer[TAM_PACKAGE];
    memset(buffer, 0, TAM_PACKAGE);
    struct pacote_binario pacoteBinario;

    pacoteBinario.header[0] = (unsigned char) pacote->inicio;
    pacoteBinario.header[1] = (unsigned char) ((pacote->destino << 6) + (pacote->origem << 4) + pacote->tam);
    pacoteBinario.header[2] = (unsigned char) ((pacote->seq << 4) + pacote->tipo);

    if(pacote->tam > 0){
        pacote->data[pacote->tam] = '\0';
        memcpy(pacoteBinario.data, pacote->data, pacote->tam);
    }

    pacoteBinario.data[pacote->tam] = (unsigned char) Paridade(&pacoteBinario, pacote->tam);
    pacote->par = (unsigned char) pacoteBinario.data[pacote->tam];

    memcpy(buffer, (unsigned char *) pacoteBinario.header, 3);
    memcpy(buffer+3, (unsigned char *) pacoteBinario.data, pacote->tam+1);

    if(write(soquete, buffer, TAM_PACKAGE) == -1){
        perror("falha no envio do pacote\n");
        return(-1);
    }

    // organiza file descriptor para timeout
    struct pollfd rede;
    rede.fd = soquete;
    rede.events = POLLIN;

    if( poll(&rede, 1, 5) )
        read(soquete, buffer, TAM_PACKAGE);

    #ifdef DEBUG
        printf("\nENVIADO\n");
        printf("destino: %d, origem: %d, tam: %d\n", pacote->destino, pacote->origem, pacote->tam);
        printf("seq: %d, tipo: %d\n", pacote->seq, pacote->tipo);
        printf("data: %s, par: %d\n", pacote->data, pacote->par);
        printf("FIM ENVIADO\n");
    #endif

    return 1;
}

// Recebe o pacote
// Retorna 0 em sucesso
// Retorna -1 em caso de erro no recebimento
// Retorna 1 caso detecte erro na paridade
// Retorna 2 em timeout
int recebepacote(Pacote_legivel *pacote, int destinoEsp, int soquete){  
    unsigned char buffer[TAM_PACKAGE];
    memset(buffer, 0, TAM_PACKAGE);

    // organiza file descriptor para timeout
    struct pollfd rede;
    rede.fd = soquete;
    rede.events = POLLIN;

    // espera algum pacote, caso demore mais que TIMEOUT segundos, retorna 2
    int retorno = poll(&rede, 1, TIMEOUT*1000);
    if( retorno == 0 )
        return 2;
    else if( retorno < 0 )
        return(-1);

    int tamBuffer = read(soquete, buffer, TAM_PACKAGE);
    if( tamBuffer == -1 ){
        perror("erro ao receber pacote \n");
        return(-1);
    }  

    struct pacote_binario *pacoteBinario = (struct pacote_binario *) buffer;

    // coleta os 1° byte, onde está o marcador de inicio
    pacote->inicio = (unsigned char) (pacoteBinario->header[0]); 
    // coleta os dois bits mais significativos do 2° byte, onde está o end. destino
    pacote->destino = (unsigned char) (pacoteBinario->header[1] & 0xc0) >> 6; 

    if( (pacote->destino != destinoEsp) || (pacote->inicio != MARCA_INICIO) )
        return recebepacote(pacote, destinoEsp, soquete);

    // coleta os 3° e 4° bits mais significativos do 2° byte, onde está o end. origem
    pacote->origem = (unsigned char) (pacoteBinario->header[1] & 0x30) >> 4;
    // coleta os 4 bits menos significativos do 2° byte, onde está o tamanho
    pacote->tam = (unsigned char) (pacoteBinario->header[1] & 0x0F);

    // coleta os 4 bits mais significativos do 3° byte, onde está a sequencia
    pacote->seq = (unsigned char) (pacoteBinario->header[2] & 0xF0) >> 4;
    // coleta os 4 bits menos significativos do 3° byte, onde está o tipo
    pacote->tipo = (unsigned char) pacoteBinario->header[2] & 0x0F;

    // se tem tamanho > 0, aloca espaço para guardar dados

    pacote->data = NULL;
    if( pacote->tam > 0 ){
        pacote->data = (unsigned char *) malloc(pacote->tam);
        memcpy(pacote->data, pacoteBinario->data, pacote->tam);
        pacote->data[pacote->tam] = '\0';
    }

    pacote->par = (unsigned char) pacoteBinario->data[pacote->tam];

    #ifdef DEBUG
    if(pacote->destino != 0){
        printf("\nRECEBIDO\n");
        printf("destino: %d, origem: %d, tam: %d\n", pacote->destino, pacote->origem, pacote->tam);
        printf("seq: %d, tipo: %d\n", pacote->seq, pacote->tipo);
        printf("data: %s, par: %d\n", pacote->data, pacote->par);
        printf("FIM RECEBIDO\n");
    }
    #endif

    if(Paridade(pacoteBinario, pacote->tam) != pacote->par)
        return 1;

    if(poll(&rede, 1, 1))
        read(soquete, buffer, TAM_PACKAGE);

    return 0;
}

// Gera paridade
unsigned char Paridade(struct pacote_binario *pacoteBinario, int tam){

    unsigned char paridade = ((0x0F & pacoteBinario->header[1]) ^ pacoteBinario->header[2]);
    for (int i = 0; i < tam; i++)  
        paridade ^= pacoteBinario->data[i];

    return paridade;
}

// Envia mensagem de acknowledge
void enviaACK(int destino, int origem, uint *seq, int soquete)
{  
  Pacote_legivel pacote;

  pacote.inicio = MARCA_INICIO;
  pacote.destino = destino;
  pacote.origem = origem;
  pacote.tam = 0;
  pacote.seq = *seq;
  pacote.tipo = ACK;
  pacote.par = 0;
  pacote.data = NULL;
  
  if(enviapacote(&pacote, soquete) < 0)
    exit(-1);

  incrementaSeq(seq);
}

// Envia mensagem de NOT acknowledge
void enviaNACK(int destino, int origem, uint *seq, int soquete)
{  
  Pacote_legivel pacote;

  pacote.inicio = MARCA_INICIO;
  pacote.destino = destino;
  pacote.origem = origem;
  pacote.tam = 0;
  pacote.seq = *seq;
  pacote.tipo = NACK;
  pacote.par = 0;
  pacote.data = NULL;
  
  if( enviapacote(&pacote, soquete) < 0 )
    exit(-1);

  //incrementaSeq(seq);
}

// Envia mensagem de error
void enviaError(int destino, int origem, uint *seq, int tipo, int error, int soquete){
    Pacote_legivel pacote;

    unsigned char error_code = 0;

    if( (error == 13) || (error == 1) )
        error_code = 1;
    else if( ((error == 2) && (tipo == 0)) || (error == 20) )
        error_code = 2;
    else if( error == 2 )
        error_code = 3;
    else if( (error == -1) )
        error_code = 4;

    pacote.data = malloc(sizeof(unsigned char));
    pacote.data[0] = (unsigned char) error_code;

    pacote.inicio = MARCA_INICIO;
    pacote.destino = destino;
    pacote.origem = origem;
    pacote.tam = 1;
    pacote.seq = *seq;
    pacote.tipo = ERRO;
    pacote.par = 0;  

    if(enviapacote(&pacote, soquete) < 0)
        exit(-1);

    incrementaSeq(seq);

    free(pacote.data);
    pacote.data = NULL;
}

// Imprime mensagem de erro
void printError(Pacote_legivel *pacote){

    if( pacote->data[0] == 1 ){
        printf("acesso proibido/sem permissão\n");
    } else if( pacote->data[0] == 2 ){
        printf("diretório inexistente\n");
    } else if( pacote->data[0] == 3 ){
        printf("arquivo inexistente\n");
    } else if( pacote->data[0] == 4 ){
        printf("linha inexistente\n");
    } else {
        printf("ocorreu um erro desconhecido\n");
    }
}

// Verifica se o pacote tem o destino e tipo esperado
int pacote_tipo(Pacote_legivel *pacote, int tipo){
    if((pacote->tipo == tipo))
        return 1;
    return 0;
}

// Incrementa sequencia
void incrementaSeq(uint *seq){
    if(*seq > 14)
        *seq = 0;
    else
        (*seq)++;
}

// Envia mensagem de NOT acknowledge
void enviaEOT(int destino, int origem, uint *seq, int soquete){  
    enviarmensagemfacil(destino,origem,0,seq,EOT,NULL,soquete);
}

void enviarmensagemfacil(int destino,int origem,int tam,uint *seq,int tipo,char *data,int soquete){
    Pacote_legivel packageSend, packageRec;
    
    packageSend.inicio = MARCA_INICIO;
    packageSend.destino = destino;
    packageSend.origem = origem;
    packageSend.tam = tam;
    packageSend.seq = *seq;
    packageSend.tipo = tipo;
    packageSend.data = malloc(packageSend.tam);
    memcpy(packageSend.data, data, packageSend.tam);

    if(enviapacote(&packageSend, soquete) < 0)
        exit(-1);

    // quando tipo = ACK, sucesso no comando cd
    // quando tipo = ERRO, houve erro
    iniciapacote(&packageRec);
    do{
        resetpacote(&packageRec);

        // espera receber pacote
        int retorno = recebepacote(&packageRec, packageSend.origem, soquete);
        if( retorno == -1 ){
          exit(-1);
        } 
        if (retorno > 0)
          printf("em timeout \n");
        // se o retorno for timeout, erro de paridade
        // ou se o pacote for NACK, envia o pacote novamente
        if( (retorno > 0) || pacote_tipo(&packageRec, NACK) ){
            if(enviapacote(&packageSend, soquete) < 0)
                exit(-1);
        }
    }while( !pacote_tipo(&packageRec, ACK) && !pacote_tipo(&packageRec, ERRO));
    if(packageRec.seq != packageSend.seq){
        printf("erro de sequencia\n");
    }
    incrementaSeq(seq);

    if(pacote_tipo(&packageRec, ERRO)){
        printError(&packageRec);
    }   

    // Libera memória
    resetpacote(&packageSend);
    resetpacote(&packageRec);
}

void enviastringfacil(char *retorno, int destino, int origem, uint *seq, int tipo, int soquete){

    int tamanho = strlen(retorno);
    int contmsg = (tamanho/15);
    int restomsg = tamanho % 15;
    //printf("%i mensagens // %i resto\n buffer: %s\n", contmsg, restomsg, retorno );
    for (int i = 0; i < contmsg; ++i){
        char data[15] = "";
        for (int j = 0; j < 15; j++){
            data[j] = retorno[(15*i)+j];
        }
        data[15] = '\0';
        //printf("I: %i data:%s\n",i, data);
        enviarmensagemfacil(destino, origem, 15, seq, tipo, data, soquete);
    }
    char data[15] = "";
    for (int j = 0; j < restomsg; j++){
        data[j] = retorno[15*contmsg+j];
    }
    //printf("data:%s \n",data);
    enviarmensagemfacil(destino, origem, restomsg, seq, tipo, data, soquete);

    enviaEOT(destino, origem, seq, soquete);
}