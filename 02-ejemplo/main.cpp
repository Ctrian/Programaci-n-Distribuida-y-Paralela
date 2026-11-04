#include <iostream>
#include <fmt/core.h>

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

// Ejes en coordenadas x,y
double x_min = -1;
double x_max = 1;
double y_min = -1;
double y_max = 1;

#define WIDTH 1600
#define HEIGHT 900

// imagen, textura
uint32_t *buffer = nullptr;

void dibujar()
{
    double dx = (x_max - x_min) / WIDTH;
    double dy = (y_max - y_min) / HEIGHT;
    for (int i = 0; i < WIDTH; i++)
    {
        for (int j = 0; j < HEIGHT; j++)
        {
            double x = x_min + i * dx;
            double y = y_max - j * dy;
            // Funcion a dibujar
            double z = x * x + y * y;

            int rojo = z * 255 / 2;

            buffer[j * WIDTH + i] = rojo << 24 | 0x000000FF;

            // buffer[j*WIDTH+i] = 0xFF0000FF;
        }
    }
}

int main()
{

    buffer = new uint32_t[WIDTH * HEIGHT];

    // Create the main window
    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "Dibujo funcion");

    dibujar();

    // Load a sprite to display
    sf::Texture texture({WIDTH, HEIGHT});
    sf::Sprite sprite(texture);

    texture.update((const uint8_t *)buffer);

    // Start the game loop
    while (window.isOpen())
    {
        // Process events
        while (const std::optional event = window.pollEvent())
        {
            // Close window: exit
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        // Clear screen
        window.clear();

        // Draw the sprite
        window.draw(sprite);

        // Update the window
        window.display();
    }

    return 0;
}