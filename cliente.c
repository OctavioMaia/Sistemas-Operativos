#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/uio.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

char ficheiro[100];
int tipo;


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

void sim(int s) {
	switch(tipo) 
	{
		case 1: 
			printf("Ficheiro %s copiado\n",ficheiro);
			break;
		case 2:
			printf("Ficheiro %s recuperado\n",ficheiro);
			break;
	}
}

void nao(int s) {
	switch(tipo) 
	{
		case 1: 
			printf("Ficheiro %s nao copiado\n",ficheiro);
			break;
		case 2:
			printf("Ficheiro %s nao recuperado\n",ficheiro);
			break;
	}
}

int main(int argc, char* argv[]){
	char *utilizador;
	int i=0,pid,fd_pedido,fd_resposta,x,p=0;
	char *tok,*auxiliar,b,PATH[100],buf[100], *ficheirosexistentes;
	signal(SIGUSR1,nao);
	signal(SIGUSR2,sim);
	utilizador= (char *)getenv("USER");
	sprintf(PATH, "/home/%s/.Backup/fifo",utilizador);

	fd_pedido=open(PATH,O_WRONLY);
	pid=getpid();
	write(fd_pedido,&pid,sizeof(pid));
	while(read(0,&b,1)!=0 && b!='\n'){
		write(fd_pedido,&b,1);
		buf[i]=b;
		i++;
	}
	buf[i]='\0';
	close(fd_pedido);
	tok=strtok(buf," ");
	if(strcmp(tok,"sobucli")==0)
	{
		tok=strtok(NULL," \r\n");
		if(tok!=NULL)
		{
			if(strcmp(tok,"backup")==0)
			{
				tipo=1;
				tok=strtok(NULL," ");
				if (tok[0]=='*') 
				{
					ficheirosexistentes=vaibuscartodosficheiros(tok);
					auxiliar=strtok(ficheirosexistentes, " ");
					while(auxiliar!=NULL){
						strcpy(ficheiro,auxiliar);
						pause();
						auxiliar=strtok(NULL," ");
					}
				}
				else{
					while(tok!=NULL)
					{
						strcpy(ficheiro,tok);
						pause();
						tok=strtok(NULL," ");
					}
				}
			}
			else if(strcmp(tok,"restore")==0)
			{
				tipo=2;
				tok=strtok(NULL," ");
				while(tok!=NULL)
				{
					strcpy(ficheiro,tok);
					pause();
					tok=strtok(NULL," ");
				}
			}
			else printf("Introduza um comando valido\n");
		}
	}
	else printf("Introduza um comando valido\n");
}