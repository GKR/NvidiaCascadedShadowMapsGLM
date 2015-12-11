cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

set(LIBNVMODEL_SRC nvModel.cc nvModelObj.cc nvModelQuery.cc nvUtils.cc)
#set(LIBRARY_OUTPUT_PATH ../bin_test/nvmodel)
#add_library(nvmodel SHARED ${LIBNVMODEL_SRC})
add_library(nvmodel_static STATIC ${LIBNVMODEL_SRC})
set_target_properties(nvmodel_static PROPERTIES OUTPUT_NAME "nvmodel")
#set_target_properties(nvmodel_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
#set_target_properties(nvmodel PROPERTIES CLEAN_DIRECT_OUTPUT 1)

