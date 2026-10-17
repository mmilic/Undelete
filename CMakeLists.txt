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
set(UNDELETE_ENGINE_DIR "${SRC}" CACHE PATH "Directory with the engine sources (NTFSDrive.cpp, FATDrive.cpp...)")
set(UNDELETE_HARNESS_SOURCE "${CMAKE_SOURCE_DIR}/tests/ntfs-image/harness.cpp" CACHE FILEPATH "The test harness source")
option(UNDELETE_BUILD_APP "Build the GUI app" ON)
option(UNDELETE_BUILD_TESTS "Build the disk image test harness (NTFS and FAT)" ON)

# WIN32_LEAN_AND_MEAN: a full <windows.h> brings the old winsock.h, which clashes with the winsock2.h of wxWidgets
add_compile_definitions(UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS WIN32_LEAN_AND_MEAN NOMINMAX)
if(MSVC)
    add_compile_options(/W3 /utf-8 /MP)
endif()

# the recovery engine: NTFS and FAT parsing, recoverability, content check - no GUI
set(ENGINE_SOURCES "${UNDELETE_ENGINE_DIR}/NTFSDrive.cpp" "${UNDELETE_ENGINE_DIR}/MFTRecord.cpp")
# older engines (git HEAD for -WithOld) may not have these
foreach(ENGINE_FILE FileContents.cpp RecoveryDrive.cpp FATDrive.cpp IsValidFileName.cpp)
    if(EXISTS "${UNDELETE_ENGINE_DIR}/${ENGINE_FILE}")
        list(APPEND ENGINE_SOURCES "${UNDELETE_ENGINE_DIR}/${ENGINE_FILE}")
    endif()
endforeach()
add_library(undelete_engine STATIC ${ENGINE_SOURCES})
target_include_directories(undelete_engine PUBLIC "${UNDELETE_ENGINE_DIR}")
target_link_libraries(undelete_engine PUBLIC wx::base)

if(UNDELETE_BUILD_APP)
    # An explicit list, not a glob: the source tree may hold stray merge leftovers
    # (e.g. Page_SelectFilesPage.cpp.BACKUP.*.cpp) that must never be compiled. FAT/ (TestDisk) isn't
    # linked either - FAT drives are read by the engine's own FATDrive.cpp (README.md, "FAT support").
    add_executable(GetMyFilesBack WIN32
        "${SRC}/main.cpp"
        "${SRC}/MainFrame.cpp"
        "${SRC}/RecoverDialog.cpp"
        "${SRC}/Widgets.cpp"
        "${SRC}/Theme.cpp"
        "${SRC}/FileDetailsPanel.cpp"
        "${SRC}/DeletedFilesList.cpp"
        "${SRC}/MyThreadClass.cpp"
        "${SRC}/Volumes.cpp"
        "${SRC}/AppLog.cpp"
        "${SRC}/languages/LanguageDialog.cpp"
        "${SRC}/languages/LanguageFactory.cpp"
        "${SRC}/languages/GermanTexts.cpp"
        "${SRC}/languages/Translate.cpp"
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
    # runs the engine against a disk image - see tests/ntfs-image/README.md and tests/fat-image/README.md
    add_executable(engine_harness "${UNDELETE_HARNESS_SOURCE}")
    target_link_libraries(engine_harness PRIVATE undelete_engine wx::core wx::base)
endif()
