#ifndef CAMERA_SPACE_H
#define CAMERA_SPACE_H

#include "world_space.h"
#include <stdio.h>
#include "camera_space.h"
#include "D3_object_model.h"
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_block.h>

typedef struct camera_space_objects
{
    float camera_world_space_postion[3]; 
    gsl_matrix *orientation;
    struct world_space *Camera_space_objects;
    float FOV;
    float near_plane;
    float far_plane;
    float focal_length;
    float lambda;
    float aspect_ratio;

    float camera_rotation_x; // Up/down (pitch)
    float camera_rotation_y; // Left/right (yaw)

} camera_space_objects;


void init_camera(struct camera_space_objects *camera, float x, float y, float z, float FOV, float near_plane, 
    float far_plane, float aspect_ratio, float rotation_x, float rotation_y);
    
struct camera_space_objects *transform_world_space_objects_to_camera_space_objects(struct camera_space_objects *camera, struct world_space *world);
void calculate_culling(struct model_faces *face, struct camera_space_objects *camera);

void transform_face_normal_camera_space(float nx, float ny, float nz, 
    gsl_matrix *camera_orientation, 
    float transformed_normal[3]);





#endif 

