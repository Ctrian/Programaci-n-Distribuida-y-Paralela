/*
 * ESTADO ACTUAL: este archivo es una copia SERIAL, no una versión SIMD.
 * Incluye fractal-serial.h y define c, acotado y julia_serial otra vez.
 * Al enlazarlo junto con fractal-serial.cpp se duplican esas definiciones;
 * además falta la definición de julia_simd que main.cpp intenta llamar.
 * Los comentarios explican lo que hay y cómo se relaciona con el SIMD futuro.
 *
 * Una implementación SIMD podría trabajar con ocho píxeles de una misma fila:
 *   X = [x0, x1, ..., x7], Y = [y0, y1, ..., y7]
 * y aplicar en paralelo, componente a componente:
 *   X_nuevo = X*X - Y*Y - 0.7
 *   Y_nuevo = 2*X*Y + 0.27015
 * usando X e Y anteriores en ambas expresiones. En AVX de 256 bits, ocho
 * posiciones requieren float; con double caben cuatro posiciones por vector.
 *
 * Cada posición (carril) corresponde a UN píxel, con su propia órbita.
 * Se puede comparar X*X + Y*Y < 4, equivalente a |z| < 2, sin calcular sqrt.
 * Como unos píxeles escapan antes, hacen falta máscaras de carriles activos
 * y contadores por píxel: un píxel escapado no debe seguir sumando pasos.
 * El grupo termina cuando no quedan activos o se alcanza el límite.
 * Después se guardan sus colores y se avanza al siguiente grupo de píxeles.
 *
 * Cambiar solamente i++ por i += 8 dejaría siete columnas sin calcular:
 * también hay que hacer el cálculo y guardar el resultado de los ocho píxeles.
 * Si el ancho no es múltiplo de ocho, se debe atender al grupo final incompleto.
 */

#include "fractal-serial.h"
#include "palette.h"

#include <complex> // std::complex<double> representa un número x + y*i.

// Parámetro fijo para TODOS los píxeles: c = -0.7 + 0.27015*i.
// En Julia cambia z0 con cada píxel; cambiar c genera otro conjunto de Julia.
std::complex<double> c(-0.7, 0.27015);
// La variable se define en main.cpp; extern permite consultar el mismo límite.
extern int max_iteraciones;

// Calcula el color de UN punto inicial. El nombre no significa que demuestre
// que el punto esté acotado para siempre: solo observa una cantidad finita de pasos.
uint32_t acotado(std::complex<double> z0)
{
    /*
     * Órbita de un punto: z0 -> z1 -> z2 -> ..., con z(n+1) = z(n)^2 + c.
     * Si z = x + y*i y c = a + b*i, usando i^2 = -1:
     *   (x + y*i)^2 = (x*x - y*y) + (2*x*y)*i
     *   x_nuevo = x*x - y*y - 0.7
     *   y_nuevo = 2*x*y + 0.27015
     * Las dos expresiones usan x e y ANTERIORES. std::complex hace esa cuenta.
     * Ejemplo: z0 = 0 -> z1 = c -> z2 = -0.2829810225 - 0.10806*i.
     */

    // Esto inicializa un contador; NO limita el fractal a una sola iteración.
    // Como empieza en 1, se hacen como máximo max_iteraciones - 1 actualizaciones
    // si el límite es positivo. Con límite 10 hay como máximo 9 actualizaciones.
    int iter = 1;

    std::complex<double> z = z0;

    // abs(z) = sqrt(x*x + y*y): distancia al origen, no solo la parte real.
    // Repetimos mientras quede presupuesto Y el punto esté dentro del radio 2.
    // Para este c, alcanzar |z| >= 2 garantiza escape hacia el infinito.
    // Ese radio no debe generalizarse sin más a cualquier valor arbitrario de c.
    while (iter < max_iteraciones && std::abs(z) < 2.0)
    {
        z = z * z + c; // UNA iteración matemática de ESTE píxel.
        iter++;
    }
    if (iter < max_iteraciones)
    {
        // Salimos por |z| >= 2 antes de agotar el límite: colorear el escape.
        // El módulo entero % repite la paleta; con 16 colores, 18 % 16 = 2.
        int index = iter % PALETTE_SIZE;
        return color_ramp[index];
    }
    // Se alcanzó el límite: no se detectó escape ANTES de ese límite.
    // No demuestra pertenencia al conjunto. Además, si escapa justo en la última
    // actualización, este código también llega aquí: no distingue ese caso.
    // Se conserva el valor existente. No es blanco: en el entorno little-endian
    // del proyecto sus bytes RGBA son FF 00 00 00 (rojo con alfa 0).
    return 0X000000FF;
}

void julia_serial(
    double x_min, double y_min, double x_max, double y_max,
    uint32_t width, uint32_t height,
    uint32_t *pixel_buffer)
{
    // Distancia matemática entre muestras de columnas y filas consecutivas.
    // Los índices terminan en width-1 / height-1: no se muestrea x_max ni y_min.
    double dx = (x_max - x_min) / width;
    double dy = (y_max - y_min) / height;
    // Estos bucles recorren PÍXELES; el while de acotado recorre ITERACIONES.
    // i++ avanza una columna. No significa una sola iteración de z = z*z + c.
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            double x = x_min + i * dx;
            // Las filas crecen hacia abajo; el eje imaginario, hacia arriba.
            double y = y_max - j * dy;
            // Cada píxel empieza una órbita independiente en z0 = x + y*i.
            // La i matemática es la unidad imaginaria, no el índice de columna.
            std::complex<double> z0(x, y);

            uint32_t color = acotado(z0);

            // Guardar por filas: saltar j filas completas y avanzar i columnas.
            pixel_buffer[j * width + i] = color;
        }
    }
}
