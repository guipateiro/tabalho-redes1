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
#include "protocolo.h"
#include "funcoes_cliente.h"

void pwd(char* resposta);

void lcd();

void lls();

void cd(uint *seq, int soquete);

void ls(uint *seq, int soquete);

void ver(uint *seq, int soquete);

void linhas(uint *seq, int soquete);

void linha(uint *seq, int soquete);

void edit(uint *seq, int soquete);

void compila(uint *seq, int soquete);

#endif