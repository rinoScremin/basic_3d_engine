#include "renderer.h"


void init_screen(float x, float y, char *game_screen_title, Renderer *render) {
    render->screen_width = x;
    render->screen_height = y;
    render->aspect_ratio = x/y;
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }

    render->window = SDL_CreateWindow(game_screen_title,
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      (int)x, (int)y,
                                      SDL_WINDOW_SHOWN);
    if (!render->window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }

    render->renderer = SDL_CreateRenderer(render->window, -1, SDL_RENDERER_ACCELERATED);
    if (!render->renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(render->window);
        SDL_Quit();
        exit(1);
    }

    SDL_SetRenderDrawColor(render->renderer, 0, 0, 0, 255); // Black background
    SDL_RenderClear(render->renderer);
    SDL_RenderPresent(render->renderer);
}

int mapX(float x, int SCREEN_WIDTH) {
    return (int)((x + 1) * (SCREEN_WIDTH / 2));
}

int mapY(float y, int SCREEN_HEIGHT) {
    return (int)((1 - y) * (SCREEN_HEIGHT / 2)); // Flip Y
}


Vertex_3d **render_object(struct camera_space_objects *camera, Renderer *render)
{
    int total_faces = 0;
    int total_vertices = 0;
    for (int world_space_obj_index = 0; world_space_obj_index < camera->Camera_space_objects->object_count; world_space_obj_index++)
    {
        total_faces += camera->Camera_space_objects->world_objects[world_space_obj_index]->face_count;
        total_vertices += camera->Camera_space_objects->world_objects[world_space_obj_index]->vertex_count;
    }
    render->total_faces = total_faces;
    
    // Allocate memory for face vertex counts
    render->SDL_Point_number_of_points = malloc(render->total_faces * sizeof(int));
    if (!render->SDL_Point_number_of_points) {
        fprintf(stderr, "Error: Memory allocation for SDL_Point_number_of_points failed\n");
        return NULL;
    }

    // Allocate memory for faces
    Vertex_3d **faces = (Vertex_3d **)malloc(total_faces * sizeof(Vertex_3d *));
    int *face_visibility = (int *)malloc(total_faces * sizeof(int));

    if (!faces || !face_visibility) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        free(render->SDL_Point_number_of_points);
        return NULL;
    }
    
    render->total_objects = camera->Camera_space_objects->object_count;
    int vertex_counter = 0;
    int face_counter = 0;
    for (int world_space_obj_index = 0; world_space_obj_index < camera->Camera_space_objects->object_count; world_space_obj_index++)
    {
        struct object_model *camera_space_obj = camera->Camera_space_objects->world_objects[world_space_obj_index];
        printf("\n[INFO] Rendering Object %d: %d Faces\n", world_space_obj_index, camera_space_obj->face_count);

        for (int face_index = 0; face_index < camera_space_obj->face_count; face_index++)
        {
            
            
            struct model_faces *face = &camera_space_obj->object_mesh[face_index];
            int vertex_count = camera_space_obj->face_vertex_count[face_index];
            render->SDL_Point_number_of_points[face_counter] = vertex_count;

            
        
            printf("face->cull : %d\n", face->cull);
            if (!face->cull)
            {

                printf("[INFO] Face %d has %d vertices\n", face_index, vertex_count);

                // Allocate memory for each face's vertices
                faces[face_counter] = (Vertex_3d *)malloc(vertex_count * sizeof(Vertex_3d));
                if (!faces[face_counter]) {
                    fprintf(stderr, "Error: Memory allocation for face %d failed\n", face_counter);
                    
                    for (int i = 0; i < face_counter; i++) {
                        free(faces[i]);
                    }
                    free(faces);
                    free(render->SDL_Point_number_of_points);
                    return NULL;
                }
    

                face_visibility[face_counter] = 1;  // Mark this face as visible
                faces[face_counter] = (Vertex_3d *)malloc(vertex_count * sizeof(Vertex_3d));
                if (!faces[face_counter]) {
                    fprintf(stderr, "Error: Memory allocation for face %d failed\n", face_counter);
                    free(face_visibility);
                    return NULL;
                }
                for (int vertex_index = 0; vertex_index < vertex_count; vertex_index++) {
                    faces[face_counter][vertex_index] = face->face[vertex_index];
                }
                face_counter++;
            }
            render->total_faces = face_counter;
        }
    }
    return faces;
}
