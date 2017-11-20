/* Context class
 * Contains members to display, mouse, and keyboard 
 * Keeps track of running time and FPS */
#pragma once
#ifndef _CONTEXT_HPP_
#define _CONTEXT_HPP_

#include "Context/Display.hpp"
#include "Hardware/Mouse.hpp"
#include "Hardware/Keyboard.hpp"

class World;
class Context {
    public:
        /* References */
        Display display;
        Mouse mouse;
        Keyboard keyboard;

        /* Global window timer */
        float runningTime;

        // TODO : world type enum 
        // TODO : Set world + default
        // TODO : arg <optional value> booleans or something

        /* Process command line args */
        int processArgs(int, char**);

        /* Init self and members */
        void init();

        /* Create application world based on command line args */
        World* createWorld(); 

        /* Update self and members */
        void update();

        /* Returns End of Life */
        bool shouldClose();

        /* Wrap up */
        void cleanUp();

    private: 
        /* FPS */
        double fps = 0.0;
        double lastTime; 
        double nbFrames; 
};

#endif 
