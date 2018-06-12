#include <stdio.h>
#include <time.h>
#include "SDL.h"

#include "tetromino.h"


SDL_Color LIGHTBLUE = {173,216,230,255};
SDL_Color DARKBLUE = {0,0,255,255};
SDL_Color RED = {255,0,0,255};
SDL_Color YELLOW = {255,255,0,255};
SDL_Color GREEN = {0,255,0,255};
SDL_Color ORANGE = {255,165,0,255};
SDL_Color PURPLE = {128,0,128,255};
SDL_Color GRAY = {128,128,128,255};


enum {
	SIZE = 20,
	WIDTH = 10,
	HEIGHT = 20,
	MARGIN = 2
};

enum fallingState {
	FALL,
	STOP,
	NEW,
	IGNORE
};

SDL_Color GRID[HEIGHT][WIDTH];
/*
int L_blocks[4*4*4]=
		{
			0,0,0,0,
		 	1,1,1,1,
			0,0,0,0,
			0,0,0,0,

			0,0,1,0,
		 	0,0,1,0,
			0,0,1,0,
			0,0,1,0,

			0,0,0,0,
			0,0,0,0,
		 	1,1,1,1,
			0,0,0,0,

		 	0,1,0,0,
		 	0,1,0,0,
			0,1,0,0,
			0,1,0,0
			
		};
*/
struct TETRIS_BLOCK{ 
	int x;
	int y;
	int size;
	SDL_Color color;
	int *block;
	int block_pointer;
};

struct TETRIS_BLOCK newBlock();
enum fallingState checkPosition(struct TETRIS_BLOCK block, int x_offset, int y_offset, int rotation);
int drawBlock(SDL_Renderer *renderer, struct TETRIS_BLOCK block); 
int initialiseField(SDL_Renderer *renderer); 
int drawField(SDL_Renderer *renderer);
int removeBlock(SDL_Renderer *renderer, struct TETRIS_BLOCK block);
enum fallingState updateBlock(SDL_Renderer *renderer, struct TETRIS_BLOCK *block, int x_offset, int y_offset, int rotation_offset);
int updateScore(SDL_Renderer *renderer);
void printBlock(struct TETRIS_BLOCK block);
void printField();
int equalColors(SDL_Color c1, SDL_Color c2);


