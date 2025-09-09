//
// Created by Luc_Creeper74 on 20/08/2025.
// Copyright (c) 2025 - All rights reserved.
//


#include "simulator/fluidSim.h"
#include "rendering/fps_counter.h"
#include "rendering/grid_renderer.h"

#include <SFML/Graphics.hpp>
#include <cstdio>

#include "../cmake-build-debug/_deps/sfml-src/src/SFML/Window/InputImpl.hpp"

/**
 *  From https://matthias-research.github.io/pages/tenMinutePhysics/index.html works
 */

int main() {
    printf("Fluid-Sim - 1st Attempt\n");

    sim_params sim1_params = {
        .gravity = 9.81, // in m/s²
        .deltaTime = 1.f / 10.f, // in s (but depend on the frame rate simulation)
        .numIter = 50, // Number of iterations for Gauss-Seidel Method
        .overRelaxation = 1.9f, // Factor used to accelerate convergence of Gauss-Seidel method
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

        //fluid_sim1.update();
        int integrateTime = 0.f;
        int collisionTime = 0.f;
        int incompressibleTime = 0.f;
        int toGridTime = 0.f;
        int toParticleTime = 0.f;

        int renderingTime = 0.f;

        renderingTime = clock.getElapsedTime().asMicroseconds();
        clock.restart();

        fluid_sim1.integrateParticles();
        integrateTime = clock.getElapsedTime().asMicroseconds();
        clock.restart();

        fluid_sim1.handleParticleCollisions();
        collisionTime = clock.getElapsedTime().asMicroseconds();
        clock.restart();

        fluid_sim1.transfertVelocities(true);
        toGridTime = clock.getElapsedTime().asMicroseconds();
        clock.restart();

        fluid_sim1.computeDensity();
        fluid_sim1.solveIncompressibility();
        incompressibleTime = clock.getElapsedTime().asMicroseconds();
        clock.restart();

        fluid_sim1.transfertVelocities(false);
        toParticleTime = clock.getElapsedTime().asMicroseconds();
        clock.restart();

        printf("IntegrateTime: %i us\n", integrateTime);
        printf("CollisionTime: %i us\n", collisionTime);
        printf("IncompressibleTime: %i us\n", incompressibleTime);
        printf("ToGridTime: %i us\n", toGridTime);
        printf("ToParticleTime: %i us\n\n\n", toParticleTime);
        printf("RenderingTime: %i us\n\n\n", renderingTime);


        bool key_pressed2 = priv::InputImpl::isKeyPressed(Keyboard::Key::C);
        fluid_sim1.compensateDrift = !key_pressed2;

        if (priv::InputImpl::isKeyPressed(Keyboard::Key::P))
            drawParticles(&window, &fluid_sim1, render_params);
        drawGrid(&window, &fluid_sim1, render_params);

        if (priv::InputImpl::isKeyPressed(Keyboard::Key::F))
            drawFPS(&window, &fps);

        window.display(); // End the current frame
    }
}
