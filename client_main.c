#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "ConexaoRawSocket.h"
#include "protocolo.h"
#include "funcoes_cliente.h"

int main(){

    char *device = "lo";
    int soquete;

    soquete = ConexaoRawSocket(device);

    char diretorio[TAM_DIRETORIO];
    char comando[9] = "";

    uint seq = 0;

    while(1){  
        pwd(diretorio);
        printf("client:%s > ", diretorio);    
        scanf("%s", comando);

        if(strncmp(comando,"cd", 2) == 0){
            cd(&seq, soquete);
        }
        else if(strncmp(comando,"lcd", 3) == 0){
            lcd();      
        }
        else if(strncmp(comando,"ls", 2) == 0){
            ls(&seq, soquete);
        } 
        else if(strncmp(comando,"lls", 3) == 0){
            lls(); 
        }
        else if(strncmp(comando,"ver", 3) == 0){
            ver(&seq, soquete);
        }
        else if(strncmp(comando,"linhas", 6) == 0){
            linhas(&seq, soquete);
        } 
        else if(strncmp(comando,"linha", 5) == 0){
            linha(&seq, soquete);
        } 
        else if(strncmp(comando,"edit", 4) == 0){
            edit(&seq, soquete);
        } 
        else if(strncmp(comando,"compilar", 7) == 0){
            compila(&seq, soquete);
        }
        else{
            printf("%s: comando desconhecido\n", comando);
        }

    }

  return 0;
}