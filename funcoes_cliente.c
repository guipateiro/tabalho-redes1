#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>           
#include <dirent.h>         
#include <unistd.h>          
#include "ConexaoRawSocket.h"
#include "kermitProtocol.h"
#include "funcoes_cliente.h"

#define CD 0
#define LS 1
#define	VER 2
#define LINHA 3
#define LINHAS 4
#define EDIT 5
#define COMPILAR 6
#define ACK 8
#define NACK 9 
#define LIN 10
#define	CONLS 11
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

void cd(int *seq, int soquete){
	char buffer[500];
	scanf("%s",buffer);
	int tamanho = strlen(buffer);
	int contmsg = (tamanho/15);
	int restomsg = tamanho % 15;
	//printf("%i mensagens // %i resto\n buffer: %s\n", contmsg, restomsg, buffer );
	for (int i = 0; i < contmsg; i++){
		char data[15] = "";
		for (int j = 0; j < 15; j++){
			data[j] = buffer[(15*i)+j];
		}
		data[15] = '\0';
		printf("I: %i data:%s\n",i, data);
		enviarmensagemfacil(soquete,SERVER,CLIENT,15,seq,CD,data);
	}
	char data[15] = "";
	for (int j = 0; j < restomsg; j++){
		data[j] = buffer[15*contmsg+j];
	}
	printf("data:%s \n",data);
	enviarmensagemfacil(soquete, SERVER,CLIENT,restomsg,seq,CD,data);

	enviaEOT(SERVER,CLIENT,seq,soquete);

	kermitHuman package;
	iniciaPackage(&package);
	if(receivePackage(&package,CLIENT, soquete) < 0){
		exit(-1);
	}		
    if(ehPack(&package, ERROR)){
		printError(&package);
		incrementaSeq(seq);
    }
	resetPackage(&package);

}


void ls(int *seq, int soquete){

	enviarmensagemfacil(soquete,SERVER,CLIENT,0,seq,LS,NULL);
	kermitHuman package;
	iniciaPackage(&package);

	if( receivePackage(&package, CLIENT, soquete) < 0 )
            exit(-1);

	char buffer[2000] = "";
    for (int i = 0; i < package.tam; i++){
        buffer[i] = package.data[i];
    }
    int contador = package.tam;
    //printf("contador = %i\n",contador );
    sendACK(package.orig, package.dest, seq, soquete);
    while( !ehPack(&package, EOT)){
        resetPackage(&package);
        if( receivePackage(&package, CLIENT, soquete) < 0 )
            exit(-1);

        for (int i = 0; i < package.tam; i++){
            buffer[contador+i] = package.data[i];
        }
        contador += package.tam;
        //printf("contador = %i\n",contador );
        sendACK(package.orig, package.dest, seq, soquete);
    }    
    int coisa = strlen(buffer);
    printf("%i\n",coisa);
    printf("%s",buffer);

    resetPackage(&package);

}



void ver(int *seq, int soquete){

	char buffer[500];
	scanf("%s",buffer);
	int tamanho = strlen(buffer);
	int contmsg = (tamanho/15);
	int restomsg = tamanho % 15;
	//printf("%i mensagens // %i resto\n buffer: %s\n", contmsg, restomsg, buffer );
	for (int i = 0; i < contmsg; i++){
		char data[15] = "";
		for (int j = 0; j < 15; j++){
			data[j] = buffer[(15*i)+j];
		}
		data[15] = '\0';
		printf("I: %i data:%s\n",i, data);
		enviarmensagemfacil(soquete,SERVER,CLIENT,15,seq,VER,data);
	}
	char data[15] = "";
	for (int j = 0; j < restomsg; j++){
		data[j] = buffer[15*contmsg+j];
	}
	printf("data:%s \n",data);
	enviarmensagemfacil(soquete,SERVER,CLIENT,restomsg,seq,VER,data);

	enviaEOT(SERVER,CLIENT,seq,soquete);

	kermitHuman package;
	iniciaPackage(&package);
	if(receivePackage(&package,CLIENT, soquete) < 0){
		exit(-1);
	}		
    if(ehPack(&package, ERROR)){
		printError(&package);
		incrementaSeq(seq);
    }
	else{
		char buffer2[2000] = "";
	    for (int i = 0; i < package.tam; i++){
	        buffer2[i] = package.data[i];
	    }
	    int contador = package.tam;
	    //printf("contador = %i\n",contador );
	    sendACK(package.orig, package.dest, seq, soquete);
	    while( !ehPack(&package, EOT)){
	        resetPackage(&package);
	        if( receivePackage(&package, CLIENT, soquete) < 0 )
	            exit(-1);

	        for (int i = 0; i < package.tam; i++){
	            buffer2[contador+i] = package.data[i];
	        }
	        contador += package.tam;
	        //printf("contador = %i\n",contador );
	        sendACK(package.orig, package.dest, seq, soquete);
	    }    
	    int coisa = strlen(buffer2);
	    printf("%i\n",coisa);
	    printf("%s",buffer2);
	}
	resetPackage(&package);
}

