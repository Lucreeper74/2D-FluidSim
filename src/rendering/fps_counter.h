//
// Created by Luc_Creeper74 on 20/08/2025.
// Copyright (c) 2025 - MIT.
//

#ifndef FPS_COUNTER_H
#define FPS_COUNTER_H

#include <SFML/Graphics.hpp>

class FPS {
    public:
        int fps;
        int frames;
        sf::Clock clock;

        /**
         * @brief Constructor for initialization
         */
        FPS();

        int getFPS() const { return fps; }
        int getFrames() const { return frames; }
        void updateFPS();
};

#endif //FPS_COUNTER_H