#ifndef __FUNCOES_CLIENTE__
#define __FUNCOES_CLIENTE__

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

void pwd(char* resposta);

void lcd();

void lls();

void cd(int *seq, int soquete);

void ls(int *seq, int soquete);

void ver(int *seq, int soquete);

void linhas(int *seq, int soquete);

void linha(int *seq, int soquete);

void edit(int *seq, int soquete);

#endif