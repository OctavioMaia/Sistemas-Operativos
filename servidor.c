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

#define MAX 512

void kill_process(int pid){
	kill(pid, SIGCONT);
	kill(pid, SIGUSR2);
}

void doDigest(char *ficheiro){
	int pipefd[2];
	int flag = 0;
	int n,i;
	char buf[MAX], file[MAX], digest[MAX], aux[MAX];
	pipe(pipefd);

	if(fork()==0){
		printf("cenas\n");
		dup2(pipefd[1], 1);
		close(pipefd[1]);
		
		execlp("sha1sum", "sha1sum", ficheiro, NULL);
		
		printf("executei sha1sum\n");
		_exit(1);
	}else{
		close(pipefd[1]);
		wait(&flag);
		
		if (WEXITSTATUS(flag) == 0) {
		    n = (int) read(pipefd[0], buf, MAX);

		    close(pipefd[0]);

            strcpy(digest, strtok(buf, " "));
		    strcpy(aux, strtok(NULL, "\r\n"));
		    printf("aux %s\n",aux );
		    
		    for (; i < strlen(aux); i++)
			    file[i - 1] = aux[i];
		    
		    //backup(digest, file, pid);
		    //createMetadata(file, digest, pid);
	    }
	}
}

int main(int argc, char* argv[]){
	char *utilizador, PATH[MAX], PATH_DATA[MAX], PATH_META[MAX], PATH_ATUAL[1024], FICHEIRO[MAX], DECISAO[MAX], ch;
	char resposta[MAX], pedido[MAX];
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

	//getcwd(PATH_ATUAL, sizeof(PATH_ATUAL));
	fd_pedido=open("pipe_pedido",O_RDONLY);

	read(fd_pedido,resposta,MAX);

	pid = atoi(strtok(resposta, " "));
    strcpy(DECISAO, strtok(NULL, " "));
    strcpy(FICHEIRO, strtok(NULL, "\n"));

	//printf("Comando: %s Ficheiro: %s",DECISAO,FICHEIRO);

	if(strcmp(DECISAO,"backup")==0){
		printf("entrei backup\n");
		doDigest(FICHEIRO);
		kill_process(pid);
	}else if(strcmp(DECISAO,"restore")==0){
		printf("restore\n");
	}else{
		printf("comando invalido\n");
	}

	close(fd_pedido);

	return 0;
}