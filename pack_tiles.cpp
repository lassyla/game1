// based on https://github.com/15-466/15-466-f19-base1/blob/master/pack-sprites.cpp
// places sprite info and level info into binary file
#include "load_save_png.hpp"
#include "read_write_chunk.hpp"
#include "PPU466.hpp"
#include "data_path.hpp"
#include "HotDogLevel.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <fstream>

int main(int argc, char **argv) {
    std::cout << "packing tiles...\n"; 

    uint8_t num_levels = 3; 
    //files to read
	std::string sprite_filenames[3] = {"../images/sprite1.png", "../images/sprite2.png","../images/sprite3.png"};
	std::string level_filename = "../images/level";
    //png size in each file 
    glm::uvec2 sizes[3] = {glm::uvec2(24,41), glm::uvec2(24,17), glm::uvec2(24,41)};

	std::vector< PPU466::Tile > tile_table (16 * 16);
	std::vector< PPU466::Palette > palette_table (8);
    std::vector< Level > levels; 
    std::vector< glm::u8vec4 > data; 

	uint8_t currentTile = 0; 

    //load all sprite pngs
    for(int i = 0; i < 3; i++) {

        std::cout << "loading " << sprite_filenames[i] << "\n"; 
		load_png(data_path(sprite_filenames[i]), &sizes[i], &data, UpperLeftOrigin); 

		//palette is contained in the first 4 pixels in first row TODO
		PPU466::Palette palette; 
        for(uint8_t p = 0; p < 4; p++) {
            palette[p] = data[p];
        }

		assert(sizes[i].x % 8 == 0);         
		assert((sizes[i].y - 1) % 8 == 0); 
		assert((sizes[i].x - 1) * sizes[i].y <= data.size());

		//copy tile information 
		PPU466::Tile tile; 
        for(uint32_t tileY = 0; tileY < sizes[i][1] - 1; tileY+= 8){
       		for(uint32_t tileX = 0; tileX < sizes[i][0]; tileX+= 8){
                memset(&tile.bit0, 0, 8); 
                memset(&tile.bit1, 0, 8); 
                for(uint8_t pixelY = 0; pixelY < 8; pixelY++){
    				for(uint8_t pixelX = 0; pixelX < 8; pixelX++) {
						for(uint8_t p = 0; p < 4; p++){
							if(data[(tileX + pixelX) + sizes[i].x * (tileY + 8 - pixelY)] == palette[p]){
								tile.bit0[pixelY] = tile.bit0[pixelY] | ((p & 1) << pixelX); 
								tile.bit1[pixelY] = tile.bit1[pixelY] | (((p & 2) << (pixelX)) >> 1); 
							}
						}
					}
				}
                tile_table[currentTile] = tile; 
		        currentTile++; 
			}
		}
        palette_table[i] = palette;
    }

    glm::uvec2 level_size = glm::uvec2(32, 30); 
    //load all level pngs
    for(int i = 0; i < num_levels; i++) {
        Level level; 

        std::cout << "loading " << (level_filename + std::to_string(i) + ".png") << "\n"; 
        load_png(data_path(level_filename  + std::to_string(i) + ".png"), &level_size, &data, LowerLeftOrigin); 

        for(int x = 0; x < 32; x++) {
            for(int y = 0; y < 30; y++) {
                glm::u8vec4 color = data[x + y * 32];
                if(color[0] == 0xff && color[1] == 0xff && color[2] == 0xff){
                    level.player_start_head = glm::ivec2(x, y);
                }
                else if(color[0] == 0xff && color[1] == 0xff && color[2] == 0x00){
                    level.player_start_butt = glm::ivec2(x, y);
                }
                else if(color[0] == 0xff && color[1] == 0x00 && color[2] == 0x00){
                    level.walls[x][y] = true; 
                }
                else if(color[0] == 0x00 && color[1] == 0xff && color[2] == 0x00){
                    level.meats[x][y] = true; 
                }
                else if(color[0] == 0x00 && color[1] == 0x00 && color[2] == 0xff){
                    level.bun = glm::ivec2(x, y);
                }
                else if(color[0] == 0x00 && color[1] == 0xff && color[2] == 0xff){
                    level.text_pos = glm::ivec2(x, y);
                }

            }
        }
        levels.push_back(level); 
    }

	std::ofstream out(data_path("../tiles.bin"), std::ios::binary);
    write_chunk("tile", tile_table, &out); 
    write_chunk("pale", palette_table, &out); 
    write_chunk("levl", levels, &out); 

    std::cout << "done! created " << currentTile + 1 << " tiles and " << levels.size() << " levels."; 

	return 0;
}