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
        /* Enum for world types */
        enum WorldTypes {
            DEV_WORLD_TYPE
        };
        /* Default world type is dev world */
        WorldTypes selectedWorld = DEV_WORLD_TYPE;

        /* References */
        Display display;
        Mouse mouse;
        Keyboard keyboard;

        float runningTime;      /* Global timer                             */
        double lastTime;        /* Time at which last frame was rendered    */
        float displayTime;      /* Time to render last frame in seconds     */
        bool verbose = false;   /* Print things as they happen */

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
        /* Utility vars */
        double fps = 0.0;       /* FPS                                      */
        double lastFpsTime;     /* Time at which last FPS was calculated    */
        double nbFrames;        /* Number of frames in current second       */
        
        /* Utility functions */
        void printUsage();
};

#endif 
