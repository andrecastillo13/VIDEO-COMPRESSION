#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FichBits.h"
#include "Histograma.h"
#include "Huffman.h"

#define RAMA_DER 1
#define RAMA_IZQ 0

static unsigned int *codigos;
static unsigned int *longCodigos;
static TNodo *nodoRaiz;
static TNodo *inicioHoriz;
static int tamAlfabeto;

static int cuentaDeBits;
static int cuentaDeSimbolos;

//
// Rutinas auxiliares para operar con el arbol de Huffman
//

// Devuelve el nodo menor y lo saca del arbol
TNodo *SacaMenor()
{
  TNodo *menor, *menorIzq, *busca;

  if (inicioHoriz==NULL) return NULL;
  menorIzq=NULL;
  menor=inicioHoriz;
  busca=inicioHoriz;
  while (busca->der!=NULL)
  {
	 if (busca->der->cuenta<menor->cuenta) // Si el de la derecha es menor...
	 {
        menor=busca->der; // ... lo marcamos como menor ...
		menorIzq=busca;   // ... y nos guardamos su izquierdo (el actual) para reenlazarlo
	 }
     busca=busca->der;
  }
  // sacamos al menor del conjunto
  if (menorIzq==NULL) // (con cuidado si es el primero, ya que no tiene nodo izq)
	 inicioHoriz=menor->der;
  else
	 menorIzq->der=menor->der;
  // devolvemos el menor
  return menor;
}

// Inserta en el subarbol resultante de la union de dos nodos, Nodo1 y Nodo2
void JuntaNodo(TNodo *Nodo1, TNodo *Nodo2)
{
	  TNodo *NuevoNodo=(TNodo *)malloc(sizeof(TNodo));
      NuevoNodo->infDer=Nodo1;
	  NuevoNodo->infIzq=Nodo2;
	  NuevoNodo->cuenta=Nodo1->cuenta+Nodo2->cuenta;
	  NuevoNodo->der=inicioHoriz;
	  inicioHoriz=NuevoNodo;  
}

//
// Rutinas globales del codificador y decodificador
//

// Construye el arbol a partir de la informacion del histograma
// y devuelve el nodo raiz del arbol
TNodo *ConstruyeArbol()
{
  int f;
  TNodo *NuevoNodo,*menor1,*menor2;
// construye el grafo inicial
  inicioHoriz=NULL;
  for (f=0;f<tamAlfabeto;f++)
  {
	  NuevoNodo=(TNodo *)malloc(sizeof(TNodo));
	  NuevoNodo->simbolo=f;
	  NuevoNodo->cuenta=LeeHistograma(f);
	  NuevoNodo->der=inicioHoriz;
	  NuevoNodo->infDer=NuevoNodo->infIzq=NULL;
	  inicioHoriz=NuevoNodo;
  }
// construye el árbol 
  do
  { 
     menor1=SacaMenor();
	 menor2=SacaMenor();
	 if (menor2!=NULL)
	 {
		 JuntaNodo(menor1,menor2);
	 }
  } while(menor2!=NULL);
  return menor1;
}

// Libera la memoria ocupada por el arbol (recursivamente) a partir de su raiz
void DestruyeArbol(TNodo *Raiz)
{
  if (Raiz->infDer==NULL) 
  {
	free(Raiz);
	return;
  }
  DestruyeArbol(Raiz->infDer);
  DestruyeArbol(Raiz->infIzq);
  free(Raiz);
  return;
}

//
// Rutinas para la codificacion
//

// Construye la tabla de codigos para la codificacion a partir del arbol (recursivamente)
void ConstruyeCodigos(TNodo *Raiz, unsigned int codigo, unsigned char longcodigo)
{
  if (Raiz->infDer==NULL) 
  {
    codigos[Raiz->simbolo]=codigo;
	longCodigos[Raiz->simbolo]=longcodigo;
	return;
  }
  ConstruyeCodigos(Raiz->infDer,RAMA_DER|(codigo<<1),longcodigo+1);
  ConstruyeCodigos(Raiz->infIzq,RAMA_IZQ|(codigo<<1),longcodigo+1);
  return;
}

