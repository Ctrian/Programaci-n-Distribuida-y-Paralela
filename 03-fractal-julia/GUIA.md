# Guía del fractal Julia: de Java a C++

Esta guía explica cómo se conectan `main.cpp`, `fractal-serial.h` y `fractal-serial.cpp` en este ejemplo. Las comparaciones con Java sirven como orientación; no son equivalencias exactas.

## 1. La idea principal

| Archivo | Responsabilidad |
| --- | --- |
| `fractal-serial.h` | Declara la función: indica su nombre, parámetros y retorno. |
| `fractal-serial.cpp` | Define o implementa la función: contiene el cálculo del fractal. |
| `main.cpp` | Usa la función y SFML para mostrar los píxeles en una ventana. |
| `CMakeLists.txt` | Configura cómo construir el ejecutable y qué bibliotecas enlazar. |

Puedes pensar en tres responsabilidades: **contrato, cálculo y uso**.

En Java podrías tener una clase `FractalSerial` con un método `static` que llamas desde otra clase. En C++ también existen clases, pero este ejemplo utiliza una **función libre**: no pertenece a ninguna clase y no necesitas crear un objeto para llamarla.

## 2. Declarar no es lo mismo que definir

En `fractal-serial.h` aparece:

```cpp
void julia_serial(
    double x_min, double y_min, double x_max, double y_max,
    uint32_t width, uint32_t height,
    uint32_t* pixel_buffer
);
```

Esto es una **declaración**. Termina en `;` y no tiene cuerpo. Le comunica al compilador cómo se puede llamar a la función:

- `void`: no devuelve un valor.
- Los cuatro `double`: límites de la región del plano que se calculará.
- `width` y `height`: dimensiones de la imagen en píxeles.
- `pixel_buffer`: dirección de la memoria donde se escribirán los colores.

En `fractal-serial.cpp` aparece la **definición**, con la misma firma y un cuerpo:

```cpp
void julia_serial(
    double x_min, double y_min, double x_max, double y_max,
    uint32_t width, uint32_t height,
    uint32_t* pixel_buffer)
{
    // Instrucciones que calculan y guardan los píxeles.
}
```

La declaración anuncia qué existe; la definición explica qué hace.

Un `.h` suele contener declaraciones, aunque también puede contener otras construcciones y ciertas definiciones. La extensión no impone por sí sola esa separación. Tampoco es una `interface` de Java: aquí simplemente publica una función.

## 3. ¿Qué hace `#include`?

Tanto `main.cpp` como `fractal-serial.cpp` contienen:

```cpp
#include "fractal-serial.h"
```

Antes de compilar, el preprocesador incorpora el contenido del encabezado en cada archivo que lo incluye.

- En `main.cpp`, permite conocer la función antes de llamarla.
- En `fractal-serial.cpp`, permite que el compilador compruebe que la implementación coincide con la declaración.

**No funciona igual que `import` en Java.** Incluir el `.h` no compila ni incorpora automáticamente el `.cpp` correspondiente. Ese `.cpp` también debe formar parte de la construcción del programa.

En este proyecto se usan comillas para el encabezado propio y ángulos para encabezados de bibliotecas:

```cpp
#include "fractal-serial.h"
#include <cstdint>
#include <SFML/Graphics.hpp>
```

## 4. El recorrido completo del programa

```text
main() reserva memoria para los píxeles
    → llama a julia_serial(..., buffer)
    → julia_serial calcula el color de cada píxel
    → escribe los colores en esa misma memoria
    → main() copia los píxeles a una textura de SFML
    → SFML muestra el sprite que utiliza esa textura
```

### Paso 1: reservar el arreglo

```cpp
uint32_t* buffer = nullptr;
// Dentro de main():
buffer = new uint32_t[WIDTH * HEIGHT];
```

`uint32_t` es un entero sin signo de 32 bits. Aquí cada elemento guarda los cuatro bytes de un píxel.

`uint32_t*` significa «puntero a un `uint32_t`». En este caso apunta al primer elemento del arreglo. `nullptr` indica inicialmente que no apunta a un objeto válido.

Con las dimensiones del ejemplo se reservan `1600 * 900 = 1 440 000` elementos.

En Java escribirías algo como `int[] buffer = new int[WIDTH * HEIGHT]`. La comparación ayuda a entender el arreglo, pero en C++ un puntero no lleva consigo su longitud: por eso la función también recibe las dimensiones.

