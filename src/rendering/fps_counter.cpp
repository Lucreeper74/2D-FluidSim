//
// Created by Luc_Creeper74 on 20/08/2025.
// Copyright (c) 2025 - MIT.
//

#include "fps_counter.h"


FPS::FPS() : fps(0), frames(0) {}

void FPS::updateFPS() {
    if (clock.getElapsedTime().asSeconds() >= 1.f) {
        fps = frames;
        frames = 0;
        clock.restart();
    }
    frames++;
}