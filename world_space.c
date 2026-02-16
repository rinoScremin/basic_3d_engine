#include <stdio.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_block.h>
#include "D3_object_model.h"
#include "world_space.h"
#include <gsl/gsl_blas.h>  // Required for gsl_blas_dgemv
#include <math.h>

void transform_object_to_world(struct object_model *obj, struct world_space *worldSpace)
{
    printf("**********************transform_object_to_world****************************************\n");
    printf("\t[INFO] Transforming object with %d faces\n\n", obj->face_count);

    for (int face_index = 0; face_index < obj->face_count; face_index++)
    {
        printf("[INFO] Processing Face %d\n", face_index);
        struct model_faces *face = &obj->object_mesh[face_index];
        if (!face || !face->face) continue;

        gsl_matrix_view rotation_matrix = gsl_matrix_submatrix(obj->orientation, 0, 0, 3, 3);

        for (int vertex_index = 0; vertex_index < obj->face_vertex_count[face_index]; vertex_index++)
        {
            Vertex_3d *vertex = &face->face[vertex_index];  // FIXED: Properly reference the vertex

            gsl_vector *rotated_position = gsl_vector_alloc(3);
            gsl_blas_dgemv(CblasNoTrans, 1.0, &rotation_matrix.matrix, vertex->vertex_position_vector, 0.0, rotated_position);
                        
            gsl_vector_add(rotated_position, obj->world_space_postion_vector);
            
            vertex->x = gsl_vector_get(rotated_position, 0);
            vertex->y = gsl_vector_get(rotated_position, 1);
            vertex->z = gsl_vector_get(rotated_position, 2);
            
            gsl_vector *rotated_normal = gsl_vector_alloc(3);
            gsl_vector *temp_normal = gsl_vector_alloc(3);
            
            gsl_blas_dgemv(CblasNoTrans, 1.0, &rotation_matrix.matrix, vertex->vertex_normal_vector, 0.0, temp_normal);
            gsl_vector_memcpy(rotated_normal, temp_normal);
            gsl_vector_free(temp_normal);
            
            double normal_length = gsl_blas_dnrm2(rotated_normal);
            if (normal_length > 0.0)
            {
                gsl_vector_scale(rotated_normal, 1.0 / normal_length);
            }
            
            vertex->nx = gsl_vector_get(rotated_normal, 0);
            vertex->ny = gsl_vector_get(rotated_normal, 1);
            vertex->nz = gsl_vector_get(rotated_normal, 2);
            
            printf("[UPDATED] Face %d, Vertex %d\n", face_index, vertex_index);
            printf("   Position -> x: %f, y: %f, z: %f\n", vertex->x, vertex->y, vertex->z);
            printf("   Normal   -> nx: %f, ny: %f, nz: %f\n", vertex->nx, vertex->ny, vertex->nz);
            printf("   Texture  -> u: %f, v: %f\n\n", vertex->texture_Coordinates_x, vertex->texture_Coordinates_y);
            
            gsl_vector_free(rotated_position);
            gsl_vector_free(rotated_normal);
        }
    }

    if (worldSpace->object_count == 0)
        worldSpace->world_objects = malloc(sizeof(struct object_model *));
    else
        worldSpace->world_objects = realloc(worldSpace->world_objects, (worldSpace->object_count + 1) * sizeof(struct object_model *));

    if (!worldSpace->world_objects) {
        printf("[ERROR] Memory allocation failed!\n");
        exit(1);
    }

    worldSpace->world_objects[worldSpace->object_count] = obj;
    worldSpace->object_count++;
}

