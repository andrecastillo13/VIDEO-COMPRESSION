#include <stdlib.h>
#include "Bloques.h"

//
// funciones para reservar y liberar memoria para bloques
//

void ReservaBloques(int numero,double ****Bloques)
{
	int f,g;

	*Bloques=(double ***)malloc(numero*sizeof(double **));
	for (f=0;f<numero;f++)
	{
	  (*Bloques)[f]=(double **)malloc(8*sizeof(double*));
	  for (g=0;g<8;g++)
	    (*Bloques)[f][g]=(double*)malloc(8*sizeof(double));
	}
}

void LiberaBloques(int numero,double ****Bloques)
{
	int f,g;

	for (f=0;f<numero;f++)
	{
	  for (g=0;g<8;g++)
	    free((*Bloques)[f][g]);
	  free((*Bloques)[f]);
	}
	free(*Bloques);
}

//
// funciones para asignacion de los bloques a planos (e inversa)
//

void AsignaBloques(int ancho,int alto,unsigned char **Plano,int bloqueInicial,double ***Bloques)
{
	int bloqueActual=bloqueInicial;
	int x,y,i,j;

	for (x=0;x<ancho;x+=8)
	  for (y=0;y<alto;y+=8)
	  {
		for (i=0;i<8;i++)
		  for (j=0;j<8;j++)
			Bloques[bloqueActual][j][i]=Plano[y+j][x+i];
		bloqueActual++;
	  }
}

void RecuperaDesdeBloques(int ancho,int alto,unsigned char **Plano,int bloqueInicial,double ***Bloques)
{
	int bloqueActual=bloqueInicial;
	int x,y,i,j;

	for (x=0;x<ancho;x+=8)
	  for (y=0;y<alto;y+=8)
	  {
		for (i=0;i<8;i++)
		  for (j=0;j<8;j++)
		  {
			if (Bloques[bloqueActual][j][i]>255.0) Plano[y+j][x+i]=255;
			else if (Bloques[bloqueActual][j][i]<0.0) Plano[y+j][x+i]=0;
			else Plano[y+j][x+i]=(unsigned char)Bloques[bloqueActual][j][i];
		  }
		bloqueActual++;
	  }
}

void AsignaBloques(int ancho,int alto, signed char **Plano,int bloqueInicial,double ***Bloques)
{
	int bloqueActual=bloqueInicial;
	int x,y,i,j;

	for (x=0;x<ancho;x+=8)
	  for (y=0;y<alto;y+=8)
	  {
		for (i=0;i<8;i++)
		  for (j=0;j<8;j++)
			Bloques[bloqueActual][j][i]=Plano[y+j][x+i];
		bloqueActual++;
	  }
}

void RecuperaDesdeBloques(int ancho,int alto,signed char **Plano,int bloqueInicial,double ***Bloques)
{
	int bloqueActual=bloqueInicial;
	int x,y,i,j;

	for (x=0;x<ancho;x+=8)
	  for (y=0;y<alto;y+=8)
	  {
		for (i=0;i<8;i++)
		  for (j=0;j<8;j++)
			Plano[y+j][x+i]=(unsigned char)Bloques[bloqueActual][j][i];
		bloqueActual++;
	  }
}

//
// funciones para construir cadenas y reconstruir bloques a partir de las cadenas
//

void MeteSimboloEnCadena(int simbolo, int minSimb, int maxSimb, int EOB, int ESC, 
					     char *cad, unsigned int *ncad, int *cadXTRA, unsigned int *ncadXTRA)
{
	if (simbolo<minSimb || simbolo>maxSimb)
	{
		cad[*ncad]=ESC;
		(*ncad)++;
		cadXTRA[*ncadXTRA]=simbolo; 
		(*ncadXTRA)++;
	}
	else
	{ 
		cad[*ncad]=simbolo-minSimb;
		(*ncad)++;
	}
}