void linhas(int *seq, int soquete){

	enviarmensagemfacil(soquete,SERVER,CLIENT,0,seq,LINHAS,NULL);

	char buffer[500];
	unsigned int linhas_inicio, linhas_fim;
	scanf("%u %u",&linhas_inicio,&linhas_fim);
	scanf("%s",buffer);
	int tamanho = strlen(buffer);
	int contmsg = (tamanho/15);
	int restomsg = tamanho % 15;

	unsigned char buffer_linhas[9];
	for (int i = 0; i < 4; i++){
		buffer_linhas[3-i] = linhas_inicio >> (8*i);
		buffer_linhas[8-i] = linhas_fim >> (8*i);
		printf("%x, %x\n",buffer_linhas[3-i],buffer_linhas[8-i]);
	}
	buffer_linhas[4] = ' ';
	buffer_linhas[9] = '\0';

	for (int i = 0; i < 9; i++){
		printf("%x",buffer_linhas[i]);
	}

	unsigned int linha_inicio = 0;
    unsigned int linha_fim = 0;
    for (int i = 0; i < 4; i++){
		linha_inicio += (unsigned int) (buffer_linhas[i] << (8*(3-i)));
		linha_fim += (unsigned int) (buffer_linhas[i+5] << (8*(3-i)));
	}
   
	enviarmensagemfacil(soquete,SERVER,CLIENT,9,seq,LIN,buffer_linhas);
	//printf("%i mensagens // %i resto\n buffer: %s\n", contmsg, restomsg, buffer );
	for (int i = 0; i < contmsg; i++){
		char data[15] = "";
		for (int j = 0; j < 15; j++){
			data[j] = buffer[(15*i)+j];
		}
		data[15] = '\0';
		printf("I: %i data:%s\n",i, data);
		enviarmensagemfacil(soquete,SERVER,CLIENT,15,seq,LINHAS,data);
	}
	char data[15] = "";
	for (int j = 0; j < restomsg; j++){
		data[j] = buffer[15*contmsg+j];
	}
	printf("data:%s \n",data);
	enviarmensagemfacil(soquete,SERVER,CLIENT,restomsg,seq,LINHAS,data);

	enviaEOT(SERVER,CLIENT,seq,soquete);

	kermitHuman package;
	iniciaPackage(&package);
	if(receivePackage(&package,CLIENT, soquete) < 0){
		exit(-1);
	}		
    if(ehPack(&package, ERROR)){
		printError(&package);
		incrementaSeq(seq);
    }
	else{
		char buffer2[2000] = "";
	    for (int i = 0; i < package.tam; i++){
	        buffer2[i] = package.data[i];
	    }
	    int contador = package.tam;
	    //printf("contador = %i\n",contador );
	    sendACK(package.orig, package.dest, seq, soquete);
	    while( !ehPack(&package, EOT)){
	        resetPackage(&package);
	        if( receivePackage(&package, CLIENT, soquete) < 0 )
	            exit(-1);

	        for (int i = 0; i < package.tam; i++){
	            buffer2[contador+i] = package.data[i];
	        }
	        contador += package.tam;
	        //printf("contador = %i\n",contador );
	        sendACK(package.orig, package.dest, seq, soquete);
	    }    
	    int coisa = strlen(buffer2);
	    printf("%i\n",coisa);
	    printf("%s",buffer2);
	}
	resetPackage(&package);


}	


