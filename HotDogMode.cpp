//based on https://github.com/15-466/15-466-f20-base1/blob/master/PlayMode.cpp

#include "HotDogMode.hpp"

//for reading tile data
#include "read_write_chunk.hpp"
#include "data_path.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

//For asset loading:
#include "Load.hpp"

#include <random>
#include <fstream>


HotDogMode::HotDogMode() {

    std::vector< PPU466::Tile > tile_table (16 * 16);
	std::vector< PPU466::Palette > palette_table (8);

	std::vector< Level > level_table;
    std::ifstream in(data_path("../tiles.bin"), std::ios::binary); 

    read_chunk(in, "tile", &tile_table); 
    read_chunk(in, "pale", &palette_table); 
    read_chunk(in, "levl", &levels); 


    //copy read data into ppu 
    for(int i = 0; i < 16 * 16; i++) {
        ppu.tile_table[i] = tile_table[i]; 
    }
    for(int i = 0; i < 8; i++) {
        ppu.palette_table[i] = palette_table[i]; 
    }
    level = levels[level_index]; 
    player_pos.push_back(level.player_start_butt);
    player_pos.push_back(level.player_start_head);
}

HotDogMode::~HotDogMode() {
}

bool HotDogMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
        glm::ivec2 newPos = player_pos.back(); 
		if (evt.key.keysym.sym == SDLK_LEFT) {
            newPos.x--; 
		} 
        else if (evt.key.keysym.sym == SDLK_RIGHT) {
            newPos.x++; 
		} 
        else if (evt.key.keysym.sym == SDLK_UP) {
            newPos.y++; 
		} 
        else if (evt.key.keysym.sym == SDLK_DOWN) {
            newPos.y--; 
		}
        //restart the level 
        else if (evt.key.keysym.sym == SDLK_r) {
            level = levels[level_index]; 
            player_pos.clear(); 
            player_pos.push_back(level.player_start_butt);
            player_pos.push_back(level.player_start_head);
            player_length = 2; 
            extra_length = 0; 
            body_start = 0; 
            return true; 
		}

        // //deleted reverse functionality!!! so people will have to restart more >:()

        //check if the player is trying to reverse (move back from where they just came from) 
        // if(newPos.y == player_pos[body_start + player_length - 2].y && newPos.x == player_pos[body_start + player_length - 2].x) {
            
        //     if(body_start > 0) {
        //         body_start--;
        //         player_pos.pop_back(); 
        //     }
        //     //if you reverse to the very beginning. don't let length go below 2
        //     else if(body_start == 0 && player_length > 2) {
        //         player_length--;
        //         extra_length++; 
        //         player_pos.pop_back(); 
        //     }
        //     return true; 
        // }

        bool grounded = false; 

        //you can fly if you won :) 
        if(level_index == levels.size()) grounded = true; 

        //check that you are not escaping the bounds of the game 
        if(newPos.y < 0 || newPos.y >= 30  || newPos.x < 0 || newPos.x >= 32) return true; 

        //check that you are not colliding with your own body, and that you are still grounded
        //if you are about to grow, include the butt. otherwise don't. 
        int grow = extra_length > 0 ? 0 : 1;
        for(int i = body_start + grow; i < body_start + player_length; i++)
        {
            if(player_pos[i].x == newPos.x && player_pos[i].y == newPos.y) return true; 
            if(player_pos[i].y > 0 && level.walls[player_pos[i].x][player_pos[i].y-1]) grounded = true; 
        }
        if(newPos.y > 0 && level.walls[newPos.x][newPos.y-1]) grounded = true; 
        //if(extra_length > 0 && player_pos[body_start].y > 0 && level.walls[player_pos[body_start].x][player_pos[body_start].y-1]) grounded = true; 

        if(!grounded) return true; 

        //check that you are not colliding with a wall
        if(level.walls[newPos.x][newPos.y]) return true;   

        player_pos.push_back(newPos); 
        //grow if the size allows 
        if(extra_length == 0) {
            body_start++; 
        }
        //otherwise take length from the back of the hotdog
        else {
            extra_length--; 
            player_length++; 
        }       
        return true; 

	}
	return false;
}

