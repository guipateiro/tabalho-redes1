#ifndef __FUNCOES_SERVER__
#define __FUNCOES_SERVER__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "ConexaoRawSocket.h"
#include "protocolo.h"
#include "funcoes_server.h"

void pwd(char* resposta);

void cd_server(Pacote_legivel *pacote, uint *seq, int soquete);

void ls_server(Pacote_legivel *pacote, uint *seq, int soquete);

void ver_server(Pacote_legivel *pacote, uint *seq, int soquete);

void linha_server(Pacote_legivel *pacote, uint *seq, int soquete);

void linhas_server(Pacote_legivel *pacote, uint *seq, int soquete);

void edit_server(Pacote_legivel *pacote, uint *seq, int soquete);

void compila_server(Pacote_legivel *pacote, uint *seq, int soquete);

#endif