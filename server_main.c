#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "ConexaoRawSocket.h"
#include "kermitProtocol.h"
#include "funcoes_server.h"

int main()
{
  char *device = "lo";
  int soquete;

  soquete = ConexaoRawSocket(device);

  kermitHuman package;
  int seq = 0;
  iniciaPackage(&package);

  while(1) {

    resetPackage(&package);

    if( receivePackage(&package, 2, soquete) < 0 )
      exit(-1);
   
    switch (package.tipo){//  Verifica o comando dado pelo usuário
      case 0:
        comando_cd(&package, &seq, soquete);
      break;

      case 1:
        comando_ls(&package, &seq, soquete);
      break;
       
      case 2:
        comando_ver(&package, &seq, soquete);
      break;      

      case 3:
        comando_linha(&package, &seq, soquete);
      break;
       
      case 4:
        comando_linhas(&package, &seq, soquete);
      break;
       
      case 5:
        //comando_edit(&package, &seq, soquete);
      break;
       
      case 6:
        //compilar(&package, &seq, soquete);
      break;
    }
  }
  return 0;
}