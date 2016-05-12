#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/uio.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>

int main(int argc, char* argv[]){
	int fd,x;
	char b;
	fd=open("fifo",O_WRONLY);
	while(read(0,&b,1)!=0 && b!='\n'){
		write(fd,&b,1);
	}
	close(fd);
	fd=open("fifo",O_RDONLY);
	while(read(fd,&b,1)!=0 && b!='\0'){
		write(1,&b,1);
	}
	close(fd);
}