cmake_minimum_required(VERSION 2.8)

project(tinychess2 C CXX)

set(EXETYPE)

if(MSVC AND NOT LLVM)
    set(EXETYPE WIN32)
    set(CMAKE_CXX_FLAGS "/MT /DWIN32 /D_WINDOWS /W3 /FAcs /Gy")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "/DNDEBUG /MT /O1 /Oi /Os /Oy /GS- /fp:fast /Gr")
    set(CMAKE_CXX_FLAGS_RELEASE "/DNDEBUG /MT /Ox /Oi /Ot /Oy /Ob2 /GS- /fp:fast /Gr")
    set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL}")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
    set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "/INCREMENTAL:NO")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/INCREMENTAL:NO")
    set(CMAKE_STATIC_LINKER_FLAGS_MINSIZEREL "")
    set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "")
    add_definitions(-D_HAS_EXCEPTIONS=0)
endif()

file(GLOB CH_SOURCES "ch/*")
add_library(ch STATIC ${CH_SOURCES})

add_executable(test_perft test_perft.cpp)
target_link_libraries(test_perft ch)

file(GLOB GUI_SOURCES "gui/*")
add_executable(${PROJECT_NAME} ${EXETYPE} ${GUI_SOURCES})
target_link_libraries(${PROJECT_NAME} ch)
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC "ch")
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
    target_compile_options(${PROJECT_NAME} PRIVATE /Gr)
    target_compile_options(${PROJECT_NAME} PRIVATE /EHsc)
    #target_compile_options(${PROJECT_NAME} PRIVATE /d2noftol3)
    #target_compile_options(${PROJECT_NAME} PRIVATE
    #    $<$<CONFIG:MINSIZEREL>:/Os /Oy>)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /GS- /Gs9999999)

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
endif()
