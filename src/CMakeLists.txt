
# collect all source files for sandbox-lib
file(GLOB_RECURSE SANDBOX_SOURCE ${CMAKE_SOURCE_DIR}/src/*.cc)
# create a library object with these sources
add_library(sandboxlib STATIC ${SANDBOX_SOURCE})
# add the header files
target_include_directories(sandboxlib 
  PUBLIC
  ${CMAKE_SOURCE_DIR}/include/sandbox/
  ${CMAKE_SOURCE_DIR}/deps/glfw/include
  ${CMAKE_SOURCE_DIR}/deps/glad/include
  ${CMAKE_SOURCE_DIR}/deps/glm/
  ${CMAKE_SOURCE_DIR}/deps/stb/
  ${CMAKE_SOURCE_DIR}/deps/argh/
  ${CMAKE_SOURCE_DIR}/deps/assimp/include
  ${CMAKE_SOURCE_DIR}/deps/freetype/include
)

target_compile_options(sandboxlib PUBLIC -g)