// Inicializa Huffman, a partir de (1) la longitud del alfabeto, (2) la longitud de la cadena, 
// (3) la cadena a codificar y (4) el nombre del fichero que va a contener la informacion codificada
int InicializaHuffmanCod(int longitudAlfabeto, unsigned int longCadena, char *cadena, char *nombre)
{
  int error;
  error=InicializaEscritura(nombre);
  if (error!=0) return error;

  EscribePalabra(32,longCadena);
  tamAlfabeto=longitudAlfabeto;
  codigos=(unsigned int*)malloc(tamAlfabeto*sizeof(unsigned int));
  longCodigos=(unsigned int*)malloc(tamAlfabeto*sizeof(unsigned int));

  cuentaDeBits=cuentaDeSimbolos=0;
  InicializaHistograma(tamAlfabeto,longCadena,cadena);
  nodoRaiz=ConstruyeArbol();
  GuardaHistograma();
  ConstruyeCodigos(nodoRaiz,0,0);
  DestruyeArbol(nodoRaiz);

  return 0;
}

// Codifica un simbolo
void CodificaSimbolo(int simbolo)
{  	
	EscribePalabra(longCodigos[simbolo], codigos[simbolo]);
	cuentaDeBits+=longCodigos[simbolo];
	cuentaDeSimbolos++;
} 

// Finaliza Huffman, liberando recursos
void FinalizaHuffmanCod()
{
  free(longCodigos);
  free(codigos);    
  FinalizaHistograma();
  FinalizaEscritura();
}

//
// Rutinas para la decodificacion
//

// A partir de (1) un nodo del arbol y (2) la rama descendiente a seguir
// devuelve su nodo hijo por la rama indicada por (2)
TNodo *Descendiente(TNodo *nodo, unsigned char bit)
{
  if (bit==RAMA_DER) return nodo->infDer;
         else return nodo->infIzq;
}

// Inicializa decodificacion Huffman, a partir de (1) la longitud del alfabeto, 
// (2) la longitud de la cadena y (4) el nombre del fichero que contiene la informacion codificada
int InicializaHuffmanDecod(int longitudAlfabeto, unsigned int *longCadena, char *nombre)
{
  int error;
  tamAlfabeto=longitudAlfabeto;
  error=InicializaLectura(nombre);
  if (error!=0) return error;
  *longCadena=LeePalabra(32);
  InicializaHistogramaDeDisco(tamAlfabeto);
  nodoRaiz=ConstruyeArbol();
  FinalizaHistograma();
  return 0;
}

// Decodifica un simbolo
int DecodificaSimbolo()
{
	TNodo *Desc=nodoRaiz;

	while (Desc->infIzq!=NULL)
		Desc=Descendiente(Desc, LeeBit());
	return Desc->simbolo;
}

// Finaliza la decodificacion Huffman, liberando recursos
void FinalizaHuffmanDecod()
{
  DestruyeArbol(nodoRaiz);
  FinalizaLectura();
}

//
// Rutinas para calcular informacion
//

// Calcula los bits por simbolo usados en la codificacion
float CalculaBitsPorSimbolo()
{
  return ((float)cuentaDeBits)/((float)cuentaDeSimbolos);
}

// Imprime los codigos a emplear para la codificacion
void ImprimeCodigos()
{
  int f,g;
  printf("\nTABLA DE CODIGOS:\n");
  for (f=0;f<tamAlfabeto;f++) 
  {
	printf("- Simbolo %d ('%c'): ",f,f+'a');
	for (g=longCodigos[f]-1;g>=0;g--) printf("%d",(codigos[f]>>g)&0x01);
	printf("\n");

  }
}
