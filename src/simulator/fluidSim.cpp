//
// Created by Luc_Creeper74 on 20/08/2025.
// Copyright (c) 2025 - All rights reserved.
//

#include "FluidSim.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

// TODO: Change all Particle* p = &particle to Particle& p = particle

FluidSim::FluidSim(sim_params* sim_params) {
    this->gravity = sim_params->gravity;
    this->timeStep = sim_params->deltaTime;
    this->numIter = sim_params->numIter;
    this->overRelaxation = sim_params->overRelaxation;
    this->h = sim_params->h;
    this->flip_ratio = sim_params->flipRatio;
    compensateDrift = true;


    for (int j = 0; j < GRID_Y; j++) {
        for (int i = 0; i < GRID_X; i++) {
            CELL_TYPE s_val = AIR_CELL;

            // Add borders / walls
            if (i == 0 || i == (GRID_X - 1) || j == 0 || j == (GRID_Y - 1))
                s_val = WALL_CELL;

            s[i * GRID_Y + j] = s_val;
        }
    }

    int p = 0; // Particle index
    float dx = 2.f * P_RADIUS;
    float dy = sqrt(3.0) / 2.0 * dx;

    for (int j = 0; j < PARTICLE_Y; j++) {
        for (int i = 0; i < PARTICLE_X; i++) {
            particles[p].radius = P_RADIUS;

            particles[p].x = h + P_RADIUS + dx * (i + 3) + (j % 2 == 0 ? 0.0 : P_RADIUS);
            particles[p].y = h + P_RADIUS + dy * (j + 3);
            p++;
        }
    }
}

void FluidSim::update() {
    integrateParticles();
    handleParticleCollisions();
    transfertVelocities(true);
    computeDensity();
    solveIncompressibility();
    transfertVelocities(false);
}

void FluidSim::integrateParticles() {
    for (int i = 0; i < PARTICLE_NUM; i++) {
        float grav = gravity * timeStep;
        particles[i].v += grav;
        particles[i].x += particles[i].u * timeStep;
        particles[i].y += particles[i].v * timeStep;
    }
}

void FluidSim::transfertVelocities(bool particlesToGrid) {
    float h2 = h / 2.f;
    float h1 = 1.f / h;

    if (particlesToGrid) {
        rU.fill(0.0f);
        rV.fill(0.0f);
        u.fill(0.0f);
        v.fill(0.0f);

        for (int i = 0; i < GRID_CELLS; i++)
            if (s[i] != WALL_CELL)
                s[i] = AIR_CELL;

        for (int i = 0; i < PARTICLE_NUM; i++) {
            Particle p = particles[i];
            int xi = p.getCellX(h, 0.f, GRID_X);
            int yi = p.getCellY(h, 0.f, GRID_Y);
            int cellIndex = xi * GRID_Y + yi;
            if (s[cellIndex] == AIR_CELL)
                s[cellIndex] = FLUID_CELL;
        }
    }

    for (int component = 0; component < 2; component++) {
        // For each component U and V
        bool isU = component == 0;

        float dx = isU ? 0.f : h2;
        float dy = isU ? h2 : 0.f;

        std::array<float, GRID_CELLS>& q = isU ? u : v; // Current component that will be computed
        std::array<float, GRID_CELLS>& prevQ = isU ? prevU : prevV;
        std::array<float, GRID_CELLS>& rQ = isU ? rU : rV;

        for (int i = 0; i < PARTICLE_NUM; i++) {
            // For each particle
            Particle& particle = particles[i];

            int x0 = particle.getCellX(h, dx, GRID_X);
            int x1 = std::min(x0 + 1, GRID_X - 1);
            float tx = ((particle.x - dx) - x0 * h) * h1;
            float sx = 1 - tx;

            int y0 = particle.getCellY(h, dy, GRID_Y);
            int y1 = std::min(y0 + 1, GRID_Y - 1);
            float ty = ((particle.y - dy) - y0 * h) * h1;
            float sy = 1 - ty;

            // Weights for Weighted Average
            float w1 = sx * sy;
            float w2 = tx * sy;
            float w3 = tx * ty;
            float w4 = sx * ty;

            // all indexes
            int index1 = x0 * GRID_Y + y0;
            int index2 = x1 * GRID_Y + y0;
            int index3 = x1 * GRID_Y + y1;
            int index4 = x0 * GRID_Y + y1;

            /*
             * q4, r4 +-------------+ q3, r3
             *        |             |
             *        |      X qp   |
             *        |             |
             *        |             |
             * q1, r1 +-------------+ q2, r4
             */
            float& qp = isU ? particle.u : particle.v; // Current particle component computed
            if (particlesToGrid) {
                // From particles -> grid
                q[index1] += w1 * qp; // q1
                q[index2] += w2 * qp; // q2
                q[index3] += w3 * qp; // q3
                q[index4] += w4 * qp; // q4

                rQ[index1] += w1; // r1
                rQ[index2] += w2; // r2
                rQ[index3] += w3; // r3
                rQ[index4] += w4; // r4
            } else {
                w1 *= s[index1] != AIR_CELL ? 1.f : 0.f;
                w2 *= s[index2] != AIR_CELL ? 1.f : 0.f;
                w3 *= s[index3] != AIR_CELL ? 1.f : 0.f;
                w4 *= s[index4] != AIR_CELL ? 1.f : 0.f;
                float weight_sum = w1 + w2 + w3 + w4;

                if (weight_sum > 0.0f) {
                    // From grid -> particles
                    float PIC = ((w1 * q[index1])
                            + (w2 * q[index2])
                            + (w3 * q[index3])
                            + (w4 * q[index4]))
                        / weight_sum;

                    float changes = ((w1 * (q[index1] - prevQ[index1]))
                            + (w2 * (q[index2] - prevQ[index2]))
                            + (w3 * (q[index3] - prevQ[index3]))
                            + (w4 * (q[index4] - prevQ[index4])))
                        / weight_sum;

                    float FLIP = qp + changes;

                    qp = (1 - flip_ratio) * PIC + flip_ratio * FLIP;
                }
            }
        }

        if (particlesToGrid) {
            for (int n = 0; n < GRID_CELLS; n++) {
                if (rQ[n] > 0.0f)
                    q[n] /= rQ[n];
            }

            // Store initial grid values (just after the projection to grid)
            prevU = u;
            prevV = v;
        }
    }
}

