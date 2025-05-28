//
// PRACTICAS REDES MULTIMEDIA (RMM)
// FACULTAD DE INFORMATICA - UPV
//
// COMPRESION DE VIDEO 
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CompVideo.h"
#include "Cuantiza.h"
#include "DCT.h"
#include "Huffman.h"

#define COMPRIME 1
#define DESCOMPRIME 2

#define VERBOSE 1
#define MIN_SIMBOL_DE_HUFFMAN -15
#define MAX_SIMBOL_DE_HUFFMAN +15

// PARAMETROS DEL VIDEO A COMPRIMIR
static char *ficheroEntradaPorDefecto="salesman";
static int opcionPorDefecto=COMPRIME;
static int anchoPorDefecto=352;
static int altoPorDefecto=288;
static float cuantizacionPorDefecto=3.5;
static int NFRAMESPorDefecto=21;
static int MetodoPorDefecto=COD_DIF;
static int PframesPorDefecto=0;
static int TamVentanaPorDefecto=8;

// FIN DE PARAMETROS DEL VIDEO A COMPRIMIR

static char *ficheroEntrada=ficheroEntradaPorDefecto;
static int opcion=opcionPorDefecto;
static int ancho=anchoPorDefecto;
static int alto=altoPorDefecto;
static float cuantizacion=cuantizacionPorDefecto;
static int NFRAMES=NFRAMESPorDefecto;
static int Metodo=MetodoPorDefecto;
static int Pframes=PframesPorDefecto;
static int TamVentana=TamVentanaPorDefecto;

void CompruebaError(int error)
{
  if (error!=0)
  {
    if (error==1) printf("\nError al leer fichero\n");
    if (error==2) printf("\nEl ancho y alto de todos los planos deben de ser multiplos de 16\n");
    if (error==3) printf("\nError al escribir fichero\n");
    if (error==4) printf("\nCuantizacion fuera de rango\n");
    exit(1);
  }
}

void ProcesaArgumentos(int argc, char **argv)
{
  if (argc>=2)
  {
	  if ( 
		  (argc!=8 && argc!=10) || (strcmp(argv[1],"-c") && strcmp(argv[1],"-d")) || atoi(argv[3])==0 || atoi(argv[4])==0 ||
		   atoi(argv[6])==0 || atof(argv[5])==0.0 || (argc==10 && strcmp(argv[8],"-e")) || (argc==10 && atoi(argv[9])==0)
         )
	  { 
		printf("\nTeclea \"{-c|-d} DirVideo ancho alto cuantizacion NFrames PFrames {-e tamVent}\"\n"); 
		exit(1); 
	  }
	  if (!strcmp(argv[1],"-c")) opcion=COMPRIME; else opcion=DESCOMPRIME;
	  ficheroEntrada=argv[2];
	  ancho=atoi(argv[3]);
	  alto=atoi(argv[4]); 
	  cuantizacion=(float)atof(argv[5]); 
	  NFRAMES=atoi(argv[6]);
	  Pframes=atoi(argv[7]);
	  if (argc==10)
	  {
		  Metodo=ESTIM_MOV;
		  TamVentana=atoi(argv[9]);
	  }
  }
}

void CompruebaHuffman(char *NombreFich)
{
    unsigned char cadenaPrueba[19]={0,0,1,0,0,0,2,2,4,0,0,2,2,2,0,3,4,4,4};
	int f; unsigned int ns;

	printf("Histograma.cpp y Huffman.cpp... ");
    if (InicializaHuffmanCod(5,19,(char *)cadenaPrueba, NombreFich)!=0)
	{
	  printf("\nFALLO AL INICIALIZAR CODIFICACION HUFFMAN\n");
	  exit(1);
	}
    for (f=0;f<19;f++) CodificaSimbolo(cadenaPrueba[f]);
    FinalizaHuffmanCod(); 
    if (InicializaHuffmanDecod(5,&ns,NombreFich)!=0)
	{
	  printf("\nFALLO AL INICIALIZAR DECODIFICACION HUFFMAN\n");
	  exit(1);
	}
    for (f=0;f<19;f++) if (DecodificaSimbolo()!=cadenaPrueba[f])
	{
	  printf("\nHUFFMAN MAL IMPLEMENTADO, revisa la practica de Huffman\n");
	  exit(1);
	}
	printf("OK.\n");
}

