/*

TEETH
A byte pattern visualizer tool for SG-1000 ROMs

Copyright (c) 2016, Brian Puthuff
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <stdio.h>
#include <string.h>
#include "SDL2/SDL.h"

#define LIMIT 262144
#define W 512
#define H 512
#define BD 8
#define TILES_PER_ROW 64

/* function declarations */
void renderByte(SDL_Surface* s, char byte, int y);
void clearSurface(SDL_Surface* s);
void renderSurface(char bytes[], int offset, SDL_Surface* block, SDL_Surface* image);
void updateTexture(SDL_Texture* texture, SDL_Surface* image);
void renderDisplay(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Texture* zoom, int x, int y, char zoom_state);
void updateZOOM(SDL_Surface* image, SDL_Surface* block, SDL_Surface* z_block, SDL_Texture* zoom, int x, int y);
void originCapture(int mouse_x, int mouse_y, int* x, int* y);
void saveAs(SDL_Surface* image, char arg[], int offset);

int main(int argc, char* argv[])
{
	FILE* fp;
	char bytes[LIMIT];
	char window_title[20] = "TEETH ... offset: 0";
	char is_running, show_zoom;
	int i, offset;
	int mouse_x, mouse_y, x, y;
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* texture;
	SDL_Texture* zoom;
	SDL_Surface* image;
	SDL_Surface* block;
	SDL_Surface* z_block;
	Uint32 rm, gm, bm, am;
	SDL_Event event;	

	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("Could not initialize SDL.\n");		
		return 1;
	}

	/* create window and renderer */
	SDL_CreateWindowAndRenderer(W, H, SDL_WINDOW_SHOWN, &window, &renderer);
	SDL_SetWindowTitle(window, window_title);
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);
	
	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		rm = 0xff000000;
		gm = 0x00ff0000;
		bm = 0x0000ff00;
		am = 0x000000ff;
	#else
		rm = 0x000000ff;
		gm = 0x0000ff00;
		bm = 0x00ff0000;
		am = 0xff000000;
	#endif
	
	image = SDL_CreateRGBSurface(0, W, H, 32, rm, gm, bm, am);
	block = SDL_CreateRGBSurface(0, BD, BD, 32, rm, gm, bm, am);		
	z_block = SDL_CreateRGBSurface(0, 64, 64, 32, rm, gm, bm, am);
	
	if((image == NULL) || (block == NULL) || (z_block == NULL))
	{
		printf("Could not create surface.\n");
		return 1;
	}
	
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,  W, H);
	zoom = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,  64, 64);
	if((texture == NULL) || (zoom == NULL))
	{
		printf("Could not create texture.\n");
		return 1;
	}

	/* initialize surface */
	clearSurface(image);
	
	/* initialize bytes */
	for(i = 0; i < LIMIT; ++i)
		bytes[i] = 0;	

	if(argc > 1)
	{
		fp = fopen(argv[1], "rb");
		if(fp != NULL)
		{
			/* read file contents into buffer */
			i = 0;
			while(fread(&bytes[i], sizeof(char), 1, fp) != 0)
				++i;
		}
		fclose(fp);

		/* initial display */
		offset = 0;
		show_zoom = 0;
		renderSurface(bytes, offset, block, image);
		updateTexture(texture, image);
		renderDisplay(renderer, texture, zoom, x, y, show_zoom);

		is_running = 1;

		/* loop */
		while(is_running == 1)
		{
			while(SDL_PollEvent(&event) > 0)
			{
				if(event.type == SDL_QUIT)
					is_running = 0;
				else if(event.type == SDL_KEYDOWN)
				{
					switch(event.key.keysym.sym)
					{
						case SDLK_q:
							is_running = 0;
							break;

						case SDLK_s:
							saveAs(image, argv[1], offset);
							break;

						case SDLK_UP:
						case SDLK_a:
							offset--;
							if(offset < 0)
								offset = 7;
							renderSurface(bytes, offset, block, image);
							updateTexture(texture, image);
							window_title[18] = offset + '0';
							SDL_SetWindowTitle(window, window_title);
							if((mouse_x > -1 && mouse_x < W) && (mouse_y > -1 && mouse_y < H))
							{
								originCapture(mouse_x, mouse_y, &x, &y);
								updateZOOM(image, block, z_block, zoom, x, y);
								show_zoom = 1;
							}
							else
								show_zoom = 0;
							break;

						case SDLK_DOWN:
						case SDLK_z:
							offset++;
							if(offset > 7)
								offset = 0;
							renderSurface(bytes, offset, block, image);
							updateTexture(texture, image);
							window_title[18] = offset + '0';
							SDL_SetWindowTitle(window, window_title);
							if((mouse_x > -1 && mouse_x < W) && (mouse_y > -1 && mouse_y < H))
							{
								originCapture(mouse_x, mouse_y, &x, &y);
								updateZOOM(image, block, z_block, zoom, x, y);
								show_zoom = 1;
							}
							else
								show_zoom = 0;
							break;
					}
				}
				if(event.type == SDL_MOUSEMOTION)
				{
					SDL_GetMouseState(&mouse_x, &mouse_y);
					if((mouse_x > -1 && mouse_x < W) && (mouse_y > -1 && mouse_y < H))
					{
						originCapture(mouse_x, mouse_y, &x, &y);
						updateZOOM(image, block, z_block, zoom, x, y);
						show_zoom = 1;
					}
					else
						show_zoom = 0;
				}
			}

			renderDisplay(renderer, texture, zoom, x, y, show_zoom);
		}
	}

	/* Freedom */	
	SDL_FreeSurface(block);
	SDL_FreeSurface(z_block);
	SDL_FreeSurface(image);
	SDL_DestroyTexture(zoom);
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}

