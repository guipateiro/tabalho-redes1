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

}




// Comando edit - server side
// troca a linha <numero_linha> do arquivo <nome_arq>, que está no servidor, pelo texto <NOVO_TEXTO> que deve ser digitado entre aspas.
void comando_edit(kermitHuman *package, int *seq, int soquete){

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
    char imput[500] = "";
    char content[500] = "";
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

    //scanf("%[^\n]",imput);
    char c;
    int l = 0;
    while(buffer[l] != '\"'){
        imput[l] = buffer[l];
        l++;
    }
    l++;
    int k = 0;
    while(buffer[l] != '\"'){
        content[k] = buffer[l];
        l++;
        k++;
    }
    content[k] = '\n';
    printf("[%s] %s\n", imput, content );
    // abre <ARQUIVO> no servidor

    arquivo = fopen("teste", "r");
    if (arquivo == NULL){
        sendError(package->orig, package->dest, seq, package->tipo, errno, soquete);
        return;
    }
    
    char **linhas;
    size_t size = 20;
    linhas = malloc(1000 * sizeof(char*));
    for (int i = 0; i < 1000; i++){
        linhas[i] = malloc(size * sizeof(char));
    }

    size_t tamline;
    int i = 1; 
    while (getline(&linhas[i], &size, arquivo) != -1){
        printf("%s", linhas[i]);
        i++;
    }
    printf("iniciar substituição\n");
    printf("pelo amor de deus [%lu] %s",strlen(content),linhas[linha_inicio]);
    if(linha_inicio < i){
        free (linhas[linha_inicio]);
        linhas[linha_inicio] = malloc((strlen(content)+2) * sizeof(char));
        if (linhas[linha_inicio] == NULL){
            sendError(package->orig, package->dest, seq, package->tipo, 1 /*errno*/, soquete);
            return;
        }
        strcpy(linhas[linha_inicio], content);
    }
    else if(linha_inicio == i){
        linhas[linha_inicio] = malloc((strlen(content)+2) * sizeof(char));
        if (linhas[linha_inicio] == NULL){
            sendError(package->orig, package->dest, seq, package->tipo, 1 /*errno*/, soquete);
            return;
        }
        strcpy(linhas[linha_inicio], content);
    }
    printf("pelo amor de deus [%lu] %s",strlen(content),linhas[linha_inicio]);
    for (int k = 0; k <= i; k++){
        printf("%s", linhas[k]);
    }
}