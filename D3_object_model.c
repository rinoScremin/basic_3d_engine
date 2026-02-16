#include <stdio.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_block.h>
#include "object_file_handler.h"
#include "D3_object_model.h"
#include <string.h>
#include <stdlib.h>


struct object_model *build_model(struct Obj_file_data_object *OFDO, float world_space_postion_x, float world_space_postion_y, float world_space_postion_z)
{
    printf("build_model_function_function_entry\n");

    // Allocate memory for the object model
    struct object_model *final_model = malloc(sizeof(struct object_model));
    if (!final_model)
    {
        printf("[ERROR] Failed to allocate memory for object model!\n");
        exit(EXIT_FAILURE);
    }

    // Allocate memory for faces
    struct model_faces *Model_faces = malloc(OFDO->face_count * sizeof(struct model_faces));
    if (!Model_faces)
    {
        printf("[ERROR] Failed to allocate memory for model faces!\n");
        free(final_model);
        exit(EXIT_FAILURE);
    }

    for (int face_index = 0; face_index < OFDO->face_count; face_index++)
    {
        int number_of_vertexs = OFDO->face_vertex_count[face_index];

        // Allocate memory for the face's vertices
        Model_faces[face_index].face = malloc(number_of_vertexs * sizeof(Vertex_3d));
        if (!Model_faces[face_index].face)
        {
            printf("[ERROR] Failed to allocate memory for face %d vertices!\n", face_index);
            
            // Free previously allocated faces before exiting
            for (int i = 0; i < face_index; i++) {
                free(Model_faces[i].face);
            }
            free(Model_faces);
            free(final_model);
            
            exit(EXIT_FAILURE);
        }

        for (int vertex_index = 0; vertex_index < number_of_vertexs; vertex_index++)
        {
            int Vertex_3d_arr_index = OFDO->parse_face_data[face_index][vertex_index][0];
            int normal_3d_arr_index = OFDO->parse_face_data[face_index][vertex_index][2];
            int Texture_Coordinates_index = OFDO->parse_face_data[face_index][vertex_index][1];

            Vertex_3d *vertex = &Model_faces[face_index].face[vertex_index];

            // Assign vertex positions
            vertex->x = OFDO->Vertex_3d_arr[Vertex_3d_arr_index * 3];
            vertex->y = OFDO->Vertex_3d_arr[Vertex_3d_arr_index * 3 + 1];
            vertex->z = OFDO->Vertex_3d_arr[Vertex_3d_arr_index * 3 + 2];

            printf("[DEBUG] Face %d, Vertex %d\n", face_index, vertex_index);
            printf("   vertex Before Assignment -> x: %f, y: %f, z: %f\n",
                OFDO->Vertex_3d_arr[Vertex_3d_arr_index * 3],
                OFDO->Vertex_3d_arr[Vertex_3d_arr_index * 3 + 1],
                OFDO->Vertex_3d_arr[Vertex_3d_arr_index * 3 + 2]);

            // Assign normal values
            vertex->nx = OFDO->Normal_3d_arr[normal_3d_arr_index * 3];
            vertex->ny = OFDO->Normal_3d_arr[normal_3d_arr_index * 3 + 1];
            vertex->nz = OFDO->Normal_3d_arr[normal_3d_arr_index * 3 + 2];


            printf("[DEBUG] Face %d, Vertex %d\n", face_index, vertex_index);
            printf("   Normal Before Assignment -> nx: %f, ny: %f, nz: %f\n",
                OFDO->Normal_3d_arr[normal_3d_arr_index * 3],
                OFDO->Normal_3d_arr[normal_3d_arr_index * 3 + 1],
                OFDO->Normal_3d_arr[normal_3d_arr_index * 3 + 2]);

            // Assign texture coordinates
            vertex->texture_Coordinates_x = OFDO->Texture_Coordinates[Texture_Coordinates_index * 2];
            vertex->texture_Coordinates_y = OFDO->Texture_Coordinates[Texture_Coordinates_index * 2 + 1];

            // Allocate and set position vector
            vertex->vertex_position_vector = gsl_vector_calloc(3);
            vertex->vertex_normal_vector = gsl_vector_calloc(3);
            vertex->vertex_texture_vector = gsl_vector_calloc(2);

            gsl_vector_set(vertex->vertex_position_vector, 0, vertex->x);
            gsl_vector_set(vertex->vertex_position_vector, 1, vertex->y);
            gsl_vector_set(vertex->vertex_position_vector, 2, vertex->z);

            gsl_vector_set(vertex->vertex_normal_vector, 0, vertex->nx);
            gsl_vector_set(vertex->vertex_normal_vector, 1, vertex->ny);
            gsl_vector_set(vertex->vertex_normal_vector, 2, vertex->nz);

            gsl_vector_set(vertex->vertex_texture_vector, 0, vertex->texture_Coordinates_x);
            gsl_vector_set(vertex->vertex_texture_vector, 1, vertex->texture_Coordinates_y);
        }
    }

