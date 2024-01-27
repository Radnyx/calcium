cmake_minimum_required(VERSION 3.5)
project(Calcium VERSION 0.1.0 LANGUAGES C CXX)
set (CMAKE_CXX_STANDARD 17)

execute_process(COMMAND llvm-config "--cxxflags" OUTPUT_VARIABLE CXXFLAGS)
set(CMAKE_CXX_FLAGS "${CXXFLAGS}")

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
include(CTest)
enable_testing()

file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_executable(Calcium ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "calcium")

find_package(Vulkan REQUIRED)

# Includes

execute_process(COMMAND llvm-config "--includedir" OUTPUT_VARIABLE LLVM_INCLUDEDIR)

target_include_directories(Calcium PRIVATE ${LLVM_INCLUDEDIR})
target_include_directories(Calcium PRIVATE "C:/VulkanSDK/1.3.275.0/Include")

# Libraries

execute_process(COMMAND llvm-config "--libdir" OUTPUT_VARIABLE LLVM_LIBDIR)
execute_process(COMMAND llvm-config "--libs" all OUTPUT_VARIABLE LLVM_LIBS)

string(STRIP ${LLVM_LIBS} LLVM_LIBS)
separate_arguments(LLVM_LIBS)

link_directories(${LLVM_LIBDIR})

target_link_libraries(Calcium ${LLVM_LIBS} ${Vulkan_LIBRARIES})
if(WIN32)
  target_link_libraries(Calcium wsock32 ws2_32)
endif()