#include <NeoEngine.hpp>

using namespace neo;

int main() {
    NeoEngine::init("TestApp", 1280, 720);
    NeoEngine::run();

    return 0;
}