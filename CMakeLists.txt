# Library Manager - CMake Build Configuration
# Author: Dawid Papaj

cmake_minimum_required(VERSION 3.15)
project(LibraryManager VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows-specific settings
if(WIN32)
    set(CMAKE_WIN32_EXECUTABLE ON)
    add_definitions(-DUNICODE -D_UNICODE)
endif()

# Main executable with SQLite3 compiled directly
set(SOURCES
    src/main.cpp
    src/database.cpp
    lib/sqlite3.c
)

# Compile sqlite3.c as C++
set_source_files_properties(lib/sqlite3.c PROPERTIES LANGUAGE CXX)

set(HEADERS
    src/database.h
    src/resource.h
    lib/sqlite3.h
)

# Resource file for Windows
if(WIN32)
    set(RESOURCES src/resource.rc)
    enable_language(RC)
endif()

add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS} ${RESOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/lib
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    comctl32
    comdlg32
)

# Install rules
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(FILES ${CMAKE_SOURCE_DIR}/src/app.ico DESTINATION bin OPTIONAL)

# CPack for installer
set(CPACK_PACKAGE_NAME "LibraryManager")
set(CPACK_PACKAGE_VENDOR "Dawid Papaj")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Library Management Application")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "LibraryManager")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")

if(WIN32)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_DISPLAY_NAME "Library Manager")
    set(CPACK_NSIS_PACKAGE_NAME "Library Manager")
    set(CPACK_NSIS_EXECUTABLES_DIRECTORY "bin")
    set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/src/app.ico")
    set(CPACK_NSIS_CREATE_ICONS_EXTRA
        "CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Library Manager.lnk' '$INSTDIR\\\\bin\\\\LibraryManager.exe'"
    )
    set(CPACK_NSIS_DELETE_ICONS_EXTRA
        "Delete '$SMPROGRAMS\\\\$START_MENU\\\\Library Manager.lnk'"
    )
endif()

include(CPack)