### Paso 2: entregar la memoria al cálculo

```cpp
julia_serial(x_min, y_min, x_max, y_max, WIDTH, HEIGHT, buffer);
```

El valor del puntero se copia al parámetro `pixel_buffer`. Ambos apuntan a la **misma memoria**; no se copia todo el arreglo.

Por eso esta escritura dentro de la función:

```cpp
pixel_buffer[j * width + i] = color;
```

modifica el arreglo que creó `main()`.

Esto recuerda a pasar un arreglo en Java: se puede cambiar su contenido desde el método. En ambos casos se pasa un valor que permite acceder al mismo almacenamiento; no conviene describirlo simplemente como «paso por referencia».

### Paso 3: calcular los colores

`julia_serial` recorre columnas y filas mediante dos bucles. Para cada píxel:

1. Calcula sus coordenadas en el plano: `x` e `y`.
2. Construye un número complejo: `std::complex<double> z0(x, y)`.
3. Llama a `acotado(z0)` para obtener un color.
4. Guarda ese color en el arreglo.

La imagen es bidimensional, pero el arreglo es lineal. La fórmula:

```cpp
j * width + i
```

significa «saltar `j` filas completas y avanzar `i` columnas». Por ejemplo, con ancho 4, la posición de columna 2 y fila 1 tiene índice `1 * 4 + 2 = 6`. Los índices comienzan en cero.

`acotado` aplica repetidamente:

```cpp
z = z * z + c;
```

Se detiene cuando alcanza el límite de iteraciones o cuando `std::abs(z)` deja de ser menor que 2. Según cómo termina, devuelve uno de los colores. El límite permite aproximar el comportamiento del punto; no demuestra que permanezca acotado para siempre.

Se llama **serial** porque el código recorre los píxeles secuencialmente, sin distribuir el cálculo entre varios hilos.

### Paso 4: mostrar el resultado con SFML

```cpp
sf::Texture texture({WIDTH, HEIGHT});
sf::Sprite sprite(texture);
texture.update((const uint8_t*)buffer);
```

- El **buffer** contiene los datos de los píxeles en memoria.
- La **textura** recibe esos datos para usarlos en el dibujo.
- El **sprite** es el objeto dibujable que usa la textura.

El cast `(const uint8_t*)buffer` permite leer esa memoria como una secuencia de bytes, que es lo que espera `texture.update`. No crea otro arreglo ni convierte numéricamente cada elemento. `const` indica que esos bytes no se modificarán a través de ese puntero.

Después, el programa dibuja:

```cpp
window.clear();
window.draw(sprite);
window.draw(text);
window.display();
```

Dentro de `while (window.isOpen())`, también procesa eventos, recalcula el fractal, actualiza la textura y mide los FPS. En el código actual se hace además un cálculo inicial antes de entrar al bucle.

## 5. La otra conexión: `extern`

En `main.cpp` se define una variable global:

```cpp
int max_iteraciones = 100;
```

En `fractal-serial.cpp` se declara:

```cpp
extern int max_iteraciones;
```

Aquí `extern` comunica: «esta variable existe y está definida en otra parte». No crea otra variable con valor 100. El enlazador conecta esa declaración con la definición de `main.cpp`.

Esto permite que `acotado` consulte el límite. Aunque funciona, crea una dependencia del cálculo respecto a una variable global. Una alternativa para un diseño futuro sería pasar el límite como parámetro. No hace falta cambiarlo para entender este ejemplo.

## 6. ¿Para qué sirven `#ifndef`, `#define` y `#endif`?

El encabezado está protegido así:

```cpp
#ifndef FRACTAL_SERIAL_H
#define FRACTAL_SERIAL_H

// Contenido del encabezado.

#endif
```

Es una **guarda de inclusión**: evita procesar varias veces el contenido del encabezado durante el preprocesamiento de un mismo archivo fuente, por ejemplo si llega por varios `#include`.

No controla cuántas veces se llama a `julia_serial` ni hace que una función se ejecute una sola vez. `#pragma once` es otra forma habitual de proteger un encabezado, pero está comentada en este ejemplo.

## 7. ¿Cómo se conectan los `.cpp` al construir el programa?

Tu `CMakeLists.txt` contiene:

```cmake
add_executable(fractal-julia
    main.cpp
    fractal-serial.cpp
)
```

Eso indica que ambos archivos forman parte del ejecutable. De forma simplificada:

