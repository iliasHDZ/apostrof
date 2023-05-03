cmake_minimum_required(VERSION 3.22.1)

set(PROJECT_NAME Apostrof)

set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

project(${PROJECT_NAME} VERSION 1.0.0 LANGUAGES C CXX ASM_NASM)

file(GLOB_RECURSE C_SRCS   src/**.c)
file(GLOB_RECURSE CXX_SRCS src/**.cpp)
file(GLOB_RECURSE ASM_SRCS src/**.asm)

set(C_FLAGS   "-fno-pie -m32 -ffreestanding -nostdlib")
set(CXX_FLAGS "-fno-exceptions -fno-rtti")
set(ASM_FLAGS "-f elf32")

set(LD_FLAGS "-Wl,--demangle,-m,elf_i386,-nostdlib,-n,-T,${CMAKE_CURRENT_LIST_DIR}/linker.ld")

set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> ${ASM_FLAGS} -o <OBJECT> <SOURCE>")

set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}        ${C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}      ${CXX_FLAGS} ${C_FLAGS}")

add_executable(ApostrofKernel ${C_SRCS} ${CXX_SRCS} ${ASM_SRCS})

add_custom_command(
    TARGET ApostrofKernel POST_BUILD
    COMMAND bash compile_iso.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Compiling OS filesystem into ISO file..."
)

set_target_properties(ApostrofKernel PROPERTIES LINK_FLAGS ${LD_FLAGS})