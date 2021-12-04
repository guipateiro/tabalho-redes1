#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "ConexaoRawSocket.h"
#include "protocolo.h"
#include "funcoes_server.h"

int main(){

	char *device = "lo";
	int soquete;

	soquete = ConexaoRawSocket(device);

	Pacote_legivel pacote;
	uint seq = 16;
	iniciapacote(&pacote);
	while(seq > 15){
		if(recebepacote(&pacote, SERVER, soquete) < 0)
			exit(-1);  
		seq = pacote.seq;
  	}  
  	while(1) {
		switch (pacote.tipo){
	  		case 0:
				cd_server(&pacote, &seq, soquete);
	  		break;

		  	case 1:
				ls_server(&pacote, &seq, soquete);
		 	break;
	   
		  	case 2:
				ver_server(&pacote, &seq, soquete);
		  	break;      

		  	case 3:
				linha_server(&pacote, &seq, soquete);
		  	break;
		   
		  	case 4:
				linhas_server(&pacote, &seq, soquete);
		  	break;
		   
		  	case 5:
				edit_server(&pacote, &seq, soquete);
		  	break;
		   
		  	case 6:
				compila_server(&pacote, &seq, soquete);
		  	break;
		}
		resetpacote(&pacote);

		if(recebepacote(&pacote, SERVER, soquete) < 0)
	  		exit(-1);
  	}
  	return 0;
}