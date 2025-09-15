//
// Created by Luc_Creeper74 on 20/08/2025.
// Copyright (c) 2025 - MIT.
//

#ifndef GRID_RENDERER_H
#define GRID_RENDERER_H

#include "../simulator/fluidSim.h"
#include "../rendering/fps_counter.h"

#include <SFML/Graphics.hpp>

using namespace sf;

typedef struct {
    bool showDensity;
    bool showPressure;
    float p_radius;
} render_params;

void drawGrid(RenderWindow* window, FluidSim* sim, render_params params);
void drawFPS(RenderWindow* window, FPS* fps);
void drawParticles(RenderWindow* window, FluidSim* sim, render_params params);

Color getColor(float value, float minV, float maxV);

#endif //GRID_RENDERER_H