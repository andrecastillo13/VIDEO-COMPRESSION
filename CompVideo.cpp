#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "CompVideo.h"
#include "DCT.h"
#include "Huffman.h"
#include "Histograma.h"
#include "Cuantiza.h"
#include "Bloques.h"
#include "FichBits.h"
#include "ImagenES.h"
#include <sys/stat.h>

// FIN DE PARAMETROS DEL COMPRESOR DE VIDEO

#define MINQ 0.5
#define MAXQ 30.0

#ifdef WIN32
#define SEPARADOR "\\"
#else
#define SEPARADOR "/"
#endif

//
//
// RUTINAS AUXILIARES
//
//

unsigned char SaturaAByte(int numero)
{
  if (numero<0) return 0;
  else if (numero>255) return 255;
  else return numero;
}

int nbits(int x)
{
  int n=0;
  while (x!=0)
  {
	  x=x>>1;
	  n++;
  }
  return n;
}

void ConstruyeNombreSinExt(char *Nombre, char *NombreOrig, int frame)
{
  char c1,c2,c3;

  c3='0'+((frame%1000)/100);
  c2='0'+((frame%100)/10);
  c1='0'+(frame%10);
  sprintf(Nombre,"%s%s%s_%c%c%c",NombreOrig,SEPARADOR,NombreOrig,c3,c2,c1);
}

// Calcula el MSE de los planos Y1, Y2 de dimensiones ancho x alto
float CalculaMSE(unsigned char **Y1, unsigned char **Y2, int ancho, int alto)
{
  double mse = 0.0;
  for (int y = 0; y < alto; y++) {
    for (int x = 0; x < ancho; x++) {
      int diff = Y1[y][x] - Y2[y][x];
      mse += diff * diff;
    }
  }
  mse /= (ancho * alto);
  return (float)mse;
}


// Calcula el PSNR de los planos Y1, Y2 de dimensiones ancho x alto
float CalculaPSNR(unsigned char **Y1, unsigned char **Y2, int ancho, int alto)
{
  float mse = CalculaMSE(Y1, Y2, ancho, alto);
  if (mse == 0.0) return INFINITY;
  return 10.0f * log10f((255.0f * 255.0f) / mse);
}

//
//
// RUTINAS PARA CODIFICACION Y DECODIFICACION UNARIA (PARA LOS VECTORES DE MOVIMIENTO)
//
//

// Codifica una palabra con codificacion unaria
void CodificaUnaria(int palabra)
{
  int absPal;

  if (palabra==0)
	  EscribeBit(1);
  else
  {
	  absPal=abs(palabra);
	  while (absPal!=0)
	  {
		EscribeBit(0);
		absPal--;
	  }
	  EscribeBit(1);
	  if (palabra<0) EscribeBit(1); else EscribeBit(0);
  }
}

// Decodifica una palabra con decodificacion unaria
int DecodificaUnaria()
{
  int palabra=1;
  int bit=LeeBit();

  if (bit==1) return 0;
  else
  {
	while (LeeBit()==0) palabra++;
	if (LeeBit()==1) palabra=-palabra;
	return palabra;
  }
}

//
//
// RUTINAS PARA COMPRESION Y DESCOMPRESION DE PLANOS DE LUMINANCIA
//
//


