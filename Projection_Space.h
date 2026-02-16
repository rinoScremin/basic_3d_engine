#ifndef PROJECTION_SPACE_H
#define PROJECTION_SPACE_H

#include <stdio.h>
#include "world_space.h"
#include "camera_space.h"
#include "D3_object_model.h"
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_block.h>
#include "renderer.h"
#include <math.h>
#include <stdbool.h>

struct camera_space_objects *transform_camera_space_objects_to_projection_space_objects(struct camera_space_objects *camera, Renderer *render);
void calculate_culling_by_area(struct model_faces *face, int face_vertex_count);
bool does_vertex_need_clipping(float x, float y, float z);
Vertex_3d clip_vertex_to_frustum(Vertex_3d v1, Vertex_3d v2);

Vertex_3d calculate_clipped_vertex(Vertex_3d v1, Vertex_3d v2, float near_plane);

#endif // 3D_OBJECT_MODEL_H