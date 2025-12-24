#include <iostream>
#include "game.h"

int main() {
    try {
        Game game;
        
        std::cout << "=== NPC Battle Simulation ===" << std::endl;
        std::cout << "Rules:" << std::endl;
        std::cout << "  - Orcs kill Bears" << std::endl;
        std::cout << "  - Bears kill Knights" << std::endl;
        std::cout << "  - Knights kill Orcs" << std::endl;
        std::cout << "  - Map size: 100x100" << std::endl;
        std::cout << "  - Initial NPCs: 50" << std::endl;
        std::cout << "  - Game duration: 30 seconds" << std::endl;
        std::cout << std::endl;
        
        std::cout << "Press Enter to start..." << std::endl;
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
