cmake_minimum_required(VERSION 3.1)
project(dbgModule)

set ( PLATFORM "aarch64" CACHE STRING "CPU Architecture (X86_64/ARM64)" )
message ( STATUS "PLATFORM ${PLATFORM} mode" )

if (${PLATFORM} MATCHES "x64")
	set ( ARCH "x64" )

	add_definitions (-D__X64__)
	add_definitions (-DUSE_SIGSTACK_LVL=2)
else()
	set ( ARCH "aarch64" )
	set ( CROSS_COMPILE "aarch64-linux-gnu-" CACHE STRING "ARM64 build toolset" )
	set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv8-a+crypto -mcpu=cortex-a57+crypto" )

	add_definitions (-D__ARM64__)
	add_definitions (-DUSE_SIGSTACK_LVL=1)
endif()

set ( CMAKE_C_COMPILER   "${CROSS_COMPILE}gcc"     CACHE STRING "Build Tools" )
set ( CMAKE_CXX_COMPILER "${CROSS_COMPILE}g++"     CACHE STRING "Build Tools" )
set ( CMAKE_LINKER       "${CROSS_COMPILE}ld"      CACHE STRING "Build Tools" )
set ( CMAKE_AR           "${CROSS_COMPILE}ar"      CACHE STRING "Build Tools" )
set ( CMAKE_NM           "${CROSS_COMPILE}nm"      CACHE STRING "Build Tools" )
set ( CMAKE_OBJCOPY      "${CROSS_COMPILE}objcopy" CACHE STRING "Build Tools" )
set ( CMAKE_OBJDUMP      "${CROSS_COMPILE}objdump" CACHE STRING "Build Tools" )
set ( CMAKE_RANLIB       "${CROSS_COMPILE}ranlib"  CACHE STRING "Build Tools" )
set ( CMAKE_STRIP        "${CROSS_COMPILE}strip"   CACHE STRING "Build Tools" )

set ( DBG_SRC_FILES
		${CMAKE_SOURCE_DIR}/arch/${ARCH}/context_${ARCH}.c
		${CMAKE_SOURCE_DIR}/arch/${ARCH}/dsm_util.c
		${CMAKE_SOURCE_DIR}/arch/${ARCH}/get_frame.c
		${CMAKE_SOURCE_DIR}/arch/backtrace.c
		${CMAKE_SOURCE_DIR}/arch/dsm.c
		${CMAKE_SOURCE_DIR}/arch/exception_handler.c
		${CMAKE_SOURCE_DIR}/comm/ipc_comm.c
		${CMAKE_SOURCE_DIR}/debug/cmd_func.c
		${CMAKE_SOURCE_DIR}/debug/cmd_input.c
		${CMAKE_SOURCE_DIR}/debug/cmd_shell.c
		${CMAKE_SOURCE_DIR}/symbol/addr2line.c
		${CMAKE_SOURCE_DIR}/symbol/demangle.c
		${CMAKE_SOURCE_DIR}/symbol/dyn_sym.c
		${CMAKE_SOURCE_DIR}/symbol/symbol.c
		${CMAKE_SOURCE_DIR}/thread/signal.c
		${CMAKE_SOURCE_DIR}/thread/thread_util.c
		${CMAKE_SOURCE_DIR}/thread/threadlib.c
		${CMAKE_SOURCE_DIR}/xlibc/xlibc_hooklib
		${CMAKE_SOURCE_DIR}/xlibc/xlibc_termios.c
		${CMAKE_SOURCE_DIR}/xlibc/xlibc_input.c
		${CMAKE_SOURCE_DIR}/xlibc/xlibc_output.c
		${CMAKE_SOURCE_DIR}/xlibc/xlibc_cpp_out.cpp
		${CMAKE_SOURCE_DIR}/xlibc/xlibc_util.c
		${CMAKE_SOURCE_DIR}/root.c
	)

set (prebuilt_objs
		${CMAKE_SOURCE_DIR}/prebuilt_lib/${ARCH}/pbobj/capstone/*.po
	)

set (BIG_ENDIAN    "0x1001")
set (LITTLE_ENDIAN "0x1002")
set (BIT_32        "0x1003")
set (BIT_64        "0x1004")

add_definitions (-D__LINUX__)
add_definitions (-D_GNU_SOURCE)
add_definitions (-DBIG_ENDIAN=${BIG_ENDIAN})
add_definitions (-DLITTLE_ENDIAN=${LITTLE_ENDIAN})
add_definitions (-DBIT_32=${BIT_32})
add_definitions (-DBIT_64=${BIT_64})
add_definitions (-DENDIAN_TYPE=${LITTLE_ENDIAN})
add_definitions (-DBIT_WIDTH=${BIT_64})
add_definitions (-DCPP_DEMANGLE)


set ( dbg_inc_paths
		${CMAKE_SOURCE_DIR}
		${CMAKE_SOURCE_DIR}/inc
		${CMAKE_SOURCE_DIR}/inc/xlibc
		${CMAKE_SOURCE_DIR}/inc/comm
		${CMAKE_SOURCE_DIR}/inc/debug
		${CMAKE_SOURCE_DIR}/inc/thread
		${CMAKE_SOURCE_DIR}/inc/symbol
		${CMAKE_SOURCE_DIR}/inc/capstone
		${CMAKE_SOURCE_DIR}/inc/arch
		${CMAKE_SOURCE_DIR}/inc/arch/${ARCH}
		)
INCLUDE_DIRECTORIES( ${dbg_inc_paths} )

#set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR} )
add_library (dbgModule_${ARCH} STATIC ${DBG_SRC_FILES} )

add_custom_target ( listup COMMAND echo ${prebuilt_objs})

add_custom_command (
		TARGET dbgModule_${ARCH}
		POST_BUILD
		COMMENT "Add prebuilt libraries"
		COMMAND ${CROSS}ar -r libdbgModule_${ARCH}.a ${prebuilt_objs}
	)
