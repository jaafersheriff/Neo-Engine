#pragma once

#define NUM_KEYS 1024

namespace neo {

    class Keyboard {

        public:
            /* Denotes whether a key is pressed */
            bool isKeyPressed(int);

            void setKeyStatus(int, int);

            void reset();

        private:
            int mKeyStatus[NUM_KEYS] = { GLFW_RELEASE };
        };
}
