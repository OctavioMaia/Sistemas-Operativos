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

void backup(char * ficheiro, char* path, char *path_data, char * path_meta,int pid){
	int fd[2],n, fp_file=0,fp_digest=0,status,f,i,fdCod,xx,a=0,encontrei=0;
	FILE *fil;
	char  *string, *aux, divisao[1024], cod[128],codigo[1024],fichiroajuda[1024], *nameFile,cwd[1024],diretFicheiro[1024],dataFicheiro[1024],metaFicheiro[1024];

	pipe(fd);

	sprintf(codigo,"sha1sum %s",ficheiro);
	//printf("dadad%s\n",codigo);
	fil=popen(codigo,"r");
	fdCod=fileno(fil);
	read(fdCod,cod,128);
	close(fdCod);
	pclose(fil);
	getcwd(cwd, sizeof(cwd));

	aux=(char *) strdup(cod);
	nameFile=strsep(&aux," \t");
	printf("digest: %s\n",nameFile);

	sprintf(diretFicheiro,"%s/%s",cwd,ficheiro);
	sprintf(dataFicheiro,"%s/%s",path_data,nameFile);
	sprintf(metaFicheiro,"%s/%s",path_meta,ficheiro);


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
				if(aux[n]!='/'){
					divisao[a++]=aux[n--];
				}else{
					encontrei=1;
				}
			}
			string = reverse(divisao);
			if(strcmp(string,nameFile)!=0){
				sprintf(fichiroajuda,"%s/%s",path_data,nameFile);
				if(access(dataFicheiro,F_OK) == -1){
					if(xx=(fork()==0)) {
						fp_file = open(ficheiro, O_RDONLY,0664);
					   
						dup2(fp_file,0);
						close(fp_file);
					   
						fp_digest = open(fichiroajuda, O_CREAT | O_WRONLY, 0664);
					
						dup2(fp_digest, 1);
						close(fp_digest);
						    
						execlp("gzip", "gzip", NULL);
						_exit(1);
					}else{
						waitpid(xx,&status,0);
					}
				}
				if(xx=(fork() == 0)) {        	
					execlp("rm", "rm", metaFicheiro, NULL);
					_exit(1);
				}else{
					waitpid(xx,&status,0);
					if(xx=(fork() == 0)) {        	
						execlp("ln", "ln", "-s", fichiroajuda,metaFicheiro, NULL);
						_exit(1);
					}else{
						waitpid(xx,&status,0);
					}
					printf("%s copiado com sucesso\n",ficheiro); //ficheiro foi mudado
				}
			}else{
				printf("Já existe um backup deste ficheiro\n");
			}
		}else{
			sprintf(fichiroajuda,"%s/%s",path_data,nameFile);
			if(access(dataFicheiro,F_OK) == -1){
				sprintf(fichiroajuda,"%s/%s",path_data,nameFile);
				if(xx=(fork()==0)) {
					fp_file = open(ficheiro, O_RDONLY);
				   
					dup2(fp_file,0);
					close(fp_file);
				   
					fp_digest = open(fichiroajuda, O_CREAT | O_WRONLY, 0664);
				
					dup2(fp_digest, 1);
					close(fp_digest);	       
					    
					execlp("gzip", "gzip", NULL);
					_exit(1);
				}else{
					waitpid(xx,&status,0);
				}
			}
			if(xx=(fork() == 0)) {        	
				execlp("ln", "ln", "-s", fichiroajuda,metaFicheiro, NULL);
				_exit(1);
			}else{
				waitpid(xx,&status,0);
			}
			printf("%s copiado com sucesso\n",ficheiro);
		}
	}else{
		printf("Ficheiro especificado não existe!\n");
	}
}

void restore(char * ficheiro, char * path_meta,int pid){
	int fd[2];
	char metaFicheiro[1024],cwd[1024],fichiroajuda[1024];
	int fp1,fp2,status,xx;

	getcwd(cwd, sizeof(cwd));
	sprintf(metaFicheiro,"%s/%s",path_meta,ficheiro);

	if (access(metaFicheiro, F_OK) != -1) {	  
		sprintf(fichiroajuda,"%s/%s",cwd,ficheiro);
		if (xx=(fork()==0)) {
			fp1 = open(metaFicheiro, O_RDONLY);
		
			dup2(fp1, 0);
			close(fp1);
			   
			fp2 = open(fichiroajuda, O_CREAT | O_WRONLY, 0664);
			
			dup2(fp2, 1);
			close(fp2);
			
			execlp("gunzip", "gunzip", NULL);

			_exit(1);
		}
		else {
			waitpid(xx,&status,0);

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

int main(int argc, char* argv[]){
	char *utilizador, path[MAX], path_data[MAX], path_meta[MAX], ficheiro[MAX], decisao[MAX], path_fifo[MAX], ch, resposta[MAX];
	int fd=0, pid, i;
	struct stat fileStat;

	utilizador = (char *)getenv("USER");

	sprintf(path, "/home/%s/.Backup",utilizador);
	sprintf(path_data,"/home/%s/.Backup/data",utilizador);
	sprintf(path_meta,"/home/%s/.Backup/metadata",utilizador);

	if (stat(path, &fileStat) == -1)
    	mkdir(path, 0700);
	if (stat(path_data, &fileStat) == -1)
    	mkdir(path_data, 0700);
	if (stat(path_meta, &fileStat) == -1)
    	mkdir(path_meta, 0700);


    sprintf(path_fifo,"%s/fifo",path);
 	mkfifo(path_fifo,0666);

	fd=open(path_fifo,O_RDONLY);

	
	while(read(fd,&ch,1)!=0 && ch!='\n'){
		resposta[i]=ch;
		i++;
	}
	resposta[i]='\0';

	pid = atoi(strtok(resposta, " "));
	strcpy(decisao, strtok(NULL, " "));
	strcpy(ficheiro, strtok(NULL, " \r\n"));

	if(strcmp(decisao,"backup")==0){
		backup(ficheiro,path,path_data,path_meta,pid);
	}else if(strcmp(decisao,"restore")==0){
		restore(ficheiro,path_meta,pid);
	}else{
		printf("comando invalido\n");
	}
	

	close(fd);

	return 0;
}