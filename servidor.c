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
   	int a = j+1;
   	palavra[a]='\0';
   	while (i < j) {
      	temp = palavra[i];
      	palavra[i] = palavra[j];
      	palavra[j] = temp;
      	i++;
      	j--;
   	}
   	return palavra;
}

char* vaibuscartodosficheiros(char * string)
{
  FILE * file;
  char c,ficheirospasta[50][20],ficheirostodos[1024]="";
  int filecodigo,x,y,auxiliar,contador,tamanho;
  x=0;y=0;auxiliar=0;
  tamanho = strlen(string);

  file=popen("ls","r");
  filecodigo=fileno(file);

	while (read(filecodigo,&c,1)!=0)
	{
	    if (c != ' ' && c!= '\n') {
	      ficheirospasta[x][y] = c;
	      y++;
	    }

	    else {
	      ficheirospasta[x][y] = '\0';
	      x++;y=0;
	    }
	}
	x--;
	close(filecodigo);
	pclose(file);
  while(x>=0)
  {
    for(contador=1,y=0;ficheirospasta[x][y]!='\0';y++){
        if (ficheirospasta[x][y]==string[contador]) contador++;
    }

        if (contador == tamanho){
          strcat(ficheirostodos,ficheirospasta[x]);
          strcat(ficheirostodos," ");
          auxiliar++;
        }
    x--;
  }
  return strdup(ficheirostodos);
}

void backup(char * ficheiro, char* path, char *path_data, char *path_meta, int pid){
	int fd[2],n, fp_file=0,fp_digest=0,status,fileDescritor,id,a=0,encontrei=0;
	FILE *file;
	char *string, *aux, div[MAX], cod[128],codigo[MAX],fileAtual[MAX], *nomeFicheiro,cwd[MAX],path_ficheiro[MAX],path_dataFicheiro[MAX],path_metaFicheiro[MAX];

	pipe(fd);

	sprintf(codigo,"sha1sum %s",ficheiro);

	file=popen(codigo,"r");
	fileDescritor=fileno(file);
	read(fileDescritor,cod,128);
	close(fileDescritor);
	pclose(file);
	getcwd(cwd, sizeof(cwd));

	aux=(char *) strdup(cod);
	nomeFicheiro=strsep(&aux," \t");

	sprintf(path_ficheiro,"%s/%s",cwd,ficheiro);
	sprintf(path_dataFicheiro,"%s/%s",path_data,nomeFicheiro);
	sprintf(path_metaFicheiro,"%s/%s",path_meta,ficheiro);


	if(access(path_ficheiro, F_OK) == 0){
		if(access(path_metaFicheiro,F_OK) == 0){
				sprintf(fileAtual,"%s/%s",path_data,nomeFicheiro);
				if(access(path_dataFicheiro,F_OK) == -1){
					if(id=(fork()==0)) {
						fp_file = open(ficheiro, O_RDONLY,0664);
					   
						dup2(fp_file,0);
						close(fp_file);
					   
						fp_digest = open(fileAtual, O_CREAT | O_WRONLY, 0664);
					
						dup2(fp_digest, 1);
						close(fp_digest);
						    
						execlp("gzip", "gzip", NULL);
						_exit(1);
					}else{
						waitpid(id,&status,0);
					}
					if(id=(fork() == 0)) {        	
						execlp("rm", "rm", path_metaFicheiro, NULL);
						_exit(1);
					}else{
						waitpid(id,&status,0);
						if(id=(fork() == 0)) {        	
							execlp("ln", "ln", "-s", fileAtual,path_metaFicheiro, NULL);
							_exit(1);
						}else{
							waitpid(id,&status,0);
						}
						kill(pid,SIGUSR2); //ficheiro foi mudado
					}
				}
				else{
					kill(pid,SIGUSR1);
				}
		}else{
			sprintf(fileAtual,"%s/%s",path_data,nomeFicheiro);
			if(access(path_dataFicheiro,F_OK) == -1){
				sprintf(fileAtual,"%s/%s",path_data,nomeFicheiro);
				if(id=(fork()==0)) {
					fp_file = open(ficheiro, O_RDONLY);
				   
					dup2(fp_file,0);
					close(fp_file);
				   
					fp_digest = open(fileAtual, O_CREAT | O_WRONLY, 0664);
				
					dup2(fp_digest, 1);
					close(fp_digest);	       
					    
					execlp("gzip", "gzip", NULL);
					_exit(1);
				}else{
					waitpid(id,&status,0);
				}
			}
			if(id=(fork() == 0)) {        	
				execlp("ln", "ln", "-s", fileAtual,path_metaFicheiro, NULL);
				_exit(1);
			}else{
				waitpid(id,&status,0);
			}
			kill(pid,SIGUSR2);
		}
	}else{
		kill(pid,SIGUSR1);
	}
}

