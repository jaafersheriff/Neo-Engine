#define GLEW_STATIC
#include <GL/glew.h>

#include "Mouse.hpp"
#include "Keyboard.hpp"

        /* ImGui */
        static void toggleImGui();
        static bool isImGuiEnabled() { return s_imGuiEnabled; }

        static void setCursorEnabled(bool enabled);
        static void toggleCursorEnabled();

    private:
    
        /* Reference to GLFW window */
        static bool s_cursorEnabled;
        static bool s_imGuiEnabled;

#endif
