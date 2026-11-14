#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <SDL3/SDL.h>
#include "map.h"
#define SDL_MAIN_HANDLED

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef float f32;
typedef double d32;
#define ASSERT(_e, ...) if(!(_e)) { fprintf(stderr, __VA_ARGS__); exit(1); }



#define SCREENW 600
#define SCREENH 400


typedef struct Vector{d32 x,y;}Vec;


typedef struct GameState {
    Vec dir, plane, pos;
    SDL_Window *window;
    SDL_Renderer *renderer;
}state;



static void render(state s){
    // Create local copies of GameState variables
    const Vec dir = s.dir;
    const Vec plane = s.plane;
    const Vec pos = s.pos;
    
    for(int x = 0; x < SCREENW; x++){
        // cameraspace [-1 <---> 1]
        double cameraX = 2 * ((double)x / SCREENW) - 1;
        // Ray direction shot out from every x with respect to dir and plane vectors 
        const Vec rayDir = {
            dir.x + plane.x * cameraX,
            dir.y + plane.y * cameraX
        };
        // Distance from one gridline to another
        const Vec deltaRay = {
            .x = (rayDir.x == 0) ? 1e30 : fabs(1/rayDir.x),
            .y = (rayDir.y == 0) ? 1e30 : fabs(1/rayDir.y)
        };
        // Player positon, cast to int to trunicate; positon will land in map grid.
        Vec mapPos = { (int) pos.x, (int) pos.y};
        Vec dst1 = {
            deltaRay.x * ((rayDir.x > 0) ? (mapPos.x + 1 - pos.x) : (pos.x - mapPos.x)),
            deltaRay.y * ((rayDir.y > 0) ? (mapPos.y + 1 - pos.y) : (pos.y - mapPos.y))
        };

        Vec stepDir = {
            .x = (rayDir.x > 0) ? 1 : -1,
            .y = (rayDir.y > 0) ? 1 : -1
        };
        int hit = 0;
        int side = 0;
// DDA
    while(hit == 0){   
    if(dst1.x < dst1.y){
        dst1.x += deltaRay.x;
        mapPos.x += stepDir.x;
        side = 0;
    }
    else{
        dst1.y += deltaRay.y;
        mapPos.y += stepDir.y;
        side = 1;
    } // Ray hit wall?
    if(Map[(int)mapPos.x][(int)mapPos.y] > 0){
        hit = 1;
    }
    }
    ASSERT(
        mapPos.x >= 0 && mapPos.x < MAP_WIDTH && mapPos.y >= 0 && mapPos.y < MAP_HEIGHT,
        "Outside of Map");
    double truewallDist;
    if(side == 0){
        // east-west facing wall or vertical gridlines
        truewallDist = (dst1.x - deltaRay.x);
    }else{
        // north-south facing wall or horizontal gridlines
        truewallDist = (dst1.y - deltaRay.y);
    }
    float lineHeight = (int)(SCREENH / truewallDist);
    float lineBeg = (-lineHeight / 2) + (SCREENH / 2); // Neg lineheight due to computer coordinates. -1 Lineheight/2 will yield
    float lineEnd = (lineHeight / 2) + (SCREENH / 2);
    if(lineBeg < 0) lineBeg = 0;
    if(lineEnd > SCREENH) lineEnd = SCREENH - 1;
    
    SDL_Color wallColor = {0,0,0,0};
    switch(Map[(int)mapPos.x][(int)mapPos.y]){
    case 1:{
     //Cyan
    SDL_Color Cyan = {
    .r = 0,
    .g = 255,
    .b = 255,
    .a = 255};
    wallColor = Cyan;
    break;
    //case 2:
    }
}

    SDL_SetRenderDrawColor(s.renderer, wallColor.r, wallColor.g, wallColor.b, wallColor.a);
    SDL_RenderLine(s.renderer, x, lineBeg, x, lineEnd);
    }
}
 static void rotMatrix(state *s, f32 value){ 
    const Vec origDir =s->dir;
    const Vec origPlane =s->plane;

    s->dir.x = origDir.x * cos(value) - origDir.y * sin(value);
    s->dir.y = origDir.x * sin(value) + origDir.y * cos(value);

    s->plane.x = origPlane.x * cos(value) - origPlane.y * sin(value);
    s->plane.y = origPlane.x * sin(value) + origPlane.y * cos(value);
   
}

int main(int argc, char *argv[]) {
const f32 rotationS = 6.0 * .0016;
const f32 moveSpeed = 6.0 * .0016;
 state Game = {
    .dir = {-1,0},
    .plane = {0,0.66},
    .pos = {3, 3}
};

ASSERT(SDL_Init(SDL_INIT_VIDEO), "SDL_Init failed: %s\n", SDL_GetError());

Game.window = SDL_CreateWindow(
    "RayCaster",  
    SCREENW,      
    SCREENH,      
    0             
);
ASSERT(Game.window, "SDL_CreateWindow failed: %s\n", SDL_GetError());

Game.renderer = 
SDL_CreateRenderer(Game.window, NULL);

ASSERT(Game.renderer, "SDL_CreateRenderer failed: %s\n", SDL_GetError());

bool running = true;

    while(running){
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            if(event.type == SDL_EVENT_QUIT){
                running = false;
            }
        }
        const bool *keystate = SDL_GetKeyboardState(NULL);

        if (keystate[SDL_SCANCODE_ESCAPE]) {
            running = false;
        }
        if(keystate[SDL_SCANCODE_LEFT]){
            rotMatrix(&Game, +rotationS);
        }
        if(keystate[SDL_SCANCODE_RIGHT]){
            rotMatrix(&Game, -rotationS);
        }
        if(keystate[SDL_SCANCODE_UP]){
            Game.pos.x += Game.dir.x * moveSpeed;
            Game.pos.y += Game.dir.y * moveSpeed;
        }
         if(keystate[SDL_SCANCODE_DOWN]){
            Game.pos.x -= Game.dir.x * moveSpeed;
            Game.pos.y -= Game.dir.y * moveSpeed;
        }
        SDL_SetRenderDrawColor(Game.renderer, 0, 0, 0, 255);
        SDL_RenderClear(Game.renderer);
          render(Game);
          SDL_RenderPresent(Game.renderer);
          SDL_Delay(16);
    }
    SDL_DestroyRenderer(Game.renderer);
    SDL_DestroyWindow(Game.window);
    SDL_Quit();
    return 0;
}
