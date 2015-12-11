cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

set(
  LIBNVIMAGE_SRC
  nvImage.cpp
  nvImageDDS.cpp
  nvImageHdr.cpp
  nvImagePng.cpp
  rgbe.c
)

#set(LIBRARY_OUTPUT_PATH ../bin_test/nvmodel)
#add_library(nvmodel SHARED ${LIBNVMODEL_SRC})

add_library(nvimage_static STATIC ${LIBNVIMAGE_SRC})
set_target_properties(nvimage_static PROPERTIES OUTPUT_NAME "nvimage")

#set_target_properties(nvmodel_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
#set_target_properties(nvmodel PROPERTIES CLEAN_DIRECT_OUTPUT 1)