void MeteBloqueEnCadena(int minSimb, int maxSimb, int EOB, int ESC, 
					char *cad, unsigned int *ncad, int *cadXTRA, unsigned int *ncadXTRA, 
					int anterior,double **block)
{
	int x,y,run;

	MeteSimboloEnCadena((int)(block[0][0]-anterior), minSimb, maxSimb, EOB, ESC, cad, ncad, cadXTRA, ncadXTRA);
	x=1;y=0;run=0; 
	while (1)
	{
	  do 
	  {
		if (x<8 && y<8) 
		{ 
		  if (block[y][x]==0.0) run++;
		  else 
		  {
			  MeteSimboloEnCadena(run, minSimb, maxSimb, EOB, ESC, cad, ncad, cadXTRA, ncadXTRA);
			  MeteSimboloEnCadena((int)block[y][x], minSimb, maxSimb, EOB, ESC, cad, ncad, cadXTRA, ncadXTRA);
			  run=0;
		  }
		}
		x--;
		y++;
	  } while (x>=0);
	  x++;
	  do 
	  {
		if (x<8 && y<8) 
		{ 
		  if (block[y][x]==0.0) run++;
		  else 
		  {
			  MeteSimboloEnCadena(run, minSimb, maxSimb, EOB, ESC, cad, ncad, cadXTRA, ncadXTRA);
			  MeteSimboloEnCadena((int)block[y][x], minSimb, maxSimb, EOB, ESC, cad, ncad, cadXTRA, ncadXTRA);
			  run=0;
		  }
		}
		if (x==7 && y==7) break;
		y--;
		x++;
	  } while (y>=0);
	  if (x==7 && y==7) break;
	  y++;
	}
	cad[*ncad]=EOB;
	(*ncad)++;
}

int SacaSimboloXTRA(int *cadXTRA, unsigned int *ncadXTRA)
{
	int simbolo=cadXTRA[*ncadXTRA]; 
	(*ncadXTRA)++;
	return simbolo;

}

int SacaSimboloDeCadena(int minSimb, int maxSimb, int EOB, int ESC, char *cad, unsigned int *ncad)
{
	int simbolo=cad[*ncad];
	(*ncad)++;
	return simbolo;
}

void SacaBloqueDeCadena(int minSimb, int maxSimb, int EOB, int ESC, 
					char *cad, unsigned int *ncad, int *cadXTRA, unsigned int *ncadXTRA, 
					double anterior,double **block)
{
	int x,y,run,sim;
	double valor;

	for (x=0;x<8;x++) for (y=0;y<8;y++) block[y][x]=0.0;

	sim=SacaSimboloDeCadena(minSimb, maxSimb, EOB, ESC, cad, ncad);
	if (sim==ESC) sim=SacaSimboloXTRA(cadXTRA, ncadXTRA); else sim+=minSimb;
	block[0][0]=(double)(anterior+sim);

    sim=SacaSimboloDeCadena(minSimb, maxSimb, EOB, ESC, cad, ncad);
	if (sim==EOB) return;
	if (sim==ESC) sim=SacaSimboloXTRA(cadXTRA, ncadXTRA);else sim+=minSimb;
	run=sim;

    sim=SacaSimboloDeCadena(minSimb, maxSimb, EOB, ESC, cad, ncad);
	if (sim==ESC) sim=SacaSimboloXTRA(cadXTRA, ncadXTRA);else sim+=minSimb;
	valor=(double)sim;

	x=1;y=0;
	while (1)
	{
	  do 
	  {
		if (x<8 && y<8) 
		{ 
		  if (run>0) run--;
		  else
		  {
			 block[y][x]=valor;
             sim=SacaSimboloDeCadena(minSimb, maxSimb, EOB, ESC, cad, ncad);
	         if (sim==EOB) return;
	         if (sim==ESC) sim=SacaSimboloXTRA(cadXTRA, ncadXTRA);else sim+=minSimb;
	         run=sim;

             sim=SacaSimboloDeCadena(minSimb, maxSimb, EOB, ESC, cad, ncad);
	         if (sim==ESC) sim=SacaSimboloXTRA(cadXTRA, ncadXTRA);else sim+=minSimb;
	         valor=(double)sim;
		  }
		}
		x--;
		y++;
	  } while (x>=0);
	  x++;
	  do 
	  {
		if (x<8 && y<8) 
		{ 
		  if (run>0) run--;
		  else
		  {
			 block[y][x]=valor;
             sim=SacaSimboloDeCadena(minSimb, maxSimb, EOB, ESC, cad, ncad);
	         if (sim==EOB) return;
	         if (sim==ESC) sim=SacaSimboloXTRA(cadXTRA, ncadXTRA);else sim+=minSimb;
	         run=sim;

             sim=SacaSimboloDeCadena(minSimb, maxSimb, EOB, ESC, cad, ncad);
	         if (sim==ESC) sim=SacaSimboloXTRA(cadXTRA, ncadXTRA);else sim+=minSimb;
	         valor=(double)sim;
		  }
		}
		if (x==7 && y==7) break;
		y--;
		x++;
	  } while (y>=0);
	  if (x==7 && y==7) break;
	  y++;
	}
}
