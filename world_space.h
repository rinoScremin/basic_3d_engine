#ifndef WORLD_SPACE_H
#define WORLD_SPACE_H

#include <stdio.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_block.h>
#include "D3_object_model.h"
#include <gsl/gsl_blas.h>  // ✅ Required for gsl_blas_dgemv
#include <math.h>


typedef struct world_space
{
    struct object_model **world_objects; // Array of object pointers
    int object_count; // Number of objects in the world
} world_space;

void transform_object_to_world(struct object_model *obj, struct world_space *worldSpace);



#endif // 3D_OBJECT_MODEL_H
