#include <stdio.h>
#include "SDL.h"

enum {
	SIZE = 20,
	WIDTH = 10,
	HEIGHT = 20,
	MARGIN = 2
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

typedef enum {
	BLUE,
	RED,
	YELLOW,
	GREEN,
	ORANGE
} COLOR;


struct TETRIS_BLOCK{ 
	int x;
	int y;
	int size;
	COLOR color;
	int *block;
	int block_pointer;
};


int drawBlock(SDL_Renderer *renderer, struct TETRIS_BLOCK block); 
int initialiseField(SDL_Renderer *renderer); 
int drawField(SDL_Renderer *renderer);
int removeBlock(SDL_Renderer *renderer, struct TETRIS_BLOCK block);
void updateBlock(SDL_Renderer *renderer, struct TETRIS_BLOCK block, int x_offset, int y_offset);
void printBlock(struct TETRIS_BLOCK block);

int main(int argc, char* argv[]) {
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
		return 1;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	if (renderer == NULL) {
		printf("Could not create renderer: %s\n", SDL_GetError());
		return 1;
	}	

	SDL_RenderClear(renderer);
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

	initialiseField(renderer);
	SDL_Delay(3000);

	struct TETRIS_BLOCK L;
	L.x=0;
	L.y=0;
	L.size=20;
	L.color = BLUE;
	L.block=L_blocks;
	L.block_pointer=0;

	if(drawBlock(renderer,L)) {
		printf("something went wrong drawing the block: %s\n ", SDL_GetError());
		return 1;
	}
	printBlock(L);


	SDL_Event test_event;
	int quit = 0;
	while(!quit) {
		while(SDL_PollEvent(&test_event)) {
			if(test_event.type == SDL_KEYUP) {
				switch(test_event.key.keysym.sym) {
					case SDLK_DOWN:
						updateBlock(renderer, L,0,1);
						printf("down\n");
						break;
					case SDLK_UP:
						break;
					case SDLK_LEFT:
						break;
					case SDLK_RIGHT:
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
	return 0;
}

int initialiseField(SDL_Renderer *renderer) {
	SDL_SetRenderDrawColor(renderer,128,128,128,255); 
	for(int h=0; h<HEIGHT;h++) {
		for(int w=0;w<WIDTH;w++) {
			GRID[h][w] = 0; 
		}
	}
	drawField(renderer);
	return 0;
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
	return 0;
}

int drawBlock(SDL_Renderer *renderer, struct TETRIS_BLOCK block) {
	int x = block.x;
	int y = block.y;
	int size = block.size;
	int block_pointer = block.block_pointer;
	int *current_block = (block.block + block_pointer*4*4);

	switch(block.color) {
		case BLUE:
			SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
			break;
		case RED:
			SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
			break;
		case YELLOW:
			SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
			break;
		case GREEN:
			SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
			break;
		case ORANGE:
			SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255);
			break;
		default:
			printf("no correct color found, shouldn't come till this point\n");
			return 1;
	}
	for(int i=0;i<4;i++) {
		for(int j=0;j<4;j++) {
			if(*(current_block + i*4 + j)) {
				GRID[j][i] = 1;
				int newX = j*(SIZE+MARGIN) + x;
				int newY = i*(SIZE+MARGIN) + y;
				SDL_Rect rect;
				rect.x=newX;
				rect.y=newY;
				rect.w=size;
				rect.h=size;
				if(SDL_RenderFillRect(renderer, &rect)) {
					printf("Problem drawing rectangle: %s\n", SDL_GetError());
					return 1;
				}	
			}
		}
	}
	SDL_RenderPresent(renderer);
	return 0;
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
				int newX = i*(SIZE+MARGIN) + x;
				int newY = j*(SIZE+MARGIN) + y;
				SDL_Rect rect;
				rect.x=newX;
				rect.y=newY;
				rect.w=SIZE;
				rect.h=SIZE;
				if(SDL_RenderFillRect(renderer, &rect)) {
					printf("Problem drawing rectangle: %s\n", SDL_GetError());
					return 1;
				}	
			}
		}
	}
	return 0;
}


// TODO change in coordinates of block not passed to the L-block
void updateBlock(SDL_Renderer *renderer, struct TETRIS_BLOCK block, int x_offset, int y_offset) {
	block.x = block.x + x_offset;
	block.y = block.y + y_offset;
	drawBlock(renderer, block);
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

