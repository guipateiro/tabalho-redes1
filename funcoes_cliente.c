#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>           
#include <dirent.h>         
#include <unistd.h>          
#include "ConexaoRawSocket.h"
#include "protocolo.h"
#include "funcoes_cliente.h"


void pwd(char* resposta){
    getcwd(resposta,TAM_DIRETORIO*sizeof(char));
    //printf("%li\n",sizeof(resposta));
        if (resposta == NULL){
            perror("erro ao receber diretorio do executavel");
            exit(1);
        }    
    return;
}

void lcd(){
	char diretorio[TAM_DIRETORIO];
	scanf("%s",diretorio);
	if (chdir(diretorio) < 0){
		perror("erro ao mudar de diretorio");
	}
}

void lls(){

	char diretorio[TAM_DIRETORIO];
	pwd(diretorio);

	struct dirent **nomes;

	int n = scandir(diretorio ,&nomes, NULL, alphasort);

    if (n == -1){
        perror("erro ao ler diretorio\n");
        exit(1);
    }
    
    for (int k = 0 ; k < n; k++){
    	printf("%s\n",nomes[k]->d_name);
    }    
    printf("\n");
}

void cd(uint *seq, int soquete){
	char buffer[500];
	scanf("%s",buffer);

	enviastringfacil(buffer, SERVER, CLIENT, seq, CD, soquete);

	Pacote_legivel pacote;
	iniciapacote(&pacote);
	if(recebepacote(&pacote,CLIENT, soquete) < 0){
		exit(-1);
	}		
    if(pacote_tipo(&pacote, ERRO)){
		printError(&pacote);
		incrementaSeq(seq);
    }
	resetpacote(&pacote);

}


void ls(uint *seq, int soquete){

	enviarmensagemfacil(SERVER,CLIENT,0,seq,LS,NULL,soquete);
	Pacote_legivel pacote;
	iniciapacote(&pacote);

	if( recebepacote(&pacote, CLIENT, soquete) < 0 )
            exit(-1);

	char buffer[2000] = "";
    for (int i = 0; i < pacote.tam; i++){
        buffer[i] = pacote.data[i];
    }
    int contador = pacote.tam;
    //printf("contador = %i\n",contador );
    enviaACK(pacote.origem, pacote.destino, seq, soquete);
    while( !pacote_tipo(&pacote, EOT)){
        resetpacote(&pacote);
        if( recebepacote(&pacote, CLIENT, soquete) < 0 )
            exit(-1);

        for (int i = 0; i < pacote.tam; i++){
            buffer[contador+i] = pacote.data[i];
        }
        contador += pacote.tam;
        //printf("contador = %i\n",contador );
        enviaACK(pacote.origem, pacote.destino, seq, soquete);
    }    
    printf("%s",buffer);

    resetpacote(&pacote);
    printf("%d\n",*seq);
}



void ver(uint *seq, int soquete){

	char buffer[500];
	scanf("%s",buffer);

	enviastringfacil(buffer, SERVER, CLIENT, seq, VER, soquete);

	Pacote_legivel pacote;
	iniciapacote(&pacote);
	if(recebepacote(&pacote,CLIENT, soquete) < 0){
		exit(-1);
	}		
    if(pacote_tipo(&pacote, ERRO)){
		printError(&pacote);
		incrementaSeq(seq);
    }
	else{
		char buffer2[2000] = "";
	    for (int i = 0; i < pacote.tam; i++){
	        buffer2[i] = pacote.data[i];
	    }
	    int contador = pacote.tam;
	    //printf("contador = %i\n",contador );
	    enviaACK(pacote.origem, pacote.destino, seq, soquete);
	    while( !pacote_tipo(&pacote, EOT)){
	        resetpacote(&pacote);
	        if( recebepacote(&pacote, CLIENT, soquete) < 0 )
	            exit(-1);

	        for (int i = 0; i < pacote.tam; i++){
	            buffer2[contador+i] = pacote.data[i];
	        }
	        contador += pacote.tam;
	        //printf("contador = %i\n",contador );
	        enviaACK(pacote.origem, pacote.destino, seq, soquete);
	    }    
	    printf("%s\n",buffer2);
	}
	resetpacote(&pacote);
}

