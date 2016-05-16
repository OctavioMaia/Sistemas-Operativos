#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>
#include <ctype.h>
#include "rd.h"


#define SIZE 512
#define MAX_LINE 256


char path[SIZE];


static void updatePath(char *aux, const char *folder, const char *file)
{
	strcpy(aux, path);
	strcat(aux, folder);
	strcat(aux, file);
}


static void restore(const char *file)
{
	int fp1 = 0;
	int fp2 = 0;
	char aux[SIZE] = "";
	char aux2[SIZE] = "";	    
	    
	if (fork() == 0) {
		updatePath(aux, "metadata/", file);
		   
		fp1 = open(aux, O_RDONLY);
		
		dup2(fp1, 0);
		close(fp1);

		strcat(aux2, file);
		strcat(aux2, " DESCOMPRIMIDO");
		   
		fp2 = open(aux2, O_CREAT | O_WRONLY, 0664);
			
		dup2(fp2, 1);
		close(fp2);
			
		execlp("gunzip", "gunzip", NULL);

		perror("Falhou o gunzip!\n");
		_exit(1);
	}
	else {
		wait(NULL);
	}
}


static void backup(const char *digest, const char *file)
{
	int fp_file = 0;
	int fp_digest = 0;
	char aux_digest[SIZE] = "";

	if (fork() == 0) {          
		fp_file = open(file, O_RDONLY);
        
		dup2(fp_file, 0);
		close(fp_file);

		updatePath(aux_digest, "data/", digest);
        
		fp_digest = open(aux_digest, O_CREAT | O_WRONLY, 0664);
		
		dup2(fp_digest, 1);
		close(fp_digest);	       
	    
		execlp("gzip", "gzip", NULL);

		perror("Falhou o gzip!\n");
		_exit(1);
	}
	else {
		wait(NULL);
	}
}


static void createMetadata(const char *file, const char *digest)
{
	char path_file[SIZE] = "";
	char path_digest[SIZE] = "";
        
	if(fork() == 0) {        
		updatePath(path_file, "metadata/", file);
		updatePath(path_digest, "data/", digest);
	
		execlp("ln", "ln", "-s", path_digest, path_file, NULL);

		printf("Falhou o ln\n");
		_exit(1);
	}
	else {
		wait(NULL);             
	}
}


static void createDigest(const char *str, int pid)
{
	int pfd[2];
	char buffer[MAX_LINE] = "";
	char digest[MAX_LINE] = "";
	char aux[MAX_LINE] = "";
	char file[MAX_LINE] = "";
	int n = 0;
	unsigned int i = 1;
	int status = 0;

	pipe(pfd);
	
	if (fork() == 0) {
		dup2(pfd[1], 1);
		close(pfd[1]);
		
		execlp("sha1sum", "sha1sum", str, NULL);
		
		_exit(1);
	}
	else {
		close(pfd[1]);
		wait(&status);
		
		if (WEXITSTATUS(status) == 0) {

		    n = (int) read(pfd[0], buffer, MAX_LINE);

		    close(pfd[0]);

           	strcpy(digest, strtok(buffer, " "));
		    strcpy(aux, strtok(NULL, "\r\n"));
        
		    for (; i < strlen(aux); i++) {
			    file[i - 1] = aux[i];
		    }

		    backup(digest, file);
			
		    createMetadata(file, digest);  
	    }  
	    else {
	        kill(pid, SIGUSR2);
	    }
    }
}


static void calcDest(void)
{
	strcpy(path, getenv("HOME"));
        
	strcat(path, "/.Backup/");
}


int main(void)
{
	int fd_response = 0;
	char response[SIZE] = "";
	int fd_request = 0;
	char request[SIZE] = "";
	char command[SIZE] = "";
	char file[SIZE] = "";        
	char aux_response[SIZE] = "";
	char aux_request[SIZE] = "";
	int pid = 0;

	calcDest();

	updatePath(aux_response, "", "p_response");
	mkfifo(aux_response, 0666);

	updatePath(aux_request, "", "p_request");
	mkfifo(aux_request, 0666);
        
	fd_request = open(aux_request, O_RDONLY);
    fd_response = open(aux_response, O_WRONLY);    
	
	read(fd_request, request, SIZE);
	
    pid = atoi(strtok(request, " "));
    strcpy(command, strtok(NULL, " "));
    strcpy(file, strtok(NULL, "\r\n"));

	if (strcmp(command, "backup") == 0) {
		//strcpy(file, strtok(NULL, "\r\n"));
        
		createDigest(file, pid);

		sprintf(response, "%s: copiado\n", file);
		write(fd_response, response, SIZE);

	}
	else if (strcmp(command, "restore") == 0) {
		//strcpy(file, strtok(NULL, "\n\r"));
        
		restore(file);
        
		sprintf(response, "%s: restaurado\n", file);
		write(fd_response, response, SIZE);
	}
	else {
		sprintf(response, "Comando e/ou ficheiro inválido(s)\n");
		write(fd_response, response, SIZE);
	}    

	close(fd_response);
	close(fd_request);

	return 0;
}

