cmake_minimum_required(VERSION 3.7)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
project(boosted-recon)
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(ROOT REQUIRED COMPONENTS ROOTDataFrame XMLParser ROOTVecOps )
find_package(Delphes)
add_definitions(-Wfatal-errors -Wall -Wextra)

include(${ROOT_USE_FILE})
include_directories(BEFORE include ${DELPHES_INCLUDE_DIRS})
add_subdirectory(include/fmt EXCLUDE_FROM_ALL)
ROOT_GENERATE_DICTIONARY(G__dictionary utils.h Cutflow.h LINKDEF LinkDef.h)
set(boosted-recon_SRC
    boost-recon.cpp
    Cutflow.cpp
)


# Tell CMake to create the helloworld executable
add_executable(boosted-recon ${boosted-recon_SRC} G__dictionary.cxx)

target_link_libraries(boosted-recon PRIVATE ${ROOT_LIBRARIES} ${DELPHES_LIBRARY} fmt::fmt-header-only)

