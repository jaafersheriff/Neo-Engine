/* Context class
 * Contains members to display, mouse, and keyboard 
 * Keeps track of running time and FPS */
#pragma once
#ifndef _CONTEXT_HPP_
#define _CONTEXT_HPP_

#include "Context/Display.hpp"
#include "Hardware/Mouse.hpp"
#include "Hardware/Keyboard.hpp"

#include <string>

class World;
class Context {
    public:
        /* Enum for world types */
        enum WorldTypes {
            ERROR,
            CLOUD_WORLD,
            SKY_WORLD
        };
        /* Default members */
        WorldTypes selectedWorld = ERROR;           /* World type */
        std::string RESOURCE_DIR = "../resources/"; /* Resrouce dir */

        /* References */
        Display display;
        Mouse mouse;
        Keyboard keyboard;

        double timeStep;        /* Time to render last frame in seconds     */
        bool verbose = false;   /* Print things as they happen              */

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
        int nFrames;            /* Number of frames in current second       */
        double lastFrameTime;   /* Time at which last frame was rendered    */
        double runTime;         /* Global timer                             */

        /* Utility functions */
        void printUsage();
};

#endif 