int main(int argc, char* argv[]) {
	srand(time(NULL));

	int interval = 0.05; // seconds
	time_t start;
	time_t end;
	int x_offset = 0;
	int y_offset = 0;
	int rotation_offset = 0;
	enum fallingState state = FALL;

	SDL_Window *window;
	SDL_Renderer *renderer;

	SDL_Init(SDL_INIT_VIDEO);

	window = SDL_CreateWindow(
		"An SDL2 window",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		640,
		480,
		SDL_WINDOW_OPENGL
	);

	if (window == NULL) {
		printf("Could not create window: %s\n", SDL_GetError());
		return 0;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	if (renderer == NULL) {
		printf("Could not create renderer: %s\n", SDL_GetError());
		return 0;
	}	

	SDL_RenderClear(renderer);
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

	initialiseField(renderer);

	struct TETRIS_BLOCK BLOCK = newBlock();

	if(!drawBlock(renderer,BLOCK)) {
		printf("something went wrong drawing the block: %s\n ", SDL_GetError());
		return 1;
	}

	SDL_Event test_event;
	int quit = 0;
	int score = 0;
	start = time(NULL);
	while(!quit) {
		end = time(NULL);
		if(difftime(end, start) > interval) {
			y_offset = 1;
			start = time(NULL);
		}
		else if(SDL_PollEvent(&test_event)) {
			if(test_event.type == SDL_KEYUP) {
				switch(test_event.key.keysym.sym){
					case SDLK_DOWN:
						y_offset = 1;
						break;
					case SDLK_UP:
						rotation_offset = 1;
						break;
					case SDLK_LEFT:
						x_offset = -1;
						break;
					case SDLK_RIGHT:
						x_offset = 1;
						break;
					case SDLK_ESCAPE:
						quit = 1;
						break;
					case SDLK_p:
						printf("x: %i\n", BLOCK.x);
						printf("y: %i\n", BLOCK.y);
						printField();
						break;		
				}
			} else if (test_event.type == SDL_QUIT) {
				quit = 1;
			}
		}	
		state = updateBlock(renderer,&BLOCK,x_offset,y_offset,rotation_offset);
		x_offset = 0;
		y_offset = 0;
		rotation_offset = 0;

		if(state == NEW) {
			BLOCK = newBlock();
			score += updateScore(renderer);
			printf("score: %i\n", score);
		} 
	}

	SDL_DestroyRenderer(renderer);

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 1;
}

int initialiseField(SDL_Renderer *renderer) {
	SDL_SetRenderDrawColor(renderer,GRAY.r,GRAY.g,GRAY.b,GRAY.a); 
	for(int h=0; h<HEIGHT;h++) {
		for(int w=0;w<WIDTH;w++) {
			GRID[h][w] = GRAY; 
		}
	}
	drawField(renderer);
	return 1;
}

int drawField(SDL_Renderer *renderer) {
	for(int h=0;h<HEIGHT;h++) {
		for(int w=0;w<WIDTH;w++) {
			if(equalColors(GRID[h][w],GRAY)) {
				SDL_Rect rect = {w*(SIZE+MARGIN),h*(SIZE+MARGIN),SIZE,SIZE}; 
				SDL_RenderFillRect(renderer,&rect);
				SDL_RenderPresent(renderer);
			}
		}
	}
	return 1;
}

enum fallingState checkPosition(struct TETRIS_BLOCK block, int x_offset, int y_offset, int rotation) {
	int x = block.x;
	int y = block.y;
	int block_pointer = block.block_pointer;
	int next_rotation = (block_pointer+rotation)%4;
	int *next_block = (block.block + next_rotation*4*4);

	int newX;
	int newY;

	for(int h=0;h<4;h++) {
		for(int w=0;w<4;w++) {
			if(*(next_block + h*4 + w)) {
				newX = x+w+x_offset;
				newY = y+h+y_offset;
				// check if block horizontally exceeds grid
				if(newX < 0 || newX >= WIDTH) {
					return IGNORE;
				} else if(newY >= HEIGHT) { // check vertically
					return NEW;
				} else if(!equalColors(GRID[newY][x+w],GRAY)) {
					return NEW;
				} else if(!equalColors(GRID[newY][newX],GRAY)) {
					return IGNORE;
				}	
			}
		}
	}
	
	return FALL;
}

int drawBlock(SDL_Renderer *renderer, struct TETRIS_BLOCK block) {
	int x = block.x;
	int y = block.y;
	SDL_Color color = block.color;
	int block_pointer = block.block_pointer;
	int *current_block = (block.block + block_pointer*4*4);

	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	for(int i=0;i<4;i++) {
		for(int j=0;j<4;j++) {
			if(*(current_block + i*4 + j)) {
				GRID[y+i][x+j] = color;
				int newX = (x+j)*(SIZE+MARGIN);
				int newY = (y+i)*(SIZE+MARGIN);
				SDL_Rect rect = {newX,newY,SIZE,SIZE};
				if(SDL_RenderFillRect(renderer, &rect)) {
					printf("Problem drawing rectangle: %s\n", SDL_GetError());
					return 0;
				}	
			}
		}
	}
	SDL_RenderPresent(renderer);
	return 1;
}

int removeBlock(SDL_Renderer *renderer, struct TETRIS_BLOCK block) {
	int x = block.x;
	int y = block.y;
	int current_block_pointer = block.block_pointer;
	int *current_block = (block.block+current_block_pointer*4*4);
	SDL_SetRenderDrawColor(renderer,128,128,128,255);
	for(int i=0;i<4;i++) {
		for(int j=0;j<4;j++) {
			if(*(current_block + i*4 + j)) {
				GRID[y+i][x+j] = GRAY;
				int newX = (x+j)*(SIZE+MARGIN);
				int newY = (y+i)*(SIZE+MARGIN);
				SDL_Rect rect = {newX,newY,SIZE,SIZE};
				if(SDL_RenderFillRect(renderer, &rect)) {
					printf("Problem drawing rectangle: %s\n", SDL_GetError());
					return 0;
				}	
			}
		}
	}
	return 1;
}


struct TETRIS_BLOCK newBlock() {
	struct TETRIS_BLOCK new_block;
	new_block.x = 0;
	new_block.y = 0;
	new_block.size = SIZE;
	new_block.block_pointer = 0;
	int r = rand() % 7;
	
	switch(r) {
		case 0:
			new_block.color = LIGHTBLUE;
			new_block.block = I_blocks;
			break;
		case 1:
			new_block.color = DARKBLUE;
			new_block.block = J_blocks;
			break;
		case 2:
			new_block.color = ORANGE;
			new_block.block = L_blocks;
			break;
		case 3:
			new_block.color = YELLOW;
			new_block.block = O_blocks;
			break;
		case 4:
			new_block.color = GREEN;
			new_block.block = S_blocks;
			break;
		case 5:
			new_block.color = PURPLE;
			new_block.block = T_blocks;
			break;
		case 6:
			new_block.color = RED;
			new_block.block = Z_blocks;
			break;
	}
	return new_block;
}

enum fallingState updateBlock(SDL_Renderer *renderer, struct TETRIS_BLOCK *block, int x_offset, int y_offset, int rotation_offset) {
	removeBlock(renderer, *block);
	enum fallingState position_status = checkPosition(*block, x_offset, y_offset, rotation_offset); 
	if(position_status == FALL) {
		block->block_pointer = (block->block_pointer + rotation_offset) % 4;
		block->x = block->x + x_offset;
		block->y = block->y + y_offset;
	}
	drawBlock(renderer, *block);
	return position_status;
}

int updateScore(SDL_Renderer *renderer) {
	int full_line;
	int nb_lines = 0;
	for(int h=HEIGHT-1;h>0;h--) {
		full_line = 1;
		for(int w=0;w<WIDTH;w++) {
			if(equalColors(GRID[h][w],GRAY)) {
				full_line = 0;
				break;
			}	
		}
		if(full_line) {
			nb_lines++;
			for(int y=h;y>0;y--) {
				for(int x=0;x<WIDTH;x++) {
					// copy line above
					SDL_Color color = GRID[y-1][x];
					GRID[y][x] = color;
					SDL_SetRenderDrawColor(renderer,color.r,color.g,color.b,color.a);
					int newX = x*(SIZE+MARGIN);
					int newY = y*(SIZE+MARGIN);
					SDL_Rect rect = {newX,newY,SIZE,SIZE};
					SDL_RenderFillRect(renderer, &rect);
				}
				
			}
			h++;
		}
	}
	return nb_lines*10;
}


void printBlock(struct TETRIS_BLOCK block) {
	int position = block.block_pointer;
	int *blocks = block.block;

	for(int i=0;i<4;i++) {
		for(int j=0;j<4;j++) {
			printf("%i ",*(blocks + position*4*4 + i*4 + j));
		}
		printf("\n");
	}
	printf("\n----------------\n");
}

void printField() {
	for(int h=0;h<HEIGHT;h++) {
		for(int w=0;w<WIDTH;w++) {
			printf("%i ", !equalColors(GRID[h][w],GRAY));
		}
		printf("\n");
	}
	printf("\n----------------\n");
}

int equalColors(SDL_Color c1, SDL_Color c2) {
	if(c1.r != c2.r) {
		return 0;
	} else if(c1.g != c2.g) {
		return 0;
	} else if(c1.b != c2.b) {
		return 0;
	} else if(c1.a != c2.a) {
		return 0;
	}

	return 1;
}
