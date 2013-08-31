#include <SDL_image.h>
#include <SDL_rotozoom.h>
#include "sprite.h"

#define RGBA_FORMAT 32,0x00ff0000,0x0000ff00,0x000000ff,0xff000000

void sprite_origin_rect(Sprite *sprite, Action action, int frame, SDL_Rect *rect)
{
    frame = frame % sprite->frame_count;
    rect->x = sprite->origin.x + frame *sprite->frame_size.x;
    rect->y = sprite->origin.y + action*sprite->frame_size.y;
    rect->w = sprite->frame_size.x;
    rect->h = sprite->frame_size.y;
}

#define ANGLE_STEP 15
#define ZOOM 1

void sprite_rotated_rect(Sprite *sprite, Action action, int frame, int angle, SDL_Rect *rect)
{
    frame = frame % sprite->frame_count;
    int angle_index = ((int)(360+angle+ANGLE_STEP/2) % 360) / ANGLE_STEP;
    rect->x = frame *sprite->rotated_frame_size.x;
    rect->y = action*sprite->rotated_frame_size.y+
        + sprite->rotated_frame_size.y*ACTION_COUNT*angle_index;
    rect->w = sprite->rotated_frame_size.x;
    rect->h = sprite->rotated_frame_size.y;
}

void sprite_gen_rotation(Sprite *sprite)
{
    rotozoomSurfaceSize(
            sprite->frame_size.x,
            sprite->frame_size.y,
            45, // to maximize size
            ZOOM,  // no zoom
            &sprite->rotated_frame_size.x,
            &sprite->rotated_frame_size.y
            );

    if(sprite->rotated)
        SDL_FreeSurface(sprite->rotated);

    sprite->rotated = SDL_CreateRGBSurface(SDL_HWSURFACE, 
            sprite->rotated_frame_size.x * sprite->frame_count,
            sprite->rotated_frame_size.y * ACTION_COUNT * 360/ANGLE_STEP,
            RGBA_FORMAT);
    if(sprite->rotated == NULL) {
        fprintf(stderr, "CreateRGBSurface failed: %s\n", SDL_GetError());
        exit(1);
    }
    //printf("cache size %dx%d for %d angles\n", sprite->rotated->w, sprite->rotated->h, 360/ANGLE_STEP);

    SDL_Surface *element = SDL_CreateRGBSurface(SDL_HWSURFACE, 
            sprite->frame_size.x, 
            sprite->frame_size.y,
            RGBA_FORMAT);

    SDL_SetAlpha(sprite->source,0,0xff);
    SDL_SetAlpha(element,0,0xff);
    SDL_SetAlpha(sprite->rotated,SDL_SRCALPHA,0xff);

    int frame, action, angle;
    for(action=0; action<ACTION_COUNT; action++) {
        for(frame=0; frame<sprite->frame_count; frame++) {
            SDL_Rect src;
            sprite_origin_rect(sprite, action, frame, &src);
            for(angle=0; angle<360; angle+=ANGLE_STEP) {
                SDL_Rect dst;
                sprite_rotated_rect(sprite, action, frame, angle, &dst);
                SDL_FillRect(element, NULL, 0x00000000);
                SDL_BlitSurface( sprite->source, &src, element, NULL );
                SDL_Surface *rotozoom = rotozoomSurface(element, angle, ZOOM, SMOOTHING_ON);
                SDL_SetAlpha(rotozoom,0,0);
                SDL_SetColorKey(rotozoom,0,0);
                dst.x += dst.w/2 - rotozoom->w/2;
                dst.y += dst.h/2 - rotozoom->h/2; // center
                SDL_BlitSurface(rotozoom, NULL, sprite->rotated, &dst );
                SDL_FreeSurface(rotozoom);
            }
        }
    }

    SDL_FreeSurface(element);
}

void sprite_init(Sprite *sprite, int ox, int oy, int fx, int fy, int c, char *filename)
{
    memset(sprite,0,sizeof(Sprite));
    sprite->origin.x = ox;
    sprite->origin.y = oy;
    sprite->frame_size.x = fx;
    sprite->frame_size.y = fy;
    sprite->frame_count = c;
    sprite->source = IMG_Load( filename );
    sprite->rotated = NULL;
    sprite_gen_rotation(sprite);
}



