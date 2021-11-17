#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>         
#include "ConexaoRawSocket.h"
#include "kermitProtocol.h"
#include "funcoes_server.h"

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
#define TAM_DIRETORIO 150


void pwd(char* resposta){
    getcwd(resposta,TAM_DIRETORIO*sizeof(char));
    //printf("%li\n",sizeof(resposta));
        if (resposta == NULL){
            perror("erro ao receber diretorio do executavel");
            exit(1);
        }    
    return;
}


// Comando cd - server side
// Executa change directory no server
void comando_cd(kermitHuman *package, int *seq, int soquete){

    char buffer[500] = "";
    for (int i = 0; i < package->tam; i++){
        buffer[i] = package->data[i];
    }
    int contador = package->tam;
    //printf("contador = %i\n",contador );
    sendACK(package->orig, package->dest, seq, soquete);
    while( !ehPack(package, EOT)){
        resetPackage(package);
        if( receivePackage(package, 2, soquete) < 0 )
            exit(-1);

        for (int i = 0; i < package->tam; i++){
            buffer[contador+i] = package->data[i];
        }
        contador += package->tam;
        //printf("contador = %i\n",contador );
        sendACK(package->orig, package->dest, seq, soquete);
    }    
    //printf("diretorio a ser trocado %s >\n",buffer );

    if(chdir(buffer)){
        sendError(package->orig, package->dest, seq, package->tipo, errno, soquete);
        printf("erro codigo: %i\n",errno );
        return;
    }
    return;
}



// Comando ls - server side
// Executa ls no server
void comando_ls(kermitHuman *package, int *seq, int soquete){  
    kermitHuman packageSend, packageRec;
    sendACK(package->orig, package->dest, seq, soquete);
    char diretorio[TAM_DIRETORIO];
    pwd(diretorio);

    struct dirent **nomes;

    int n = scandir(diretorio ,&nomes, NULL, alphasort);
    if (n == -1){
        perror("erro ao ler diretorio\n");
        exit(1);
    }
    char retorno[2000] = "";
    int contador = 0;
    for (int k = 0 ; k < n; k++){
        int tamanho = strlen(nomes[k]->d_name);
        for (int i = 0; i < tamanho; i++){
            (retorno[contador+i] = nomes[k]->d_name[i]);
        }
        contador += tamanho;
        retorno[contador] = '\n';
        contador++;
    }    
    retorno[contador] = '\0';
    printf("%s",retorno);


    int tamanho = strlen(retorno);
    int contmsg = (tamanho/15);
    int restomsg = tamanho % 15;
    printf("%i mensagens // %i resto\n buffer: %s\n", contmsg, restomsg, retorno );
    for (int i = 0; i < contmsg; ++i){
        char data[15] = "";
        for (int j = 0; j < 15; j++){
            data[j] = retorno[(15*i)+j];
        }
        data[15] = '\0';
        printf("I: %i data:%s\n",i, data);
        enviarmensagemfacil(soquete,CLIENT,SERVER,15,seq,CONLS,data);
    }
    char data[15] = "";
    for (int j = 0; j < restomsg; j++){
        data[j] = retorno[15*contmsg+j];
    }
    printf("data:%s \n",data);
    enviarmensagemfacil(soquete,CLIENT,SERVER,restomsg,seq,CONLS,data);

    enviaEOT(CLIENT,SERVER,seq,soquete);
}





