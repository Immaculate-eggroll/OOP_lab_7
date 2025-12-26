#include <iostream>
#include "game.h"

int main() {
    try {
        Game game;
        
        std::cout << "Press Enter to start simulation..." << std::endl;
        std::cin.get();
        
        game.start();
        game.waitForFinish();
        
        std::cout << "\nPress Enter to exit..." << std::endl;
        std::cin.get();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
