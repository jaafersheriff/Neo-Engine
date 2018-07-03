#include <Engine.hpp>

using namespace neo;

int main() {
    Engine::init("", 1280, 720);
    Engine::run();
    return 0;
}