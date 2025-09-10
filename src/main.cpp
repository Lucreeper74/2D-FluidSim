//
// Created by Luc_Creeper74 on 20/08/2025.
// Copyright (c) 2025 - All rights reserved.
//

#include <SFML/Graphics.hpp>
#include <cstdio>

#include "rendering/fps_counter.h"
#include "rendering/grid_renderer.h"
#include "simulator/fluidSim.h"

/**
 *  From https://matthias-research.github.io/pages/tenMinutePhysics/index.html works
 */

int main() {
    printf("Fluid-Sim - 1st Attempt\n");

    sim_params sim1_params = {
        .gravity = 9.81,         // in m/s²
        .deltaTime = 1.f / 20.f, // in s (but depend on the frame rate simulation)
        .numIter = 50,           // Number of iterations for Gauss-Seidel Method
        .overRelaxation = 1.9f,  // Factor used to accelerate convergence of Gauss-Seidel method
        .h = RES_Y / (float)GRID_Y,
        .flipRatio = 0.9f,
    };

    render_params render_params = {
        .showDensity = true,
        .showPressure = false,
        .p_radius = P_RADIUS,
    };

    FluidSim fluid_sim1(&sim1_params);

    RenderWindow window(VideoMode({RES_X, RES_Y}), "Fluid Sim - 1st Attempt");
    FPS fps;
    Clock clock;

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<Event::Closed>())
                window.close();
        }

        window.clear(Color::Black);

        int renderingTime = 0.f;
        renderingTime = clock.getElapsedTime().asMicroseconds();
        clock.restart();
        printf("RenderingTime: %i us\n\n\n", renderingTime);

        fluid_sim1.update(clock);

        bool key_pressed2 = sf::Keyboard::isKeyPressed(Keyboard::Key::C);
        fluid_sim1.compensateDrift = !key_pressed2;

        if (sf::Keyboard::isKeyPressed(Keyboard::Key::P))
            drawParticles(&window, &fluid_sim1, render_params);
        drawGrid(&window, &fluid_sim1, render_params);

        if (sf::Keyboard::isKeyPressed(Keyboard::Key::F))
            drawFPS(&window, &fps);

        window.display(); // End the current frame
    }
}
