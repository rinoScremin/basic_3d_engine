# basic_3d_engine
Here is a clean, professional `README.md` you can paste into your repo.

---

# Basic 3D Engine (C)

A simple 3D wireframe rendering engine written entirely in C.

This project loads Wavefront `.obj` files, converts them into internal model structures, transforms them through world space → camera space → projection space, and renders the result as a 3D wireframe using SDL.

The camera can be moved interactively using keyboard controls.

---

## Features

* Parses Wavefront `.obj` files
* Builds internal 3D model structures
* World space transformations
* Camera space transformations
* Perspective projection
* Wireframe rendering
* Interactive camera movement (WASD + Arrow Keys)
* Multiple objects rendered at once

---

## How It Works

The rendering pipeline follows this order:

1. Read `.obj` file into a raw data buffer
2. Parse vertex and face data
3. Build internal object model
4. Transform object into world space
5. Transform world space into camera space
6. Project 3D coordinates into 2D screen space
7. Render edges using SDL line drawing

---

## Using Custom `.obj` Files

To render a model, you must provide the path to a `.obj` file inside `main()`.

Example:

```c
char *object_file_path = "monky.obj";
struct Object_file_data *Raw_object_file = read_object_file(object_file_path);
struct Obj_file_data_object *parsed = get_object_vertex_3d(Raw_object_file);
struct object_model *model = build_model(parsed, 7.0f, 7.0f, 7.0f);
transform_object_to_world(model, world);
```

You can load multiple objects by repeating this process with different file paths.

Important:

* The `.obj` file must exist in the project directory, or
* You must provide the full absolute path

Example:

```c
char *object_file_path = "/home/user/models/model.obj";
```

---

## Rendering

After loading and transforming objects, the rendering process works like this:

```c
camera = transform_world_space_objects_to_camera_space_objects(camera, world);
projection_space_objects = transform_camera_space_objects_to_projection_space_objects(camera, render);
Vertex_3d **faces = render_object(projection_space_objects, render);
```

The engine then:

* Maps projected coordinates to screen space
* Draws lines between face vertices using SDL_RenderDrawLine
* Displays the result as a wireframe

---

## Controls

Movement:

* W → Move forward
* S → Move backward
* A → Move left
* D → Move right
* Up Arrow → Move up
* Down Arrow → Move down
* ESC → Exit

Camera movement updates world-to-camera and projection transforms in real time.

---

## Requirements

* C compiler (gcc recommended)
* SDL2

On Ubuntu:

```bash
sudo apt install libsdl2-dev
```

Build using:

```bash
make
```

Run:

```bash
./C_3D_engine
```

---

## What This Project Demonstrates

* Manual 3D graphics pipeline implementation
* Matrix-style coordinate transformations
* Parsing structured file formats (.obj)
* Real-time rendering loop with SDL
* Memory management in C
* Modular engine architecture (world, camera, projection, renderer)

---

<img width="1517" height="708" alt="image" src="https://github.com/user-attachments/assets/1dd35865-104d-4481-a411-80358eb098a0" />


<img width="1498" height="985" alt="Screenshot from 2026-02-16 10-24-29" src="https://github.com/user-attachments/assets/911e4c67-75ed-446e-a81f-322650a3a7af" />


<img width="1498" height="985" alt="Screenshot from 2026-02-16 10-24-48" src="https://github.com/user-attachments/assets/2fae0227-84e4-4b69-8704-270c6c32f927" />


