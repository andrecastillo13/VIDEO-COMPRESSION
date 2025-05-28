#include <math.h>
#include "DCT.h"

#define PI 3.14159265358979323846

// calculo normal
void DCTn_1D(double *s)
{

// COMPLETAR

double S[8];
    for (int u = 0; u < 8; u++) {
        double Cu = (u == 0) ? sqrt(1.0 / 8.0) : sqrt(2.0 / 8.0);
        S[u] = 0.0;
        for (int x = 0; x < 8; x++) {
            S[u] += s[x] * cos((PI / 8.0) * (x + 0.5) * u);
        }
        S[u] *= Cu;
    }
    for (int i = 0; i < 8; i++) {
        s[i] = S[i];
    }

}

void IDCTn_1D(double *s)
{

// COMPLETAR

double S[8];
    for (int x = 0; x < 8; x++) {
        S[x] = 0.0;
        for (int u = 0; u < 8; u++) {
            double Cu = (u == 0) ? sqrt(1.0 / 8.0) : sqrt(2.0 / 8.0);
            S[x] += Cu * s[u] * cos((PI / 8.0) * (x + 0.5) * u);
        }
    }
    for (int i = 0; i < 8; i++) {
        s[i] = S[i];
    }

}

// vetterli y ligtenberg
void DCTvl_1D(double *s)
{

// COMPLETAR

}

void IDCTvl_1D(double *s)
{
  double f0,f1,f2,f3,f4,f5,f6,f7;
  double g0,g1,g2,g3,g4,g5,g6,g7;
  double h0,h1,h2,h3,h4,h5,h6,h7;
  double i0,i1,i2,i3,i4,i5,i6,i7;

  double C4=cos(4*PI/16.0);
  double C6=cos(6*PI/16.0);
  double Cm1=cos(-PI/16.0);
  double Cm3=cos((-3*PI)/16.0);
  double S6=sin(6*PI/16.0);
  double Sm1=sin(-PI/16.0);
  double Sm3=sin((-3*PI)/16.0);

  f0=s[0];f1=s[4];f2=s[2];f3=s[6];
  f4=s[1];f5=s[7];f6=s[3];f7=s[5];

  g0=(f0*C4);
  g1=(f1*C4);
  g2=(C6*(f2+f3)-(S6+C6)*f3);
  g3=((S6-C6)*f2+C6*(f2+f3));
  g5=(Cm1*(f4+f5)-((Sm1+Cm1)*f5));
  g4=-((Sm1-Cm1)*f4+(Cm1*(f4+f5)));
  g7=(Cm3*(f7+f6)-(Sm3+Cm3)*f7);
  g6=(Sm3-Cm3)*f6+Cm3*(f7+f6);

  h0=g0+g1;
  h1=g0-g1;
  h2=g2;
  h3=g3;
  h4=g6+g4;
  h5=(g5-g7)*C4;
  h6=(g4-g6)*C4;
  h7=g7+g5;

  i0=h0+h3;
  i1=h5+h1;
  i2=h2+h6;
  i3=h0-h3;
  i4=h4;
  i5=h1-h5;
  i6=h6-h2;
  i7=h7;

  s[0]=(i7+i0)/2;
  s[1]=(i1+i2)/2;
  s[2]=(i1-i2)/2;
  s[3]=(i3+i4)/2;
  s[4]=(i3-i4)/2;
  s[5]=(i5+i6)/2;
  s[6]=(i5-i6)/2;
  s[7]=(i0-i7)/2;
}

void DCT_2Dn(double **block)
{
  int x,y;
  double aux[8];
  // primero por filas
  for (x=0; x<8; x++)
    DCTn_1D(block[x]);
  // después por columnas
  for (x=0; x<8; x++)
  {
    for (y=0; y<8; y++) aux[y]=block[y][x]; // leemos las columnas
    DCTn_1D(aux);
    for (y=0; y<8; y++) block[y][x]=aux[y]; // escribimos las columnas
  }
}

void IDCT_2Dn(double **block)
{
  int x,y;
  double aux[8];
  // primero por filas
  for (x=0; x<8; x++)
    IDCTn_1D(block[x]);
  // después por columnas
  for (x=0; x<8; x++)
  {
    for (y=0; y<8; y++) aux[y]=block[y][x]; // leemos las columnas
    IDCTn_1D(aux);
    for (y=0; y<8; y++) block[y][x]=aux[y]; // escribimos las columnas
  }
}

void DCT_2Dvl(double **block)
{
  int x,y;
  double aux[8];
  // primero por filas
  for (x=0; x<8; x++)
    DCTvl_1D(block[x]);
  // después por columnas
  for (x=0; x<8; x++)
  {
    for (y=0; y<8; y++) aux[y]=block[y][x]; // leemos las columnas
    DCTvl_1D(aux);
    for (y=0; y<8; y++) block[y][x]=aux[y]; // escribimos las columnas
  }
}

void IDCT_2Dvl(double **block)
{
  int x,y;
  double aux[8];
  // primero por filas
  for (x=0; x<8; x++)
    IDCTvl_1D(block[x]);
  // después por columnas
  for (x=0; x<8; x++)
  {
    for (y=0; y<8; y++) aux[y]=block[y][x]; // leemos las columnas
    IDCTvl_1D(aux);
    for (y=0; y<8; y++) block[y][x]=aux[y]; // escribimos las columnas
  }
}