void linha(int *seq, int soquete){

	enviarmensagemfacil(soquete,SERVER,CLIENT,0,seq,LINHA,NULL);

	char buffer[500];
	unsigned int linhas_inicio, linhas_fim;
	scanf("%u",&linhas_inicio);
	linhas_fim = linhas_inicio;
	scanf("%s",buffer);
	int tamanho = strlen(buffer);
	int contmsg = (tamanho/15);
	int restomsg = tamanho % 15;

	unsigned char buffer_linhas[9];
	for (int i = 0; i < 4; i++){
		buffer_linhas[3-i] = linhas_inicio >> (8*i);
		buffer_linhas[8-i] = linhas_fim >> (8*i);
		printf("%x, %x\n",buffer_linhas[3-i],buffer_linhas[8-i]);
	}
	buffer_linhas[4] = ' ';
	buffer_linhas[9] = '\0';

	for (int i = 0; i < 9; i++){
		printf("%x",buffer_linhas[i]);
	}

	unsigned int linha_inicio = 0;
    unsigned int linha_fim = 0;
    for (int i = 0; i < 4; i++){
		linha_inicio += (unsigned int) (buffer_linhas[i] << (8*(3-i)));
		linha_fim += (unsigned int) (buffer_linhas[i+5] << (8*(3-i)));
	}
   
	enviarmensagemfacil(soquete,SERVER,CLIENT,9,seq,LIN,buffer_linhas);
	//printf("%i mensagens // %i resto\n buffer: %s\n", contmsg, restomsg, buffer );
	for (int i = 0; i < contmsg; i++){
		char data[15] = "";
		for (int j = 0; j < 15; j++){
			data[j] = buffer[(15*i)+j];
		}
		data[15] = '\0';
		printf("I: %i data:%s\n",i, data);
		enviarmensagemfacil(soquete,SERVER,CLIENT,15,seq,LINHA,data);
	}
	char data[15] = "";
	for (int j = 0; j < restomsg; j++){
		data[j] = buffer[15*contmsg+j];
	}
	printf("data:%s \n",data);
	enviarmensagemfacil(soquete,SERVER,CLIENT,restomsg,seq,LINHA,data);

	enviaEOT(SERVER,CLIENT,seq,soquete);

	kermitHuman package;
	iniciaPackage(&package);
	if(receivePackage(&package,CLIENT, soquete) < 0){
		exit(-1);
	}		
    if(ehPack(&package, ERROR)){
		printError(&package);
		incrementaSeq(seq);
    }
	else{
		char buffer2[2000] = "";
	    for (int i = 0; i < package.tam; i++){
	        buffer2[i] = package.data[i];
	    }
	    int contador = package.tam;
	    //printf("contador = %i\n",contador );
	    sendACK(package.orig, package.dest, seq, soquete);
	    while( !ehPack(&package, EOT)){
	        resetPackage(&package);
	        if( receivePackage(&package,CLIENT, soquete) < 0 )
	            exit(-1);

	        for (int i = 0; i < package.tam; i++){
	            buffer2[contador+i] = package.data[i];
	        }
	        contador += package.tam;
	        //printf("contador = %i\n",contador );
	        sendACK(package.orig, package.dest, seq, soquete);
	    }    
	    int coisa = strlen(buffer2);
	    printf("%i\n",coisa);
	    printf("%s",buffer2);
	}
	resetPackage(&package);


}	


void edit(int *seq, int soquete){

	enviarmensagemfacil(soquete,SERVER,CLIENT,0,seq,EDIT,NULL);

	char buffer[500];
	char content[500];
	char imput[500];
	unsigned int linhas_inicio, linhas_fim;
	scanf("%u",&linhas_inicio);
	linhas_fim = linhas_inicio;

	unsigned char buffer_linhas[9];
	for (int i = 0; i < 4; i++){
		buffer_linhas[3-i] = linhas_inicio >> (8*i);
		buffer_linhas[8-i] = linhas_fim >> (8*i);
		printf("%x, %x\n",buffer_linhas[3-i],buffer_linhas[8-i]);
	}
	buffer_linhas[4] = ' ';
	buffer_linhas[9] = '\0';

	for (int i = 0; i < 9; i++){
		printf("%x",buffer_linhas[i]);
	}
   
	enviarmensagemfacil(soquete,SERVER,CLIENT,9,seq,LIN,buffer_linhas);

	scanf("%[^\n]",buffer);
	int tamanho = strlen(buffer);
	int contmsg = (tamanho/15);
	int restomsg = tamanho % 15;

	for (int i = 0; i < contmsg; i++){
		char data[15] = "";
		for (int j = 0; j < 15; j++){
			data[j] = buffer[(15*i)+j];
		}
		data[15] = '\0';
		printf("I: %i data:%s\n",i, data);
		enviarmensagemfacil(soquete,SERVER,CLIENT,15,seq,LINHA,data);
	}
	char data[15] = "";
	for (int j = 0; j < restomsg; j++){
		data[j] = buffer[15*contmsg+j];
	}
	printf("data:%s \n",data);
	enviarmensagemfacil(soquete,SERVER,CLIENT,restomsg,seq,LINHA,data);

	enviaEOT(SERVER,CLIENT,seq,soquete);

	kermitHuman package;
	iniciaPackage(&package);
	if(receivePackage(&package,CLIENT, soquete) < 0){
		exit(-1);
	}		
    if(ehPack(&package, ERROR)){
		printError(&package);
		incrementaSeq(seq);
    }
	resetPackage(&package);


}	