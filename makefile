CC = gcc

clean: 
	rm cliente
	rm servidor
	rm fifo

servidor:
	$(CC) -o servidor servidor.c

cliente:
	$(CC) -o cliente cliente.c

exec_servidor:
	./servidor

exec_cliente:
	./cliente