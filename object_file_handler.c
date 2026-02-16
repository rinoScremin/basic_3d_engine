#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "object_file_handler.h" 
#include <string.h>

struct Object_file_data *read_object_file(char *file_path)
{
    struct Object_file_data *OFD = malloc(sizeof(Object_file_data));

    OFD->obj_file_path = file_path;

    struct stat st;
    if (stat(file_path, &st) == -1)
    {
        perror("Error getting file size");
        return NULL; // Return NULL on failure
    }
    int file_size = st.st_size;
    OFD->file_size = file_size;
    printf("File size: %d bytes\n", file_size);
    // Open the file
    int fd = open(file_path, O_RDONLY);
    if (fd == -1) 
    {
        perror("Error opening file");
        return NULL; // Return NULL on failure
    }
    // Allocate memory to hold the file content
    char *buf = malloc(file_size + 1);
    if (!buf) 
    {
        perror("Error allocating memory");
        close(fd);
        return NULL; // Return NULL on failure
    }
    // Read the file content
    if (read(fd, buf, file_size) != file_size) 
    {
        perror("Error reading file");
        free(buf);
        close(fd);
        return NULL; // Return NULL on failure
    }
    buf[file_size] = '\0'; // Null-terminate the string

    close(fd);

    OFD->buf = buf;
    return OFD; // Return the buffer containing the file content
}
struct Obj_file_data_object *get_object_vertex_3d(struct Object_file_data *raw_object_file)
{
    int *Vertex_normal_texture_face_count = vertex_normal_texture_face_count(raw_object_file);
    printf("vertex_count: %d \t normal_count: %d \t Texture_Coordinates_Count: %d \t face_count: %d\n", 
        Vertex_normal_texture_face_count[0], 
        Vertex_normal_texture_face_count[1], 
        Vertex_normal_texture_face_count[2], 
        Vertex_normal_texture_face_count[3]);
    // ✅ Allocate memory for the object data
    Obj_file_data_object *obj_file_data_object = malloc(sizeof(Obj_file_data_object));
    obj_file_data_object->Vertex_count = Vertex_normal_texture_face_count[0];
    obj_file_data_object->Normal_count = Vertex_normal_texture_face_count[1];
    obj_file_data_object->Texture_Coordinates_count = Vertex_normal_texture_face_count[2];
    obj_file_data_object->face_count = Vertex_normal_texture_face_count[3];

    obj_file_data_object->Vertex_3d_arr = malloc(obj_file_data_object->Vertex_count * 3 * sizeof(float));  
    obj_file_data_object->Normal_3d_arr = malloc(obj_file_data_object->Normal_count * 3 * sizeof(float));  
    obj_file_data_object->Texture_Coordinates = malloc(obj_file_data_object->Texture_Coordinates_count * 2 * sizeof(float));  
    // ✅ Allocate space for face data (array of char pointers)
    obj_file_data_object->face_data = malloc(obj_file_data_object->face_count * sizeof(char *));
    for (int i = 0; i < obj_file_data_object->face_count; i++)
    {
        obj_file_data_object->face_data[i] = malloc(128 * sizeof(char));  // ✅ Each face line can hold 128 characters
    }
    char *obj_file_lines = strtok(raw_object_file->buf, " \n");  
    int vertex_index = 0, normal_index = 0, texture_index = 0, face_index = 0;

