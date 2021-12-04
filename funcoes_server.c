#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>         
#include "ConexaoRawSocket.h"
#include "protocolo.h"
#include "funcoes_server.h"

void pwd(char* resposta){
    getcwd(resposta,TAM_DIRETORIO*sizeof(char));
    if (resposta == NULL){
        perror("erro ao receber diretorio do executavel");
        exit(1);
    }    
}

void cd_server(Pacote_legivel *pacote, uint *seq, int soquete){

    char buffer[500] = "";
    for (int i = 0; i < pacote->tam; i++){
        buffer[i] = pacote->data[i];
    }
    int contador = pacote->tam;
    enviaACK(pacote->origem, pacote->destino, seq, soquete);
    while( !pacote_tipo(pacote, EOT)){
        resetpacote(pacote);
        if( recebepacote(pacote, 2, soquete) < 0 )
            exit(-1);

        for (int i = 0; i < pacote->tam; i++){
            buffer[contador+i] = pacote->data[i];
        }
        contador += pacote->tam;
        enviaACK(pacote->origem, pacote->destino, seq, soquete);
    }    

    if(chdir(buffer)){
        enviaError(pacote->origem, pacote->destino, seq, CD, errno, soquete);
        printf("erro codigo: %i\n",errno );
        return;
    }
    return;
}

void ls_server(Pacote_legivel *pacote, uint *seq, int soquete){   
    
    enviaACK(pacote->origem, pacote->destino, seq, soquete);

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
    //printf("%s",retorno);

    enviastringfacil(retorno, CLIENT, SERVER, seq, ARQ, soquete);
}


