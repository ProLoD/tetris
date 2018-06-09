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


enum fallingState checkPosition(struct TETRIS_BLOCK block, int x_offset, int y_offset, int rotation);
int drawBlock(SDL_Renderer *renderer, struct TETRIS_BLOCK block); 
int initialiseField(SDL_Renderer *renderer); 
int drawField(SDL_Renderer *renderer);
int removeBlock(SDL_Renderer *renderer, struct TETRIS_BLOCK block);
void updateBlock(SDL_Renderer *renderer, struct TETRIS_BLOCK *block, int x_offset, int y_offset, int rotation_offset);
void printBlock(struct TETRIS_BLOCK block);

int main(int argc, char* argv[]) {
	int interval = 1; // seconds
	time_t start;
	time_t end;

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

	struct TETRIS_BLOCK L;
	L.x=0;
	L.y=0;
	L.size=20;
	L.color = RED;
	L.block=L_blocks;
	L.block_pointer=0;

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
			updateBlock(renderer, &L,0,1,0);
			start = time(NULL);
		}
		while(SDL_PollEvent(&test_event)) {
			end = time(NULL);
			if(difftime(end, start) > interval) {
				updateBlock(renderer, &L,0,1,0);
				start = time(NULL);
			}
			if(test_event.type == SDL_KEYUP) {
				switch(test_event.key.keysym.sym) {
					case SDLK_DOWN:
						updateBlock(renderer, &L,0,1,0);
						printf("down\n");
						break;
					case SDLK_UP:
						// removeBlock(renderer,L);
						// L.block_pointer = (L.block_pointer + 1) % 4;
						updateBlock(renderer,&L,0,0,1);
						printf("turn\n");
						break;
					case SDLK_LEFT:
						updateBlock(renderer, &L,-1,0,0);
						printf("left\n");
						break;
					case SDLK_RIGHT:
						updateBlock(renderer,&L,1,0,0);
						printf("right\n");
						break;
					case SDLK_ESCAPE:
						quit = 1;


				}
			} else if (test_event.type == SDL_QUIT) {
				quit = 1;
			}
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
				SDL_Rect rect; 
				rect.x = w*(SIZE+MARGIN); 
				rect.y = h*(SIZE+MARGIN); 
				rect.w = SIZE; 
				rect.h = SIZE;			
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

	for(int h=0;h<4;h++) {
		for(int w=0;w<4;w++) {
			if(*(next_block + h*4 + w)) {
				// check if block horizontally exceeds grid
				if(x + w + x_offset < 0 || x + w + x_offset >= WIDTH) {
					return IGNORE;
				} else if(y + h + y_offset >= HEIGHT) { // check vertically
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
	int size = block.size;
	SDL_Color color = block.color;
	int block_pointer = block.block_pointer;
	int *current_block = (block.block + block_pointer*4*4);

	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	for(int i=0;i<4;i++) {
		for(int j=0;j<4;j++) {
			if(*(current_block + i*4 + j)) {
				GRID[j][i] = 1;
				int newX = (x+j)*(SIZE+MARGIN);
				int newY = (y+i)*(SIZE+MARGIN);
				SDL_Rect rect;
				rect.x=newX;
				rect.y=newY;
				rect.w=size;
				rect.h=size;
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
				GRID[i][j] = 0;
				int newX = (x+j)*(SIZE+MARGIN);
				int newY = (y+i)*(SIZE+MARGIN);
				SDL_Rect rect;
				rect.x=newX;
				rect.y=newY;
				rect.w=SIZE;
				rect.h=SIZE;
				if(SDL_RenderFillRect(renderer, &rect)) {
					printf("Problem drawing rectangle: %s\n", SDL_GetError());
					return 0;
				}	
			}
		}
	}
	return 1;
}


void updateBlock(SDL_Renderer *renderer, struct TETRIS_BLOCK *block, int x_offset, int y_offset, int rotation_offset) {
	int position_status = checkPosition(*block, x_offset, y_offset, rotation_offset); 
	if(position_status == FALL) {
		removeBlock(renderer, *block);
		block->block_pointer = (block->block_pointer + rotation_offset) % 4;
		block->x = block->x + x_offset;
		block->y = block->y + y_offset;
		drawBlock(renderer, *block);
	} else {
		printf("invalid move\n");
	}
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

