#include <stdio.h>
#include "D3_object_model.h"
#include "object_file_handler.h"
#include "world_space.h"
#include "camera_space.h"
#include "renderer.h"
#include "Projection_Space.h"

int main ()
{
	struct world_space *world = malloc(sizeof(struct world_space));	// allocate memory for player world.
////////////////////////////////////////////////////////////////////////////////////////////
	char *object_file_path = "monky.obj"; //path to file 3d object file needs to be .obj
	//load the .obj file in to a data buffer.
	struct Object_file_data *Raw_object_file = read_object_file(object_file_path); 
	//and parse the file into a data struct that can be used to build mode
	struct Obj_file_data_object *test = get_object_vertex_3d(Raw_object_file); 
	//take the .obj files data struct and convert it to a 3d obect in model space.
	struct object_model *object_model_test = build_model(test, 7.0f,7.0f,7.0f);
	//convert_model_n_hedron_to_triangles(object_model_test);
	transform_object_to_world(object_model_test, world);
////////////////////////////////////////////////////////////////////////////////////////////////
	char *object_file_path2 = "test_cube.obj";
	struct Object_file_data *Raw_object_file2 = read_object_file(object_file_path2);
	struct Obj_file_data_object *test2 = get_object_vertex_3d(Raw_object_file2);
	struct object_model *object_model_test2 = build_model(test2,15.0f, 15.0f, 15.0f);
    //convert_model_n_hedron_to_triangles(object_model_test2);
	transform_object_to_world(object_model_test2, world);
//////////////////////////////////////////////////////////////////////////////////////////////
    char *object_file_path3 = "ball.obj";
    struct Object_file_data *Raw_object_file3 = read_object_file(object_file_path3);
    struct Obj_file_data_object *test3 = get_object_vertex_3d(Raw_object_file3);
    struct object_model *object_model_test3 = build_model(test3,10.0f, 7.0f, 10.0f);
    //convert_model_n_hedron_to_triangles(object_model_test2);
    transform_object_to_world(object_model_test3, world);
//////////////////////////////////////////////////////////////////////////////////////////////

    Renderer *render = malloc(sizeof(Renderer));
    init_screen(1500, 1000, "OpenGL Triangle", render);
    struct camera_space_objects *camera = malloc(sizeof(camera_space_objects));
    init_camera(camera, 9.0f, 15.0f, 20.0f, 90.0f, 0.1f, 1000.0f, render->aspect_ratio, 00.0f, 0.0f);
    camera = transform_world_space_objects_to_camera_space_objects(camera, world);
    struct camera_space_objects *projection_space_objects = malloc(sizeof(camera_space_objects));

    projection_space_objects = transform_camera_space_objects_to_projection_space_objects(camera, render);
    Vertex_3d **faces = render_object(projection_space_objects, render);

    int running = 1;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }         
            else if (event.type == SDL_MOUSEMOTION) { 
                switch (event.type) 
                {
                    case SDL_MOUSEMOTION:
                        printf("Mouse moved: X=%d, Y=%d, RelX=%d, RelY=%d\n",
                            event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
                        // Handle camera rotation based on mouse movement
                        //update_camera_rotation(camera, render, event.motion.x, event.motion.y);
                        //camera = transform_world_space_objects_to_camera_space_objects(camera, world);
                        //projection_space_objects = transform_camera_space_objects_to_projection_space_objects(camera, render);
                        //faces = render_object(projection_space_objects, render);
                        break;
                }

                // Handle mouse movement separately
                printf("Mouse moved: X=%d, Y=%d, RelX=%d, RelY=%d\n",
                       event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
            }            
            else if (event.type == SDL_KEYDOWN) { 
                // Key press event
                switch (event.key.keysym.sym) {
                    case SDLK_w:
                        printf("W key pressed (move forward)\n");
                        camera->camera_world_space_postion[2] += 0.5f;
                        camera = transform_world_space_objects_to_camera_space_objects(camera, world);
                        projection_space_objects = transform_camera_space_objects_to_projection_space_objects(camera, render);
                        faces = render_object(projection_space_objects, render);
                        break;
                    case SDLK_s:
                        printf("S key pressed (move backward)\n");
                        camera->camera_world_space_postion[2] -= 0.5f;
                        camera = transform_world_space_objects_to_camera_space_objects(camera, world);
                        projection_space_objects = transform_camera_space_objects_to_projection_space_objects(camera, render);
                        faces = render_object(projection_space_objects, render);
                        break;
                    case SDLK_a:
                        printf("A key pressed (move left)\n");
                        camera->camera_world_space_postion[0] -= 0.5f;
                        camera = transform_world_space_objects_to_camera_space_objects(camera, world);
                        projection_space_objects = transform_camera_space_objects_to_projection_space_objects(camera, render);
                        faces = render_object(projection_space_objects, render);
                        break;
                    case SDLK_d:
                        printf("D key pressed (move right)\n");
                        camera->camera_world_space_postion[0] += 0.5f;
                        camera = transform_world_space_objects_to_camera_space_objects(camera, world);
                        projection_space_objects = transform_camera_space_objects_to_projection_space_objects(camera, render);
                        faces = render_object(projection_space_objects, render);
                        break;
                    case SDLK_UP:
                        printf("Up arrow pressed (move up)\n");
                        camera->camera_world_space_postion[1] += 0.5f; // Move camera up in world space
                        camera = transform_world_space_objects_to_camera_space_objects(camera, world);
                        projection_space_objects = transform_camera_space_objects_to_projection_space_objects(camera, render);
                        faces = render_object(projection_space_objects, render);
                        break;
                    case SDLK_DOWN:
                        printf("Down arrow pressed (move down)\n");
                        camera->camera_world_space_postion[1] -= 0.5f; // Move camera down in world space
                        camera = transform_world_space_objects_to_camera_space_objects(camera, world);
                        projection_space_objects = transform_camera_space_objects_to_projection_space_objects(camera, render);
                        faces = render_object(projection_space_objects, render);
                        break;
                    case SDLK_ESCAPE:
                        printf("Escape key pressed (exit)\n");
                        running = 0; // Exit the loop
                        break;
                    default:
                        printf("Some other key pressed: %d\n", event.key.keysym.sym);
                        break;
                }
                
                //printf("[DEBUG] Player position | x: %d, y: %d \n", main_player->x, main_player->y);
            }
        }
    
        // Clear screen
        SDL_SetRenderDrawColor(render->renderer, 0, 0, 0, 255);
        SDL_RenderClear(render->renderer);
    
        // Set drawing color
        SDL_SetRenderDrawColor(render->renderer, 255, 255, 255, 255);
    
        // Draw faces
        for (int face_index = 0; face_index < render->total_faces; face_index++) {
            SDL_Point points[render->SDL_Point_number_of_points[face_index]];
            for (int j = 0; j < render->SDL_Point_number_of_points[face_index]; j++) {
                points[j].x = mapX(faces[face_index][j].x, render->screen_width);
                points[j].y = mapY(faces[face_index][j].y, render->screen_height);
            }
    
            // Draw lines between the points to form a shape
            for (int j = 0; j < render->SDL_Point_number_of_points[face_index]; j++) {
                SDL_RenderDrawLine(
                    render->renderer, 
                    points[j].x, points[j].y, 
                    points[(j+1) % render->SDL_Point_number_of_points[face_index]].x, 
                    points[(j+1) % render->SDL_Point_number_of_points[face_index]].y
                );
            }
        }
    
        SDL_RenderPresent(render->renderer);
        SDL_Delay(16);
    }
    
    // Free memory and cleanup
    //for (int i = 0; i < render->total_faces; i++) {
    //    free(faces[i]);
   // }
    //free(faces);
    //free(render->SDL_Point_number_of_points);
    SDL_DestroyRenderer(render->renderer);
    SDL_DestroyWindow(render->window);
    SDL_Quit();
    
}

void update_camera_rotation(camera_space_objects *camera, Renderer *render, int rel_x, int rel_y)
{
    const float sensitivity = 0.005f; // Adjust for more/less sensitivity

    // Convert mouse movement to rotation
    camera->camera_rotation_y += rel_x * sensitivity; // Yaw rotation (left/right)
    camera->camera_rotation_x -= rel_y * sensitivity; // Pitch rotation (up/down)

    // Clamp pitch to avoid flipping
    if (camera->camera_rotation_x > M_PI / 2)
        camera->camera_rotation_x = M_PI / 2;
    if (camera->camera_rotation_x < -M_PI / 2)
        camera->camera_rotation_x = -M_PI / 2;
}
