
set(dir_SOURCES
	camera.h
	camera.cpp

	framework.h
	framework.cpp
	
	material.h
	material.cpp
	
	model.h
	model.cpp
	
	renderable_component.h
	renderable_component.cpp
	
	scene.h
	scene.cpp

	scene_object.h
	scene_object.cpp
)

target_sources(${BINARY} PRIVATE ${dir_SOURCES} CMakeLists.txt)