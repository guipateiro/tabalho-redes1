CFLAGS = -Wall
CC = gcc

MAIN = client server
 
OBJS = ConexaoRawSocket.o kermitProtocol.o

 # regra default
all: $(MAIN)
 
# regras de ligacao
client: $(OBJS) funcoes_cliente.o client_main.c
	$(CC) $(OBJS) funcoes_cliente.o client_main.c -o client
server: $(OBJS) funcoes_server.o server_main.c
	$(CC) $(OBJS) funcoes_server.o server_main.c -o server

# regras de compilação
ConexaoRawSocket.o: ConexaoRawSocket.c ConexaoRawSocket.h
	$(CC) -c $< 
kermitProtocol.o: kermitProtocol.c kermitProtocol.h
	$(CC) -c $<
funcoes_cliente.o: funcoes_cliente.c funcoes_cliente.h
	$(CC) -c $<
funcoes_server.o: funcoes_server.c funcoes_server.h
	$(CC) -c $<

# compila com flags de depuração
debug: CFLAGS += -DDEBUG -g
debug: all

# remove arquivos temporários
clean:
	-rm -f *.o

# remove tudo o que não for o código-fonte
purge: clean
	-rm -f $(MAIN)