//based on https://github.com/15-466/15-466-f20-base1/blob/master/PlayMode.hpp


#include "PPU466.hpp"
#include "Mode.hpp"
#include "HotDogLevel.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct HotDogMode : Mode {
	HotDogMode();
	virtual ~HotDogMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

    std::vector< Level > levels; 
    uint8_t level_index = 0; 
    Level level; 


	//player position:
	std::vector <glm::ivec2> player_pos = {};
    uint8_t player_length = 2; 
    uint8_t extra_length = 0; 
    uint8_t body_start = 0; 

	//----- drawing handled by PPU466 -----

	PPU466 ppu;
};

