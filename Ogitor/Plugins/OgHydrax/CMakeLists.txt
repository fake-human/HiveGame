######################################################################
# OGRE BUILD SYSTEM
# Welcome to the CMake build system for OGRE.
# This is the main file where we prepare the general build environment
# and provide build configuration options.
######################################################################



cmake_minimum_required(VERSION 2.6)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)
cmake_policy(SET CMP0003 NEW)

project(OgHydrax)


set(HEADER_FILES
    HydraxWaterEditor.h
)
set(SOURCE_FILES
    HydraxWaterEditor.cpp
)


SET(OgHydrax_VERSION 0)
SET(OgHydrax_VERSION_MAJOR 1)

include_directories (include)
# Terrain Scene Manager
#include_directories ( ${OGITOR_SM_INCLUDES} )
include_directories(${DEPENDENCIES_INCLUDES})
include_directories(${OGITOR_INCLUDES})
MESSAGE(STATUS "${OGITOR_INCLUDES}" )

include_directories(.)
LINK_DIRECTORIES( ${OGITOR_LIBPATH} )
LINK_DIRECTORIES( ${DEPENDENCIES_LIBPATH} )

link_libraries( ${OGRE_LIBRARIES} Ogitor Hydrax)
add_library(OgHydrax SHARED ${HEADER_FILES} ${SOURCE_FILES}  )
#target_link_libraries(OgreMain Ogitor)
SET(LIBRARY_OUTPUT_PATH ../lib)

set_target_properties(OgHydrax PROPERTIES
        INSTALL_RPATH ${OGRE_INSTALL_DIR}/lib/OGRE  )

set_target_properties(OgHydrax PROPERTIES VERSION ${OgHydrax_VERSION} SOVERSION ${OgHydrax_VERSION_MAJOR})

set_target_properties(OgHydrax PROPERTIES
COMPILE_DEFINITIONS PLUGIN_EXPORT
)

set_target_properties(OgHydrax PROPERTIES 
    SOLUTION_FOLDER Plugins) 

if(UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
endif(UNIX)

IF(UNIX AND NOT APPLE)
    add_custom_command( TARGET OgHydrax PRE_BUILD
        COMMAND mkdir -p ../../RunPath/Plugins )
    add_custom_command( TARGET OgHydrax POST_BUILD
        COMMAND cp -r ../lib/libOgHydrax.* ../../RunPath/Plugins/ )
ENDIF(UNIX AND NOT APPLE)

IF(WIN32)
    add_custom_command( TARGET OgHydrax PRE_BUILD
        COMMAND if not exist ..\\..\\RunPath\\Plugins mkdir ..\\..\\RunPath\\Plugins )
    add_custom_command( TARGET OgHydrax POST_BUILD
        COMMAND copy \"$(TargetPath)\" ..\\..\\RunPath\\Plugins & if exist ..\\..\\RunPath\\Plugins\\OgHydrax_d.dll move ..\\..\\RunPath\\Plugins\\OgHydrax_d.dll ..\\..\\RunPath\\Plugins\\debug)
ENDIF(WIN32)

IF(WIN32)
install(TARGETS OgHydrax
    RUNTIME DESTINATION Plugins
    CONFIGURATIONS Release RelWithDebInfo MinSizeRel)
ENDIF()


