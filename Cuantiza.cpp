#include "Cuantiza.h"
#include <math.h>

void CuantizaUniformeBloque(double QUniforme, double **block)
{
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            block[y][x] = (int)(block[y][x]) / (int)(2.0 * QUniforme);
        }
    }
}


void CuantizaUniformeInversaBloque(double QUniforme, double **block)
{
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            double valor = block[y][x];
            if (valor < 0)
                block[y][x] = (2 * valor + 1) * QUniforme;
            else if (valor > 0)
                block[y][x] = (2 * valor - 1) * QUniforme;
            else
                block[y][x] = 0.0;
        }
    }
}
