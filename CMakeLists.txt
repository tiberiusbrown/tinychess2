cmake_minimum_required(VERSION 2.8)

project(tinychess2 C CXX)

set(CMAKE_CXX_STANDARD 11)
set(EDITORCONFIG "${CMAKE_CURRENT_SOURCE_DIR}/.editorconfig")
set(EXETYPE)

if(MSVC)
    set(EXETYPE WIN32)
    set(CMAKE_CXX_FLAGS "/W4 /MT /DWIN32 /D_WINDOWS /FAcs /Gy /Gs65536 /wd4127")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "/DNDEBUG /MT /O1 /Oi /Os /Oy /GS- /fp:fast /Gr")
    set(CMAKE_CXX_FLAGS_RELEASE "/DNDEBUG /MT /O2 /Oi /Ot /Oy /Ob2 /GS- /fp:fast /Gr")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE} /DEBUG /Zi")
    set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL}")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
    set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "/INCREMENTAL:NO")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/INCREMENTAL:NO")
    set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "/INCREMENTAL:NO /DEBUG")
    add_definitions(-D_HAS_EXCEPTIONS=0)
elseif(UNIX)

    if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
        option(CFG_BUILD_32BIT "Build 32-bit executable" OFF)
    else()
        set(CFG_BUILD_32BIT ON)
    endif()
	if(CFG_BUILD_32BIT)
		set(LINKLD "${CMAKE_CURRENT_SOURCE_DIR}/gui/link32.ld")
	else()
		set(LINKLD "${CMAKE_CURRENT_SOURCE_DIR}/gui/link64.ld")
	endif()

    set(COMMON_FLAGS)
    set(COMMON_FLAGS "${COMMON_FLAGS} -fno-exceptions -fno-unwind-tables")
    set(COMMON_FLAGS "${COMMON_FLAGS} -fno-asynchronous-unwind-tables")
    set(COMMON_FLAGS "${COMMON_FLAGS} -fno-stack-protector")
    set(COMMON_FLAGS "${COMMON_FLAGS} -Wno-unused-function")
    set(COMMON_FLAGS "${COMMON_FLAGS} -fno-builtin")
    set(COMMON_FLAGS "${COMMON_FLAGS} -fvisibility=hidden")
    set(COMMON_FLAGS "${COMMON_FLAGS} -fdata-sections -ffunction-sections")
    set(COMMON_FLAGS "${COMMON_FLAGS} -fomit-frame-pointer")
    set(COMMON_FLAGS "${COMMON_FLAGS} -fno-ident -Qn")
    set(COMMON_FLAGS "${COMMON_FLAGS} -freg-struct-return")
    set(COMMON_FLAGS "${COMMON_FLAGS} -mpopcnt -msse2 -mavx -mavx2 -mbmi -mbmi2")
    set(COMMON_FLAGS "${COMMON_FLAGS} -fno-tree-vectorize")
    set(COMMON_FLAGS "${COMMON_FLAGS} -Wall -Wextra")
    set(COMMON_FLAGS "${COMMON_FLAGS} -std=c++11")

	if(CMAKE_C_COMPILER_VERSION VERSION_GREATER 6)
		set(COMMON_FLAGS "${COMMON_FLAGS} -fno-plt")
	endif()

    set(CMAKE_CXX_FLAGS "${COMMON_FLAGS}")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${COMMON_FLAGS}")
    set(CMAKE_CXX_FLAGS_RELEASE "${COMMON_FLAGS}")

    set(COMMON_OPT_FLAGS)
    set(COMMON_OPT_FLAGS "${COMMON_OPT_FLAGS} -DNDEBUG")
    set(COMMON_OPT_FLAGS "${COMMON_OPT_FLAGS} -fmerge-all-constants")
    set(COMMON_OPT_FLAGS "${COMMON_OPT_FLAGS} -fmodulo-sched")
    set(COMMON_OPT_FLAGS "${COMMON_OPT_FLAGS} -fmodulo-sched-allow-regmoves")
    set(COMMON_OPT_FLAGS "${COMMON_OPT_FLAGS} -flto")
    set(COMMON_OPT_FLAGS "${COMMON_OPT_FLAGS}")

    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} ${COMMON_OPT_FLAGS}")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${COMMON_OPT_FLAGS}")

    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -Os")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -falign-functions=1")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -falign-jumps=1")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -falign-labels=1")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -falign-loops=1")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Ofast")

    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE} -g")

    set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL}")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")

    set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "-flto")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-flto")
endif()