// Comando ver - server side
// Mostra o conteúdo do arquivo texto do servidor na tela do cliente
void comando_ver(kermitHuman *package, int *seq, int soquete){  

    //kermitHuman package;
    FILE *arquivo;

    char buffer[500] = "";
    for (int i = 0; i < package->tam; i++){
        buffer[i] = package->data[i];
    }
    int contador = package->tam;
    //printf("contador = %i\n",contador );
    sendACK(package->orig, package->dest, seq, soquete);

    while( !ehPack(package, EOT)){
        resetPackage(package);
        if( receivePackage(package, 2, soquete) < 0 )
            exit(-1);

        for (int i = 0; i < package->tam; i++){
            buffer[contador+i] = package->data[i];
        }
        contador += package->tam;
        //printf("contador = %i\n",contador );
        sendACK(package->orig, package->dest, seq, soquete);
    }    
    //printf("diretorio a ser trocado %s >\n",buffer );

    printf("[%s]\n", buffer );
    // abre <ARQUIVO> no servidor
    arquivo = fopen(buffer, "r");
    if (arquivo == NULL){
        sendError(package->orig, package->dest, seq, package->tipo, errno, soquete);
        return;
    }

    char retorno[10000] = "";
    int i = 0;
    char c;
    while ((c = fgetc(arquivo)) != EOF){
        retorno[i] = (char) c;
        i++;
    }
    retorno[i+1] = '\0';

    fclose(arquivo);
    //printf("%s",resposta);

    int tamanho = strlen(retorno);
    int contmsg = (tamanho/15);
    int restomsg = tamanho % 15;
    printf("%i mensagens // %i resto\n buffer: %s\n", contmsg, restomsg, retorno );
    for (int i = 0; i < contmsg; ++i){
        char data[15] = "";
        for (int j = 0; j < 15; j++){
            data[j] = retorno[(15*i)+j];
        }
        data[15] = '\0';
        printf("I: %i data:%s\n",i, data);
        enviarmensagemfacil(soquete,CLIENT,SERVER,15,seq,ARQ,data);
    }
    char data[15] = "";
    for (int j = 0; j < restomsg; j++){
        data[j] = retorno[15*contmsg+j];
    }
    printf("data:%s \n",data);
    enviarmensagemfacil(soquete,CLIENT,SERVER,restomsg,seq,ARQ,data);

    enviaEOT(CLIENT,SERVER,seq,soquete);

}




// Comando linha - server side
// Mostra a linha <numero_linha> do arquivo <nome_arq> que esta no servidor na tela do cliente.
void comando_linhas(kermitHuman *package, int *seq, int soquete){

    sendACK(package->orig, package->dest, seq, soquete);

    resetPackage(package);
    if( receivePackage(package, 2, soquete) < 0 )
        exit(-1);

    FILE *arquivo;  
    unsigned int linha_inicio = 0;
    unsigned int linha_fim = 0;
    for (int i = 0; i < 4; i++){
        linha_inicio += (unsigned int) (package->data[i] << (8*(3-i)));
        linha_fim += (unsigned int) (package->data[i+5] << (8*(3-i)));
    }

    sendACK(package->orig, package->dest, seq, soquete);

    resetPackage(package);
    if( receivePackage(package, 2, soquete) < 0 )
        exit(-1);
    char buffer[500] = "";
    for (int i = 0; i < package->tam; i++){
        buffer[i] = package->data[i];
    }
    int contador = package->tam;
    //printf("contador = %i\n",contador );
    sendACK(package->orig, package->dest, seq, soquete);

    while( !ehPack(package, EOT)){
        resetPackage(package);
        if( receivePackage(package, 2, soquete) < 0 )
            exit(-1);

        for (int i = 0; i < package->tam; i++){
            buffer[contador+i] = package->data[i];
        }
        contador += package->tam;
        //printf("contador = %i\n",contador );
        sendACK(package->orig, package->dest, seq, soquete);
    }    
    printf("diretorio a ser trocado %s >\n",buffer );

    printf("[%s]\n", buffer );
    // abre <ARQUIVO> no servidor
    arquivo = fopen(buffer, "r");
    if (arquivo == NULL){
        sendError(package->orig, package->dest, seq, package->tipo, errno, soquete);
        return;
    }

    char retorno[10000] = "";
    int i = 0;
    char c;
    int N_linhas = 1;
    while ((c = fgetc(arquivo)) != EOF){
        if (c == '\n')
            N_linhas++;
        retorno[i] = (char) c;
        i++;
    }
    retorno[i+1] = '\0';

    printf("numero de linhas: %i \n",N_linhas);

    printf("%s",retorno);
    int controle_linhas = 0;
    char resposta_buffer[10000] = "";
    i = 0;
    int j = 0;
    while ((c = retorno[i]) != EOF){
        if (controle_linhas <= linha_fim && controle_linhas >= linha_inicio ){
            resposta_buffer[j] = c;
            j++;
        }
        if (c == '\n')
            controle_linhas++;
        i++;
    }
    resposta_buffer[j] = '\0';
    fclose(arquivo);

    printf("%s",resposta_buffer);

    //controle de linhas 

    fclose(arquivo);

    int tamanho = strlen(resposta_buffer);
    int contmsg = (tamanho/15);
    int restomsg = tamanho % 15;
    printf("%i mensagens // %i resto\n buffer: %s\n", contmsg, restomsg, resposta_buffer );
    for (int i = 0; i < contmsg; ++i){
        char data[15] = "";
        for (int j = 0; j < 15; j++){
            data[j] = resposta_buffer[(15*i)+j];
        }
        data[15] = '\0';
        printf("I: %i data:%s\n",i, data);
        enviarmensagemfacil(soquete,CLIENT,SERVER,15,seq,ARQ,data);
    }
    char data[15] = "";
    for (int j = 0; j < restomsg; j++){
        data[j] = resposta_buffer[15*contmsg+j];
    }
    printf("data:%s \n",data);
    enviarmensagemfacil(soquete,CLIENT,SERVER,restomsg,seq,ARQ,data);

    enviaEOT(CLIENT,SERVER,seq,soquete);

    incrementaSeq(seq);

    // Libera memória
    //resetPackage(&packageSend);
    //resetPackage(&packageRec);
}




