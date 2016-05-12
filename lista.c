#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "lista.h"

#define MAX 100

typedef struct metadata {
	char nome_ficheiro[MAX];
	char digest[MAX];
}metadata;

typedef struct lista {
	Metadata metadata;
	struct lista *next;	
}lista;


char* getNomeFicheiro (Metadata m) {
	return m->nome_ficheiro;
}

char* getDigest (Metadata m) {
	return m->digest;
}

void setNomeFicheiro (Metadata m, char *nome) {
	strcpy(m->nome_ficheiro,nome);
}

void setDigest (Metadata m, char* digest) {
	strcpy(m->digest,digest);
}

char* metaToString (Metadata m) {
	char *s = malloc(100);
	sprintf(s,"Nome_Ficheiro: %s Digest: %s",m->nome_ficheiro,m->digest);
	return s;
}

int existeFicheiro (Lista l, char *nome) {
	Lista aux;
	for (aux=l; aux; aux=aux->next)
		if (strcmp(aux->metadata->nome_ficheiro,nome)==0) 
			return 1;

	return 0;	
}


Lista deleteFicheiro (Lista l, char *nome) {
	Lista aux;
	if (strcmp(nome,l->metadata->nome_ficheiro)==0) {
		free(l->metadata);
		l=l->next;
		return l;
	}
	for (aux=l; aux->next; aux=aux->next) {
		if (strcmp(nome,aux->next->metadata->nome_ficheiro)==0) {
			free(aux->next->metadata);
			free(aux->next);
			aux->next=aux->next->next;
			return l;
		}
	}
	return l;
}


Metadata getMetadata (Lista l, char* nome) {
	Lista aux;

	for (aux=l; aux!=NULL; aux=aux->next){
		if (strcmp(nome,aux->metadata->nome_ficheiro)==0) 
			return aux->metadata;
	}
	return NULL;
}

Lista addFile (Lista l, char *nome, char *digest) {
	Lista aux;
	Metadata m = getMetadata(l,nome);
	if (m) {
		strcpy(m->nome_ficheiro,nome);
		strcpy(m->digest,digest);
		return l;
	}else{
		m=(Metadata)malloc(sizeof(struct metadata));		
		strcpy(m->nome_ficheiro,nome);
		strcpy(m->digest,digest);
		aux=(Lista)malloc(sizeof(struct lista));
		aux->metadata=m;
		aux->next=l;
		return l;
	}
}

int length (Lista l) {
	Lista aux;
	int i;
	for (i=0, aux=l; aux!=NULL; i++, aux=aux->next);
	return i;
}