# MPEG1-Based Grayscale Video Compression (HERI SURE)

Este proyecto implementa un sistema de compresión de vídeo en C++, inspirado en el estándar MPEG1, limitado a secuencias en escala de grises. La compresión se lleva a cabo en tres fases: MJPEG (INTRA), codificación diferencial (sin movimiento), y estimación de movimiento. 

## 🧩 Componentes clave

- **MJPEG Compression:** Codificación INTRA usando compresión de imagen cuadro por cuadro.
- **Diferencial sin movimiento:** Codifica la diferencia entre cuadros consecutivos.
- **Estimación de movimiento:** Búsqueda de macrobloques para minimizar redundancia temporal.

## 🛠️ Requisitos

- Compilador C++ compatible con ANSI C++
- Sistema operativo con consola (Linux/Windows)
- Directorio con secuencias `.raw` de luminancia (ej: `salesman/`)




