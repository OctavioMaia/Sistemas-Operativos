#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/uio.h>
#include <signal.h>
#include <stdio.h>
#include <stddef.h>
#include <fcntl.h>
#include <string.h>

char* asteriscorestore(char * asd, char * pathMeta){
	FILE * fil;
	char r,stop,words[50][20],txt[1024]="",auxiliar[1024],pls[1024],matrix[1024][1024],b;
	int j, i,fmetada;
	int file,x,y,xs,token,size;
	x=0;y=0;xs=0;
	size = strlen(asd);
	i=0;
	j=0;
	int total=0;
	sprintf(pls,"%s/inf.txt",pathMeta);
	fmetada=open(pls,O_RDONLY);
	while(read(fmetada,&b,1)!=0 && b!=0)
	{
		matrix[j][i]=b;
		i++;
		if(b=='\n'){
			matrix[j++][i-1]='\0';
			i=0;
			total++;
		}
	}
	total--;
	close(fmetada);
  while(total>=0)
  {
    for(token=1,y=0;matrix[total][y]!='-';y++){
        if (matrix[total][y]==asd[token]) token++;
    }
    y--;
    if (token == size){
    	strncpy(auxiliar,matrix[total], y+1);
        strcat(txt,auxiliar);
        strcat(txt," ");
        xs++;
        }
    total--;
  }

  return txt;
}





int main(int argc, char* argv[])
{
	FILE * fil;
	char dataFile[1024],*lista,resCliente[30],copia[1024],cwd[1024],ll[1024],destiny[1024],metaFile[1024],metaFile1[1024],*nameFile,convertido[50],*aux,codigo[50],*tok,b,*user,buf[1024],path[40],pathData[50],pathMeta[55],cod[100],pls[100],pwww[100],asas[100], matrix[20][100], path2[100], auxi[100],auxili[100],nfic[50];
	int status,xx,metad,metad1,tt,a,redr,iter=0,fdCod,x,fd,j,fmetada,i=0;
	struct stat st = {0};

	user = (char *)getenv("USER");
	sprintf(path,"/home/%s/.Backup",user);
	sprintf(pathData,"/home/%s/.Backup/data",user);
	sprintf(pathMeta,"/home/%s/.Backup/metadata",user);

	if (stat(path, &st) == -1) {
    	mkdir(path, 0700);
	}
	if (stat(pathData, &st) == -1) {
    	mkdir(pathData, 0700);
	}
	if (stat(pathMeta, &st) == -1) {
    	mkdir(pathMeta, 0700);
	}
	
	mkfifo("fifo",0666);
	getcwd(cwd, sizeof(cwd));
	fd=open("fifo",O_RDONLY);

	while(read(fd,&b,1)!=0 && b!='\n')
	{
		buf[i]=b;
		i++;
	}
	buf[i]='\0';
	printf("buf %s\n",buf);

	close(fd);
	tok=strtok(buf," ");
	tok=strtok(NULL," \r\n");

	fd=open("fifo",O_WRONLY);
	if(tok!=NULL)
	{
		if(strcmp(tok,"backup")==0)
		{
		}
		else
		{
			if(strcmp(tok,"restore")==0)
		{
			tok=strtok(NULL," ");
			if (tok[0]=='*') 
			{
				lista=asteriscorestore(tok,pathMeta);
				printf("lista %s\n",lista );
				tok=strtok(lista," ");
			}
			while(tok!=NULL)
			{
				i=0;
				j=0;
				int total=0;
				sprintf(pls,"%s/inf.txt",pathMeta);
				fmetada=open(pls,O_RDONLY);
				while(read(fmetada,&b,1)!=0 && b!=0)
				{
					matrix[j][i]=b;
					i++;
					if(b=='\n'){
						matrix[j++][i-1]='\0';
						i=0;
						total++;
					}
				}
				matrix[j++][i] = '\0';
				close(fmetada);

				char *divisao;
				int ii,sai=0,indEnc;
				for(ii=0;(ii<j && sai==0);ii++)
				{
					if (strncmp(matrix[ii],tok,strlen(tok))==0) 
					{
						sai=1;
						indEnc=ii;
					}
				}
				char nomeFicheiro[200],localizacao[200],origem[200];
				if(ii==j && sai==0) printf("ficheiro nao encontrado\n");
				else
				{
					ii=0;
					divisao=strtok(matrix[indEnc],"->");
					while(divisao!=NULL)
					{
						switch(ii)
						{
							case 0:
								strcpy(nomeFicheiro,divisao);
								ii++;
								break;
							case 1:
								strcpy(localizacao,divisao);
								ii++;
								break;
							case 2:
								strcpy(origem,divisao);
								ii=0;
								break;
						}
						divisao=strtok(NULL,"->");
					}
				}
			    a=0;
				int p=j=strlen(matrix[i]);
				int encontrei =0;
				int barra =0;

				char pp[200],zz[200];
				
				sprintf(pp,"%s2.gz",localizacao);
				sprintf(ll,"%s2",localizacao);
				sprintf(zz,"%s%s",origem,nomeFicheiro);
				if((xx=fork())==0)
				{
					execlp("cp","cp",localizacao,pp,NULL);
					_exit(1);
				}
				else
				{
					waitpid(xx,&status,0);
					if((xx=fork())==0)
					{
						execlp("gunzip","gunzip",pp,NULL);
						_exit(1);
					}
					else
					{
						waitpid(xx,&status,0);
						if((xx=fork())==0)
						{
							execlp("mv","mv",ll,zz,NULL);
							_exit(1);
						}
						else
						{
							waitpid(xx,&status,0);
						}
					}
				}
				tok=strtok(NULL," ");
				printf("segundo tok: %s \n",tok);
			}
			close(fd);
		}
		else{
			if(strcmp(tok,"remove")==0)
			{
				
			}
			else{
				printf("erro ao ler comando\n");
			}
		}
	}
}
	else{ 
		printf("erro ao ler comando\n");
	}
	//printf("word: %s\n",buf);
	//close(fd);
	return 1;
}