file(GLOB CH_SOURCES "ch/*")
add_library(ch STATIC ${CH_SOURCES} ${EDITORCONFIG})
if(UNIX)
    target_compile_options(ch PRIVATE -mssse3 -fno-tree-vectorize)
    target_link_libraries(ch gcc)
endif()

add_executable(test_perft test_perft.cpp ${EDITORCONFIG})
target_link_libraries(test_perft ch)

add_executable(${PROJECT_NAME}_uci uci.cpp ${EDITORCONFIG})
target_link_libraries(${PROJECT_NAME}_uci ch)
if(UNIX)
    target_link_libraries(${PROJECT_NAME}_uci pthread)
endif()

if(TRUE)

set(BUILD_COMPRESSED_DATA_SOURCES
    gui/alldata.h
    gui/common.h
    gui/config.h
    gui/font.h
    gui/img.h
    gui/build_compressed_data.cpp
    )
add_executable(build_compressed_data ${BUILD_COMPRESSED_DATA_SOURCES})
if(MSVC)
    target_compile_options(build_compressed_data PRIVATE /wd4996)
endif()
if(UNIX)
    target_compile_options(build_compressed_data PRIVATE -std=c++11)
    target_compile_options(build_compressed_data PRIVATE -O2)
endif()
set(ALLDATA_H "${CMAKE_CURRENT_BINARY_DIR}/ALLDATA_COMPRESSED.h")
target_compile_definitions(build_compressed_data PRIVATE
    -DOUTFILEH="${ALLDATA_H}")
add_custom_command(
    OUTPUT "${ALLDATA_H}"
    DEPENDS build_compressed_data
    COMMAND "$<TARGET_FILE:build_compressed_data>"
    )
set_source_files_properties("${ALLDATA_H}"
    PROPERTIES GENERATED TRUE)
if(MSVC)
    source_group("Generated" FILES "${ALLDATA_H}")
endif()

set(GUI_SOURCE_FILE)
if(MSVC)
    set(GUI_SOURCE_FILE gui/start_windows.c)
elseif(UNIX)
    set(GUI_SOURCE_FILE gui/start_linux.c)
endif()
add_executable(${PROJECT_NAME} ${EXETYPE}
    ${GUI_SOURCE_FILE}
    gui/common.h
    gui/config.h
    gui/data.h
    gui/draw_helpers.h
    gui/font.h
    gui/format.h
    gui/img.h
    gui/settings.h
    gui/start_common.h
    gui/system.h
    gui/run.h
    gui/text.h
    gui/thread.h
    "${ALLDATA_H}"
    ${EDITORCONFIG}
    )
target_link_libraries(${PROJECT_NAME} ch)
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC "ch")
target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}")
if(MSVC)
    #target_compile_options(${PROJECT_NAME} PRIVATE /d2noftol3)

    if(MSVC_VERSION VERSION_LESS 1700)
        target_compile_options(${PROJECT_NAME} PRIVATE
            /wd4054 /wd4055 /wd4127 /wd4725)
    else()
        set_property(
            TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS "
            /EMITPOGOPHASEINFO
            ")        
    endif()

    set_property(
        TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS "
        /MANIFEST:NO
        /DYNAMICBASE:NO
        /FIXED
        /INCREMENTAL:NO
        /OPT:REF
        /OPT:ICF=16
        /ignore:4254
        ")
    foreach(B MINSIZEREL;RELEASE;RELWITHDEBINFO)
        set_property(
            TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS_${B} "
            /SAFESEH:NO
            /ENTRY:start
            /NODEFAULTLIB")
    endforeach()

    #set_property(
    #    TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS "
    #    /SECTION:.text,R
    #    /MERGE:\".rdata=.text\"
    #    /MERGE:\".xdata=.text\"
    #    /MERGE:\".pdata=.text\"
    #    ")
        
elseif(UNIX)

    target_compile_options(${PROJECT_NAME} PRIVATE -nodefaultlibs)

    target_link_libraries(${PROJECT_NAME} c)

    set_property(
        TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS
        " -Wl,-estart -nostartfiles -nostdlib -Wl,--gc-sections")

	set_target_properties(${PROJECT_NAME} PROPERTIES LINK_DEPENDS "${LINKLD}")
	set_property(
		TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS
		" -Wl,-T,${LINKLD}")

	set_property(
		TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS
		" -Wl,--build-id=none -Wl,-z,norelro -Wl,--hash-style=gnu -s")

	file(GLOB SSTRIP_SOURCES gui/sstrip/*.c gui/sstrip/*.h)
	add_executable(sstrip ${SSTRIP_SOURCES})
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND $<TARGET_FILE:sstrip> $<TARGET_FILE:${PROJECT_NAME}>
		)

endif()

endif()