// comprime la imagen en tonos de grises (NombreOrig), cuyas dimensiones son ancho x alto, usando
// una cuantizacion de Q, y codificando por Huffman los coeficientes
// entre MinSimboloEnHuffman y MaxSimboloEnHuffman, la imagen la guarda en otro fichero (NombreComp)
// NOTA: si verbose es true, se muestra informacion por pantalla
int ComprimeImagenLuminancia(unsigned char **Y, char *NombreComp, int ancho, int alto, float Q,
						     int MinSimboloEnHuffman, int MaxSimboloEnHuffman, int verbose)
{
  unsigned int nBloques;
  double ***Bloques;
  char *cadena;
  int *cadenaESC;
  int EOB, escape;
  unsigned int indiceCadena, indiceCadenaESC;
  int absSimb,nbitsAbsSimb;
  unsigned int f,g;
  int primerDC;

// Reserva los bloques y los asigna
  if (verbose) printf("\nConstruye los bloques...\n");
  nBloques=(ancho/8)*(alto/8);
  ReservaBloques(nBloques,&Bloques);
  AsignaBloques(ancho,alto,Y,0,Bloques);

// Calculamos la DCT de los bloques
  if (verbose) printf("\nCalcula la DCT 2D de los bloques...\n");
  for (f=0;f<nBloques;f++)
	  DCT_2Dn(Bloques[f]);

// Cuantiza los bloques
  if (verbose) printf("\nCuantiza los bloques...\n");
  for (f=0;f<nBloques;f++)
	  CuantizaUniformeBloque(Q, Bloques[f]);

// Reserva memoria para las cadenas para Huffman
  cadena=(char*)malloc(3*ancho*alto*sizeof(unsigned char));
  cadenaESC=(int*)malloc((ancho*alto)*sizeof(int));

// Inicializa variables para simbolos y cadenas
  indiceCadena=indiceCadenaESC=0;
  EOB=MaxSimboloEnHuffman-MinSimboloEnHuffman+1;
  escape=EOB+1;

// Calcula las cadenas para comprimir con Huffman
  if (verbose) printf("\nConstruye la cadena para Huffman...\n");
  primerDC=(int)Bloques[0][0][0];
  MeteBloqueEnCadena(MinSimboloEnHuffman, MaxSimboloEnHuffman, EOB, escape, cadena,&indiceCadena,cadenaESC,&indiceCadenaESC,(int)Bloques[0][0][0],Bloques[0]);
  for (f=1;f<nBloques;f++)
	  MeteBloqueEnCadena(MinSimboloEnHuffman, MaxSimboloEnHuffman, EOB, escape, cadena,&indiceCadena,cadenaESC,&indiceCadenaESC,(int)Bloques[f-1][0][0],Bloques[f]);

// liberamos la memoria ocupada por los bloques
  LiberaBloques(nBloques,&Bloques);

// Inicia Huffman (calculando su histograma)
  if (verbose) printf("\nInicializa Huffman...\n");
  if (InicializaHuffmanCod((MaxSimboloEnHuffman-MinSimboloEnHuffman+1)+1+1,indiceCadena,cadena, NombreComp)!=0) return 3;
  if (verbose)
  {
	  ImprimeHistograma();
	  ImprimeCodigos();
  }

// Incluye informacion adicional para la cabecera del fichero
  EscribePalabra(24,ancho);
  EscribePalabra(24,alto);
  EscribePalabra(24,primerDC);
  EscribePalabra(32,*((int*)(&Q)));

// Codifica los simbolos con Huffman a partir de la informacion de las cadenas
  if (verbose) printf("\nCodificando simbolos con Huffman...\n");
  for (f=0,g=0;f<indiceCadena;f++)
  {
	  CodificaSimbolo(cadena[f]);
	  if (cadena[f]==escape)
	  {
		  if (cadenaESC[g]<0) EscribeBit(1); else EscribeBit(0);
		  absSimb=abs(cadenaESC[g]);
		  nbitsAbsSimb=nbits(absSimb);
		  EscribePalabra(5,nbitsAbsSimb);
		  EscribePalabra(nbitsAbsSimb,absSimb);
		  g++;
	  }
  }

// Finaliza Huffman
  FinalizaHuffmanCod();

// Libera la memoria de las cadenas de Huffman
  free(cadena);
  free(cadenaESC);

  return 0;
}

