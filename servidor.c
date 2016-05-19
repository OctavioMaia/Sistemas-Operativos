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

void backup(char * ficheiro, char* path, char *path_data, char *path_meta, int pid){
	int fd[2],n, fp_file=0,fp_digest=0,status,fileDescritor,id,a=0,encontrei=0;
	FILE *file;
	char *string, *aux, div[MAX], cod[128],codigo[MAX],fileAtual[MAX], *nomeFicheiro,cwd[MAX],path_ficheiro[MAX],path_dataFicheiro[MAX],path_metaFicheiro[MAX];

	pipe(fd);

	sprintf(codigo,"sha1sum %s",ficheiro);
	//printf("dadad%s\n",codigo);
	file=popen(codigo,"r");
	fileDescritor=fileno(file);
	read(fileDescritor,cod,128);
	close(fileDescritor);
	pclose(file);
	getcwd(cwd, sizeof(cwd));

	aux=(char *) strdup(cod);
	nomeFicheiro=strsep(&aux," \t");
	//printf("digest: %s do ficheiro %s\n",nomeFicheiro,ficheiro);

	sprintf(path_ficheiro,"%s/%s",cwd,ficheiro);
	sprintf(path_dataFicheiro,"%s/%s",path_data,nomeFicheiro);
	sprintf(path_metaFicheiro,"%s/%s",path_meta,ficheiro);


	if(access(path_ficheiro, F_OK) == 0){
		if(access(path_metaFicheiro,F_OK) == 0){
			sprintf(codigo,"ls -l %s",path_metaFicheiro);
			file=popen(codigo,"r");
			fileDescritor=fileno(file);
			read(fileDescritor,cod,MAX);
			close(fileDescritor);
			pclose(file);
			aux=(char *) strdup(cod);
			n = strlen(aux)-2;

			while(encontrei==0){
				if(aux[n]!='/'){
					div[a++]=aux[n--];
				}else{
					encontrei=1;
				}
			}
			string = reverse(div);
			if(strcmp(string,nomeFicheiro)!=0){
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
					printf("%s : copiado com sucesso\n",ficheiro); //ficheiro foi mudado
				}
			}else{
				printf("%s : Já existe um backup deste ficheiro\n", ficheiro);
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
			printf("%s copiado com sucesso\n",ficheiro);
		}
	}else{
		printf("%s : Ficheiro especificado não existe!\n",ficheiro);
	}
}

void restore(char *ficheiro, char * path_meta, int pid){
	int fd[2];
	char path_metaFicheiro[MAX],cwd[MAX],fileAtual[MAX];
	int fp1,fp2,status,id;

	getcwd(cwd, sizeof(cwd));
	sprintf(path_metaFicheiro,"%s/%s",path_meta,ficheiro);

	printf("path_metaFicheiro %s\n",path_metaFicheiro );

	if (access(path_metaFicheiro, F_OK) == 0) {	  
		sprintf(fileAtual,"%s/%s",cwd,ficheiro);
		printf("fileAtual %s\n",fileAtual);
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
			kill(pid, SIGCONT);

			if (WEXITSTATUS(status) == 0) {
				kill(pid, SIGUSR1);
			}else {
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
	char *utilizador, path[MAX], path_data[MAX], path_meta[MAX], ficheiro[MAX], decisao[MAX], path_fifo[MAX], ch, resposta[MAX], *ficheiros[MAX];
	int fd=0, pid, i, j=0, branco=0, aux;
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
	strcpy(ficheiro, strtok(NULL, "\r\n"));

	//printf("ficheiro %s\n",ficheiro );

	while(ficheiro[j]!='\0'){
		if (ficheiro[j] == ' ') {
    		branco++;
		}
		j++;
	}

	//printf("tem %d espaços\n",branco );
	//printf("inseri %s\n",ficheiro);

	aux=branco;

	if(branco==0){
		ficheiros[0] = strtok(ficheiro," ");
	}else{
		ficheiros[0] = strtok(ficheiro," ");
		i=1;
		while(branco>0){
			ficheiros[i] = strtok(NULL," ");
			branco--;
			i++;
		}
	}

	//for(i=0;i<5;i++) printf("i %s\n",ficheiros[i] );

	if(strcmp(decisao,"backup")==0){
		for(i=0; i <= aux; i++){
			//printf("entrei for %s\n",ficheiros[i] );
			backup(ficheiros[i],path,path_data,path_meta,pid);
		}
	}else if(strcmp(decisao,"restore")==0){
		for(i=0; i <= aux; i++){
			printf("%s\n",ficheiros[i] );
			restore(ficheiros[i],path_meta,pid);
		}
	}else{
		printf("Introduza um comando válido.\n");
	}
	

	close(fd);

	return 0;
}