    while (obj_file_lines != NULL) 
    {        
        if (!strcmp(obj_file_lines, "v"))
        {
            float x = strtof(strtok(NULL, " \n"), NULL);
            float y = strtof(strtok(NULL, " \n"), NULL);
            float z = strtof(strtok(NULL, " \n"), NULL);
            //printf("Vertex[%d]: x=%f  y=%f  z=%f\n", vertex_index, x, y, z);
            obj_file_data_object->Vertex_3d_arr[vertex_index * 3] = x;
            obj_file_data_object->Vertex_3d_arr[vertex_index * 3 + 1] = y;
            obj_file_data_object->Vertex_3d_arr[vertex_index * 3 + 2] = z;
            vertex_index++;  
        }
        else if (!strcmp(obj_file_lines, "vn"))
        {
            float nx = strtof(strtok(NULL, " \n"), NULL);
            float ny = strtof(strtok(NULL, " \n"), NULL);
            float nz = strtof(strtok(NULL, " \n"), NULL);
            printf("Normal[%d]: nx=%f  ny=%f  nz=%f\n", normal_index, nx, ny, nz);
            obj_file_data_object->Normal_3d_arr[normal_index * 3] = nx;
            obj_file_data_object->Normal_3d_arr[normal_index * 3 + 1] = ny;
            obj_file_data_object->Normal_3d_arr[normal_index * 3 + 2] = nz;
            normal_index++;  
        }
        else if (!strcmp(obj_file_lines, "vt"))
        {
            float u = strtof(strtok(NULL, " \n"), NULL);
            float v = strtof(strtok(NULL, " \n"), NULL);
            //printf("Texture[%d]: u=%f  v=%f\n", texture_index, u, v);
            obj_file_data_object->Texture_Coordinates[texture_index * 2] = u;
            obj_file_data_object->Texture_Coordinates[texture_index * 2 + 1] = v;
            texture_index++;  
        }
        else if (!strcmp(obj_file_lines, "f"))
        {
            //printf("Processing Face[%d]\n", face_index);
            char *face_data_start = strtok(NULL, "\n");  
            if (face_data_start && face_index < obj_file_data_object->face_count)
            {
                strncpy(obj_file_data_object->face_data[face_index], face_data_start, 127);
                obj_file_data_object->face_data[face_index][127] = '\0';  // ✅ Ensure null termination
                
                //printf("Stored face line [%d]: %s\n", face_index, obj_file_data_object->face_data[face_index]);
                face_index++;  
            }
        }
        obj_file_lines = strtok(NULL, " \n");  
    }

    obj_file_data_object->parse_face_data = parse_face_data(obj_file_data_object->face_data, obj_file_data_object->face_count);

    char *obj_file_path = raw_object_file->obj_file_path;
    free(raw_object_file);
    raw_object_file = read_object_file(obj_file_path);

    obj_file_data_object->face_vertex_count = count_obj_faces_vertexs(raw_object_file, obj_file_data_object->face_count);

