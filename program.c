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

void tipo_acceso1( char *data, int length )

{
	// Datos
	//srand(

	int suma = 0;
	int numero1 = 0;
	int numero2 = 0;


	for (int i = 0; i<length; i++){
		data[i] = 0;	
	}

	while(numero2<100){
		int inicio = rand()%length;
		int tamanio = 25;
		while(numero1<100){
			data[ (inicio+rand()%tamanio)%length] = rand();
		numero1++;
		}
		numero2++;
	}


	for (int i = 0; i<length; i++){
		suma +=data[i];	
	}
	printf("El resultado del tipo de acceso1 es:%d\n", suma);		
}

void tipo_acceso2( char *data, int length )
{
	//srand
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

	printf("El resultado del tipo de acceso2 es:%d\n", suma);
}

void tipo_acceso3( char *cdata, int length )
{
	unsigned numero1 = 0;
	unsigned numero2 = 0;
	unsigned char*data = (unsigned char*) cdata;
	unsigned suma = 0;


	while(numero1<length){
		data[numero1] = numero1%256;
		numero1++;
	}

	while(numero2<10){
		while(numero1<length){
			suma += data[numero1];
			numero1++;
		}
		numero2++;
	}

	printf("El resultado del tipo de acceso3 es:%d\n", suma);	
}
