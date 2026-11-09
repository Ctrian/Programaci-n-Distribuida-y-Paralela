#include "fractal-serial.h"
#include "palette.h"

#include <complex> // libreria para trabajar con numeros complejos

std::complex<double> c(-0.7, 0.27015);
extern int max_iteraciones;


uint32_t acotado(std::complex<double> z0)
{
    /*
     * datos: c, z0
     * zn+1=zn^2 + c
     */

    int iter = 1;

    std::complex<double> z = z0;

    while (iter < max_iteraciones && std::abs(z) < 2.0)
    {
        z = z * z + c;
        iter++;
    }
    if (iter < max_iteraciones)
    {
        // norma > 2
        //return 0xFFFFFFFF; // ROJO
        int index = iter % PALETTE_SIZE;
        return color_ramp[index];
    }
    // NORMA <= 2
    return 0X000000FF; // BLANCO
}

void julia_serial(
    double x_min, double y_min, double x_max, double y_max,
    uint32_t width, uint32_t height,
    uint32_t *pixel_buffer)
{
    double dx = (x_max - x_min) / width;
    double dy = (y_max - y_min) / height;
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            double x = x_min + i * dx;
            double y = y_max - j * dy;
            // Funcion a dibujar
            std::complex<double> z0(x, y);

            uint32_t color = acotado(z0);

            pixel_buffer[j * width + i] = color;
        }
    }
}