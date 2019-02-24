#pragma once

#define NUM_KEYS 1024

namespace neo {

    class Keyboard {

        public:
            /* Denotes whether a key is pressed */
            static bool isKeyPressed(int);

            static void setKeyStatus(int, int);

            static void reset();

        private:
            static int mKeyStatus[NUM_KEYS];
        };
}
