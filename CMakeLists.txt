# GetMyFilesBack (UndeleteWizzard) - 64-bit MSVC build, wxWidgets from vcpkg (static, see vcpkg.json).
# Use build.ps1, or: cmake --preset x64 && cmake --build --preset release
cmake_minimum_required(VERSION 3.21)
project(GetMyFilesBack C CXX RC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# static C runtime, like the static wxWidgets libs (triplet x64-windows-static): one exe, no DLLs to ship.
# It is run from a USB stick rather than installed on the drive the files are recovered from.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

find_package(wxWidgets CONFIG REQUIRED)

set(SRC "${CMAKE_SOURCE_DIR}/src/UndeleteWizzard")

# where the recovery engine comes from - tests/ntfs-image/build_harness.ps1 -WithOld points it at a copy from git HEAD
set(UNDELETE_ENGINE_DIR "${SRC}" CACHE PATH "Directory with NTFSDrive.cpp, MFTRecord.cpp, FileContents.cpp")
set(UNDELETE_HARNESS_SOURCE "${CMAKE_SOURCE_DIR}/tests/ntfs-image/harness.cpp" CACHE FILEPATH "The test harness source")
option(UNDELETE_BUILD_APP "Build the GUI app" ON)
option(UNDELETE_BUILD_TESTS "Build the NTFS image test harness" ON)

# WIN32_LEAN_AND_MEAN: a full <windows.h> brings the old winsock.h, which clashes with the winsock2.h of wxWidgets
add_compile_definitions(UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS WIN32_LEAN_AND_MEAN NOMINMAX)
if(MSVC)
    add_compile_options(/W3 /utf-8 /MP)
endif()

# the recovery engine: NTFS parsing, recoverability, content check - no GUI
set(ENGINE_SOURCES "${UNDELETE_ENGINE_DIR}/NTFSDrive.cpp" "${UNDELETE_ENGINE_DIR}/MFTRecord.cpp")
if(EXISTS "${UNDELETE_ENGINE_DIR}/FileContents.cpp") # older engines (git HEAD for -WithOld) may not have it
    list(APPEND ENGINE_SOURCES "${UNDELETE_ENGINE_DIR}/FileContents.cpp")
endif()
add_library(undelete_engine STATIC ${ENGINE_SOURCES})
target_include_directories(undelete_engine PUBLIC "${UNDELETE_ENGINE_DIR}")
target_link_libraries(undelete_engine PUBLIC wx::base)

if(UNDELETE_BUILD_APP)
    # An explicit list, not a glob: the source tree may hold stray merge leftovers
    # (e.g. Page_SelectFilesPage.cpp.BACKUP.*.cpp) that must never be compiled. FAT/ isn't linked either -
    # photorecz_stub.c stands in for it (README.md, "FAT support").
    add_executable(GetMyFilesBack WIN32
        "${SRC}/main.cpp"
        "${SRC}/MyWizzard.cpp"
        "${SRC}/MyThreadClass.cpp"
        "${SRC}/DeletedFilesList.cpp"
        "${SRC}/CustomButton.cpp"
        "${SRC}/IsValidFileName.cpp"
        "${SRC}/Page_GreetingPage.cpp"
        "${SRC}/Page_SelectSourceDrivePage.cpp"
        "${SRC}/Page_SelectDestinationDirPage.cpp"
        "${SRC}/Page_SelectFilesPage.cpp"
        "${SRC}/Page_EndPage.cpp"
        "${SRC}/languages/LanguageDialog.cpp"
        "${SRC}/languages/LanguageFactory.cpp"
        "${SRC}/languages/LanguagesImpl.cpp"
        "${SRC}/photorecz_stub.c"
        "${SRC}/undelete.rc")
    target_link_libraries(GetMyFilesBack PRIVATE undelete_engine wx::core wx::base)
    # the resource script embeds these - rebuild it when they change
    set_source_files_properties("${SRC}/undelete.rc" PROPERTIES OBJECT_DEPENDS
        "${SRC}/undelete.exe.manifest;${SRC}/resources/poslednja.ico")
    if(MSVC)
        # undelete.rc embeds undelete.exe.manifest (requireAdministrator) - don't let the linker add a second one
        target_link_options(GetMyFilesBack PRIVATE /MANIFEST:NO)
    endif()
endif()

if(UNDELETE_BUILD_TESTS)
    # runs the engine against a disk image - see tests/ntfs-image/README.md
    add_executable(ntfs_harness "${UNDELETE_HARNESS_SOURCE}")
    target_link_libraries(ntfs_harness PRIVATE undelete_engine wx::core wx::base)
endif()
