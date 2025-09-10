//
// Created by Luc_Creeper74 on 27/08/2025.
// Copyright (c) 2025 - All rights reserved.
//

#ifndef PARTICLE_H
#define PARTICLE_H

class Particle {
    public:
        float x, y;
        float v, u;

        float radius;
        Particle* linkedP;

        Particle();

        int getCellX(float cell_width, float dx, int grid_width);
        int getCellY(float cell_height, float dy, int grid_height);

        void setLinkedParticle(Particle* p);
};

#endif // PARTICLE_H