```text
main.cpp + encabezados            → código objeto de main
fractal-serial.cpp + encabezados  → código objeto del cálculo
ambos objetos + bibliotecas      → ejecutable fractal-julia
```

La **compilación** comprueba y traduce cada unidad por separado. El **enlazado** une las piezas y resuelve referencias como `julia_serial` y `max_iteraciones`.

Si incluyes el `.h` pero omites `fractal-serial.cpp` de la construcción, el compilador puede reconocer la llamada, pero el enlazador no encontrará su implementación: normalmente aparecerá un error como `undefined reference`.

El archivo también enlaza las bibliotecas de SFML y `fmt`. Para este proyecto se usa el compilador de C++ (`g++`, normalmente mediante CMake), además de las dependencias de esas bibliotecas.

## 8. Diferencias útiles si vienes de Java

| En este código C++ | Cómo interpretarlo desde Java |
| --- | --- |
| `int main()` | Punto de entrada; aquí no pertenece a una clase. `return 0` indica finalización correcta. |
| Función libre `julia_serial(...)` | Se puede llamar sin crear un objeto; recuerda a un método estático, pero no pertenece a una clase. |
| `std::` y `sf::` | Acceso a nombres dentro de un namespace; no implica que sean clases. |
| `sf::Texture texture(...)` | Construye un objeto directamente, sin necesitar `new`. |
| `new uint32_t[...]` | Reserva un arreglo dinámico cuya liberación debe gestionarse. |
| `auto msg = fmt::format(...)` | El compilador deduce el tipo; recuerda a `var`, no es tipado dinámico. |
| `#define WIDTH 1600` | Macro sustituida por el preprocesador. Para constantes con tipo también existe `constexpr`. |
| `.h` y `.cpp` | Separación habitual entre declaraciones e implementación; Java normalmente no exige este reparto. |

### Memoria: un detalle del ejemplo actual

C++ no tiene el recolector de basura automático de Java para estas reservas. El arreglo creado con `new[]` debería liberarse con:

```cpp
delete[] buffer;
```

cuando termine su uso, después del bucle y antes de salir de `main`. El código actual no lo hace. En C++ moderno suele preferirse un contenedor como `std::vector<uint32_t>`, que libera su almacenamiento automáticamente. Esta guía no modifica el código del ejemplo.

Los objetos locales como `texture`, `sprite` y `window` sí tienen destrucción automática al salir de su ámbito. No todo objeto C++ requiere `new` o `delete`.

### Colores: cuidado con los comentarios

En `acotado`, los comentarios llaman «ROJO» a `0xFFFFFFFF` y «BLANCO» a `0x000000FF`. No son una descripción fiable: SFML recibe bytes RGBA, y el orden en memoria de un entero depende de la plataforma. `0xFFFFFFFF` representa cuatro bytes a 255, es decir, blanco opaco. Para interpretar el otro valor hay que considerar el orden de bytes. No memorices los comentarios como regla de colores.

## 9. La matemática del fractal, paso a paso

La fórmula central es:

```text
zₙ₊₁ = zₙ² + c
```

Significa: «para obtener el siguiente valor de z, eleva al cuadrado el valor actual y súmale c». Se repite muchas veces. Tanto `z` como `c` son números complejos.

### 9.1. Un número complejo es un punto del plano

Un complejo se escribe `z = x + yi`:

- `x` es su parte real: posición horizontal.
- `y` es su parte imaginaria: posición vertical.
- La unidad imaginaria `i` cumple `i² = -1`.

Por ejemplo, `1 + 0.5i` corresponde al punto `(1, 0.5)`. Los números complejos permiten hacer operaciones algebraicas con estos puntos; no son solamente pares de coordenadas.

En C++, la biblioteca correspondiente se incorpora con:

```cpp
#include <complex>
```

Y un valor se construye así:

```cpp
std::complex<double> z0(x, y);
```

El primer argumento es la parte real y el segundo la parte imaginaria. `double` indica el tipo de ambas componentes.

**No confundas los nombres:** el `i` del bucle `for (int i = 0; ...)` es un índice entero de columna. No representa la unidad imaginaria matemática. Esta última queda representada por el tipo `std::complex`.

### 9.2. Los dos protagonistas: c y z₀

Tu código define:

```cpp
std::complex<double> c(-0.7, 0.27015);
```

Es decir:

```text
c = -0.7 + 0.27015i
```

