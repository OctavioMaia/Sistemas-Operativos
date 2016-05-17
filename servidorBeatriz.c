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


#define SIZE 128
#define MAX_LINE 256


char path[SIZE];
int num_process;

static void replace(char *file_path, const char from, const char to)
{
	unsigned int i = 0;
	unsigned int size = strlen(file_path);

	for(; i < size; i++) {
		if(file_path[i] == from) {
			file_path[i] = to;
		}
	}
}

static void updatePath(char *aux, const char *folder, const char *file)
{
	strcpy(aux, path);
	strcat(aux, folder);
	strcat(aux, file);
}


static void restore(char *file, int pid)
{
	int fp1 = 0;
	int fp2 = 0;
	int status = 0;
	char pointer_path[SIZE] = "";
	char aux2[SIZE] = "";	    
	
	updatePath(pointer_path, "/.Backup/metadata/", file);


	if (access(pointer_path, F_OK) != -1) {	  
		if (fork() == 0) {
			fp1 = open(pointer_path, O_RDONLY);
		
			dup2(fp1, 0);
			close(fp1);

			replace(file, '>', '/');
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
			wait(&status);

			kill(pid, SIGCONT);

			if (WEXITSTATUS(status) == 0) {
				kill(pid, SIGUSR1);
			}
			else {
				kill(pid, SIGUSR2);
			}
		}
	}
	else {
		kill(pid, SIGCONT);
		kill(pid, SIGUSR2);
	}
}


static void backup(const char *digest, const char *file, int pid)
{
	int fp_file = 0;
	int fp_digest = 0;
	char aux_digest[SIZE] = "";
	int status = 0;

	
	if (access(file, F_OK) != -1) {
		if (fork() == 0) {          
			fp_file = open(file, O_RDONLY);
		   
			dup2(fp_file, 0);
			close(fp_file);

			updatePath(aux_digest, "/.Backup/data/", digest);
		   
			fp_digest = open(aux_digest, O_CREAT | O_WRONLY, 0664);
		
			dup2(fp_digest, 1);
			close(fp_digest);	       
		    
			execlp("gzip", "gzip", NULL);

			perror("Falhou o gzip!\n");
			_exit(1);
		}
		else {
			wait(&status);
		
			kill(pid, SIGCONT);

			if (WEXITSTATUS(status) != 0) {
				kill(pid, SIGUSR2);
			}
		}
	}
	else {
		kill(pid, SIGCONT);
		kill(pid, SIGUSR2);
	}
}


static void createMetadata(char *file, const char *digest, int pid)
{
	char path_file[SIZE] = "";
	char path_digest[SIZE] = "";
	int status = 0;
	
	replace(file,'/','>');

	
	updatePath(path_file, "/.Backup/metadata/", file);
	updatePath(path_digest, "/.Backup/data/", digest);        

	
	if (access(path_file, F_OK) == -1) { 	
		if(fork() == 0) {        
			execlp("ln", "ln", "-s", path_digest, path_file, NULL);
			_exit(1);
		}
		else {
			wait(&status);

			kill(pid, SIGCONT);			             

			if (WEXITSTATUS(status) == 0) {			
				kill(pid, SIGUSR1);
			}
			else {
				kill(pid, SIGUSR2);
			}			
		}
	}
	else {
		kill(pid, SIGCONT);
		kill(pid, SIGUSR2);
	}
}


static void createDigest(char *str, int pid)
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
	replace(str,'>','/');
	
	if (access(str, F_OK) != -1) {
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

			    backup(digest, file, pid);
			
			    createMetadata(file, digest, pid);  
		    }  
		    else {
				kill(pid, SIGCONT);			
				kill(pid, SIGUSR2);
		    }
	    }
	}
	else {
		kill(pid, SIGCONT);
		kill(pid, SIGUSR2);
	}
}


static void calcDest(void)
{
	strcpy(path, getenv("HOME"));
}


static void decreaseProcess()
{
	num_process--;
}


int main(void)
{
	int fd_request = 0;
	int n = 0;
	char request[SIZE] = "";
	char command[SIZE] = "";
	char file[SIZE] = "";        
	char aux_request[SIZE] = "";
	int pid = 0;

	num_process = 0;
	calcDest();
       
	strcpy(aux_request, path);
	strcat(aux_request, "/.Backup/");
	strcat(aux_request, "p_request");

	mkfifo(aux_request, 0666);
        
	fd_request = open(aux_request, O_RDONLY); 

	signal(SIGCHLD, decreaseProcess);

	while (1) {
		if (num_process < 5) {

			num_process++;

			if (fork() == 0) {
				read(fd_request, request, SIZE);
				pid = atoi(strtok(request, " "));
				strcpy(command, strtok(NULL, " "));
				strcpy(file, strtok(NULL, "\n"));

				if (strcmp(command, "backup") == 0) {  
					createDigest(file, pid);
				}
				else if (strcmp(command, "restore") == 0) {
					restore(file, pid);
				}	

				_exit(0);			
			}
		}
	}

	close(fd_request);

	return 0;
}