// descomprime la imagen en tonos de grises (NombreComp), decodificando por Huffman los coeficientes
// entre MinSimboloEnHuffman y MaxSimboloEnHuffman, la imagen descomprimida la guarda en otro fichero (NombreOrig)
// NOTA: si verbose es true, se muestra informacion por pantalla
int DescomprimeImagenLuminancia(unsigned char **Y, char *NombreComp,
						        int MinSimboloEnHuffman, int MaxSimboloEnHuffman, int verbose)
{
  int ancho,alto;
  unsigned int nBloques;
  double ***Bloques;
  char *cadena;
  int *cadenaESC;
  int EOB, escape;
  unsigned int indiceCadena, indiceCadenaESC;
  unsigned int f,g;
  int primerDC;
  int qint;
  float Q;
  int signo,nbits;

// Inicializa Huffman (arbol y ficheros)
  if (verbose) printf("\nInicializa Huffman para la decodificacion...\n");
  if (InicializaHuffmanDecod((MaxSimboloEnHuffman-MinSimboloEnHuffman+1)+1+1,&indiceCadena,NombreComp)!=0) return 1;

// Lee los datos de la cabecera
  ancho=LeePalabra(24);
  alto=LeePalabra(24);
  primerDC=LeePalabra(24);
  qint=LeePalabra(32);
  Q=*((float*)(&qint));

// Reserva memoria para las cadenas para Huffman
  cadena=(char*)malloc(3*ancho*alto*sizeof(unsigned char));
  cadenaESC=(int*)malloc((ancho*alto)*sizeof(int));
  EOB=MaxSimboloEnHuffman-MinSimboloEnHuffman+1;
  escape=EOB+1;

// Decodifica los simbolos sobre las cadenas
  if (verbose) printf("\nDecodifica simbolos con Huffman...\n");
  for (f=0,g=0;f<indiceCadena;f++)
  {
	  cadena[f]=DecodificaSimbolo();
	  if (cadena[f]==escape)
	  {
		  signo=LeeBit();
		  nbits=LeePalabra(5);
		  cadenaESC[g]=LeePalabra(nbits);
		  if (signo==1) cadenaESC[g]=-cadenaESC[g];
		  g++;
	  }
  }

// Finaliza Huffman
  FinalizaHuffmanDecod();

// Reserva bloques
  nBloques=(ancho/8)*(alto/8);
  ReservaBloques(nBloques,&Bloques);

// Calcula los bloques a partir de la informacion decodificada en las cadenas
  if (verbose) printf("\nObtiene los bloques a partir de la informacion leida de las cadenas...\n");
  indiceCadena=indiceCadenaESC=0;
  SacaBloqueDeCadena(MinSimboloEnHuffman, MaxSimboloEnHuffman, EOB, escape, cadena,&indiceCadena,cadenaESC,&indiceCadenaESC,primerDC,Bloques[0]);
  for (f=1;f<nBloques;f++)
	  SacaBloqueDeCadena(MinSimboloEnHuffman, MaxSimboloEnHuffman, EOB, escape, cadena,&indiceCadena,cadenaESC,&indiceCadenaESC,(int)Bloques[f-1][0][0],Bloques[f]);

// Libera la memoria de las cadenas de Huffman
  free(cadena);
  free(cadenaESC);

// Aplicamos la cuantizacion inversa a los bloques
  if (verbose) printf("\nAplicando la cuantizacion inversa a bloques...\n");
  for (f=0;f<nBloques;f++)
	  CuantizaUniformeInversaBloque(Q, Bloques[f]);

// Calculamos la iDCT sosbre los bloques

  if (verbose) printf("\nCalcula la IDCT de los bloques...\n");
  for (f=0;f<nBloques;f++)
	  IDCT_2Dn(Bloques[f]);

// Reconstruimos la imagen a partir de los bloques
  if (verbose) printf("\nReconstruye la imagen a partir de la informacion de los bloques...\n");
  RecuperaDesdeBloques(ancho,alto,Y,0,Bloques);

// liberamos la memoria ocupada por los bloques
  LiberaBloques(nBloques,&Bloques);

  return 0;
}

//
//
// RUTINAS PARA ESTIMACION Y COMPENSACION DE MOVIMIENTO
//
//