Este valor se mantiene igual para todos los píxeles. Elegir otro `c` produce otro conjunto de Julia.

En cambio, **cada píxel tiene su propio z₀**, calculado a partir de su posición. Para dibujar una imagen, se estudian muchas sucesiones: una por píxel, todas con el mismo `c`.

### 9.3. Cómo pasa un píxel al plano complejo

El programa muestra una región delimitada por `x_min`, `x_max`, `y_min` e `y_max`. Calcula los pasos entre píxeles:

```cpp
double dx = (x_max - x_min) / width;
double dy = (y_max - y_min) / height;
```

Luego convierte la columna `i` y la fila `j`:

```cpp
double x = x_min + i * dx;
double y = y_max - j * dy;
std::complex<double> z0(x, y);
```

En pantalla, las filas crecen hacia abajo. En el plano matemático, el eje imaginario crece hacia arriba. Por eso se resta `j * dy` a `y_max`.

Con los valores actuales:

```text
x_min = y_min = -1.5
x_max = y_max =  1.5
width = 1600; height = 900

dx = 3 / 1600 = 0.001875
dy = 3 / 900  ≈ 0.003333333
```

Ejemplos:

| Columna i | Fila j | Coordenadas (x, y) | Complejo z₀ |
| --- | --- | --- | --- |
| 0 | 0 | (-1.5, 1.5) | -1.5 + 1.5i |
| 800 | 450 | (0, 0) | 0 |
| 1200 | 450 | (0.75, 0) | 0.75 |

Los índices llegan hasta `width - 1` y `height - 1`. Con estas fórmulas, la última columna no alcanza exactamente `x_max` y la última fila no alcanza exactamente `y_min`.

Además, el rango matemático tiene el mismo ancho y alto (3 unidades), pero la ventana mide 1600 × 900. Esto estira visualmente el plano en horizontal. Para conservar las proporciones, la relación entre los rangos matemáticos debe coincidir con la relación entre ancho y alto de la imagen.

### 9.4. Qué significa elevar un complejo al cuadrado

Usando `i² = -1`:

```text
(x + yi)²
= x² + 2xyi + y²i²
= (x² - y²) + (2xy)i
```

Si `c = a + bi`, cada iteración equivale a:

```text
x_nuevo = x² - y² + a
y_nuevo = 2xy + b
```

Para el `c` de este proyecto:

```text
x_nuevo = x² - y² - 0.7
y_nuevo = 2xy + 0.27015
```

Ambas fórmulas utilizan los valores **anteriores** de `x` e `y`. Si implementaras esto con variables separadas, necesitarías temporales para no usar por accidente el nuevo `x` al calcular `y`.

Tu código ya resuelve toda esa operación en una línea:

```cpp
z = z * z + c;
```

`std::complex` implementa los operadores de suma y multiplicación complejas. Aquí `*` no significa multiplicar cada componente por separado.

### 9.5. Un ejemplo numérico desde z₀ = 0

Para el píxel central del ejemplo:

```text
z₀ = 0
z₁ = 0² + c = -0.7 + 0.27015i
```

Para el siguiente paso:

```text
parte real de z₂ = (-0.7)² - (0.27015)² - 0.7
                 = -0.2829810225

parte imaginaria de z₂ = 2(-0.7)(0.27015) + 0.27015
                       = -0.10806

z₂ = -0.2829810225 - 0.10806i
```

Después se calcula `z₃ = z₂² + c`, y así sucesivamente. El valor nuevo siempre alimenta la siguiente iteración. Esta sucesión también se llama la **órbita** del punto inicial.

### 9.6. El módulo: distancia al origen

Para `z = x + yi`, el módulo es:

```text
|z| = √(x² + y²)
```

Es la distancia desde el punto `(x, y)` hasta `(0, 0)`. Por ejemplo, `|3 + 4i| = 5`.

En el código:

```cpp
std::abs(z)
```

devuelve ese módulo, no solamente el valor absoluto de la parte real.

### 9.7. Qué comprueba acotado

La función empieza en `z₀` y repite el cálculo:

```cpp
int iter = 1;
std::complex<double> z = z0;

while (iter < max_iteraciones && std::abs(z) < 2.0)
{
    z = z * z + c;
    iter++;
}
```

El bucle continúa mientras se cumplan **ambas** condiciones:

1. Queda margen en el contador de iteraciones.
2. El punto está dentro del círculo de radio 2.