void ver_server(Pacote_legivel *pacote, uint *seq, int soquete){  
    
    FILE *arquivo;

    char buffer[500] = "";
    for (int i = 0; i < pacote->tam; i++){
        buffer[i] = pacote->data[i];
    }
    int contador = pacote->tam;
    //printf("contador = %i\n",contador );
    enviaACK(pacote->origem, pacote->destino, seq, soquete);

    while( !pacote_tipo(pacote, EOT)){
        resetpacote(pacote);
        if( recebepacote(pacote, 2, soquete) < 0 )
            exit(-1);

        for (int i = 0; i < pacote->tam; i++){
            buffer[contador+i] = pacote->data[i];
        }
        contador += pacote->tam;
        enviaACK(pacote->origem, pacote->destino, seq, soquete);
    }    

    //printf("[%s]\n", buffer );

    arquivo = fopen(buffer, "r");
    if (arquivo == NULL){
        enviaError(pacote->origem, pacote->destino, seq, VER, errno, soquete);
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
    enviastringfacil(retorno, CLIENT, SERVER, seq, ARQ, soquete);
}




// Comando linha - server side
// Mostra a linha <numero_linha> do arquivo <nome_arq> que esta no servidor na tela do cliente.
void linhas_server(Pacote_legivel *pacote, uint *seq, int soquete){
    
    enviaACK(pacote->origem, pacote->destino, seq, soquete);

    resetpacote(pacote);
    if( recebepacote(pacote, 2, soquete) < 0 )
        exit(-1);

    FILE *arquivo;  
    unsigned int linha_inicio = 0;
    unsigned int linha_fim = 0;
    for (int i = 0; i < 4; i++){
        linha_inicio += (unsigned int) (pacote->data[i] << (8*(3-i)));
        linha_fim += (unsigned int) (pacote->data[i+5] << (8*(3-i)));
    }

    enviaACK(pacote->origem, pacote->destino, seq, soquete);

    resetpacote(pacote);
    if( recebepacote(pacote, 2, soquete) < 0 )
        exit(-1);
    char buffer[500] = "";
    for (int i = 0; i < pacote->tam; i++){
        buffer[i] = pacote->data[i];
    }
    int contador = pacote->tam;
    //printf("contador = %i\n",contador );
    enviaACK(pacote->origem, pacote->destino, seq, soquete);

    while( !pacote_tipo(pacote, EOT)){
        resetpacote(pacote);
        if( recebepacote(pacote, 2, soquete) < 0 )
            exit(-1);

        for (int i = 0; i < pacote->tam; i++){
            buffer[contador+i] = pacote->data[i];
        }
        contador += pacote->tam;
        //printf("contador = %i\n",contador );
        enviaACK(pacote->origem, pacote->destino, seq, soquete);
    }    
    printf("diretorio a ser trocado %s >\n",buffer );

    printf("[%s]\n", buffer );
    // abre <ARQUIVO> no servidor
    arquivo = fopen(buffer, "r");
    if (arquivo == NULL){
        enviaError(pacote->origem, pacote->destino, seq, LINHAS, errno, soquete);
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

     fclose(arquivo);
    if (linha_inicio > controle_linhas || linha_fim > controle_linhas || linha_inicio == 0){
        enviaError(pacote->origem, pacote->destino, seq, LINHAS, 4, soquete);
        return;
    }
    enviastringfacil(resposta_buffer, CLIENT, SERVER, seq, ARQ, soquete);
}


// Comando linha - server side
// Mostra a linha <numero_linha> do arquivo <nome_arq> que esta no servidor na tela do cliente.
void linha_server(Pacote_legivel *pacote, uint *seq, int soquete){
    
    enviaACK(pacote->origem, pacote->destino, seq, soquete);

    resetpacote(pacote);
    if( recebepacote(pacote, 2, soquete) < 0 )
        exit(-1);

    FILE *arquivo;  
    unsigned int linha_inicio = 0;
    unsigned int linha_fim = 0;
    for (int i = 0; i < 4; i++){
        linha_inicio += (unsigned int) (pacote->data[i] << (8*(3-i)));
        linha_fim += (unsigned int) (pacote->data[i+5] << (8*(3-i)));
    }

    enviaACK(pacote->origem, pacote->destino, seq, soquete);

    resetpacote(pacote);
    if( recebepacote(pacote, 2, soquete) < 0 )
        exit(-1);
    char buffer[500] = "";
    for (int i = 0; i < pacote->tam; i++){
        buffer[i] = pacote->data[i];
    }
    int contador = pacote->tam;
    //printf("contador = %i\n",contador );
    enviaACK(pacote->origem, pacote->destino, seq, soquete);

    while( !pacote_tipo(pacote, EOT)){
        resetpacote(pacote);
        if( recebepacote(pacote, 2, soquete) < 0 )
            exit(-1);

        for (int i = 0; i < pacote->tam; i++){
            buffer[contador+i] = pacote->data[i];
        }
        contador += pacote->tam;
        //printf("contador = %i\n",contador );
        enviaACK(pacote->origem, pacote->destino, seq, soquete);
    }    
    printf("diretorio a ser trocado %s >\n",buffer );

    printf("[%s]\n", buffer );
    // abre <ARQUIVO> no servidor
    arquivo = fopen(buffer, "r");
    if (arquivo == NULL){
        enviaError(pacote->origem, pacote->destino, seq, LINHA, errno, soquete);
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

      fclose(arquivo);
    //controle de linhas 
    if (linha_inicio > controle_linhas || linha_fim > controle_linhas || linha_inicio == 0){
        enviaError(pacote->origem, pacote->destino, seq, LINHA, 4, soquete);
        return;
    }
    enviastringfacil(resposta_buffer, CLIENT, SERVER, seq, ARQ, soquete);    
}

void edit_server(Pacote_legivel *pacote, uint *seq, int soquete){
    
    enviaACK(pacote->origem, pacote->destino, seq, soquete);

    resetpacote(pacote);
    if( recebepacote(pacote, 2, soquete) < 0 )
        exit(-1);

    FILE *arquivo;  
    unsigned int linha_inicio = 0;
    unsigned int linha_fim = 0;
    for (int i = 0; i < 4; i++){
        linha_inicio += (unsigned int) (pacote->data[i] << (8*(3-i)));
        linha_fim += (unsigned int) (pacote->data[i+5] << (8*(3-i)));
    }

    enviaACK(pacote->origem, pacote->destino, seq, soquete);

    resetpacote(pacote);
    if( recebepacote(pacote, 2, soquete) < 0 )
        exit(-1);
    char buffer[500] = "";
    char imput[500] = "";
    char content[500] = "";
    for (int i = 0; i < pacote->tam; i++){
        buffer[i] = pacote->data[i];
    }
    int contador = pacote->tam;
    //printf("contador = %i\n",contador );
    enviaACK(pacote->origem, pacote->destino, seq, soquete);

    while( !pacote_tipo(pacote, EOT)){
        resetpacote(pacote);
        if( recebepacote(pacote, 2, soquete) < 0 )
            exit(-1);

        for (int i = 0; i < pacote->tam; i++){
            buffer[contador+i] = pacote->data[i];
        }
        contador += pacote->tam;
        //printf("contador = %i\n",contador );
        enviaACK(pacote->origem, pacote->destino, seq, soquete);
    }    
    printf("diretorio a ser trocado %s >\n",buffer );

    //scanf("%[^\n]",imput);
    int l = 0;
    int g = 0;
    while(buffer[l] != '\"'){
        if(buffer[l] != ' '){
            imput[g] = buffer[l];
            g++;
        }
        l++;
    }
    l++;
    imput[g] = '\0';
    //printf("%s\n",imput);
    int k = 0;
    while(buffer[l] != '\"'){
        content[k] = buffer[l];
        l++;
        k++;
    }
    content[k] = '\n';
    content[k+1] ='\0';
    printf("[%s] %s\n", imput, content );
    // abre <ARQUIVO> no servidor

    arquivo = fopen(imput, "r+");
    if (arquivo == NULL){
        enviaError(pacote->origem, pacote->destino, seq, EDIT, errno, soquete);
        return;
    }
    
    char **linhas;
    size_t size = 20;
    linhas = malloc(1000 * sizeof(char*));
    for (int i = 0; i < 1000; i++){
        linhas[i] = malloc(size * sizeof(char));
    }

    int i = 1; 
    while (getline(&linhas[i], &size, arquivo) >= 0){
        printf("[%i]%s",i,linhas[i]);
        i++;
    }

    if (linha_inicio > i || linha_fim > i || linha_inicio == 0){
        enviaError(pacote->origem, pacote->destino, seq, EDIT, 4, soquete);
        return;
    }

    printf("NUMERO DE LINHAS: [%i]",i);
    printf("iniciar substituição\n");
    printf("pelo amor de deus [%lu] %s",strlen(content),linhas[linha_inicio]);
    if(linha_inicio < i){
        free(linhas[linha_inicio]);
        linhas[linha_inicio] = malloc(strlen((content)+2) * sizeof(char));
        if (linhas[linha_inicio] == NULL){
            enviaError(pacote->origem, pacote->destino, seq, EDIT, errno, soquete);
            return;
        }
        strcpy(linhas[linha_inicio], content);
    }
    else if(linha_inicio == i){
        linhas[linha_inicio] = realloc(linhas[linha_inicio], strlen((content)+2) * sizeof(char));
        if (linhas[linha_inicio] == NULL){
            enviaError(pacote->origem, pacote->destino, seq, EDIT, errno, soquete);
            return;
        }
        strcpy(linhas[linha_inicio], content);
        i++;
    }

    rewind(arquivo);
    printf("pelo amor de deus [%lu] %s",strlen(content),linhas[linha_inicio]);
    for (int k = 1; k < i; k++){
        fprintf(arquivo,"%s", linhas[k]);
    }

    fclose(arquivo);

    for (int k = 0; k < 1000; k++){
        free(linhas[k]);
    }    
    free(linhas);

}            


void compila_server(Pacote_legivel *pacote, uint *seq, int soquete){
    
    char buffer[500] = "";
    for (int i = 0; i < pacote->tam; i++){
        buffer[i] = pacote->data[i];
    }
    int contador = pacote->tam;
    printf("contador = %i\n",contador );
    enviaACK(pacote->origem, pacote->destino, seq, soquete);
    while(!pacote_tipo(pacote, EOT)){
        resetpacote(pacote);
        if(recebepacote(pacote, 2, soquete) < 0)
            exit(-1);

        for (int i = 0; i < pacote->tam; i++){
            buffer[contador+i] = pacote->data[i];
        }
        contador += pacote->tam;
        printf("contador = %i\n",contador );
        enviaACK(pacote->origem, pacote->destino, seq, soquete);
    }

    printf("qqr coisa so pra testar\n");    
    printf("%s\n",buffer);
    fflush(stdout);

    char buffer2[550] = "";
    sprintf(buffer2,"gcc %s 2> saidacompilador.txt\n",buffer);

    printf("%s",buffer2);

    system(buffer2);
    //system("chmod 777 saidacompilador.txt");
    fflush(stdout);

    FILE* arquivo;
    arquivo = fopen("saidacompilador.txt", "r");
    if (arquivo == NULL){
        enviaError(pacote->origem, pacote->destino, seq, COMPILAR, errno, soquete);
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
    //system("rm saidacompilador.txt");
    enviastringfacil(retorno, CLIENT, SERVER, seq, ARQ, soquete);
}