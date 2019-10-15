/*
Main program for the virtual memory project.
Make all of your modifications to this file.
You may add or rearrange any code or data as you need.
The header files page_table.h and disk.h explain
how to use the page table and disk interfaces.
*/

#include "page_table.h"
#include "disk.h"
#include "program.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#define eliminar 0

// Datos globales
int disco_lectura =0, disco_escritura=0, paginasDefecto =0; 
int npages, nframes;
struct disk*disco =NULL;
char *physmem;
int adelante = 0;
int atras = 0;
int * arregloF;
char *algoritmoDeReemplazo;

void eliminar_pagina(struct page_table*pt, int numeroMarco);
int existencia_marco_libre();
void reemplazo_pagina_FIFO(struct page_table*pt, int page);
void reemplazo_pagina_aleatorio(struct page_table*pt, int page);


struct Marcos{
	int page;
	int bits;
	
};

struct Marcos* tabla_de_marcos = NULL;

void page_fault_handler( struct page_table *pt, int page )
{
	paginasDefecto++;
	if(!strcmp(algoritmoDeReemplazo,"fifo")){
		reemplazo_pagina_FIFO(pt, page);
	}
	else if(!strcmp(algoritmoDeReemplazo,"rand")){
		reemplazo_pagina_aleatorio(pt,page);
	}
	else{

	printf("page fault on page #%d\n",page);
	exit(1);
	}
}

int main( int argc, char *argv[] )
{
	if(argc!=5) {
		printf("use: virtmem <npages> <nframes> <lru|fifo> <access pattern>\n");
		return 1;
	}

	npages = atoi(argv[1]);
	nframes = atoi(argv[2]);
	algoritmoDeReemplazo = argv[3];
	const char *program = argv[4];

	struct disk *disk = disk_open("myvirtualdisk",npages);
	if(!disk) {
		fprintf(stderr,"couldn't create virtual disk: %s\n",strerror(errno));
		return 1;
	}


	struct page_table *pt = page_table_create( npages, nframes, page_fault_handler );
	if(!pt) {
		fprintf(stderr,"couldn't create page table: %s\n",strerror(errno));
		return 1;
	}

	arregloF = malloc(nframes*sizeof(int));
	tabla_de_marcos = malloc(nframes*sizeof(tabla_de_marcos));

	if(tabla_de_marcos==NULL){
	
		printf("Error en espacio de tablas.\n");
		exit(1);
	}

	char *virtmem = page_table_get_virtmem(pt);

	physmem = page_table_get_physmem(pt);

	if(!strcmp(program,"pattern1")) {
		access_pattern1(virtmem,npages*PAGE_SIZE);

	} else if(!strcmp(program,"pattern2")) {
		access_pattern2(virtmem,npages*PAGE_SIZE);

	} else if(!strcmp(program,"pattern3")) {
		access_pattern3(virtmem,npages*PAGE_SIZE);

	} else {
		fprintf(stderr,"unknown program: %s\n",argv[3]);

	}

	
	page_table_delete(pt);
	disk_close(disk);
	free(tabla_de_marcos);
	free(arregloF);

	printf("Cantidad de paginas: %d\n", npages);
	printf("Cantidad de marcos: %d\n", nframes);
	printf("Algoritmo utilizado: %s\n", algoritmoDeReemplazo); 
	printf("Cantidad de lecturas a disco: %d\n", disco_lectura);
	printf("Cantidad de escrituras a disco: %d\n", disco_escritura);
	printf("Numero de faltas de pagina: %d\n", paginasDefecto);


	return 0;

	
	
}

// Funciones 

void reemplazo_pagina_aleatorio(struct page_table*pt, int page){
	int cuadro;
	int bits;
	int indice;

	page_table_get_entry(pt, page, &cuadro, &bits); //fun. sirve para mapear directamente las paginas de memoria a marcos
	

	if(bits & PROT_READ){

		bits = PROT_READ | PROT_WRITE;
		indice = cuadro;
	}
	else if (!bits){

		bits = PROT_READ;
		indice = existencia_marco_libre();
		
		if(indice<0){
			indice = (int) lrand48() % nframes;
			eliminar_pagina(pt,indice);
		}
		disk_read(disco, page, &physmem[indice*PAGE_SIZE]);
		disco_lectura++;
	
	}
	else{
		printf("Error en reemplazo de pagina aleatorio.\n");
		exit(1);
	}

	page_table_set_entry(pt,page,indice, bits);

	tabla_de_marcos[indice].bits = bits;
	tabla_de_marcos[indice].page = page;
	
      // if(eliminar){page_table_print(tp);print("\n\n");}
		
}

void reemplazo_pagina_FIFO(struct page_table*pt, int page){

	int marco;
	int bits;
	int indice;
	page_table_get_entry(pt,page,&marco,&bits);

	if(bits & PROT_READ){
		bits = PROT_WRITE | PROT_READ;
		indice = marco;
	}
	else if(!bits){
		bits = PROT_READ;
		indice = existencia_marco_libre();
	
		if(indice<0){
			if((adelante > atras) || (adelante<atras) || (adelante!=atras)){
				printf("Error en desplazamiento de pagina.\n");
				exit(1);
			}
		
			else{
				indice = arregloF[adelante];
				eliminar_pagina(pt, indice);		
			}
		}
		disk_read(disco,page,&physmem[indice*PAGE_SIZE]);
		disco_lectura++;
		arregloF[atras] = indice;
		atras = (atras+1)%nframes;
	}
	else{
		printf("Error en reemplazo de pagina FIFO.\n");
		exit(1);
	}
	
	page_table_set_entry(pt,page,indice,bits);
	
	tabla_de_marcos[indice].page = page;
	tabla_de_marcos[indice].bits = bits;
	 // if(eliminar){page_table_print(tp);print("\n\n");}

}


int existencia_marco_libre(){

	int numero;
	while(numero<nframes){
		if(tabla_de_marcos[numero].bits == 0){
			return numero;
		}
		numero++;
	}
	return -1;
}

void eliminar_pagina(struct page_table*pt, int numeroMarco){

	if(tabla_de_marcos[numeroMarco].bits & PROT_WRITE){
		disk_write(disco, tabla_de_marcos[numeroMarco].page, &physmem[numeroMarco*PAGE_SIZE]);
		disco_escritura++;
	}

	page_table_set_entry(pt, tabla_de_marcos[numeroMarco].page, numeroMarco,0);
	tabla_de_marcos[numeroMarco].bits = 0;
	adelante = (adelante+1)%nframes;

}