void renderByte(SDL_Surface* s, char byte, int y)
{
	int i;
	Uint32* pixels;
	
	pixels = (Uint32*) s->pixels;
	for(i = 0; i < 8; ++i)
	{
		if((byte & (1 << i)) == (1 << i))
			pixels[(y * BD) + (7 - i)] = SDL_MapRGB(s->format, 0xff, 0xff, 0xff);
	}
}

void clearSurface(SDL_Surface* s)
{
	int r, c;
	Uint32* pixels;
	
	pixels = (Uint32*) s->pixels;
	for(r = 0; r < s->h; ++r)
		for(c = 0; c < s->w; ++c)
			pixels[r * s->w + c] = SDL_MapRGB(s->format, 0x00, 0x00, 0x00);
}

void renderSurface(char bytes[], int offset, SDL_Surface* block, SDL_Surface* image)
{
	int byte;
	int row, col, y;
	SDL_Rect position;

	position.w = position.h = BD;

	/* render surface */
	byte = offset;
	col = 0;
	row = 0;
	while(byte < LIMIT)
	{
		clearSurface(block);

		/* read and write 8 bytes to block image */
		for(y = 0; y < 8; ++y)
		{
			renderByte(block, bytes[byte], y);
				++byte;
		}
		position.x = col * BD;
		position.y = row * BD;
		SDL_BlitSurface(block, NULL, image, &position);
		++col;
		if(col == TILES_PER_ROW)
		{
			col = 0;
			++row;
		}
	}
}

void updateTexture(SDL_Texture* texture, SDL_Surface* image)
{
	Uint32* pixels;

	pixels = (Uint32*) image->pixels;
	SDL_UpdateTexture(texture, NULL, pixels, W * sizeof(Uint32));
}


void renderDisplay(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Texture* zoom, int x, int y, char zoom_state)
{
	SDL_Rect z_position;
	SDL_Rect hover;
	int w, h;	
	SDL_QueryTexture(zoom, NULL, NULL, &w, &h);

	z_position.w = w;
	z_position.h = h;
	hover.w = hover.h = BD;
	hover.x = x;
	hover.y = y;

	SDL_RenderCopy(renderer, texture, NULL, NULL);
	if(zoom_state == 1)
	{
		if(x < W / 2)
			z_position.x = W - z_position.w;
		else
			z_position.x = 0;
		z_position.y = 0;
		
		SDL_RenderCopy(renderer, zoom, NULL, &z_position);
	}
	
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0x80, 0x60);
	SDL_RenderFillRect(renderer, &hover);
	SDL_RenderPresent(renderer);
}


void updateZOOM(SDL_Surface* image, SDL_Surface* block, SDL_Surface* z_block, SDL_Texture* zoom, int x, int y)
{

	SDL_Rect c;
	int row, col;
	Uint32* pixels;
	Uint32 pixel;
	Uint8 r, g, b;

	c.w = BD;
	c.h = BD;
	c.x = x;
	c.y = y;;

	SDL_BlitSurface(image, &c, block, NULL);
	clearSurface(z_block);
	pixels = (Uint32*) block->pixels;
	for(row = 0; row < BD; ++row)
	{
		for(col = 0; col < BD; ++col)
		{
			pixel = pixels[(row * BD) + col];
			SDL_GetRGB(pixel, block->format, &r, &g, &b);
			c.x = col * c.w;
			c.y = row * c.h;
			SDL_FillRect(z_block, &c, SDL_MapRGBA(block->format, r, g, b, 0xff));
		}
	}

	pixels = (Uint32*) z_block->pixels;
	SDL_UpdateTexture(zoom, NULL, pixels, 64 * sizeof(Uint32));
}

void originCapture(int mouse_x, int mouse_y, int* x, int* y)
{
	unsigned int tx, ty;
	tx = mouse_x / 8;
	ty = mouse_y / 8;
	*x = tx * 8;
	*y = ty * 8;	
}


void saveAs(SDL_Surface* image, char arg[], int offset)
{
	char filename[64];
	char ext[18] = "_bytemarks-O0.bmp";
	
	ext[12] = (char) offset + '0';
	strncpy(filename, arg, strlen(arg) - 3);
	strcat(filename, ext);
	printf("Saved as %s.\n", filename);
	SDL_SaveBMP(image, filename);
}
