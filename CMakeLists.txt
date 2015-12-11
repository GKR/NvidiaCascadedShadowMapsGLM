cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

#link_directories (/usr/lib)
#include_directories (/usr/include)

IF(APPLE)
  FIND_LIBRARY(GLUT_LIBRARY GLUT)
  FIND_LIBRARY(OpenGL_LIBRARY OpenGL)
  find_package(PNG REQUIRED)
  include_directories(${PNG_INCLUDE_DIR})
  include_directories(/usr/X11R6/include/)
  link_directories(/usr/X11R6/lib)
  set(EXT_LIBRARIES ${OpenGL_LIBRARY} GLEW GLU ${GLUT_LIBRARY} ${PNG_LIBRARY})
ELSE()
  set(EXT_LIBRARIES GL GLEW GLU glut png)
ENDIF()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/nvModel)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/nvImage)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_subdirectory(nvModel)
add_subdirectory(nvImage)

add_executable (
  csm_demo_glm
  src/cascaded_shadow_maps.cpp
  src/terrain.cpp
  src/utility.cpp
  src/camera.cpp
  src/frustum.cpp
  src/shadow_map.cpp
)

target_link_libraries (
  csm_demo_glm
  nvimage_static
  nvmodel_static
  ${EXT_LIBRARIES}
)
