CFLAGS = -Wall
CC = gcc
MAIN = client server
OBJS = ConexaoRawSocket.o protocolo.o

all: $(MAIN)

# liga os arquivos intermediarios ao arquivo com o main
client: $(OBJS) funcoes_cliente.o client_main.c
	$(CC) $(CFLAGS) $(OBJS) funcoes_cliente.o client_main.c -o client
server: $(OBJS) funcoes_server.o server_main.c
	$(CC) $(CFLAGS) $(OBJS) funcoes_server.o server_main.c -o server

# compila os arquivos intermediarios
ConexaoRawSocket.o: ConexaoRawSocket.c ConexaoRawSocket.h
	$(CC) $(CFLAGS) -c $< 
protocolo.o: protocolo.c protocolo.h
	$(CC) $(CFLAGS) -c $<
funcoes_cliente.o: funcoes_cliente.c funcoes_cliente.h
	$(CC) $(CFLAGS) -c $<
funcoes_server.o: funcoes_server.c funcoes_server.h
	$(CC) $(CFLAGS) -c $<

# remove arquivos .o
clean:
	-rm -f *.o

# remove tudo, menos o codigo-fonte
purge: clean
	-rm -f $(MAIN)