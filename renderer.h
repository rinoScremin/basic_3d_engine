#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include <stdio.h>
#include "world_space.h"
#include <stdio.h>
#include "camera_space.h"
#include "D3_object_model.h"
#include <stdlib.h>
#include <stdbool.h>

typedef struct Renderer {
    float screen_width;
    float screen_height;
    float aspect_ratio;
    SDL_Window *window;
    SDL_Renderer *renderer;
    int total_faces;
    int total_objects;
    int *SDL_Point_number_of_points;
    //bool *face_culling_list;// need to add if face is culled 
} Renderer;

void init_screen(float x, float y, char *game_screen_title, Renderer *render);

Vertex_3d **render_object(struct camera_space_objects *camera, Renderer *render);

int mapX(float x, int SCREEN_WIDTH);
int mapY(float y, int SCREEN_HEIGHT);

#endif // RENDERER_H
