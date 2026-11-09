#include <iostream>
#include <fmt/core.h>
#include <fmt/format.h>

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include "fractal-serial.h"


// Ejes en coordenadas x,y
double x_min = -1.5;
double x_max = 1.5;
double y_min = -1.5;
double y_max = 1.5;

int max_iteraciones = 10;

#define WIDTH 1600
#define HEIGHT 900

// imagen, textura
uint32_t *buffer = nullptr;

int main()
{

    buffer = new uint32_t[WIDTH * HEIGHT];

    // Create the main window
    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "Dibujo funcion");

    // configurar textos 
    // configuracion texto Fps e iteraciones
    sf::Font font("arial.TTF");
    sf::Text text(font, "Fractal", 24);
    text.setFillColor(sf::Color::Black);
    text.setPosition({10,10});
    text.setStyle(sf::Text::Bold);
    
    //configuracion texto inferior
    std::string options = "[1] Serial | Up/Down: Iterations";
    sf::Text text_options(font, options, 20);
    text_options.setFillColor(sf::Color::Blue);
    text_options.setPosition({10, window.getView().getSize().y - 40});
    text.setStyle(sf::Text::Bold);

    // dibujar(); configurar textura inicial
    julia_serial(x_min, y_min, x_max, y_max, WIDTH, HEIGHT, buffer);

    // Load a sprite to display
    sf::Texture texture({WIDTH, HEIGHT});
    sf::Sprite sprite(texture);

    texture.update((const uint8_t *)buffer);

    int frames = 0;
    int fps = 0;
    sf::Clock clock;

    // Start the game loop
    while (window.isOpen())
    {
        // Process events
        while (const std::optional event = window.pollEvent())
        {
            // sf::Event::KeyPressed
            // Close window: exit
            if (event->is<sf::Event::Closed>())
                window.close();
                else if(event -> is <sf::Event::KeyReleased>()){
                    auto evt = event -> getIf<sf::Event::KeyReleased>();

                    switch (evt ->scancode){
                        case sf::Keyboard::Scan::Up:
                        max_iteraciones += 10;
                        break;
                        case sf::Keyboard::Scan::Down:
                        max_iteraciones -= 10;
                        break;
                    }

                }
        }

        julia_serial(x_min, y_min, x_max, y_max, WIDTH, HEIGHT, buffer);
        texture.update((const uint8_t *)buffer);

        // std::cout<<"dibujando"<<std::endl;
        // std::cout.flush();

        // medir los fps
        frames++;

        if (clock.getElapsedTime().asSeconds() >= 1.0f)
        {
            fps = frames;
            frames = 0;
            clock.restart();
        }

        std::cout << "FPS: " << fps << std::endl;

        // actualizar los textos
        auto msg = fmt::format("Fractal: Iteraciones: {} , FPS: {}", max_iteraciones, fps);
        text.setString(msg);

        // Clear screen
        window.clear();

        // Draw the sprite
        window.draw(sprite);

        //Texto de Fps e iteracciones
        window.draw(text);

        //Dibujar texto de opciones
        window.draw(text_options);

        // Update the window
        window.display();
    }

    return 0;
}