#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/uio.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>

int main(int argc, char* argv[]){
	int fd_pedido,fd_resposta,x;
	char b;
	fd_pedido=open("pipe_pedido",O_WRONLY);
	while(read(0,&b,1)!=0 && b!='\n'){
		write(fd_pedido,&b,1);
	}
	close(fd_pedido);
	fd_resposta=open("pipe_resposta",O_RDONLY);
	while(read(fd_resposta,&b,1)!=0 && b!='\0'){
		write(1,&b,1);
	}
	close(fd_resposta);
}