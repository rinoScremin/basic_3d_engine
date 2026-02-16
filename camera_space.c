#include "world_space.h"
#include <stdio.h>
#include "camera_space.h"
#include "D3_object_model.h"
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_block.h>

#include <math.h>

void init_camera(struct camera_space_objects *camera, float x, float y, float z, float FOV, float near_plane, float far_plane, float aspect_ratio, float rotation_x, float rotation_y)
{
    camera->camera_world_space_postion[0] = x;
    camera->camera_world_space_postion[1] = y;
    camera->camera_world_space_postion[2] = z;

    camera->FOV = FOV;
    camera->near_plane = near_plane;
    camera->far_plane = far_plane;
    camera->aspect_ratio = aspect_ratio;
    camera->camera_rotation_x = rotation_x;  // Store pitch
    camera->camera_rotation_y = rotation_y;  // Store yaw

    // Allocate the camera orientation matrix
    camera->orientation = gsl_matrix_alloc(4, 4);
    gsl_matrix_set_identity(camera->orientation); // Start with identity

    // Convert angles to radians
    float angle_x_radians = rotation_x * (M_PI / 180.0f);
    float angle_y_radians = rotation_y * (M_PI / 180.0f);
    
    // Precompute trigonometric values
    float cos_x = cos(angle_x_radians);
    float sin_x = sin(angle_x_radians);
    float cos_y = cos(angle_y_radians);
    float sin_y = sin(angle_y_radians);

    // **X-axis Rotation Matrix (Pitch)**
    gsl_matrix *rotation_x_matrix = gsl_matrix_alloc(3, 3);
    gsl_matrix_set(rotation_x_matrix, 0, 0, 1);
    gsl_matrix_set(rotation_x_matrix, 0, 1, 0);
    gsl_matrix_set(rotation_x_matrix, 0, 2, 0);
    gsl_matrix_set(rotation_x_matrix, 1, 0, 0);
    gsl_matrix_set(rotation_x_matrix, 1, 1, cos_x);
    gsl_matrix_set(rotation_x_matrix, 1, 2, -sin_x);
    gsl_matrix_set(rotation_x_matrix, 2, 0, 0);
    gsl_matrix_set(rotation_x_matrix, 2, 1, sin_x);
    gsl_matrix_set(rotation_x_matrix, 2, 2, cos_x);

    // **Y-axis Rotation Matrix (Yaw)**
    gsl_matrix *rotation_y_matrix = gsl_matrix_alloc(3, 3);
    gsl_matrix_set(rotation_y_matrix, 0, 0, cos_y);
    gsl_matrix_set(rotation_y_matrix, 0, 1, 0);
    gsl_matrix_set(rotation_y_matrix, 0, 2, sin_y);
    gsl_matrix_set(rotation_y_matrix, 1, 0, 0);
    gsl_matrix_set(rotation_y_matrix, 1, 1, 1);
    gsl_matrix_set(rotation_y_matrix, 1, 2, 0);
    gsl_matrix_set(rotation_y_matrix, 2, 0, -sin_y);
    gsl_matrix_set(rotation_y_matrix, 2, 1, 0);
    gsl_matrix_set(rotation_y_matrix, 2, 2, cos_y);

    // Apply Rotation X first, then Rotation Y: **Final = Y * X**
    gsl_matrix *temp_matrix = gsl_matrix_alloc(3, 3);
    gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, rotation_y_matrix, rotation_x_matrix, 0.0, temp_matrix);
    gsl_matrix_view camera_submatrix = gsl_matrix_submatrix(camera->orientation, 0, 0, 3, 3);
    gsl_matrix_memcpy(&camera_submatrix.matrix, temp_matrix);

    // Free temp matrices
    gsl_matrix_free(rotation_x_matrix);
    gsl_matrix_free(rotation_y_matrix);
    gsl_matrix_free(temp_matrix);

    // Debug: Print Updated Camera Orientation Matrix
    printf("\n[DEBUG] Updated Camera Orientation Matrix:\n");
    for (int i = 0; i < 3; i++) {
        printf("| %f %f %f |\n",
            gsl_matrix_get(camera->orientation, i, 0),
            gsl_matrix_get(camera->orientation, i, 1),
            gsl_matrix_get(camera->orientation, i, 2));
    }
    printf("\n");
}

