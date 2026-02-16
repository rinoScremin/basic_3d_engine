#include <stdio.h>
#include "world_space.h"
#include "camera_space.h"
#include "Projection_Space.h"
#include "D3_object_model.h"
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_block.h>
#include "renderer.h"
#include <math.h>


struct camera_space_objects *transform_camera_space_objects_to_projection_space_objects(struct camera_space_objects *camera, Renderer *render)
{
    printf("[ENTRY-transform_camera_space_objects_to_projection_space_objects] \n");
    printf("Creating projection space objects Copy\n");

    struct camera_space_objects *projection_space_objects = malloc(sizeof(struct camera_space_objects));
    memcpy(projection_space_objects->camera_world_space_postion, camera->camera_world_space_postion, 3 * sizeof(float));

    projection_space_objects->orientation = gsl_matrix_alloc(4, 4);
    gsl_matrix_memcpy(projection_space_objects->orientation, camera->orientation);

    projection_space_objects->Camera_space_objects = malloc(sizeof(struct world_space));
    projection_space_objects->Camera_space_objects->object_count = camera->Camera_space_objects->object_count;
    projection_space_objects->Camera_space_objects->world_objects = malloc(camera->Camera_space_objects->object_count * sizeof(struct object_model *));

    for (int world_space_obj_index = 0; world_space_obj_index < camera->Camera_space_objects->object_count; world_space_obj_index++)
    {
        struct object_model *original_obj = camera->Camera_space_objects->world_objects[world_space_obj_index];
        struct object_model *copied_obj = malloc(sizeof(struct object_model));
        memcpy(copied_obj, original_obj, sizeof(struct object_model));

        copied_obj->object_mesh = malloc(copied_obj->face_count * sizeof(struct model_faces));
        for (int face_index = 0; face_index < copied_obj->face_count; face_index++)
        {
            copied_obj->object_mesh[face_index].cull = original_obj->object_mesh[face_index].cull; // Ensure cull is copied
            copied_obj->object_mesh[face_index].face = malloc(copied_obj->face_vertex_count[face_index] * sizeof(Vertex_3d));
            memcpy(copied_obj->object_mesh[face_index].face,
                   original_obj->object_mesh[face_index].face,
                   copied_obj->face_vertex_count[face_index] * sizeof(Vertex_3d));
        }
        projection_space_objects->Camera_space_objects->world_objects[world_space_obj_index] = copied_obj;
    }
    printf("[INFO] Transforming Objects to projection space objects\n");
    for (int world_space_obj_index = 0; world_space_obj_index < projection_space_objects->Camera_space_objects->object_count; world_space_obj_index++)
    {
        struct object_model *camera_space_obj = projection_space_objects->Camera_space_objects->world_objects[world_space_obj_index];
        printf("\n[INFO] Transforming Object %d: %d Faces\n", world_space_obj_index, camera_space_obj->face_count);

        for (int face_index = 0; face_index < camera_space_obj->face_count; face_index++)
        {
            struct model_faces *face = &camera_space_obj->object_mesh[face_index];
            printf("********************************************************************************\n");
            printf("[INFO] Face %d has %d vertices\n", face_index, camera_space_obj->face_vertex_count[face_index]);

            for (int vertex_index = 0; vertex_index < camera_space_obj->face_vertex_count[face_index]; vertex_index++)
            {
                Vertex_3d *camera_space_vertex = &face->face[vertex_index];
                printf("\n[DEBUG-BEFORE] *vertex Position BEFORE Projection* %d -> x: %f, y: %f, z: %f\n",
                       vertex_index, camera_space_vertex->x, camera_space_vertex->y, camera_space_vertex->z);

                gsl_vector *camera_space_vertex_vector_4d = gsl_vector_alloc(4);
                gsl_vector_set(camera_space_vertex_vector_4d, 0, camera_space_vertex->x);
                gsl_vector_set(camera_space_vertex_vector_4d, 1, camera_space_vertex->y);
                gsl_vector_set(camera_space_vertex_vector_4d, 2, camera_space_vertex->z);
                gsl_vector_set(camera_space_vertex_vector_4d, 3, 1.0f);

                float fov_radians = camera->FOV * (M_PI / 180.0f);
                float f = 1.0f / tan(fov_radians / 2.0f);
                float aspect_ratio = camera->aspect_ratio;
                float near = camera->near_plane;
                float far = camera->far_plane;
                
                float A = (far + near) / (near - far);
                float B = (2 * far * near) / (near - far);

               // printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
               // printf("[DEBUG] f: %f, aspect_ratio: %f, near: %f, far: %f, A: %f, B: %f\n", 
               //        f, aspect_ratio, near, far, A, B);
               // printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

                gsl_matrix *projection_matrix = gsl_matrix_alloc(4, 4);
                gsl_matrix_set_zero(projection_matrix);

                // Perspective projection matrix
                gsl_matrix_set(projection_matrix, 0, 0, f / aspect_ratio);
                gsl_matrix_set(projection_matrix, 1, 1, f);
                gsl_matrix_set(projection_matrix, 2, 2, (far + near) / (near - far));
                gsl_matrix_set(projection_matrix, 2, 3, (2 * far * near) / (near - far));
                gsl_matrix_set(projection_matrix, 3, 2, -1.0f);  // Fix the last row


                gsl_vector *projected_vertex_vector = gsl_vector_alloc(4);
                gsl_blas_dgemv(CblasNoTrans, 1.0, projection_matrix, camera_space_vertex_vector_4d, 0.0, projected_vertex_vector);

                float w = gsl_vector_get(projected_vertex_vector, 3);
                //printf("[DEBUG] Perspective Divide: w = %f\n", w);

                float debug_x = gsl_vector_get(projected_vertex_vector, 0);
                float debug_y = gsl_vector_get(projected_vertex_vector, 1);
                float debug_z = gsl_vector_get(projected_vertex_vector, 2);

                //printf("[DEBUG] Projected X: %f, Projected Y: %f, Projected Z: %f\n", debug_x, debug_y, debug_z);



                if (w != 0.0f) {
                    debug_x /= w;
                    debug_y /= w;
                    debug_z /= w;
                } else {
                    printf("[WARNING] w is zero! Skipping division to prevent NaN values.\n");
                }


                if (does_vertex_need_clipping(debug_x, debug_y, debug_z))
                {
                    
                    
                    // Create a new vertex at the frustum edge
                    Vertex_3d new_vertex = calculate_clipped_vertex(face->face[vertex_index], 
                        face->face[(vertex_index + 1) % camera_space_obj->face_vertex_count[face_index]], near);
                    
                    // Assign new vertex to replace the out-of-bounds one
                    camera_space_vertex->x = new_vertex.x;
                    camera_space_vertex->y = new_vertex.y;
                    camera_space_vertex->z = new_vertex.z;

                    printf("Vertex needs clipping x: %f, y: %f, x:%f\n" , camera_space_vertex->x,camera_space_vertex->y,camera_space_vertex->z);
                }
                
                
                //printf("[DEBUG] Final Screen Space X: %f, Y: %f, Z: %f\n", debug_x, debug_y, debug_z);

                camera_space_vertex->x = debug_x;
                camera_space_vertex->y = debug_y;
                camera_space_vertex->z = debug_z;

                gsl_vector_free(camera_space_vertex_vector_4d);
                gsl_vector_free(projected_vertex_vector);
                gsl_matrix_free(projection_matrix);

            }
            printf("********************************************************************************\n\n");
            calculate_culling_by_area(face,camera_space_obj->face_vertex_count[face_index]);
        }
        
    }
    return projection_space_objects;
}


