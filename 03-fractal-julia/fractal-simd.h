#ifndef FRACTAL_SIMD_H
#define FRACTAL_SIMD_H

#include <cstdint>

// Interfaz prevista para la versión SIMD; todavía NO está implementada en
// fractal-simd.cpp. Declarar esta función no crea su implementación.
// Recibe la misma región, resolución y buffer que julia_serial.
//
// SIMD = Single Instruction, Multiple Data: una instrucción realiza la misma
// operación sobre varios valores. En este problema se pueden agrupar píxeles
// independientes; cada uno conserva su z y su contador de iteraciones.
// "De 8 en 8" describe ocho píxeles por grupo, NO ocho pasos de una órbita.
// Por ejemplo, AVX de 256 bits permite 8 float de 32 bits o 4 double de 64 bits
// por vector. Para ocho complejos se necesitan sus partes reales e imaginarias
// por separado; un solo vector de ocho float no contiene ocho complejos.
void julia_simd(
    double x_min, double y_min, double x_max, double y_max,
    uint32_t width, uint32_t height,
    uint32_t*pixel_buffer
);

#endif // FRACTAL_SIMD_H