void FluidSim::solveIncompressibility() {
    for (int n = 0; n < numIter; n++) {
        // Iteration for Gauss-seidel Method (see: https://en.wikipedia.org/wiki/Gauss%E2%80%93Seidel_method)
        for (int j = 0; j < GRID_Y; j++) {
            for (int i = 0; i < GRID_X; i++) {
                if (s[i * GRID_Y + j] != FLUID_CELL)
                    continue; // Skipping others cells

                int sx0 = getS((i - 1) * GRID_Y + j);
                int sx1 = getS((i + 1) * GRID_Y + j);
                int sy0 = getS(i * GRID_Y + (j - 1));
                int sy1 = getS(i * GRID_Y + (j + 1));
                int s_tot = sx0 + sx1 + sy0 + sy1;

                if (s_tot == 0)
                    continue;

                // Poisson System (see: https://fr.wikipedia.org/wiki/%C3%89quation_de_Poisson)

                // Add all surrounding flows to calculate the local divergence
                float div = u[(i + 1) * GRID_Y + j] - u[i * GRID_Y + j] + v[i * GRID_Y + (j + 1)] - v[i * GRID_Y + j];

                // Increases system convergence dramatically (see: https://en.wikipedia.org/wiki/Successive_over-relaxation)
                float p = (div / s_tot) * overRelaxation;

                if (compensateDrift && pRestDensity > 0.f) {
                    float k = 1.f;
                    p -= k * (density[i * GRID_Y + j] - pRestDensity);
                }

                u[i * GRID_Y + j] += sx0 * p;
                u[(i + 1) * GRID_Y + j] -= sx1 * p;
                v[i * GRID_Y + j] += sy0 * p;
                v[i * GRID_Y + (j + 1)] -= sy1 * p;
            }
        }
    }
}

void FluidSim::computeDensity() {
    // Clear density
    density = {};

    // Shifted on two axes because density store in middle for each cell
    int h2 = h / 2.f;

    for (int i = 0; i < PARTICLE_NUM; i++) {
        Particle p = particles[i];

        int x0 = p.getCellX(h, h2, GRID_X);
        int x1 = std::min(x0 + 1, GRID_X - 1);
        float tx = ((p.x - h2) - x0 * h) / h;
        float sx = 1 - tx;

        int y0 = p.getCellY(h, h2, GRID_Y);
        int y1 = std::min(y0 + 1, GRID_Y - 1);
        float ty = ((p.y - h2) - y0 * h) / h;
        float sy = 1 - ty;

        density[x0 * GRID_Y + y0] += tx * ty; // w1
        density[x1 * GRID_Y + y0] += sx * ty; // w2
        density[x1 * GRID_Y + y1] += sx * sy; // w3
        density[x0 * GRID_Y + y1] += tx * sy; // w4
    }

    if (pRestDensity == 0.f) {
        float density_sum = 0.f;
        int fluidCells = 0;

        for (int i = 0; i < GRID_CELLS; i++) {
            if (s[i] == FLUID_CELL) {
                density_sum += density[i];
                fluidCells++;
            }
        }

        if (fluidCells > 0)
            pRestDensity = density_sum / fluidCells;
    }
}