// Calcula el MSE entre el macrobloque situado en la posicion (xorig,yorig) en el plano Yref
// y la posicion (xref,yref) en el plano Yref
float CalculaMSEMacrobloque(unsigned char **Yorig, unsigned char **Yref, int xorig, int yorig, int xref, int yref, int ancho, int alto)
{
  int x1, x2, y1, y2, MSE=0;

  if (xref<0 || xref>=(ancho-15) || yref<0 || yref>=(alto-15)) return 256.0*256.0; // si el macrobloque esta fuera de la imagen de referencia

  for (x1=xorig, x2=xref; x1<xorig+16; x1++,x2++)                                  // devuelve el maximo valor posible de MSE
	for (y1=yorig, y2=yref; y1<yorig+16; y1++,y2++)
	MSE+=(Yorig[y1][x1]-Yref[y2][x2])*(Yorig[y1][x1]-Yref[y2][x2]);
  return (float) (MSE/((float)16.0*16.0));
}

// Compensa el macrobloque situado en la posicion (xorig,yorig) en el plano Yref
// con el de la posicion (xref,yref) en el plano Yref
void CompensaMacrobloque(unsigned char **Yorig, unsigned char **Yref, int xorig, int yorig, int xref, int yref)
{
  int x1,y1,x2,y2;

  for (x1=xorig, x2=xref; x1<xorig+16; x1++,x2++)
	for (y1=yorig, y2=yref; y1<yorig+16; y1++,y2++)
	  Yorig[y1][x1]=SaturaAByte(Yorig[y1][x1]-Yref[y2][x2]+128);
}

// Compensacion inversa del macrobloque situado en la posicion (xorig,yorig) en el plano Yref
// con el de la posicion (xref,yref) en el plano Yref
void CompensaMacrobloqueInversa(unsigned char **Yorig, unsigned char **Yref, int xorig, int yorig, int xref, int yref)
{
  int x1,y1,x2,y2;

  for (x1=xorig, x2=xref; x1<xorig+16; x1++,x2++)
	for (y1=yorig, y2=yref; y1<yorig+16; y1++,y2++)
	  Yorig[y1][x1]=SaturaAByte(Yorig[y1][x1]+Yref[y2][x2]-128);
}

// Busca el macrobloque mas parecido al situado en (x1,y1) del plano Y, en el plano Yref
// usando una ventana de lado "2*ladomedio+1"
// la posicion del macrobloque mas parecido lo devuelve en (Min_x,Min_y)
void EstimaMacrobloque(unsigned char **Y, unsigned char **Yref, int x1, int y1,
                       int *Min_x, int *Min_y, int ancho, int alto, int ladomedio) {
    float MinMSE = 256.0 * 256.0;
    float mse;
    int x, y;

    for (y = y1 - ladomedio; y <= y1 + ladomedio; y++) {
        for (x = x1 - ladomedio; x <= x1 + ladomedio; x++) {
            mse = CalculaMSEMacrobloque(Y, Yref, x1, y1, x, y, ancho, alto);
            if (mse < MinMSE) {
                MinMSE = mse;
                *Min_x = x;
                *Min_y = y;
            }
        }
    }
}


// Realiza el proceso de estimacion y compensacion de movimiento sobre el plano Y
// utilizando como plano de referencia Yref
void EstimacionYCompensacionDeMovimiento(unsigned char **Y, unsigned char **Yref, int ancho, int alto, int TamVentana)
{
  int x,y,xMin,yMin;

  for (x=0;x<ancho;x+=16)
	for (y=0;y<alto;y+=16)
	{
	  EstimaMacrobloque(Y,Yref,x,y,&xMin,&yMin,ancho,alto,TamVentana);
	  CompensaMacrobloque(Y, Yref, x, y, xMin, yMin);
	  CodificaUnaria(xMin-x);
	  CodificaUnaria(yMin-y);
	}
}

