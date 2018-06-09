#include <stdio.h>
#include <time.h>
#include "SDL.h"



SDL_Color BLUE = {0,0,255,255};
SDL_Color RED = {255,0,0,255};
SDL_Color YELLOW = {255,255,0,255};
SDL_Color GREEN = {0,255,0,255};
SDL_Color ORANGE = {255,165,0,255};


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

int GRID[HEIGHT][WIDTH];

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

struct TETRIS_BLOCK{ 
	int x;
	int y;
	int size;
	SDL_Color color;
	int *block;
	int block_pointer;
};

void newBlock(struct TETRIS_BLOCK *block); 
enum fallingState checkPosition(struct TETRIS_BLOCK block, int x_offset, int y_offset, int rotation);
int drawBlock(SDL_Renderer *renderer, struct TETRIS_BLOCK block); 
int initialiseField(SDL_Renderer *renderer); 
int drawField(SDL_Renderer *renderer);
int removeBlock(SDL_Renderer *renderer, struct TETRIS_BLOCK block);
enum fallingState updateBlock(SDL_Renderer *renderer, struct TETRIS_BLOCK *block, int x_offset, int y_offset, int rotation_offset);
void printBlock(struct TETRIS_BLOCK block);
void printField();

int main(int argc, char* argv[]) {
	int interval = 1; // seconds
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

	struct TETRIS_BLOCK L = {0,0,20,RED,L_blocks,0};

	if(!drawBlock(renderer,L)) {
		printf("something went wrong drawing the block: %s\n ", SDL_GetError());
		return 1;
	}
	printBlock(L);


	SDL_Event test_event;
	int quit = 0;
	start = time(NULL);
	while(!quit) {
		end = time(NULL);
		if(difftime(end, start) > interval) {
			state = updateBlock(renderer, &L,0,1,0);
			start = time(NULL);
			printField();
		}
		else if(SDL_PollEvent(&test_event)) {
			if(test_event.type == SDL_KEYUP) {
				switch(test_event.key.keysym.sym) {
					case SDLK_DOWN:
						y_offset = 1;
						printf("down\n");
						break;
					case SDLK_UP:
						rotation_offset = 1;
						printf("turn\n");
						break;
					case SDLK_LEFT:
						x_offset = -1;
						printf("left\n");
						break;
					case SDLK_RIGHT:
						x_offset = 1;
						printf("right\n");
						break;
					case SDLK_ESCAPE:
						quit = 1;
						break;
				}

				state = updateBlock(renderer,&L,x_offset,y_offset,rotation_offset);
				x_offset = 0;
				y_offset = 0;
				rotation_offset = 0;

			} else if (test_event.type == SDL_QUIT) {
				quit = 1;
			}
		}	
		if(state == NEW) {
			newBlock(&L);
		}
	}

	SDL_DestroyRenderer(renderer);

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 1;
}

int initialiseField(SDL_Renderer *renderer) {
	SDL_SetRenderDrawColor(renderer,128,128,128,255); 
	for(int h=0; h<HEIGHT;h++) {
		for(int w=0;w<WIDTH;w++) {
			GRID[h][w] = 0; 
		}
	}
	drawField(renderer);
	return 1;
}

int drawField(SDL_Renderer *renderer) {
	for(int h=0;h<HEIGHT;h++) {
		for(int w=0;w<WIDTH;w++) {
			if(!GRID[h][w]) {
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
				} else if(GRID[newY][x]) {
					printf("next\n");
					return NEW;
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
				GRID[y+i][x+j] = 1;
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
				GRID[y+i][x+j] = 0;
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

void newBlock(struct TETRIS_BLOCK *block) {
	block->x = 0;
	block->y = 0;
	block->block_pointer = 0;
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
			printf("%i ", GRID[h][w]);
		}
		printf("\n");
	}
	printf("\n----------------\n");
}