struct camera_space_objects *transform_world_space_objects_to_camera_space_objects(
    struct camera_space_objects *camera, struct world_space *world)
{
    printf("[INFO] Creating Camera Space Copy\n");

    // Step 1: Allocate memory for the camera space objects
    struct camera_space_objects *camera_space = malloc(sizeof(struct camera_space_objects));

    // Copy camera position and orientation
    memcpy(camera_space->camera_world_space_postion, camera->camera_world_space_postion, 3 * sizeof(float));
  
    camera_space->FOV = camera->FOV;
    camera_space->near_plane = camera->near_plane;
    camera_space->far_plane = camera->far_plane;
    camera_space->focal_length = camera->focal_length;
    camera_space->lambda = camera->lambda;
    camera_space->aspect_ratio = camera->aspect_ratio;

    camera_space->orientation = gsl_matrix_alloc(4, 4);
    gsl_matrix_memcpy(camera_space->orientation, camera->orientation);

    // Step 2: Allocate memory for the copied world space
    camera_space->Camera_space_objects = malloc(sizeof(struct world_space));
    camera_space->Camera_space_objects->object_count = world->object_count;
    camera_space->Camera_space_objects->world_objects = malloc(world->object_count * sizeof(struct object_model *));

    // Step 3: Deep copy each object in the world
    for (int world_space_obj_index = 0; world_space_obj_index < world->object_count; world_space_obj_index++)
    {
        struct object_model *original_obj = world->world_objects[world_space_obj_index];

        struct object_model *copied_obj = malloc(sizeof(struct object_model));
        memcpy(copied_obj, original_obj, sizeof(struct object_model));

        // Allocate and copy object mesh
        copied_obj->object_mesh = malloc(copied_obj->face_count * sizeof(struct model_faces));

        for (int face_index = 0; face_index < copied_obj->face_count; face_index++)
        {
            copied_obj->object_mesh[face_index].face = malloc(copied_obj->face_vertex_count[face_index] * sizeof(Vertex_3d));

            memcpy(copied_obj->object_mesh[face_index].face, 
                   original_obj->object_mesh[face_index].face, 
                   copied_obj->face_vertex_count[face_index] * sizeof(Vertex_3d));
        }
        camera_space->Camera_space_objects->world_objects[world_space_obj_index] = copied_obj;
    }
    printf("[INFO] Transforming Objects to Camera Space\n");
    // Step 4: Apply the camera transformation on the copied objects
    for (int world_space_obj_index = 0; world_space_obj_index < camera_space->Camera_space_objects->object_count; world_space_obj_index++)
    {
        struct object_model *obj = camera_space->Camera_space_objects->world_objects[world_space_obj_index];

        printf("\n[INFO] Transforming Object %d: %d Faces\n", world_space_obj_index, obj->face_count);

        for (int face_index = 0; face_index < obj->face_count; face_index++)
        {
            struct model_faces *face = &obj->object_mesh[face_index];

            printf("********************************************************************************\n");
            printf("[INFO] Face %d has %d vertices\n", face_index, obj->face_vertex_count[face_index]);

            for (int vertex_index = 0; vertex_index < obj->face_vertex_count[face_index]; vertex_index++)
            {
                Vertex_3d *vertex = &face->face[vertex_index];

                // Step 1: Translate by Camera Position
                float translated_x = vertex->x - camera->camera_world_space_postion[0];
                float translated_y = vertex->y - camera->camera_world_space_postion[1];
                float translated_z = vertex->z - camera->camera_world_space_postion[2];

                // Step 2: Rotate Using Camera Orientation
                gsl_vector *vertex_vec = gsl_vector_alloc(3);
                gsl_vector_set(vertex_vec, 0, translated_x);
                gsl_vector_set(vertex_vec, 1, translated_y);
                gsl_vector_set(vertex_vec, 2, translated_z);

                gsl_vector *transformed_vertex = gsl_vector_alloc(3);
                gsl_matrix_view rotation_matrix = gsl_matrix_submatrix(camera->orientation, 0, 0, 3, 3);

                gsl_blas_dgemv(CblasNoTrans, 1.0, &rotation_matrix.matrix, vertex_vec, 0.0, transformed_vertex);

                // Step 3: Store Transformed Coordinates
                vertex->x = gsl_vector_get(transformed_vertex, 0);
                vertex->y = gsl_vector_get(transformed_vertex, 1);
                vertex->z = gsl_vector_get(transformed_vertex, 2);

                //printf("N_x: %f, N_y: %f, N_z: %f\n", vertex->nx, vertex->ny, vertex->nz);

                face->face_nx = vertex->nx;
                face->face_ny = vertex->ny;
                face->face_nz = vertex->nz;

                // Free memory
                gsl_vector_free(vertex_vec);
                gsl_vector_free(transformed_vertex);
            }

            printf("[BEFORE] N_x: %f, N_y: %f, N_z: %f\n", face->face_nx, face->face_ny, face->face_nz);

            // Transform the normal using camera orientation
            float transformed_normal[3];
            transform_face_normal_camera_space(face->face_nx, face->face_ny, face->face_nz, 
                                            camera->orientation, transformed_normal);

            // Store the transformed normal
            face->face_nx = transformed_normal[0];
            face->face_ny = transformed_normal[1];
            face->face_nz = transformed_normal[2];

            printf("[AFTER] N_x: %f, N_y: %f, N_z: %f\n", face->face_nx, face->face_ny, face->face_nz);

            face->vertex_order = true; // **CCW**
            face->cull = false;
            //printf("\nFace Vertex Order : %d\n", face->vertex_order);
            //calculate_culling(face, camera);
            //printf("Face culling : %d\n", face->cull);
            printf("********************************************************************************");
        }
    }