// Realiza el proceso de estimacion y compensacion de movimiento inverso sobre el plano Y
// utilizando como plano de referencia Yref
void EstimacionYCompensacionDeMovimientoInversa(unsigned char **Y, unsigned char **Yref, int ancho, int alto)
{
  int x,y,xMin,yMin;

  for (x=0;x<ancho;x+=16)
	for (y=0;y<alto;y+=16)
	{
	  xMin=DecodificaUnaria()+x;
	  yMin=DecodificaUnaria()+y;
	  CompensaMacrobloqueInversa(Y, Yref, x, y, xMin, yMin);
	}
}


//
//
// RUTINAS PARA CODIFICACION DIFERENCIAL
//
//

// Realiza el proceso de codificacion diferencial del plano Y
// utilizando como plano de referencia Yref
void CodificacionDiferencial(unsigned char **Y, unsigned char **Yref, int ancho, int alto)
{
	// COMPLETAR
}

// Realiza el proceso de codificacion diferencial inversa del plano Y
// utilizando como plano de referencia Yref
void CodificacionDiferencialInversa(unsigned char **Y, unsigned char **Yref, int ancho, int alto)
{
  for (int x=0;x<ancho;x++) for (int y=0;y<alto;y++) Yref[y][x]=SaturaAByte(Y[y][x]+Yref[y][x]-128);
}


//
//
// RUTINAS PARA COMPRESION Y DESCOMPRESION DE VIDEO
//
//


// comprime nFrames del video en tonos de grises (NombreOrig), cuyas dimensiones son ancho x alto, usando
// una cuantizacion de Q
// NOTA: si verbose es true, se muestra informacion por pantalla

int ComprimeVideoLuminancia(char *NombreOrig, int ancho, int alto, int nFrames, float Q,
						    int MinSimboloEnHuffman, int MaxSimboloEnHuffman, int Pframes, int Metodo, int TamVentana, int verbose)
{
  unsigned char **Y,**Yref;
  char NombreFte[128];
  char NombreDest[128];
  char NombreVect[128];
  char NombreSinExt[128];
  int frame,error;
  struct stat statbuf;
  unsigned int tamfich;
  float BitRate,BitRateMedio=0,MSE,MSEMedio=0;

  if (ancho%16!=0 || alto%16!=0) return 2;
  if (Q<MINQ || Q>MAXQ) return 4;

  ReservaPlano(ancho, alto, &Y);
  ReservaPlano(ancho, alto, &Yref);

  for (frame=1;frame<=nFrames;frame++)
  {
    ConstruyeNombreSinExt(NombreSinExt,NombreOrig,frame);
    sprintf(NombreFte,"%s.raw",NombreSinExt);
    sprintf(NombreDest,"%s.cmp",NombreSinExt);
    sprintf(NombreVect,"%s.vec",NombreSinExt);
    if (CargaRawLuminancia(NombreFte, ancho, alto, Y)!=0) return 1;

	if ((frame-1)%(Pframes+1))  // ESTAMOS EN UN CUADRO DE TIPO P?
	{	// SI
		if (Metodo==COD_DIF)
		  CodificacionDiferencial(Y, Yref, ancho, alto);
		else if (Metodo==ESTIM_MOV)
		{
		  InicializaEscritura(NombreVect);
		  EstimacionYCompensacionDeMovimiento(Y, Yref, ancho, alto,TamVentana);
		  FinalizaEscritura();
		}
		if (verbose) printf("Frame %d, Tipo P, ",frame);
	}
	else if (verbose) printf("Frame %d, Tipo I, ",frame);

	error=ComprimeImagenLuminancia(Y, NombreDest, ancho, alto, Q, MinSimboloEnHuffman, MaxSimboloEnHuffman, 0);
	if (error) return error;

	if ((frame-1)%(Pframes+1))
	{
		DescomprimeImagenLuminancia(Y, NombreDest, MinSimboloEnHuffman, MaxSimboloEnHuffman, 0); // obtiene la Y despues de compresion
		if (Metodo==COD_DIF)
		  CodificacionDiferencialInversa(Y, Yref, ancho, alto);
		else if (Metodo==ESTIM_MOV)
		{
		  InicializaLectura(NombreVect);
		  EstimacionYCompensacionDeMovimientoInversa(Y,Yref,ancho,alto);
		  for (int jj=0;jj<ancho;jj++) for (int ii=0;ii<alto;ii++) Yref[ii][jj]=Y[ii][jj];
		  FinalizaLectura();
		}
	}
	else
		DescomprimeImagenLuminancia(Yref, NombreDest, MinSimboloEnHuffman, MaxSimboloEnHuffman, 0); // obtiene la Y despues de compresion

	stat(NombreDest, &statbuf);
	tamfich=statbuf.st_size;
	if (Metodo==ESTIM_MOV && (frame-1)%(Pframes+1))
	{
	    stat(NombreVect, &statbuf);
		tamfich+=statbuf.st_size;
	}
	BitRate=((float)tamfich*8)/((float)ancho*alto);
	BitRateMedio+=BitRate;
	if (verbose) printf("%.2f bpp ",BitRate);

	CargaRawLuminancia(NombreFte, ancho, alto, Y);
	MSE=CalculaMSE(Yref,Y,ancho,alto);
    if (verbose) printf(", MSE %.2f, PSNR %.2f dB\n",MSE,CalculaPSNR(Yref,Y,ancho,alto));
	MSEMedio+=MSE;
  }
  if (verbose) printf("Bit rate medio: %.2f bpp\nMSE medio: %.2f\n",BitRateMedio/nFrames,MSEMedio/nFrames);
  LiberaPlano(ancho, alto, &Y);
  LiberaPlano(ancho, alto, &Yref);
  return 0;
}


