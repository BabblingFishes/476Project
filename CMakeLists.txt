cmake_minimum_required(VERSION 2.7)

# Name of the project
project(HolyCow)

set(HOLYCOW_PROJECT_DIR "$ENV{HOLYCOW_PROJECT_DIR}")

# Use glob to get the list of all source files.
file(GLOB_RECURSE SOURCES "src/*.cpp" "ext/*/*.cpp" "ext/glad/src/*.c")

# We don't really need to include header and resource files to build, but it's
# nice to have them show up in IDEs.
file(GLOB_RECURSE HEADERS "src/*.h" "ext/*/*.h" "ext/glad/*/*.h")
file(GLOB_RECURSE GLSL "resources/*.glsl")

include_directories(
  "ext/glad"
  "ext/glad/include"
  "ext/tiny_obj_loader"
  "ext/irrKlang"
  "ext/irrKlang-64bit-1.6.0/bin/linux-gcc-64"
  "ext/imgui"
)

# Set the executable.
add_executable(${CMAKE_PROJECT_NAME} ${SOURCES} ${HEADERS} ${GLSL})


# Add GLFW
# Get the GLFW environment variable.
# There should be a CMakeLists.txt in the specified directory.
set(GLFW_DIR "$ENV{GLFW_DIR}")
if(GLFW_DIR)
  message(STATUS "GLFW environment variable found")

  option(GLFW_BUILD_EXAMPLES "GLFW_BUILD_EXAMPLES" OFF)
  option(GLFW_BUILD_TESTS "GLFW_BUILD_TESTS" OFF)
  option(GLFW_BUILD_DOCS "GLFW_BUILD_DOCS" OFF)

  if(CMAKE_BUILD_TYPE MATCHES Release)
    add_subdirectory(${GLFW_DIR} ${GLFW_DIR}/release)
  else()
    add_subdirectory(${GLFW_DIR} ${GLFW_DIR}/debug)
  endif()

  include_directories(${GLFW_DIR}/include)
  target_link_libraries(${CMAKE_PROJECT_NAME} glfw ${GLFW_LIBRARIES})

else()
  message(STATUS "GLFW environment variable `GLFW_DIR` not found, GLFW3 must be installed with the system")
endif()


#ADD IRRKLANG
if(HOLYCOW_PROJECT_DIR)
  #link_directories("${HOLYCOW_PROJECT_DIR}/ext/irrKlang-64bit-1.6.0/bin/linux-gcc-64")
  target_link_libraries(${CMAKE_PROJECT_NAME} "${HOLYCOW_PROJECT_DIR}/ext/irrKlang/bin/linux-gcc-64/libIrrKlang.so" IrrKlang)
else()
  message(STATUS "Environment variable `HOLYCOW_PROJECT_DIR` not found")
endif()


# Add GLM
# Get the GLM environment variable. Since GLM is a header-only library, we
# just need to add it to the include directory.
set(GLM_INCLUDE_DIR "$ENV{GLM_INCLUDE_DIR}")
if(GLM_INCLUDE_DIR)
  include_directories(${GLM_INCLUDE_DIR})
  message(STATUS "GLM environment variable found")
else()
# If the GLM_INCLUDE_DIR environment variable is not set, we assume
# the user has installed GLM properly on their system
  message(STATUS "GLM environment variable `GLM_INCLUDE_DIR` not found, GLM must be installed with the system")
endif()


# OS specific options and libraries
if(WIN32)
  # c++0x is enabled by default.
  # -Wall produces way too many warnings.
  # -pedantic is not supported.
  target_link_libraries(${CMAKE_PROJECT_NAME} opengl32.lib)
else()
  # Enable all pedantic warnings.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -pedantic -g")
  #reminder: -g is for debugging

  if(APPLE)
    # Add required frameworks for GLFW.
    target_link_libraries(${CMAKE_PROJECT_NAME} "-framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo")
  else()
    #Link the Linux OpenGL library
    target_link_libraries(${CMAKE_PROJECT_NAME} "GL" "dl")
  endif()
endif()