void FluidSim::handleWallCollisions(float minX, float minY, float maxX, float maxY) {
    float restitution = 0.9f;
    float damping = 0.995f;

    for (int i = 0; i < PARTICLE_NUM; i++) {
        Particle& p = particles[i];

        float x = p.x;
        float y = p.y;

        // Borders collisions
        if (x < minX) {
            x = minX;
            p.u = -p.u * restitution;
            p.v = p.v * damping;
        }

        if (x > maxX) {
            x = maxX;
            p.u = -p.u * restitution;
            p.v = p.v * damping;
        }

        if (y < minY) {
            y = minY;
            p.v = -p.v * restitution;
            p.u = p.u * damping;
        }

        if (y > maxY) {
            y = maxY;
            p.v = -p.v * restitution;
            p.u = p.u * damping;
        }

        p.x = x;
        p.y = y;
    }
}

void FluidSim::handleParticleCollisions() {
    int iteration = 5;
    float minDist = 2.f * P_RADIUS;
    float minDist2 = minDist * minDist;

    float minX = h + P_RADIUS;
    float minY = h + P_RADIUS;
    float maxX = (GRID_X - 1) * h - P_RADIUS;
    float maxY = (GRID_Y - 1) * h - P_RADIUS;

    for (int n = 0; n < iteration; n++) {
        std::array<Particle*, SPATIAL_CELLS> collision_grid{};

        // Fill up the collision grid
        for (int i = 0; i < PARTICLE_NUM; i++) {
            Particle* p = &particles[i];

            int cell_x = p->getCellX(SPACING, 0.f, SPATIAL_X);
            int cell_y = p->getCellY(SPACING, 0.f, SPATIAL_Y);

            auto& in_cell = collision_grid[cell_x * SPATIAL_Y + cell_y];

            if (in_cell == nullptr)
                in_cell = p;
            else
                in_cell->setLinkedParticle(p);
        }

        // For each particle -> get the neighbor ones
        for (int i = 0; i < PARTICLE_NUM; i++) {
            Particle* p = &particles[i];

            int cell_x = p->getCellX(SPACING, 0.f, SPATIAL_X);
            int cell_y = p->getCellY(SPACING, 0.f, SPATIAL_Y);

            const int neighbor_rad = 1; // 3x3 around
            int cell_minX = std::max(cell_x - neighbor_rad, 0);
            int cell_minY = std::max(cell_y - neighbor_rad, 0);

            int cell_maxX = std::min(cell_x + neighbor_rad, SPATIAL_X - 1);
            int cell_maxY = std::min(cell_y + neighbor_rad, SPATIAL_Y - 1);

            for (int yi = cell_minY; yi <= cell_maxY; yi++) {
                for (int xi = cell_minX; xi <= cell_maxX; xi++) {
                    Particle* qp = collision_grid[xi * SPATIAL_Y + yi];
                    // First particle in linked list of the current cell

                    while (qp != nullptr) {
                        if (qp != p) {
                            float dx = qp->x - p->x;
                            float dy = qp->y - p->y;
                            float d2 = dx * dx + dy * dy;

                            if (d2 < minDist2) {
                                // p and qp collide!
                                float dist = sqrt(d2);
                                float s = 0.5f * (minDist - dist) / dist;
                                dx *= s;
                                dy *= s;
                                p->x -= dx;
                                p->y -= dy;
                                qp->x += dx;
                                qp->y += dy;
                            }
                        }
                        qp = qp->linkedP; //Iterate through the linked list
                    }
                }
            }
        }
        handleWallCollisions(minX, minY, maxX, maxY);

        // Clear all linked list of particles
        for (int i = 0; i < PARTICLE_NUM; i++) {
            particles[i].linkedP = nullptr;
        }
    }
}

int FluidSim::getS(int index) {
    return s[index] == WALL_CELL ? 0 : 1;
}
