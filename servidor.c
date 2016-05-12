#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/uio.h>
#include <signal.h>
#include <stdio.h>
#include <stddef.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char* argv[]){
	char *utilizador, PATH[100], PATH_DATA[100], PATH_META[100], PATH_ATUAL[1024], INPUT[1024], ch;
	int fd, i;
	struct stat fileStat;

	utilizador = (char *)getenv("USER");
	sprintf(PATH, "/home/%s/.Backup",utilizador);
	sprintf(PATH_DATA,"/home/%s/.Backup/data",utilizador);
	sprintf(PATH_META,"/home/%s/.Backup/metadata",utilizador);

	if (stat(PATH, &fileStat) == -1)
    	mkdir(PATH, 0700);
	if (stat(PATH_DATA, &fileStat) == -1)
    	mkdir(PATH_DATA, 0700);
	if (stat(PATH_META, &fileStat) == -1)
    	mkdir(PATH_META, 0700);


 	mkfifo("fifo",0666);
	getcwd(PATH_ATUAL, sizeof(PATH_ATUAL));
	fd=open("fifo",O_RDONLY);

	while(ch!='\n' && read(fd,&ch,1)!=0){
		INPUT[i]=ch;
		i++;
	}
	INPUT[i]='\0';
	printf("Comando: %s\n",INPUT);   

	close(fd);

	if(strcmp(INPUT,"backup")==0){
		printf("backup\n");
	}else if(strcmp(INPUT,"restore")==0){
		printf("restore\n");
	}else{
		printf("comando invalido\n");
	}

	return 0;
}