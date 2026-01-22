
# collect all source files for sandbox-lib
file(GLOB_RECURSE SANDBOX_SOURCE ${CMAKE_SOURCE_DIR}/src/*.cc)
# create a library object with these sources
add_library(sandboxlib STATIC ${SANDBOX_SOURCE})
# add the header files
target_include_directories(sandboxlib 
  PUBLIC
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/deps/glfw/include
  ${CMAKE_SOURCE_DIR}/deps/glad/include
  ${CMAKE_SOURCE_DIR}/deps/glm/
  ${CMAKE_SOURCE_DIR}/deps/stb/
  ${CMAKE_SOURCE_DIR}/deps/argh/
)

target_compile_options(sandboxlib PUBLIC -g)