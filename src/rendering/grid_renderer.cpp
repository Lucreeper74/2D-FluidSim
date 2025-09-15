//
// Created by Luc_Creeper74 on 20/08/2025.
// Copyright (c) 2025 - MIT.
//

#include "grid_renderer.h"

#include <cmath>

void drawGrid(RenderWindow* window, FluidSim* sim, render_params params) {
    float cell_h = sim->h;
    float maxD = 0.0;

    for (int n = 0; n < GRID_CELLS-GRID_Y; n++) {
        int i = n % GRID_Y;
        int j = n - (i * GRID_Y);

        if (i == 0 || i == GRID_X - 1 || j == 0 || j == GRID_Y - 1)
            continue;
        maxD = std::max(maxD, sim->density[n]);
    }

    VertexArray grid(PrimitiveType::Triangles, GRID_X * GRID_Y * 6); // 6 = 2 * 3 because 2 Triangles for 1 Quad

    for (int j = 0; j < GRID_Y; j++) {
        for (int i = 0; i < GRID_X; i++) {
            Color color = Color::Transparent; // Color of the cell

            if (sim->s[i * GRID_Y + j] == WALL_CELL)
                color = Color::Magenta;
            else if (sim->s[i * GRID_Y + j] == FLUID_CELL)
                color = Color(0, 255, 255, .75 * 255);

            // 1st Triangle (Top left)
            grid.append(Vertex{{i * cell_h, j * cell_h}, color}); // Corner
            grid.append(Vertex{{(i + 1) * cell_h, j * cell_h}, color});
            grid.append(Vertex{{i * cell_h, (j + 1) * cell_h}, color});

            // 2nd Triangle (Bottom Right)
            grid.append(Vertex{{(i+1) * cell_h, (j+1) * cell_h}, color}); // Corner
            grid.append(Vertex{{(i + 1) * cell_h, j * cell_h}, color});
            grid.append(Vertex{{i * cell_h, (j + 1) * cell_h}, color});
        }
    }
    window->draw(grid);
}

void drawFPS(RenderWindow* window, FPS* fps) {
    fps->updateFPS();
    Font font("../src/ressources/fonts/arial.ttf");
    Text text(font, std::to_string(fps->getFPS()) + " FPS", 24);
    text.setFillColor(Color::White);
    text.setStyle(Text::Bold);
    text.setPosition(Vector2f(10, 10));
    window->draw(text);
}

void drawParticles(RenderWindow* window, FluidSim* sim, render_params params) {
    for (int i = 0; i < PARTICLE_NUM; i++) {
        CircleShape circle(params.p_radius);
        circle.setFillColor(Color::White);
        circle.setOutlineColor(Color::Green);
        circle.setOutlineThickness(-P_RADIUS / 2.f);
        circle.setPosition(Vector2f(sim->particles[i].x - params.p_radius, sim->particles[i].y - params.p_radius));
        window->draw(circle);
    }
}

Color getColor(float value, float minV, float maxV) {
    float r = 0, g = 0, b = 0;
    value = std::fmin(std::fmax(value, minV), maxV - 0.001);

    float delta = maxV - minV;
    value = delta == 0.0 ? 0.5 : (value - minV) / delta;
    float m = .25f;
    int num = std::floor(value / m);
    float s = (value - num * m) / m;


    switch (num) {
        case 0:
            r = 0.f;
            g = s;
            b = 1.f;
            break;

        case 1:
            r = 0.f;
            g = 1.f;
            b = (1 - s);
            break;

        case 2:
            r = s;
            g = 1.f;
            b = 0.f;
            break;

        case 3:
            r = 1.f;
            g = (1 - s);
            b = 0.f;
            break;

        case 4:
            r = 1.f;
            g = 0.f;
            b = 0.f;

        default:
            break;
    }

    return Color(r * 0xFF, g * 0xFF, b * 0xFF);
}
