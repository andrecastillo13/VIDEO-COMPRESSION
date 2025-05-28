#include <stdio.h>
#include <stdlib.h>
#include "ImagenES.h"

//
//  RUTINAS PARA RESERVA DE MEMORIA
//

void ReservaPlano(int ancho, int alto, unsigned char ***Plano)
{
  *Plano=(unsigned char **)malloc(alto*sizeof(unsigned char *));
  for (int f=0;f<alto;f++)
    *((*Plano)+f)=(unsigned char *)malloc(ancho*sizeof(unsigned char));
}

void LiberaPlano(int ancho, int alto, unsigned char ***Plano)
{
  for (int f=0;f<alto;f++) free(*((*Plano)+f)); free(*Plano);
}


//
// RUTINAS GENERALES PARA LECTURA/ESCRITURA DE FICHEROS
//

void LeeBloque(FILE *fich,int tam, unsigned char *PuntBufferAux)
{
  for (int f=0;f<tam;f++) *PuntBufferAux++=(unsigned char)getc(fich);
}

void GuardaBloque(FILE *fich,int tam, unsigned char *PuntBufferAux)
{
  for (int f=0;f<tam;f++) putc(*PuntBufferAux++,fich);
}

void CargaPlano(FILE *fichero, int ancho, int alto, unsigned char **Plano)
{
  for (int f=0;f<alto;f++)
    LeeBloque(fichero, ancho, Plano[f]);
}

void GuardaPlano(FILE *fichero, int ancho, int alto, unsigned char **Plano)
{
  for (int f=0;f<alto;f++)
    GuardaBloque(fichero, ancho, Plano[f]);  
}

//
// RUTINAS PARA EL MANEJO DE FICHEROS DE LUMINANCIAS (Y)
//

int CargaRawLuminancia(char *Nombre, int ancho, int alto, unsigned char **Buffer)
{
  FILE *fich;

  fich=fopen(Nombre,"rb");
  if (fich==NULL) return 1;
  CargaPlano(fich, ancho, alto, Buffer);
  fclose(fich);
  return 0;
}

int GuardaRawLuminancia(char *Nombre, int ancho, int alto, unsigned char **Buffer)
{
  FILE *fich;

  fich=fopen(Nombre,"wb");
  if (fich==NULL) return 1;
  GuardaPlano(fich, ancho, alto, Buffer);
  fclose(fich);
  return 0;
}

//
// RUTINAS PARA EL MANEJO DE FICHEROS RGB
//

int CargaRawRGB(char *Nombre, int ancho, int alto, unsigned char **R, unsigned char **G, unsigned char **B)
{
  FILE *fich;

  fich=fopen(Nombre,"rb");
  if (fich==NULL) return 1;
  CargaPlano(fich, ancho, alto, R);
  CargaPlano(fich, ancho, alto, G);
  CargaPlano(fich, ancho, alto, B);
  fclose(fich);
  return 0;
}

int GuardaRawRGB(char *Nombre, int ancho, int alto, unsigned char **R, unsigned char **G, unsigned char **B)
{
  FILE *fich;

  fich=fopen(Nombre,"wb");
  if (fich==NULL) return 1;
  GuardaPlano(fich, ancho, alto, R);
  GuardaPlano(fich, ancho, alto, G);
  GuardaPlano(fich, ancho, alto, B);
  fclose(fich);
  return 0;
}

//
// RUTINAS PARA EL MANEJO DE FICHEROS YCbCr
//

int CargaRawYCbCr420(char *Nombre, int ancho, int alto, unsigned char **Y, signed char **Cb, signed char **Cr)
{
  FILE *fich;

  fich=fopen(Nombre,"rb");
  if (fich==NULL) return 1;
  CargaPlano(fich, ancho, alto, Y);
  CargaPlano(fich, ancho/2, alto/2, (unsigned char **) Cb);
  CargaPlano(fich, ancho/2, alto/2, (unsigned char **) Cr);
  fclose(fich);
  return 0;
}

int CargaRawYCbCr444(char *Nombre, int ancho, int alto, unsigned char **Y, signed char **Cb, signed char **Cr)
{
  FILE *fich;

  fich=fopen(Nombre,"rb");
  if (fich==NULL) return 1;
  CargaPlano(fich, ancho, alto, Y);
  CargaPlano(fich, ancho, alto, (unsigned char **) Cb);
  CargaPlano(fich, ancho, alto, (unsigned char **) Cr);
  fclose(fich);
  return 0;
}


int GuardaRawYCbCr420(char *Nombre, int ancho, int alto, unsigned char **Y, signed char **Cb, signed char **Cr)
{
  FILE *fich;

  fich=fopen(Nombre,"wb");
  if (fich==NULL) return 1;
  GuardaPlano(fich, ancho, alto, Y);
  GuardaPlano(fich, ancho/2, alto/2, (unsigned char **) Cb);
  GuardaPlano(fich, ancho/2, alto/2, (unsigned char **) Cr);
  fclose(fich);
  return 0;
}

int GuardaRawYCbCr444(char *Nombre, int ancho, int alto, unsigned char **Y, signed char **Cb, signed char **Cr)
{
  FILE *fich;

  fich=fopen(Nombre,"wb");
  if (fich==NULL) return 1;
  GuardaPlano(fich, ancho, alto, Y);
  GuardaPlano(fich, ancho, alto, (unsigned char **) Cb);
  GuardaPlano(fich, ancho, alto, (unsigned char **) Cr);
  fclose(fich);
  return 0;
}