Para el `c` de este ejemplo, cruzar el radio 2 permite detectar escape: los módulos acabarán creciendo sin límite. No hay que asumir ese radio como regla válida para cualquier `c` arbitrario sin comprobar las condiciones matemáticas.

La función asigna el color así:

```cpp
if (iter < max_iteraciones)
    return 0xFFFFFFFF;

return 0x000000FF;
```

Interprétalo como «escape detectado antes del límite» frente a «límite alcanzado». Hay dos detalles para leer el código con precisión:

- Como `iter` empieza en 1, con `max_iteraciones = 100` se ejecuta como máximo 99 veces la actualización `z = z * z + c`.
- Si se cruza el radio justo en la última actualización permitida, `iter` ya alcanza el límite y se devuelve el segundo color. La implementación no distingue ese caso del resto de puntos que llegan al límite.

**No escapar durante un número finito de pasos no demuestra que nunca se vaya a escapar.** La imagen es una aproximación. Aumentar el límite puede mejorar la clasificación cerca de la frontera, a costa de más trabajo de cálculo.

### 9.8. Por qué aparece un fractal

Algunos puntos iniciales producen sucesiones que permanecen acotadas; otros escapan hacia el infinito. Cerca de la frontera, puntos muy próximos pueden tener comportamientos distintos. Al colorear muchos puntos según ese comportamiento aparece la estructura del fractal.

Con precisión matemática:

- El **conjunto de Julia lleno** contiene los puntos iniciales cuyas sucesiones permanecen acotadas.
- El **conjunto de Julia** es la frontera de ese conjunto lleno.

Este programa aproxima la separación mediante dos colores y un límite de iteraciones. No calcula una fórmula explícita para la frontera.

Una distinción útil: en este ejemplo de **Julia**, `c` es fijo y varía `z₀` entre píxeles. En el conjunto de **Mandelbrot**, se suele fijar `z₀ = 0` y variar `c` entre píxeles. La fórmula iterada puede ser la misma; cambia qué representa cada píxel.

### 9.9. Qué cambia al modificar cada parámetro

| Parámetro | Efecto |
| --- | --- |
| `c` | Cambia el conjunto de Julia que se estudia. |
| Límites `x_min`, `x_max`, `y_min`, `y_max` | Cambian la región observada: desplazamiento o zoom. |
| `width`, `height` | Cambian cuántos puntos se muestrean y el tamaño de la imagen. |
| `max_iteraciones` | Cambia cuánto tiempo se observa cada sucesión y la precisión de la aproximación. |
| Valores de color | Cambian la representación visual; no cambian la sucesión matemática. |

El recorrido matemático completo es:

```text
píxel (columna, fila)
    → coordenadas (x, y)
    → complejo inicial z₀ = x + yi
    → repetir z = z² + c
    → comprobar módulo y límite de iteraciones
    → asignar un color
    → guardar el color en el buffer
```

### 9.10. Preguntas para repasar

1. ¿Dónde se crea el complejo asociado a cada píxel? En `std::complex<double> z0(x, y)` dentro de `julia_serial`.
2. ¿Qué permanece fijo para toda la imagen? El valor de `c`.
3. ¿Qué cambia entre píxeles? El punto inicial `z₀`.
4. ¿Qué hace `std::abs(z)`? Calcula la distancia al origen del punto complejo actual.
5. ¿Por qué `julia_serial` devuelve `void`? Porque escribe los colores en el buffer recibido.
6. ¿Más iteraciones significan más píxeles? No: significan más pasos posibles por píxel. Las dimensiones determinan la cantidad de píxeles.
7. ¿Llegar al límite demuestra que un punto pertenece al conjunto lleno? No: solo indica que el criterio de escape no lo descartó antes del límite.

## 10. Qué recordar al volver al código

1. El `.h` anuncia cómo llamar a `julia_serial`.
2. `fractal-serial.cpp` contiene su implementación y la función auxiliar `acotado`.
3. `main.cpp` reserva el buffer, pide el cálculo y muestra el resultado con SFML.
4. La función modifica la memoria recibida mediante el puntero; por eso puede retornar `void` y aun así producir la imagen.
5. CMake incluye ambos `.cpp`; el enlazador conecta las llamadas con sus definiciones.
6. `extern` permite usar la variable global definida en otro archivo.
7. Las guardas del encabezado evitan inclusiones repetidas, no llamadas repetidas.
