#ifndef OBJECT_FILE_HANDLER_H
#define OBJECT_FILE_HANDLER_H

#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

// ✅ Define the structure properly before using it
typedef struct Object_file_data
{
    char *buf;
    int file_size;
    char *obj_file_path;
} Object_file_data;

typedef struct Obj_file_data_object
{
    float *Vertex_3d_arr;
    float *Normal_3d_arr;
    float *Texture_Coordinates;
    char **face_data;  // ✅ Change from char *face_data to char **face_data
    int ***parse_face_data;
    int Vertex_count;
    int Normal_count;
    int Texture_Coordinates_count;
    int face_count;
    int *face_vertex_count; 
} Obj_file_data_object;

// Function prototype
struct Object_file_data *read_object_file(char *file_path);

struct Obj_file_data_object *get_object_vertex_3d(struct Object_file_data *raw_object_file);

int *vertex_normal_texture_face_count(struct Object_file_data *raw_object_file);

int *count_obj_faces_vertexs(struct Object_file_data *raw_object_file, int numberOfFaces);

int ***parse_face_data(char **face_data, int face_count);

#endif // OBJECT_FILE_HANDLER_H
