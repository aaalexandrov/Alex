macro(set_ide_grouping _sources)
	# set IDE grouping for all sources
	foreach(FILE ${${_sources}}) 
		# Get the directory of the source file
		get_filename_component(PARENT_DIR "${FILE}" DIRECTORY)

		# Remove common directory prefix to make the group
		string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")

		# Make sure we are using windows slashes
		string(REPLACE "/" "\\" GROUP "${GROUP}")

		source_group("${GROUP}" FILES "${FILE}")
	endforeach()
endmacro()

macro(set_target_ide_grouping _group)
	foreach(TGT in ${ARGN})
		set_property(TARGET ${TGT} PROPERTY FOLDER ${_group})
	endforeach()
endmacro()