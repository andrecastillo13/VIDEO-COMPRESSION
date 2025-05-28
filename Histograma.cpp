#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "FichBits.h"
#include "Histograma.h"

static unsigned int *Histograma;
static int longCadena;
static int tamAlfabeto;

//
// Rutinas basicas para manejar el histograma
//

// Inicializa el histograma: lo calcula a partir de una cadena de bytes
void InicializaHistograma(int longitudAlfabeto, int longitudCadena, char *cadena)
{  
  int f;

  tamAlfabeto=longitudAlfabeto;
  longCadena=longitudCadena;
  Histograma=(unsigned int*)malloc(tamAlfabeto*sizeof(unsigned int));

  for (f=0;f<tamAlfabeto;f++) 
	  Histograma[f]=0;
  for (f=0;f<longCadena;f++)
	  Histograma[cadena[f]]++;
}

// Devuelve un valor del histograma
int LeeHistograma(int pos)
{
  return Histograma[pos];
}

// Finaliza el histograma, liberando recursos
void FinalizaHistograma()
{
  free(Histograma);
}

//
// Rutinas para guardar en disco y cargar el histograma
//

// Guarda el histograma en disco
void GuardaHistograma()
{
  int f;
  for (f=0;f<tamAlfabeto;f++) 
	EscribePalabra(24,Histograma[f]);
}

// Inicializa el histograma: lo lee de disco
void InicializaHistogramaDeDisco(int longitudAlfabeto)
{
  int f;
  tamAlfabeto=longitudAlfabeto;
  Histograma=(unsigned int*)malloc(tamAlfabeto*sizeof(unsigned int));
  for (f=0;f<tamAlfabeto;f++) 
	Histograma[f]=LeePalabra(24);
}

//
// Rutinas para calcular informacion a partir del histograma
//

// Imprime el histograma y las probabilidades asociadas
void ImprimeHistograma()
{
  int f;
  printf("\nCONTENIDO DEL HISTOGRAMA:\n");
  for (f=0;f<tamAlfabeto;f++) 
	printf("- Simbolo %d ('%c'): %d, probabilidad %f%%\n",f,f+'a',
	                  Histograma[f],((float)Histograma[f])*100.0/((float)longCadena));
}

// Calcula la entropia de la cadena a partir de los datos del histograma
float CalculaEntropia()
{
   float entropia=0.0,prob;
   for (int f=0;f<tamAlfabeto;f++)
   {
	   prob=((float)Histograma[f])/((float)longCadena);
	   if (prob!=0.0) entropia+=prob*((float)(log(1/prob)/log(2)));
   }	   
   return entropia;
}