void linhas(uint *seq, int soquete){

	enviarmensagemfacil(SERVER,CLIENT,0,seq,LINHAS,NULL,soquete);

	char buffer[500];
	unsigned int linhas_inicio, linhas_fim;
	scanf("%u %u",&linhas_inicio,&linhas_fim);
	
	
	unsigned char buffer_linhas[9];
	for (int i = 0; i < 4; i++){
		buffer_linhas[3-i] = linhas_inicio >> (8*i);
		buffer_linhas[8-i] = linhas_fim >> (8*i);
		//printf("%x, %x\n",buffer_linhas[3-i],buffer_linhas[8-i]);
	}
	buffer_linhas[4] = ' ';
	buffer_linhas[9] = '\0';

	unsigned int linha_inicio = 0;
    unsigned int linha_fim = 0;
    for (int i = 0; i < 4; i++){
		linha_inicio += (unsigned int) (buffer_linhas[i] << (8*(3-i)));
		linha_fim += (unsigned int) (buffer_linhas[i+5] << (8*(3-i)));
	}
   
	enviarmensagemfacil(SERVER,CLIENT,9,seq,LIN,(char*) buffer_linhas,soquete);
	//printf("%i mensagens // %i resto\n buffer: %s\n", contmsg, restomsg, buffer );
	scanf("%s",buffer);

	enviastringfacil(buffer, SERVER, CLIENT, seq, LINHAS, soquete);

	Pacote_legivel pacote;
	iniciapacote(&pacote);
	if(recebepacote(&pacote,CLIENT, soquete) < 0){
		exit(-1);
	}		
    if(pacote_tipo(&pacote, ERRO)){
		printError(&pacote);
		incrementaSeq(seq);
    }
	else{
		char buffer2[2000] = "";
	    for (int i = 0; i < pacote.tam; i++){
	        buffer2[i] = pacote.data[i];
	    }
	    int contador = pacote.tam;
	    //printf("contador = %i\n",contador );
	    enviaACK(pacote.origem, pacote.destino, seq, soquete);
	    while( !pacote_tipo(&pacote, EOT)){
	        resetpacote(&pacote);
	        if( recebepacote(&pacote, CLIENT, soquete) < 0 )
	            exit(-1);

	        for (int i = 0; i < pacote.tam; i++){
	            buffer2[contador+i] = pacote.data[i];
	        }
	        contador += pacote.tam;
	        //printf("contador = %i\n",contador );
	        enviaACK(pacote.origem, pacote.destino, seq, soquete);
	    }    
	    printf("%s",buffer2);
	}
	resetpacote(&pacote);


}	


