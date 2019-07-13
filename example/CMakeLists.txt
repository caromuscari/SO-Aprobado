cmake_minimum_required(VERSION 3.13)
project(Kernel C)

set(CMAKE_C_STANDARD 11)
set(PATH_KERNEL "TestEnvioMensaje.c")
add_executable(Kernel ${PATH_KERNEL})
target_link_libraries(Kernel -lreadline -lpthread -lcommons -lcompartidas)