void CompruebaDCT()
{
	double linea[8]={140,144,147,140,140,155,179,175};
	int result1[8]={431,-33,20,-7,-10,6,-3,2};
	int result2[8]={140,144,147,140,140,155,179,175};
	printf("DCT.cpp... ");
	int f;
	DCTn_1D(linea);
	for (f=0;f<8;f++) if ((int)linea[f]!=(int)result1[f])
	{
	  printf("\n1D_DCT NORMAL MAL IMPLEMENTADA, revisa la practica de Imagen\n");
	  exit(1);
	}
	IDCTn_1D(linea);	
	for (f=0;f<8;f++) if ((((int)linea[f])!=(int)result2[f]) && (((int)linea[f])+1!=(int)result2[f]))
	{ 
	  printf("\n1D_IDCT NORMAL MAL IMPLEMENTADA, revisa la practica de Imagen\n");
	  exit(1);
	}
	printf("OK.\n");
}

void CompruebaCuantizacion()
{
	double **bloque,**bloquecomp,Q=3.5;
	int x,y,g; 
	
	printf("Cuantiza.cpp... ");
	bloque=(double**)malloc(8*sizeof(double*));
	for (g=0;g<8;g++) bloque[g]=(double*)malloc(8*sizeof(double));
	bloquecomp=(double**)malloc(8*sizeof(double*));
	for (g=0;g<8;g++) bloquecomp[g]=(double*)malloc(8*sizeof(double));
	for (x=0;x<8;x++) for (y=0;y<8;y++) {bloque[y][x]=((double)x+y*8); bloquecomp[y][x]=((int)((double)x+y*8))/((int)(2.0*Q));}
	CuantizaUniformeBloque(Q,bloque); 
	for (x=0;x<8;x++) for (y=0;y<8;y++) if (bloque[y][x]!=bloquecomp[y][x])
	{ 
	  printf("\nCALCULO DE LA CUANTIZACION MAL IMPLEMENTADO, revisa la practica de Imagen\n");
	  exit(1);
	}
	CuantizaUniformeInversaBloque(Q,bloque); 
	for (x=0;x<8;x++) for (y=0;y<8;y++) bloquecomp[y][x]=(bloquecomp[y][x]<0.0?(2*bloquecomp[y][x]+1)*Q:(bloquecomp[y][x]>0.0?(2*bloquecomp[y][x]-1)*Q:0.0));
	for (x=0;x<8;x++) for (y=0;y<8;y++) if (bloque[y][x]!=bloquecomp[y][x])
	{ 
	  printf("\nCALCULO DE LA CUANTIZACION INVERSA MAL IMPLEMENTADO, revisa la practica de Imagen\n");
	  exit(1);
	}
	for (g=0;g<8;g++) free(bloque[g]); free(bloque);
	for (g=0;g<8;g++) free(bloquecomp[g]); free(bloquecomp);
	printf("OK.\n");
}

void CompruebaModulosParaImagen(char *NombreFich)
{
	printf("\nComprobando la correccion de los modulos:\n");
	CompruebaHuffman(NombreFich); 
	CompruebaDCT();
	CompruebaCuantizacion();
}

int main(int argc, char **argv)
{
  int error;
  printf("\n--------------------------------\n");
  printf("PRACTICAS REDES MULTIMEDIA (RMM)\n");
  printf("Facultad de Informatica, UPV\n");
  printf("--------------------------------\n");
  printf("Practica de compresion de video\n");
  printf("--------------------------------\n");

  ProcesaArgumentos(argc,argv);

// hacer que se pase por parametro el numero de frames en ambos casos

  if (opcion==COMPRIME)
  {
	CompruebaModulosParaImagen("~temp.tmp");
	printf("\n\nCOMPRIMIENDO VIDEO %s\n\n",ficheroEntrada);
	error=ComprimeVideoLuminancia(ficheroEntrada,ancho,alto,NFRAMES,cuantizacion,MIN_SIMBOL_DE_HUFFMAN,MAX_SIMBOL_DE_HUFFMAN,Pframes,Metodo,TamVentana,VERBOSE);
	CompruebaError(error);
	printf("\n\nCOMPRESION TERMINADA\n\n");  
  }
  else
  {
	printf("\n\nDESCOMPRIMIENDO VIDEO %s\n\n",ficheroEntrada);
	error=DescomprimeVideoLuminancia(ficheroEntrada,ancho,alto,NFRAMES,MIN_SIMBOL_DE_HUFFMAN,MAX_SIMBOL_DE_HUFFMAN,Pframes,Metodo,VERBOSE);
	CompruebaError(error);
	printf("\n\nDESCOMPRESION TERMINADA\n\n");
  }
  return 0;
}


