/*
Do not modify this file.
Make all of your changes to main.c instead.
*/

#include "program.h"
#include <stdio.h>
#include <stdlib.h>


static int compare_bytes( const void *pa, const void *pb )
{

	int a = *(char*)pa;
	int b = *(char*)pb;

	if(a<b) {
		return -1;
	} else if(a==b) {
		return 0;
	} else {
		return 1;
	}
}

void access_pattern1( char *data, int length )

{
	// Datos
	srand(5000);

	int suma = 0;
	int numero1 = 0;
	int numero2 = 0;


	for (numero1 = 0; numero1<length; numero1++){
		data[numero1] = 0;	
	}

	while(numero2<101){
		int inicio = rand()%length;
		int tamanio = 20;
		while(numero1<101){
			data[ (inicio+rand()%tamanio)%length] = rand();
		numero1++;
		}
		numero2++;
	}


	for (numero1 = 0; numero1<length; numero1++){
		suma +=data[numero1];	
	}
	printf("El resultado del tipo de acceso1 es:\n");		
}

void access_pattern2( char *data, int length ) //sort
{
	srand(5000); 
	int suma = 0;
	int numero1= 0;

	while(numero1<length){
		data[numero1] = rand();
		numero1++;
	}

	qsort(data,length,1,compare_bytes);

	while(numero1<length){
		suma+= data[numero1];
		numero1++;
	}

	printf("El resultado del tipo de acceso2 es:\n");
}

void access_pattern3( char *cdata, int length ) //secuencial
{
	

	for (int numero1 = 0; numero1<length; numero1++){
		cdata[numero1] = 0;	
	}
	printf("El resultado del tipo de acceso3 es:\n");	
}
