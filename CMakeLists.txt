cmake_minimum_required(VERSION 3.20)
project(sandbox)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(${CMAKE_SOURCE_DIR}/src)

# glfw
add_subdirectory(${CMAKE_SOURCE_DIR}/deps/glfw/)

# glad
add_library(glad STATIC ${CMAKE_SOURCE_DIR}/deps/glad/src/glad.c)
target_include_directories(glad PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/deps/glad/include
)

# stb
add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE
  ${CMAKE_SOURCE_DIR}/deps/stb
)

# stb
add_library(argh INTERFACE)
target_include_directories(argh INTERFACE
  ${CMAKE_SOURCE_DIR}/deps/argh
)

# freetype
add_subdirectory(${CMAKE_SOURCE_DIR}/deps/freetype)

# assimp
add_subdirectory(${CMAKE_SOURCE_DIR}/deps/assimp)

# gifdec
add_library(gifdec STATIC ${CMAKE_SOURCE_DIR}/deps/gifdec/gifdec.c)
target_include_directories(gifdec PRIVATE
  ${CMAKE_SOURCE_DIR}/deps/gifdec/
)

enable_testing()
add_subdirectory(${CMAKE_SOURCE_DIR}/tests)

add_executable(${PROJECT_NAME} main.cc)
target_link_libraries(${PROJECT_NAME} PUBLIC
  sandboxlib
  freetype
  assimp
  glfw
  glad
  gifdec
)

target_compile_options(${PROJECT_NAME} PUBLIC -g)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "a")