void restore(char *ficheiro, char * path_meta, int pid){
	int fd[2];
	char path_metaFicheiro[MAX],cwd[MAX],fileAtual[MAX];
	int fp1,fp2,status,id;

	getcwd(cwd, sizeof(cwd));
	sprintf(path_metaFicheiro,"%s/%s",path_meta,ficheiro);


	if (access(path_metaFicheiro, F_OK) == 0) {	  
		sprintf(fileAtual,"%s/%s",cwd,ficheiro);
		if (id=(fork()==0)) {
			fp1 = open(path_metaFicheiro, O_RDONLY);
		
			dup2(fp1, 0);
			close(fp1);
			   
			fp2 = open(fileAtual, O_CREAT | O_WRONLY, 0664);
			
			dup2(fp2, 1);
			close(fp2);
			
			execlp("gunzip", "gunzip", NULL);

			_exit(1);
		}
		else {
			waitpid(id,&status,0);

			if (WEXITSTATUS(status) == 0) {
				kill(pid, SIGUSR2);
			}else {
				kill(pid, SIGUSR1);
			}
		}
	}
	else {
		kill(pid, SIGUSR1);
	}

}

int main(int argc, char* argv[]){
	char verificacao[MAX],*utilizador, *ver,path[MAX], path_data[MAX], path_meta[MAX], *auxiliar,ficheiro[MAX], decisao[MAX], path_fifo[MAX], ch, resposta[MAX], *ficheiros[MAX];
	int status,id,fd=0, pid, i, j=0, ind=0, branco=0, p=0,aux;
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
 	while(read(0,&ch,1)!=0 && ch!='\n'){
		resposta[i]=ch;
		i++;
	}
	resposta[i]='\0';
	while(strcmp(resposta,"sobusrv")!=0) {
		printf("Inicie o servidor com sobusrv\n");
		i=0;
		while(read(0,&ch,1)!=0 && ch!='\n'){
			resposta[i]=ch;
			i++;
		}
		resposta[i]='\0';
	}
	printf("servidor online\n");

 	mkfifo(path_fifo,0666);
	while(1){
		fd=open(path_fifo,O_RDONLY);
		read(fd,&pid,sizeof(pid));
		ind=0;
		while(read(fd,&ch,1)!=0 && ch!='\n'){
			resposta[ind]=ch;
			ind++;
		}
		resposta[ind]='\0';
		close(fd);
		if(resposta[0]!='\0'){
			if(fork()==0){
				// a primeira palavra tem de ser sob
				char sas[122];
				ver = strtok(resposta," ");
				if(ver!=NULL){
					strcpy(sas, ver);
					ver = strtok(NULL, " ");
					if(ver!=NULL){
						strcpy(decisao,ver);
						ver = strtok(NULL, "\r\n");
						if(ver!=NULL){
							strcpy(ficheiro, ver);
							if(strcmp(sas,"sobucli")==0)
							{
								if(strcmp(decisao,"backup")==0){
									p=0;
									if(ficheiro[j]=='*'){
										auxiliar=vaibuscartodosficheiros(ficheiro);
										p=1;

									}
									else{
										auxiliar=strdup(ficheiro);
									}
									while(auxiliar[j]!='\0'){
										if (auxiliar[j] == ' ') {
								    		branco++;
										}
										j++;
									}

									aux=branco;
									i=1;
									if(branco==0){
										ficheiros[0] = strtok(auxiliar," ");
										aux=1;
									}
									else{
										ficheiros[0] = strtok(auxiliar," ");
										if(p==1) branco--;
										while(branco>0){
											ficheiros[i] = strtok(NULL," ");
											branco--;
											i++;
										}
										if(p==0) aux+=1;
									}
									for(i=0; i < aux; i++){
										backup(ficheiros[i],path,path_data,path_meta,pid);
									}
								}else if(strcmp(decisao,"restore")==0){
									auxiliar=strdup(ficheiro);
									while(auxiliar[j]!='\0'){
										if (auxiliar[j] == ' ') {
								    		branco++;
										}
										j++;
									}

									aux=branco;
									i=1;
									if(branco==0){
										ficheiros[0] = strtok(auxiliar," ");
										aux=1;
									}
									else{
										ficheiros[0] = strtok(auxiliar," ");
										if(p==1) branco--;
										while(branco>0){
											ficheiros[i] = strtok(NULL," ");
											branco--;
											i++;
										}
										if(p==0) aux+=1;
									}
									for(i=0;i<aux;i++){
										restore(ficheiros[i],path_meta,pid);
									}
								}else{
									return 1;
								}
							}
						}
					}
				}
				_exit(0);
			}
		}

	}

	
	return 0;
}