void HotDogMode::update(float elapsed) {
    if(level_index == levels.size()) return; 
    glm::ivec2 head = player_pos[body_start + player_length - 1]; 

    //check for collision with bun, if so you win!!  
    if((level.bun.x == head.x && level.bun.y == head.y)
    || (level.bun.x + 1 == head.x && level.bun.y + 1 == head.y)){
        std::cout << "\nlevel " << std::to_string(level_index) <<" complete!"; 
        level_index++;
        if(level_index == levels.size()) {
            std::cout << "\nyou win!"; 
            extra_length = 20; 
        }
        else {
            level = levels[level_index]; 
            player_pos.clear(); 
            player_pos.push_back(level.player_start_butt);
            player_pos.push_back(level.player_start_head);
            player_length = 2; 
            extra_length = 0; 
            body_start = 0; 

        } 
    }

    //check for collision with meat
    if(level.meats[head.x][head.y]) {
        level.meats[head.x][head.y] = false; 
        extra_length++; 
    }
}

void HotDogMode::draw(glm::uvec2 const &drawable_size) {
   	//--- set ppu state based on game state ---
    int currentSprite = 0; 

    //draw bun 
    ppu.sprites[currentSprite].x = level.bun.x * 8;
    ppu.sprites[currentSprite].y = level.bun.y * 8; 
    ppu.sprites[currentSprite].index = 19;
    ppu.sprites[currentSprite].attributes = 0b10000001;
    currentSprite++; 

    ppu.sprites[currentSprite].x = level.bun.x * 8 + 8;
    ppu.sprites[currentSprite].y = level.bun.y * 8; 
    ppu.sprites[currentSprite].index = 20;
    ppu.sprites[currentSprite].attributes = 0b10000001;
    currentSprite++; 

    ppu.sprites[currentSprite].x = level.bun.x * 8;
    ppu.sprites[currentSprite].y = level.bun.y * 8 + 8; 
    ppu.sprites[currentSprite].index = 16;
    ppu.sprites[currentSprite].attributes = 0b10000001;
    currentSprite++; 

    ppu.sprites[currentSprite].x = level.bun.x * 8 + 8;
    ppu.sprites[currentSprite].y = level.bun.y * 8 + 8; 
    ppu.sprites[currentSprite].index = 17;
    ppu.sprites[currentSprite].attributes = 0b10000001;
    currentSprite++; 
    
    //draw player body
    for(int i = body_start + 1; i < body_start + player_length - 1; i++)
    {
        ppu.sprites[currentSprite].x = player_pos[i].x * 8;
        ppu.sprites[currentSprite].y = player_pos[i].y * 8;
        glm::ivec2 prev = player_pos[i-1]; 
        glm::ivec2 next = player_pos[i+1]; 
        if(abs(prev.x - next.x) == 2) ppu.sprites[currentSprite].index = 0;
        else if(abs(prev.y - next.y) == 2) ppu.sprites[currentSprite].index = 1;
        else if(next.x > player_pos[i].x) {
            if(prev.y < player_pos[i].y) ppu.sprites[currentSprite].index = 2;
            else ppu.sprites[currentSprite].index = 5;
        }
        else if(next.x < player_pos[i].x){
            if(prev.y < player_pos[i].y) ppu.sprites[currentSprite].index = 3;
            else ppu.sprites[currentSprite].index = 4;
        }
        else if(prev.x > player_pos[i].x) {
            if(next.y < player_pos[i].y) ppu.sprites[currentSprite].index = 2;
            else ppu.sprites[currentSprite].index = 5;
        }
        else if(prev.x < player_pos[i].x) {
            if(next.y < player_pos[i].y) ppu.sprites[currentSprite].index = 3;
            else ppu.sprites[currentSprite].index = 4;
        }
        ppu.sprites[currentSprite].attributes = 0b10000000;
        currentSprite++; 
    }
    
    //draw butt
    ppu.sprites[currentSprite].x = player_pos[body_start].x * 8;
    ppu.sprites[currentSprite].y = player_pos[body_start].y * 8;
    glm::ivec2 next = player_pos[body_start + 1]; 
    if(player_pos[body_start].x > next.x) ppu.sprites[currentSprite].index = 6;
    else if(player_pos[body_start].x < next.x) ppu.sprites[currentSprite].index = 7;
    else if(player_pos[body_start].y < next.y) ppu.sprites[currentSprite].index = 8;
    else if(player_pos[body_start].y > next.y) ppu.sprites[currentSprite].index = 9;
    ppu.sprites[currentSprite].attributes = 0b10000000;
    currentSprite++; 

    //draw head 
    glm::ivec2 prev = player_pos[body_start + player_length - 2]; 
    glm::ivec2 head = player_pos[body_start + player_length - 1]; 
    ppu.sprites[currentSprite].x = head.x * 8;
    ppu.sprites[currentSprite].y = head.y * 8;
    if(head.x > prev.x) ppu.sprites[currentSprite].index = 10;
    else if(head.y < prev.y) ppu.sprites[currentSprite].index = 11;
    else if(head.x < prev.x) ppu.sprites[currentSprite].index = 12;
    else if(head.y > prev.y) ppu.sprites[currentSprite].index = 13;
    ppu.sprites[currentSprite].attributes = 0b10000000;
    currentSprite++; 


    //draw meats 
    for(int x = 0; x < 32; x++) {
        for(int y = 0; y < 30; y++) {
            if(level.meats[x][y]) {
                ppu.sprites[currentSprite].x = x * 8;
                ppu.sprites[currentSprite].y = y * 8;
                ppu.sprites[currentSprite].index = 14;
                ppu.sprites[currentSprite].attributes = 0b10000000;

                currentSprite++;
            }
        } 
    }

    //draw text
    //if you won the game, write "nice :)"; 
    if(level_index == levels.size()){
        int text[6] = {34, 35, 36, 22, 37, 38};
        for(int i = 0; i < 6; i++) {
            ppu.sprites[currentSprite].x = level.text_pos.x * 8 + 5 * i;
            ppu.sprites[currentSprite].y = level.text_pos.y * 8; 
            ppu.sprites[currentSprite].index = text[i];
            ppu.sprites[currentSprite].attributes = 0b10000010;
            currentSprite++; 
        }
    }
    //otherwise write the level number
    else{
        int text[8] = {21, 22, 33, 22, 21, 37, 37, 37}; 
        text[7] = 23 + level_index % 10; 
        text[6] = 23 + level_index / 10; 
        //single digits
        if(text[6] == 23) {
            text[6] = text[7]; 
            text[7] = 255;
        }
        for(int i = 0; i < 8; i++) {
            ppu.sprites[currentSprite].x = level.text_pos.x * 8 + 5 * i;
            ppu.sprites[currentSprite].y = level.text_pos.y * 8; 
            ppu.sprites[currentSprite].index = text[i];
            ppu.sprites[currentSprite].attributes = 0b10000010;
            currentSprite++; 
        }
    }


    
    //draw remaining sprites 
    while(currentSprite < 64) {
        ppu.sprites[currentSprite].x = -1; 
        currentSprite ++; 
    }


    //background color light yellow:
    ppu.background_color = glm::u8vec4(
        0xff,
        0xdd,
        0xaa,
        0xff
    );

    //draw walls on background
	for (uint32_t y = 0; y < PPU466::BackgroundHeight; ++y) {
		for (uint32_t x = 0; x < PPU466::BackgroundWidth; ++x) {
			ppu.background[x+PPU466::BackgroundWidth*y] = 255;
            if(y < 30 && x < 32 && level.walls[x][y]) ppu.background[x+PPU466::BackgroundWidth*y] = 15 + (1 << 8); 
		}
	}

	//--- actually draw ---
	ppu.draw(drawable_size);
}
