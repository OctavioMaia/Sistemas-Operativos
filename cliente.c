#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/uio.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

int main(int argc, char* argv[]){
	char *utilizador;
	int fd_pedido,fd_resposta,x,p=0;
	char b,PATH[100];

	utilizador= (char *)getenv("USER");
	sprintf(PATH, "/home/%s/.Backup/fifo",utilizador);

	fd_pedido=open(PATH,O_WRONLY);
	while(read(0,&b,1)!=0 && b!='\n'){
		write(fd_pedido,&b,1);
	}
	close(fd_pedido);
}