void linha(uint *seq, int soquete){

	enviarmensagemfacil(SERVER,CLIENT,0,seq,LINHA,NULL,soquete);

	char buffer[500];
	unsigned int linhas_inicio, linhas_fim;

	scanf("%u",&linhas_inicio);
	linhas_fim = linhas_inicio;
	
	unsigned char buffer_linhas[9];
	for (int i = 0; i < 4; i++){
		buffer_linhas[3-i] = linhas_inicio >> (8*i);
		buffer_linhas[8-i] = linhas_fim >> (8*i);
		//printf("%x, %x\n",buffer_linhas[3-i],buffer_linhas[8-i]);
	}
	buffer_linhas[4] = ' ';
	buffer_linhas[9] = '\0';

	unsigned int linha_inicio = 0;
    unsigned int linha_fim = 0;
    for (int i = 0; i < 4; i++){
		linha_inicio += (unsigned int) (buffer_linhas[i] << (8*(3-i)));
		linha_fim += (unsigned int) (buffer_linhas[i+5] << (8*(3-i)));
	}
   
	enviarmensagemfacil(SERVER,CLIENT,9,seq,LIN,(char*)buffer_linhas,soquete);
	//printf("%i mensagens // %i resto\n buffer: %s\n", contmsg, restomsg, buffer );
	scanf("%s",buffer);

	enviastringfacil(buffer, SERVER, CLIENT, seq, LINHA, soquete);

	Pacote_legivel pacote;
	iniciapacote(&pacote);
	if(recebepacote(&pacote,CLIENT, soquete) < 0){
		exit(-1);
	}		
    if(pacote_tipo(&pacote, ERRO)){
		printError(&pacote);
		incrementaSeq(seq);
    }
	else{
		char buffer2[2000] = "";
	    for (int i = 0; i < pacote.tam; i++){
	        buffer2[i] = pacote.data[i];
	    }
	    int contador = pacote.tam;
	    //printf("contador = %i\n",contador );
	    enviaACK(pacote.origem, pacote.destino, seq, soquete);
	    while( !pacote_tipo(&pacote, EOT)){
	        resetpacote(&pacote);
	        if( recebepacote(&pacote,CLIENT, soquete) < 0 )
	            exit(-1);

	        for (int i = 0; i < pacote.tam; i++){
	            buffer2[contador+i] = pacote.data[i];
	        }
	        contador += pacote.tam;
	        //printf("contador = %i\n",contador );
	        enviaACK(pacote.origem, pacote.destino, seq, soquete);
	    }    
	    printf("%s",buffer2);
	}
	resetpacote(&pacote);


}	


void edit(uint *seq, int soquete){

	enviarmensagemfacil(SERVER,CLIENT,0,seq,EDIT,NULL,soquete);

	char buffer[500];
	unsigned int linhas_inicio, linhas_fim;
	scanf("%u",&linhas_inicio);
	linhas_fim = linhas_inicio;

	unsigned char buffer_linhas[9];
	for (int i = 0; i < 4; i++){
		buffer_linhas[3-i] = linhas_inicio >> (8*i);
		buffer_linhas[8-i] = linhas_fim >> (8*i);
		//printf("%x, %x\n",buffer_linhas[3-i],buffer_linhas[8-i]);
	}
	buffer_linhas[4] = ' ';
	buffer_linhas[9] = '\0';

	enviarmensagemfacil(SERVER,CLIENT,9,seq,LIN,(char*)buffer_linhas,soquete);

	scanf("%[^\n]",buffer);
	enviastringfacil(buffer, SERVER, CLIENT, seq, EDIT, soquete);

	Pacote_legivel pacote;
	iniciapacote(&pacote);
	if(recebepacote(&pacote,CLIENT, soquete) < 0){
		exit(-1);
	}		
    if(pacote_tipo(&pacote, ERRO)){
		printError(&pacote);
		incrementaSeq(seq);
    }
	resetpacote(&pacote);

}

void compila(uint *seq, int soquete){

	char buffer[500];

	scanf("%[^\n]",buffer);
	enviastringfacil(buffer, SERVER, CLIENT, seq, COMPILAR, soquete);

	Pacote_legivel pacote;
	iniciapacote(&pacote);
	if(recebepacote(&pacote,CLIENT, soquete) < 0){
		exit(-1);
	}		
    if(pacote_tipo(&pacote, ERRO)){
		printError(&pacote);
		incrementaSeq(seq);
    }
	else{
		char buffer2[10000] = "";
	    for (int i = 0; i < pacote.tam; i++){
	        buffer2[i] = pacote.data[i];
	    }
	    int contador = pacote.tam;
	    enviaACK(pacote.origem, pacote.destino, seq, soquete);
	    while( !pacote_tipo(&pacote, EOT)){
	        resetpacote(&pacote);
	        if( recebepacote(&pacote, CLIENT, soquete) < 0 )
	            exit(-1);

	        for (int i = 0; i < pacote.tam; i++){
	            buffer2[contador+i] = pacote.data[i];
	        }
	        contador += pacote.tam;

	        enviaACK(pacote.origem, pacote.destino, seq, soquete);
	    }    
	    printf("%s",buffer2);
	}
	resetpacote(&pacote);

}	