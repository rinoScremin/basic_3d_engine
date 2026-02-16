# Main program compilation
C_3D_engine:
	gcc -g C_3D_engine.c D3_object_model.c object_file_handler.c world_space.c camera_space.c renderer.c Projection_Space.c -o C_3D_engine -lgsl -lgslcblas -lm -lOpenCL -lSDL2 -lGL -lGLEW

run: C_3D_engine
	./C_3D_engine

clean:
	rm -f C_3D_engine