void calculate_culling_by_area(struct model_faces *face, int face_vertex_count)
{
    printf("****************************[calculate_culling_by_area ENTRY]***************************\n");

    float area = 0.0f;
    
    for (int vertex_index = 0; vertex_index < face_vertex_count; vertex_index++)
    {
        Vertex_3d *v1 = &face->face[vertex_index];
        Vertex_3d *v2 = &face->face[(vertex_index + 1) % face_vertex_count];

        area += (v1->x * v2->y) - (v1->y * v2->x);
    }

    area = area / 2.0f;

    // Determine culling based on the sign of the area
    face->cull = (area < 0); // Cull only if area is negative
}


bool does_vertex_need_clipping(float x, float y, float z) {
    // Define clipping boundaries
    const float min_x = -1.0f, max_x = 1.0f;
    const float min_y = -1.0f, max_y = 1.0f;
    const float min_z = 0.0f, max_z = 1.0f; // Perspective z-clipping range

    // Check if the vertex is outside the frustum
    if (x < min_x || x > max_x || 
        y < min_y || y > max_y || 
        z < min_z || z > max_z) {
        return true; // Vertex is outside the frustum
    }
    return false; // Vertex is inside the frustum
}


Vertex_3d clip_vertex_to_frustum(Vertex_3d v1, Vertex_3d v2) 
{
    Vertex_3d clipped_vertex;
    float t = 1.0f;  // Default interpolation factor

    // Define the clipping planes (assuming frustum x, y in [-1,1] and z in [0,1])
    const float min_x = -1.0f, max_x = 1.0f;
    const float min_y = -1.0f, max_y = 1.0f;
    const float min_z = 0.0f, max_z = 1.0f;

    if (v1.x < min_x) {
        t = (min_x - v1.x) / (v2.x - v1.x);
    } else if (v1.x > max_x) {
        t = (max_x - v1.x) / (v2.x - v1.x);
    } else if (v1.y < min_y) {
        t = (min_y - v1.y) / (v2.y - v1.y);
    } else if (v1.y > max_y) {
        t = (max_y - v1.y) / (v2.y - v1.y);
    } else if (v1.z < min_z) {
        t = (min_z - v1.z) / (v2.z - v1.z);
    } else if (v1.z > max_z) {
        t = (max_z - v1.z) / (v2.z - v1.z);
    }

    // Interpolate between the two vertices
    clipped_vertex.x = v1.x + t * (v2.x - v1.x);
    clipped_vertex.y = v1.y + t * (v2.y - v1.y);
    clipped_vertex.z = v1.z + t * (v2.z - v1.z);

    return clipped_vertex;
}


Vertex_3d calculate_clipped_vertex(Vertex_3d v1, Vertex_3d v2, float near_plane) 
{
    Vertex_3d new_vertex;
    float denom = (v2.z - v1.z);

    // Prevent division by zero
    if (fabs(denom) < 1e-6f) {  
        printf("[ERROR] Division by zero in clipping calculation! Keeping original vertex.\n");
        return v1; // Avoid NaN issues
    }

    // Compute interpolation factor (t)
    float t = (near_plane - v1.z) / denom;

    // Ensure t is within valid range
    if (t < 0.0f || t > 1.0f) {
        printf("[WARNING] Clipping factor t=%f is out of range. Using fallback.\n", t);
        return v1;  // Return original vertex instead of interpolating
    }

    // Compute new clipped vertex position
    new_vertex.x = v1.x + t * (v2.x - v1.x);
    new_vertex.y = v1.y + t * (v2.y - v1.y);
    new_vertex.z = near_plane; // Set z to near plane for correct clipping

    return new_vertex;
}