    return camera_space;
}


void transform_face_normal_camera_space(float nx, float ny, float nz, 
                                        gsl_matrix *camera_orientation, 
                                        float transformed_normal[3]) 
{
    gsl_vector *normal_vector = gsl_vector_alloc(3);
    gsl_vector_set(normal_vector, 0, nx);
    gsl_vector_set(normal_vector, 1, ny);
    gsl_vector_set(normal_vector, 2, nz);

    gsl_vector *transformed_vector = gsl_vector_alloc(3);
    gsl_matrix_view rotation_matrix = gsl_matrix_submatrix(camera_orientation, 0, 0, 3, 3);

    // Apply the rotation transformation using the camera's orientation matrix
    gsl_blas_dgemv(CblasNoTrans, 1.0, &rotation_matrix.matrix, normal_vector, 0.0, transformed_vector);

    // Normalize the transformed normal
    float length = gsl_blas_dnrm2(transformed_vector);
    if (length > 0.00001f) {
        transformed_normal[0] = gsl_vector_get(transformed_vector, 0) / length;
        transformed_normal[1] = gsl_vector_get(transformed_vector, 1) / length;
        transformed_normal[2] = gsl_vector_get(transformed_vector, 2) / length;
    } else {
        transformed_normal[0] = 0.0f;
        transformed_normal[1] = 0.0f;
        transformed_normal[2] = 0.0f;
    }

    gsl_vector_free(normal_vector);
    gsl_vector_free(transformed_vector);
}


void calculate_culling(struct model_faces *face, struct camera_space_objects *camera)
{
    printf("\n[ENTRY] - Performing Backface Culling Calculation\n");

    // ✅ Get the camera's forward direction (negative Z-axis from the orientation matrix)
    float cam_dir_x = -gsl_matrix_get(camera->orientation, 0, 2);
    float cam_dir_y = -gsl_matrix_get(camera->orientation, 1, 2);
    float cam_dir_z = -gsl_matrix_get(camera->orientation, 2, 2);

    // ✅ Ensure the face normal is **normalized**
    float normal_length = sqrt(face->face_nx * face->face_nx +
                               face->face_ny * face->face_ny +
                               face->face_nz * face->face_nz);

    if (normal_length > 0.00001f) { 
        face->face_nx /= normal_length;
        face->face_ny /= normal_length;
        face->face_nz /= normal_length;
    }

    // ✅ Compute the dot product (determines whether the face is facing the camera)
    float dot_product = (-face->face_nx * cam_dir_x) + 
    (-face->face_ny * cam_dir_y) + 
    (-face->face_nz * cam_dir_z);

    printf("[DEBUG] Dot Product: %f\n", dot_product);

    // ✅ CULL ONLY IF DOT PRODUCT IS GREATER THAN 0
    if (dot_product > 0.0001f) {  
        face->cull = true;  // ✅ CULLED
        printf("[DEBUG] Face CULLED         FUCKKKKKKKK!\n");
    } else {
        face->cull = false; // ✅ VISIBLE
        printf("[DEBUG] Face VISIBLE!\n");
    }
    
    printf("--------------------------------------------------------------------------\n");
}


