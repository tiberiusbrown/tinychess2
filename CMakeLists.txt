cmake_minimum_required(VERSION 2.8)

project(tinychess2 C CXX)

set(CMAKE_CXX_STANDARD 17)
set(EDITORCONFIG "${CMAKE_CURRENT_SOURCE_DIR}/.editorconfig")
set(EXETYPE)

if(MSVC AND NOT LLVM)
    set(EXETYPE WIN32)
    set(CMAKE_CXX_FLAGS "/W4 /MT /DWIN32 /D_WINDOWS /FAcs /Gy /EHsc /Gs65536 /wd4127")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "/DNDEBUG /MT /O1 /Oi /Os /Oy /GS- /fp:fast /Gr")
    set(CMAKE_CXX_FLAGS_RELEASE "/DNDEBUG /MT /Ox /Oi /Ot /Oy /Ob2 /GS- /fp:fast /Gr")
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
		set(LINKLD "${CMAKE_CURRENT_SOURCE_DIR}/link32.ld")
	else()
		set(LINKLD "${CMAKE_CURRENT_SOURCE_DIR}/link64.ld")
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
    set(COMMON_FLAGS "${COMMON_FLAGS} -Wall -Wextra")

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

file(GLOB GUI_SOURCES "gui/*")
add_executable(${PROJECT_NAME} ${EXETYPE} ${GUI_SOURCES} ${EDITORCONFIG})
target_link_libraries(${PROJECT_NAME} ch)
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC "ch")
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
        /ENTRY:start
        /NODEFAULTLIB
        /MANIFEST:NO
        /DYNAMICBASE:NO
        /FIXED
        /SAFESEH:NO
        /INCREMENTAL:NO
        /OPT:REF
        /OPT:ICF=16
        /SECTION:.text,R
        /MERGE:\".rdata=.text\"
        /MERGE:\".xdata=.text\"
        /MERGE:\".pdata=.text\"
        /ignore:4254
        ")
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

	file(GLOB SSTRIP_SOURCES sstrip/*.c sstrip/*.h)
	add_executable(sstrip ${SSTRIP_SOURCES})
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND $<TARGET_FILE:sstrip> $<TARGET_FILE:${PROJECT_NAME}>
		)

endif()
