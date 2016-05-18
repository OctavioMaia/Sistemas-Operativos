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

char* reverse(char* palavra){
	char temp;
   	int i=0, j = strlen(palavra)-1;

   	while (i < j) {
      	temp = palavra[i];
      	palavra[i] = palavra[j];
      	palavra[j] = temp;
      	i++;
      	j--;
   	}
   	return palavra;
}

void backup(char * FICHEIRO, char* PATH, char *PATH_DATA, char * PATH_META){
	int fd[2];
	int flag = 0;
	int fp_file = 0;
	int fp_digest = 0;
	int status;
	FILE *fil;
	int n,i,fdCod,xx,a=0,encontrei=0;
	char buf[MAX], file[MAX], digest[MAX], * aux, divisao[1024], cod[128],codigo[1024], * tok,fichiroajuda[1024], *nameFile,cwd[1024],diretFicheiro[1024],dataFicheiro[1024],metaFicheiro[1024],* comparar;
	char *string;
	pipe(fd);

	sprintf(codigo,"sha1sum %s",FICHEIRO);
	printf("dadad%s\n",codigo);
	fil=popen(codigo,"r");
	fdCod=fileno(fil);
	read(fdCod,cod,128);
	close(fdCod);
	pclose(fil);
	getcwd(cwd, sizeof(cwd));

	aux=(char *) strdup(cod);
	nameFile=strsep(&aux," \t");
	printf("nome do ficheiro %s\n",nameFile);

	sprintf(diretFicheiro,"%s/%s",cwd,FICHEIRO);
	sprintf(dataFicheiro,"%s/%s",PATH_DATA,nameFile);
	sprintf(metaFicheiro,"%s/%s",PATH_META,FICHEIRO);


	if(access(diretFicheiro, F_OK) == 0){
		if(access(metaFicheiro,F_OK) == 0){
			sprintf(codigo,"ls -l %s",metaFicheiro);
			fil=popen(codigo,"r");
			fdCod=fileno(fil);
			read(fdCod,cod,1024);
			close(fdCod);
			pclose(fil);
			aux=(char *) strdup(cod);
			n = strlen(aux)-2;
			while(encontrei==0){
				if(aux[n]!='/')divisao[a++]=aux[n--];
				else{
					encontrei=1;
				}
			}
			string = reverse(divisao);
			if(strcmp(string,nameFile)!=0){
				sprintf(fichiroajuda,"%s/%s",PATH_DATA,nameFile);
				if(access(dataFicheiro,F_OK) == -1){
					if(xx=(fork()==0)) {
						fp_file = open(FICHEIRO, O_RDONLY,0664);
					   
						dup2(fp_file,0);
						close(fp_file);
					   
						fp_digest = open(fichiroajuda, O_CREAT | O_WRONLY, 0664);
					
						dup2(fp_digest, 1);
						close(fp_digest);
						    
						execlp("gzip", "gzip", NULL);
						_exit(1);
					}
					else{
						waitpid(xx,&status,0);
					}
				}
				if(xx=(fork() == 0)) {        	
					execlp("rm", "rm", metaFicheiro, NULL);
					_exit(1);
				}
				else{
					waitpid(xx,&status,0);
					if(xx=(fork() == 0)) {        	
						execlp("ln", "ln", "-s", fichiroajuda,metaFicheiro, NULL);
						_exit(1);
					}
					else{
						waitpid(xx,&status,0);
					}
					printf("sucesso\n");
				}
			}
			else{
				printf("insucesso\n");
			}
		}
		else{
			sprintf(fichiroajuda,"%s/%s",PATH_DATA,nameFile);
			if(access(dataFicheiro,F_OK) == -1){
				sprintf(fichiroajuda,"%s/%s",PATH_DATA,nameFile);
				if(xx=(fork()==0)) {
					fp_file = open(FICHEIRO, O_RDONLY);
				   
					dup2(fp_file,0);
					close(fp_file);
				   
					fp_digest = open(fichiroajuda, O_CREAT | O_WRONLY, 0664);
				
					dup2(fp_digest, 1);
					close(fp_digest);	       
					    
					execlp("gzip", "gzip", NULL);
					_exit(1);
				}
				else{
					waitpid(xx,&status,0);
				}
			}
			if(xx=(fork() == 0)) {        	
				execlp("ln", "ln", "-s", fichiroajuda,metaFicheiro, NULL);
				_exit(1);
			}
			else{
				waitpid(xx,&status,0);
			}
			printf("sucesso\n");
		}
	}
	else{
		printf("insucesso\n");
	}
}

int main(int argc, char* argv[]){
	char *utilizador, PATH[MAX], PATH_DATA[MAX], PATH_META[MAX], PATH_ATUAL[1024], FICHEIRO[MAX], DECISAO[MAX], FIFO_PATH[MAX], ch;
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


    sprintf(FIFO_PATH,"%s/fifo",PATH);
 	mkfifo(FIFO_PATH,0666);

	//getcwd(PATH_ATUAL, sizeof(PATH_ATUAL));
	fd_pedido=open(FIFO_PATH,O_RDONLY);

	while(read(fd_pedido,&ch,1)!=0 && ch!='\n'){
		resposta[i]=ch;
		i++;
	}
	resposta[i]='\0';

	pid = atoi(strtok(resposta, " "));
    strcpy(DECISAO, strtok(NULL, " "));
    strcpy(FICHEIRO, strtok(NULL, " \r\n"));

	//printf("Comando: %s Ficheiro: %s",DECISAO,FICHEIRO);

	if(strcmp(DECISAO,"backup")==0){
		backup(FICHEIRO,PATH,PATH_DATA,PATH_META);
	}else if(strcmp(DECISAO,"restore")==0){
		printf("restore\n");
	}else{
		printf("comando invalido\n");
	}
	close(fd_pedido);

	return 0;
}