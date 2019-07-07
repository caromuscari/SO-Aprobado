cmake_minimum_required(VERSION 3.13)
project(fileSystem C)

set(CMAKE_C_STANDARD 11)

set(PATH_FS "./src")

FILE(GLOB FILE_FS ${PATH_FS}/*.c ${PATH_FS}/*.h )

add_executable(fileSystem ${FILE_FS} src/hiloInotify.c src/hiloInotify.h)

target_link_libraries(fileSystem -lreadline -lpthread -lcommons -lcompartidas)