//
// Created by Luc_Creeper74 on 27/08/2025.
// Copyright (c) 2025 - All rights reserved.
//

#include "particle.h"

#include <algorithm>
#include <cmath>

#include "fluidSim.h"

Particle::Particle() {
    x = 0.f;
    y = 0.f;
    v = 0.f;
    u = 0.f;
    radius = 1.f;
    linkedP = nullptr;
}

int Particle::getCellX(float cell_width, float dx, int grid_width) {
    return std::clamp((int)std::floor((x - dx) / cell_width), 0, grid_width - 1);
}

int Particle::getCellY(float cell_height, float dy, int grid_height) {
    return std::clamp((int)std::floor((y - dy) / cell_height), 0, grid_height - 1);
}

void Particle::setLinkedParticle(Particle* p) {
    if (p != this) {
        if (linkedP == nullptr)
            linkedP = p;
        else
            linkedP->setLinkedParticle(p);
    }
}