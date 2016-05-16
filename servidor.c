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
	char *utilizador, PATH[100], PATH_DATA[100], PATH_META[100], PATH_ATUAL[1024], FICHEIRO[100], DECISAO[100], ch;
	char resposta[100], pedido[100];
	int fd_resposta=0, fd_pedido=0, pid, i;
	struct stat fileStat;

	utilizador = (char *)getenv("USER");
	char* command, file;


	sprintf(PATH, "/home/%s/.Backup",utilizador);
	sprintf(PATH_DATA,"/home/%s/.Backup/data",utilizador);
	sprintf(PATH_META,"/home/%s/.Backup/metadata",utilizador);

	if (stat(PATH, &fileStat) == -1)
    	mkdir(PATH, 0700);
	if (stat(PATH_DATA, &fileStat) == -1)
    	mkdir(PATH_DATA, 0700);
	if (stat(PATH_META, &fileStat) == -1)
    	mkdir(PATH_META, 0700);

 	mkfifo("pipe_pedido",0666);
 	mkfifo("pipe_resposta",0666);

	//getcwd(PATH_ATUAL, sizeof(PATH_ATUAL));
	fd_pedido=open("pipe_pedido",O_RDONLY);
	fd_resposta=open("pipe_resposta",O_WRONLY);

	read(fd_pedido,resposta,100);
	pid = atoi(strtok(resposta, " "));
    strcpy(DECISAO, strtok(NULL, " "));
    strcpy(FICHEIRO, strtok(NULL, "\r\n"));

	//printf("Comando: %s Ficheiro: %s",DECISAO,FICHEIRO);

	if(strcmp(DECISAO,"backup")==0){
		printf("entrei backup\n");
		doDigest(FICHEIRO);
	}else if(strcmp(DECISAO,"restore")==0){
		printf("restore\n");
	}else{
		printf("comando invalido\n");
	}

	close(fd_resposta);
	close(fd_pedido);

	return 0;
}