// Comando linha - server side
// Mostra a linha <numero_linha> do arquivo <nome_arq> que esta no servidor na tela do cliente.
void comando_linha(kermitHuman *package, int *seq, int soquete){

    sendACK(package->orig, package->dest, seq, soquete);

    resetPackage(package);
    if( receivePackage(package, 2, soquete) < 0 )
        exit(-1);

    FILE *arquivo;  
    unsigned int linha_inicio = 0;
    unsigned int linha_fim = 0;
    for (int i = 0; i < 4; i++){
        linha_inicio += (unsigned int) (package->data[i] << (8*(3-i)));
        linha_fim += (unsigned int) (package->data[i+5] << (8*(3-i)));
    }

    sendACK(package->orig, package->dest, seq, soquete);

    resetPackage(package);
    if( receivePackage(package, 2, soquete) < 0 )
        exit(-1);
    char buffer[500] = "";
    for (int i = 0; i < package->tam; i++){
        buffer[i] = package->data[i];
    }
    int contador = package->tam;
    //printf("contador = %i\n",contador );
    sendACK(package->orig, package->dest, seq, soquete);

    while( !ehPack(package, EOT)){
        resetPackage(package);
        if( receivePackage(package, 2, soquete) < 0 )
            exit(-1);

        for (int i = 0; i < package->tam; i++){
            buffer[contador+i] = package->data[i];
        }
        contador += package->tam;
        //printf("contador = %i\n",contador );
        sendACK(package->orig, package->dest, seq, soquete);
    }    
    printf("diretorio a ser trocado %s >\n",buffer );

    printf("[%s]\n", buffer );
    // abre <ARQUIVO> no servidor
    arquivo = fopen(buffer, "r");
    if (arquivo == NULL){
        sendError(package->orig, package->dest, seq, package->tipo, errno, soquete);
        return;
    }

    char retorno[10000] = "";
    int i = 0;
    char c;
    int N_linhas = 1;
    while ((c = fgetc(arquivo)) != EOF){
        if (c == '\n')
            N_linhas++;
        retorno[i] = (char) c;
        i++;
    }
    retorno[i+1] = '\0';

    printf("numero de linhas: %i \n",N_linhas);

    printf("%s",retorno);
    int controle_linhas = 0;
    char resposta_buffer[10000] = "";
    i = 0;
    int j = 0;
    while ((c = retorno[i]) != EOF){
        if (controle_linhas <= linha_fim && controle_linhas >= linha_inicio ){
            resposta_buffer[j] = c;
            j++;
        }
        if (c == '\n')
            controle_linhas++;
        i++;
    }
    resposta_buffer[j] = '\0';
    fclose(arquivo);

    printf("%s",resposta_buffer);

    //controle de linhas 

    fclose(arquivo);

    int tamanho = strlen(resposta_buffer);
    int contmsg = (tamanho/15);
    int restomsg = tamanho % 15;
    printf("%i mensagens // %i resto\n buffer: %s\n", contmsg, restomsg, resposta_buffer );
    for (int i = 0; i < contmsg; ++i){
        char data[15] = "";
        for (int j = 0; j < 15; j++){
            data[j] = resposta_buffer[(15*i)+j];
        }
        data[15] = '\0';
        printf("I: %i data:%s\n",i, data);
        enviarmensagemfacil(soquete,CLIENT,SERVER,15,seq,ARQ,data);
    }
    char data[15] = "";
    for (int j = 0; j < restomsg; j++){
        data[j] = resposta_buffer[15*contmsg+j];
    }
    printf("data:%s \n",data);
    enviarmensagemfacil(soquete,CLIENT,SERVER,restomsg,seq,ARQ,data);

    enviaEOT(CLIENT,SERVER,seq,soquete);

    incrementaSeq(seq);

    // Libera memória
    //resetPackage(&packageSend);
    //resetPackage(&packageRec);
}

















