// descomprime nFrames del video en tonos de grises (NombreOrig), cuyas dimensiones son ancho x alto
// NOTA: si verbose es true, se muestra informacion por pantalla

int DescomprimeVideoLuminancia(char *NombreOrig,int ancho, int alto, int nFrames,
							   int MinSimboloEnHuffman, int MaxSimboloEnHuffman, int Pframes, int Metodo, int verbose)
{
  char NombreFte[128];
  char NombreDest[128];
  char NombreVect[128];
  char NombreSinExt[128];

  int frame;
  unsigned char **Y,**Yref;

  ReservaPlano(ancho, alto, &Y);
  ReservaPlano(ancho, alto, &Yref);
  for (frame=1;frame<=nFrames;frame++)
  {
    ConstruyeNombreSinExt(NombreSinExt,NombreOrig,frame);
    sprintf(NombreFte,"%s.cmp",NombreSinExt);
    sprintf(NombreDest,"%sRec.raw",NombreSinExt);
    sprintf(NombreVect,"%s.vec",NombreSinExt);

	if ((frame-1)%(Pframes+1))
	{
		if (DescomprimeImagenLuminancia(Y, NombreFte, MinSimboloEnHuffman, MaxSimboloEnHuffman, 0)!=0) return 1;
		if (Metodo==COD_DIF)
          CodificacionDiferencialInversa(Y, Yref, ancho, alto);
		else if (Metodo==ESTIM_MOV)
		{
		  InicializaLectura(NombreVect);
		  EstimacionYCompensacionDeMovimientoInversa(Y,Yref,ancho,alto);
		  for (int jj=0;jj<ancho;jj++) for (int ii=0;ii<alto;ii++) Yref[ii][jj]=Y[ii][jj];
		  FinalizaLectura();
		}
		if (verbose) printf("Descomprimiendo frame de tipo P\n");
	}
	else
	{
	    if (DescomprimeImagenLuminancia(Yref, NombreFte, MinSimboloEnHuffman, MaxSimboloEnHuffman, 0)!=0) return 1;
		if (verbose) printf("Descomprimiendo frame de tipo I\n");
	}
	GuardaRawLuminancia(NombreDest, ancho, alto, Yref);
  }
  LiberaPlano(ancho, alto, &Y);
  LiberaPlano(ancho, alto, &Yref);
  return 0;
}
