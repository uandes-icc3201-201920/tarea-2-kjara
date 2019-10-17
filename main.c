/*
Main program for the virtual memory project.
Make all of your modifications to this file.
You may add or rearrange any code or data as you need.
The header files page_table.h and disk.h explain
how to use the page table and disk interfaces.
*/

#include "page_table.h"
#include "program.h"
#include "disk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

// DATOS 

int paginasDefecto = 0;
int disco_lectura = 0;
int disco_escritura = 0;
int npages;
int nframes;
int adelante=0;
int atras=0;
int *arrayF;
int access_pattern;
char* virtmem;
char* physmem;
char* algoritmoReemplazo;
struct disk* disk;
struct Marco{
	int page;
	int bits;
};
struct Marco* marcos;

void eliminar_pagina(struct page_table *pt, int numeroMarco);
void reemplazo_pagina_aleatorio(struct page_table *pt, int page);
void reemplazo_pagina_FIFO(struct page_table *pt, int page);

void page_fault_handler( struct page_table *pt, int page )
{
	paginasDefecto++;
	if(!strcmp(algoritmoReemplazo,"rand")){
		reemplazo_pagina_aleatorio(pt, page);
	}
	else if(!strcmp(algoritmoReemplazo,"fifo")){
		reemplazo_pagina_FIFO(pt, page);
	}
	else {
		printf("use: virtmem <npages> <nframes> <rand|fifo> <a1|a2|a3>\n");
		exit(1);
	}
}

int main( int argc, char *argv[] )
{
	if(argc!=5){
		printf("use: virtmem <npages> <nframes> <rand|fifo> <a1|a2|a3>\n");
		return 1;
	}

	npages = atoi(argv[1]);
	nframes = atoi(argv[2]);
	algoritmoReemplazo = argv[3];
	const char *program = argv[4];
	disk = disk_open("myvirtualdisk",npages);

	if(!disk){
		fprintf(stderr,"couldn't create virtual disk: %s\n",strerror(errno));
		return 1;
	}
	struct page_table *pt = page_table_create( npages, nframes, page_fault_handler );

	if(!pt){
		fprintf(stderr,"couldn't create page table: %s\n",strerror(errno));
		return 1;
	}
	arrayF = malloc (nframes * sizeof(int));
	marcos = malloc(nframes * sizeof(struct Marco));
	
	virtmem = page_table_get_virtmem(pt);
	physmem = page_table_get_physmem(pt);

	if(!strcmp(program,"a1")){
		access_pattern1(virtmem,npages*PAGE_SIZE);
		access_pattern = 1;
	}
	else if(!strcmp(program,"a2")){
		access_pattern2(virtmem,npages*PAGE_SIZE);
		access_pattern = 2;
	}
	else if(!strcmp(program,"a3")){
		access_pattern3(virtmem,npages*PAGE_SIZE);
		access_pattern = 3;
	}
	else{
		fprintf(stderr,"Programa desconocido: %s\n",argv[4]);
		printf("use: virtmem <npages> <nframes> <rand|fifo> <a1|a2|a3>\n");
		exit(1);
	}
	printf(" ----=== Resultados patron de acceso %d ===----\n", access_pattern);

	// limpiar
	free(marcos);
	free(arrayF);
	page_table_delete(pt);
	disk_close(disk);
	
	//Resultados
	printf("Cantidad de paginas: %d\n", npages);
	printf("Cantidad de marcos: %d\n", nframes);
	printf("Algoritmo utilizado: %s\n", algoritmoReemplazo);
	printf("Cantidad de lecturas a disco: %d\n", disco_lectura);
	printf("Cantidad de escrituras a disco: %d\n", disco_escritura);
	printf("Numero de faltas de pagina: %d\n", paginasDefecto);

	return 0;
}

void reemplazo_pagina_aleatorio(struct page_table *pt, int page){
	int cuadro;
	int bits;
	int existencia_marco=-1;
	page_table_get_entry(pt, page, &cuadro, &bits);


	if(bits & PROT_READ){
		bits = PROT_READ | PROT_WRITE;
		existencia_marco = cuadro;
	} 
	else if (!bits){ 
		bits = PROT_READ;
		
		int numero=0;
		while(numero<nframes){
			if(marcos[numero].bits==0)
			existencia_marco= numero;
		numero++;
	}
		if(existencia_marco<0){	
			existencia_marco = (int) lrand48() % nframes;
			eliminar_pagina(pt, existencia_marco);
		}
		disk_read(disk, page, &physmem[existencia_marco*PAGE_SIZE]);
		disco_lectura++;

	}
	page_table_set_entry(pt, page, existencia_marco, bits);
	
	marcos[existencia_marco].page = page;
	marcos[existencia_marco].bits = bits;	
}

void reemplazo_pagina_FIFO(struct page_table *pt, int page){
	int frame;
	int bits;
	int existencia_marco=-1;
	page_table_get_entry(pt, page, &frame, &bits);

	if(bits & PROT_READ){
		
		bits = PROT_READ | PROT_WRITE;
		existencia_marco = frame;
	}
	else if (!bits){ 	
		bits = PROT_READ;
		
		
		int numero=0;
		while(numero<nframes){
			if(marcos[numero].bits==0)
			existencia_marco=numero;
		numero++;
		}
		if(existencia_marco< 0){
			if (adelante==atras){	
				existencia_marco = arrayF[adelante];
				eliminar_pagina(pt, existencia_marco);
			}
		}
		disk_read(disk, page, &physmem[existencia_marco*PAGE_SIZE]);
		disco_lectura++;
		
		arrayF[atras]=existencia_marco;
		atras =(atras+1)%nframes;
	}
	page_table_set_entry(pt, page, existencia_marco, bits);
	marcos[existencia_marco].page = page;
	marcos[existencia_marco].bits = bits;
}

void eliminar_pagina(struct page_table *pt, int numeroMarco)
{	
	if(marcos[numeroMarco].bits & PROT_WRITE){
		disk_write(disk, marcos[numeroMarco].page, &physmem[numeroMarco*PAGE_SIZE]);
		disco_escritura++;
	}
	page_table_set_entry(pt, marcos[numeroMarco].page, numeroMarco, 0);
	marcos[numeroMarco].bits = 0;
	adelante=(adelante+1)%nframes;
}