    // Assign face data to the model
    final_model->object_mesh = Model_faces;
    final_model->vertex_count = OFDO->Vertex_count;
    final_model->face_count = OFDO->face_count;
    final_model->face_vertex_count = OFDO->face_vertex_count;
    final_model->scale_factor = 1.0f;

    // Allocate and initialize orientation matrix
    final_model->orientation = gsl_matrix_alloc(4, 4);
    gsl_matrix_set_zero(final_model->orientation);
    gsl_matrix_set(final_model->orientation, 0, 0, 1);
    gsl_matrix_set(final_model->orientation, 1, 1, 1);
    gsl_matrix_set(final_model->orientation, 2, 2, 1);
    gsl_matrix_set(final_model->orientation, 3, 3, 1); // Ensure homogeneous coordinate is correct

    // Allocate and initialize world space position vector
    final_model->world_space_postion_vector = gsl_vector_calloc(3);
    gsl_vector_set(final_model->world_space_postion_vector, 0, world_space_postion_x);
    gsl_vector_set(final_model->world_space_postion_vector, 1, world_space_postion_y);
    gsl_vector_set(final_model->world_space_postion_vector, 2, world_space_postion_z);

    // Also keep the original float array for compatibility
    final_model->world_space_postion[0] = gsl_vector_get(final_model->world_space_postion_vector, 0);
    final_model->world_space_postion[1] = gsl_vector_get(final_model->world_space_postion_vector, 1);
    final_model->world_space_postion[2] = gsl_vector_get(final_model->world_space_postion_vector, 2);
    

    return final_model;
}

void convert_model_n_hedron_to_triangles(struct object_model *obj_model)
{
    // First, count total triangles required
    int total_triangles = 0;
    for (int i = 0; i < obj_model->face_count; i++) 
    {
        total_triangles += obj_model->face_vertex_count[i] - 2;
    }

    // Allocate new array for triangulated faces
    struct model_faces *new_faces = malloc(total_triangles * sizeof(struct model_faces));

    int new_face_index = 0; // Track new faces count

    for (int face_index = 0; face_index < obj_model->face_count; face_index++)
    {
        int num_triangles = obj_model->face_vertex_count[face_index] - 2;
        
        if (num_triangles <= 0) continue; // Skip triangles

        struct Vertex_3d *hedron_vertex_3d = obj_model->object_mesh[face_index].face;

        for (int i = 0; i < num_triangles; i++) 
        {
            // Allocate and assign a new triangle
            new_faces[new_face_index].face = malloc(3 * sizeof(Vertex_3d));

            // Use the first vertex as a base, iterate through the rest
            new_faces[new_face_index].face[0] = hedron_vertex_3d[0];
            new_faces[new_face_index].face[1] = hedron_vertex_3d[i + 1];
            new_faces[new_face_index].face[2] = hedron_vertex_3d[i + 2];

            printf("Triangle %d (from Face %d):\n", new_face_index, face_index);
            printf("  Vertex 0 -> x: %f, y: %f, z: %f\n", new_faces[new_face_index].face[0].x, new_faces[new_face_index].face[0].y, new_faces[new_face_index].face[0].z);
            printf("  Vertex 1 -> x: %f, y: %f, z: %f\n", new_faces[new_face_index].face[1].x, new_faces[new_face_index].face[1].y, new_faces[new_face_index].face[1].z);
            printf("  Vertex 2 -> x: %f, y: %f, z: %f\n", new_faces[new_face_index].face[2].x, new_faces[new_face_index].face[2].y, new_faces[new_face_index].face[2].z);

            new_face_index++;
        }
    }

    // Free old face memory
    for (int i = 0; i < obj_model->face_count; i++) 
    {
        free(obj_model->object_mesh[i].face);
    }

    // Replace object faces with new triangulated faces
    free(obj_model->object_mesh); // Free old array
    obj_model->object_mesh = new_faces; // Assign new triangulated faces

    // Update the face count
    obj_model->face_count = total_triangles;

    // Free the old face_vertex_count array
    free(obj_model->face_vertex_count);

    // Allocate new face_vertex_count array
    obj_model->face_vertex_count = malloc(obj_model->face_count * sizeof(int));

    // Set all values to 3 (since each face is a triangle)
    for (int i = 0; i < obj_model->face_count; i++) 
    {
        obj_model->face_vertex_count[i] = 3;
    }

    
}

