#ifndef FRACTAL_SERIAL_H
#define FRACTAL_SERIAL_H

// #pragma once PARA POR SI ACASO LA FUNCION VUELVE A LLAMARSE DESDE OTRO ARCHIVO

#include <cstdint>

void julia_serial(
    double x_min, double y_min, double x_max, double y_max,
    uint32_t width, uint32_t height,
    uint32_t*pixel_buffer
);

#endif //FRACTAL_SERIAL_H "puede ser cualquier nombre"