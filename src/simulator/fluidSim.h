//
// Created by Luc_Creeper74 on 20/08/2025.
// Copyright (c) 2025 - MIT.
//

#ifndef GRID_H
#define GRID_H

#include <SFML/Graphics.hpp>
#include <array>

#include "particle.h"

// For Eularian simulation
#define GRID_X 30
#define GRID_Y 25
#define GRID_CELLS GRID_X* GRID_Y

// For particle simulation
#define PARTICLE_X 15
#define PARTICLE_Y 25
#define PARTICLE_NUM PARTICLE_X* PARTICLE_Y

// For collision grid
#define P_RADIUS 0.3f * (RES_Y / (float)GRID_Y)
#define SPACING (2.2f * P_RADIUS)
#define SPATIAL_Y (int)(RES_Y / SPACING)
#define SPATIAL_X (int)(RES_X / SPACING)
#define SPATIAL_CELLS (int)(SPATIAL_X * SPATIAL_Y)

// For render simulation
#define RES_X GRID_X * 20
#define RES_Y GRID_Y * 20

typedef enum {
    U_FIELD,
    V_FIELD,
    S_FIELD
} FIELD_TYPE;

typedef enum {
    WALL_CELL,
    FLUID_CELL,
    AIR_CELL
} CELL_TYPE;

typedef struct {
    float gravity;
    float deltaTime;
    int numIter;
    float overRelaxation;
    float h;
    float flipRatio;
} sim_params;

class FluidSim {
  public:
    float gravity;
    float timeStep;
    int numIter;
    float overRelaxation;
    float h;
    float flip_ratio;
    bool compensateDrift;
    float pRestDensity;
    std::array<Particle, PARTICLE_NUM> particles{};
    std::array<float, GRID_CELLS> density{};
    std::array<float, GRID_CELLS> u{}, v{};
    std::array<float, GRID_CELLS> prevU{}, prevV{};
    std::array<float, GRID_CELLS> rU{}, rV{};
    std::array<CELL_TYPE, GRID_CELLS> s{};

    FluidSim(sim_params* sim_params);

    void update(sf::Clock clock);

  private:
    void integrateParticles();
    void transfertVelocities(bool particlesToGrid);
    void solveIncompressibility();
    void computeDensity();

    void handleWallCollisions(float minX, float minY, float maxX, float maxY);
    void handleParticleCollisions();

    int getS(int index);
};

#endif // GRID_H
