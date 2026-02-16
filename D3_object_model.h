#ifndef D3_OBJECT_MODEL_H
#define D3_OBJECT_MODEL_H

#include <stdio.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_block.h>
#include "object_file_handler.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Vertex_3d
{
    float x, y, z;      // Position coordinates
    float nx, ny, nz;   // Normal vector components
    float texture_Coordinates_x, texture_Coordinates_y;
    gsl_vector *vertex_position_vector;
    gsl_vector *vertex_normal_vector;
    gsl_vector *vertex_texture_vector;
} Vertex_3d;

typedef struct model_faces
{
    Vertex_3d *face;
    bool vertex_order;
    bool cull;
    float face_nx, face_ny, face_nz;
    gsl_vector *face_normal_vector;
} model_faces;

typedef struct object_model
{
    float world_space_postion[3];
    gsl_vector *world_space_postion_vector;
    struct model_faces *object_mesh;
    float scale_factor;             
    gsl_matrix *orientation;
    int vertex_count;
    int face_count;
    int *face_vertex_count; 
} object_model;

struct object_model *build_model(struct Obj_file_data_object *OFDO, float world_space_postion_x, float world_space_postion_y, float world_space_postion_z);

void convert_model_n_hedron_to_triangles(struct object_model *obj_model);

#endif // 3D_OBJECT_MODEL_H