// Comando edit - server side
// troca a linha <numero_linha> do arquivo <nome_arq>, que está no servidor, pelo texto <NOVO_TEXTO> que deve ser digitado entre aspas.
/*void comando_edit(kermitHuman *package, int *seq, int soquete)
{
  kermitHuman packageRec;

  FILE *arquivo; 
  tad_texto buffer; 
  
  // abre <ARQUIVO> no servidor
  arquivo = fopen((char*) package->data, "r");
  if (arquivo == NULL){
    sendError(package->orig, package->dest, seq, package->tipo, errno, soquete);
    return;
  }  

  // aloca o arquivo de texto na memória
  if (!aloca_arq(arquivo, &buffer)){
    sendError(package->orig, package->dest, seq, package->tipo, -2, soquete);
    return;
  }

  fclose(arquivo);

  sendACK(package->orig, package->dest, seq, soquete);

  int retorno;

  iniciaPackage(&packageRec);
  // espera receber o número da linha
  while( !ehPack(&packageRec, LINE_NUMBER) )
  {
    resetPackage(&packageRec);

    // espera receber pacote
    retorno = receivePackage(&packageRec, SERVER, soquete);
    if( retorno == -1 ){
      exit(-1);
    }

    // se o retorno for timeout, erro de paridade
    // ou se o pacote for NACK, envia o pacote novamente
    if( (retorno > 0) || ehPack(&packageRec, NACK) ){
      sendACK(package->orig, package->dest, seq, soquete);
    }
    
  }

  unsigned int linha;
  // pega bytes da esquerda e direita e transforma em unsigned int
  linha = (unsigned int) ((packageRec.data[0] << 8) | (packageRec.data[1]));

  // se a linha for menor que 1 ou maior que o total do arquivo, ela não existe
  if( (linha < 1) || (linha > buffer.num_linhas))
  {
    sendError(package->orig, package->dest, seq, package->tipo, -1, soquete);
    // Libera memória
    resetPackage(&packageRec);
    return;
  }

  sendACK(package->orig, package->dest, seq, soquete);

  // MODIFICAR O ARQUIVO DE TEXTO

  // seta sequencia esperada e incrementa
  int seqEsperada = packageRec.seq;
  incrementaSeq(&seqEsperada);  

  // tamanho da linha
  int tam = 0;  

  // libera espaço na memória referente a linha
  free(buffer.linhas[linha-1]);
  buffer.linhas[linha-1] = NULL;

  // espera receber dados, parar quando receber EOT  
  resetPackage(&packageRec);
  while( !ehPack(&packageRec, EOT) )
  {
    resetPackage(&packageRec);

    retorno = receivePackage(&packageRec, SERVER, soquete);
    if( retorno == -1 ){
      exit(-1);
    } else if( retorno > 0 ){ 
      // caso seja timeout ou erro na paridade, envia NACK
      sendNACK(packageRec.orig, packageRec.dest, seq, soquete);
    } else {      
      // verifica se é pacote tipo conteúdo arquivo
      if( ehPack(&packageRec, FILE_CONTENT) )
      {
        // verifica a sequência 
        if( packageRec.seq == seqEsperada )
        { 
          // realoca a linha para escrever novos dados
          buffer.linhas[linha-1] = (unsigned char *) realloc(buffer.linhas[linha-1], tam + packageRec.tam );
          memcpy(buffer.linhas[linha-1]+tam, packageRec.data, packageRec.tam);
          tam += packageRec.tam;

          sendACK(packageRec.orig, packageRec.dest, seq, soquete);

          incrementaSeq(&seqEsperada);
        } else {
          sendNACK(packageRec.orig, packageRec.dest, seq, soquete);
        }
      }
    }
    
  }

  buffer.linhas[linha-1][tam] = '\n';

  sendACK(packageRec.orig, packageRec.dest, seq, soquete);

  // reescreve ARQUIVO
  arquivo = fopen((char*) package->data, "w");

  for(int i = 0; i < buffer.num_linhas; i++)
  {
    fprintf(arquivo, "%s", buffer.linhas[i]);
    free(buffer.linhas[i]);
    buffer.linhas[i] = NULL;
  }
  free(buffer.linhas);
  buffer.linhas = NULL;
  fclose(arquivo);

  // Libera memória
  resetPackage(&packageRec);

}*/