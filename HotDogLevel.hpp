
#include <glm/glm.hpp>

struct Level {
    bool walls[32][30] = {}; 
    bool meats[32][30] = {}; 
    glm::ivec2 bun = glm::ivec2(20, 6); 
    glm::ivec2 player_start_head = glm::ivec2(2, 1); 
    glm::ivec2 player_start_butt = glm::ivec2(1, 1); 
    glm::ivec2 text_pos = glm::ivec2(15, 15); 
};