    return obj_file_data_object;
}
int *vertex_normal_texture_face_count(struct Object_file_data *raw_object_file)
{
    //printf("the raw_object_file->buf starting pointer = %p\n",raw_object_file->buf);
    int vertex_count = 0;
    int normal_count = 0;
    int texture_count = 0;
    int face_count = 0;
    //int face_vertex_count = 0; 
    for (int char_index = 0; char_index <= raw_object_file->file_size - 1; char_index++)
    {
        if (raw_object_file->buf[char_index] == 'v' && raw_object_file->buf[char_index + 1] == ' ') 
            vertex_count++;
        if (raw_object_file->buf[char_index] == 'v' && raw_object_file->buf[char_index + 1] == 'n') 
            normal_count++;
        if (raw_object_file->buf[char_index] == 'v' && raw_object_file->buf[char_index + 1] == 't') 
            texture_count++;
        if (raw_object_file->buf[char_index] == 'f' && raw_object_file->buf[char_index + 1] == ' ') 
            face_count++;    
    }
    static int count[4];
    count[0] = vertex_count;
    count[1] = normal_count;
    count[2] = texture_count;
    count[3] = face_count;
    return count;
    
}
int *count_obj_faces_vertexs(struct Object_file_data *raw_object_file, int numberOfFaces)
{
    printf("count_obj_faces_vertexs [ENTRY]\n");
    int *faces_vertices = calloc(numberOfFaces, sizeof(int));  // Allocate and initialize to 0
    if (!faces_vertices) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    int face_count = 0;
    for (int char_index = 0; char_index < raw_object_file->file_size - 1; char_index++)
    {
        if (raw_object_file->buf[char_index] == 'f' && raw_object_file->buf[char_index + 1] == ' ') 
        { 
            //printf("✅ Found face at index %d\n", char_index);
            // Find the end of the line
            int start_index = char_index + 2;  // Skip "f "
            int end_index = start_index;
            while (raw_object_file->buf[end_index] != '\n' && raw_object_file->buf[end_index] != '\0') {
                end_index++;
            }
            // Extract the face line
            int len = end_index - start_index;
            char *faceLine = malloc(len + 1);
            strncpy(faceLine, &raw_object_file->buf[start_index], len);
            faceLine[len] = '\0';
            //printf("Face Line: %s\n", faceLine);
            // Count number of tokens
            int tokenCount = 0;
            char *saveptr = NULL;
            char *token = strtok_r(faceLine, " ", &saveptr);
            while (token != NULL)
            {
                tokenCount++;
                token = strtok_r(NULL, " ", &saveptr);
            }
            //printf("Face %d has %d vertices\n", face_count, tokenCount);
            faces_vertices[face_count] = tokenCount;
            free(faceLine);
            face_count++;
        }
    }
    //printf("Total faces counted: %d\n", face_count);
    return faces_vertices;  // ✅ Return pointer to array
}
int ***parse_face_data(char **face_data, int face_count)
{
    printf("parse_face_data_function_entry\n");
    printf("Printing face data:\n");

    // Allocate an array to hold pointers for each face.
    int ***allFaces = malloc(face_count * sizeof(int **));
    if (!allFaces) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < face_count; i++)
    {
        // Make a copy to count the number of vertex tokens.
        char *lineCopy = strdup(face_data[i]);
        if (!lineCopy) {
            perror("strdup");
            exit(EXIT_FAILURE);
        }

        int vertexCount = 0;
        char *saveptr_space = NULL;
        char *token = strtok_r(lineCopy, " ", &saveptr_space);
        while (token != NULL) {
            vertexCount++;
            token = strtok_r(NULL, " ", &saveptr_space);
        }
        free(lineCopy);

        // Allocate an array of pointers for this face's vertices.
        int **faceVertices = malloc(vertexCount * sizeof(int *));
        if (!faceVertices) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }

        // Tokenize the original string again.
        char *faceLineCopy = strdup(face_data[i]);
        if (!faceLineCopy) {
            perror("strdup");
            exit(EXIT_FAILURE);
        }

        saveptr_space = NULL;
        token = strtok_r(faceLineCopy, " ", &saveptr_space);
        int vertexIndex = 0;

        while (token != NULL && vertexIndex < vertexCount)
        {
            // Allocate an array for 3 ints (vertex, texture, normal).
            faceVertices[vertexIndex] = malloc(3 * sizeof(int));
            if (!faceVertices[vertexIndex]) {
                perror("malloc");
                exit(EXIT_FAILURE);
            }

            // Default values in case texture or normal index is missing.
            faceVertices[vertexIndex][0] = 0;  // Vertex index
            faceVertices[vertexIndex][1] = 0;  // Texture index
            faceVertices[vertexIndex][2] = 0;  // Normal index

            // Tokenize the vertex token by "/" to extract each component.
            char *saveptr_slash = NULL;
            char *numToken = strtok_r(token, "/", &saveptr_slash);
            int componentIndex = 0;

            while (numToken != NULL && componentIndex < 3)
            {
                int parsedIndex = atoi(numToken) - 1; // Convert from 1-based to 0-based.
                if (parsedIndex < 0) {
                    parsedIndex = 0;  // Prevent negative indices
                }

                faceVertices[vertexIndex][componentIndex] = parsedIndex;
                numToken = strtok_r(NULL, "/", &saveptr_slash);
                componentIndex++;
            }

            vertexIndex++;
            token = strtok_r(NULL, " ", &saveptr_space);
        }
        free(faceLineCopy);

        // Store the parsed vertices for this face.
        allFaces[i] = faceVertices;
    }
    return allFaces;
}
