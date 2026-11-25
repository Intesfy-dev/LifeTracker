cmake_minimum_required(VERSION 3.28)

# NOTE: update EXECUTABLE_NAME in .github/workflows/cmake.yml when changing executable name in this file
# for now, the project name is used as the executable name
set(MAIN_PROJECT_NAME "oop")
set(MAIN_EXECUTABLE_NAME "${MAIN_PROJECT_NAME}_main")
set(MAIN_LIBRARY_NAME "${MAIN_PROJECT_NAME}")
set(TESTS_EXECUTABLE_NAME "${MAIN_PROJECT_NAME}_test")

# needs to be before `project()` to take effect
option(CMAKE_EXPORT_COMPILE_COMMANDS "Export compilation database" ON)

project(${MAIN_PROJECT_NAME})
# set(CMAKE_PROJECT_VERSION_MAJOR 0)
# set(CMAKE_PROJECT_VERSION_MINOR 0)
# set(CMAKE_PROJECT_VERSION_PATCH 1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

include(cmake/Options.cmake)
include(cmake/CompilerFlags.cmake)
include(cmake/CopyHelper.cmake)

include(CTest)
include(cmake/RegisterTests.cmake)

###############################################################################

# External dependencies with FetchContent

# include(FetchContent)
#
# set(FETCHCONTENT_QUIET OFF CACHE INTERNAL "Override verbose output" FORCE)
# set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE INTERNAL "Override update strategy" FORCE)
#
# NOTE: Also update env vars used for caching in
#     - .github/actions/configure-cmake/action.yml
#     - .github/workflows/cmake.yml
# FetchContent_Declare(
#     SomeLib
#     SYSTEM         # keeps ext libs out of static code analysis
#     GIT_REPOSITORY https://github.com/<SomeUser>/<SomeLib>.git
#     GIT_TAG        <some_git_hash> # <which tag/branch @ <some_date>>
#     GIT_SHALLOW    1  # works only with branches or tags, not with arbitrary commit hashes
#     GIT_PROGRESS   ON
# )
#
# FetchContent_MakeAvailable(SomeLib)

###############################################################################

# External dependencies with find_package

# find_package(Threads REQUIRED)

# Qt is optional: with it the GUI is built (+ terminal UI as fallback), without it only the terminal UI (TUI)
find_package(Qt6 COMPONENTS Widgets QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Widgets QUIET)
endif()

###############################################################################

# Configure sources

# always STATIC (simpler); TBD if we ever want dynamic for this; BUILD_SHARED_LIBS is for ext deps
add_library(${MAIN_LIBRARY_NAME} STATIC)

# NOTE: update EXECUTABLE_NAME in .github/workflows/cmake.yml when changing name here
add_executable(${MAIN_EXECUTABLE_NAME}
    app/main.cpp
    app/account_manager.cpp
    app/terminal_ui.cpp
    app/gui_window.cpp
)
target_sources(${MAIN_LIBRARY_NAME}
    PRIVATE
        # domain logic (no UI)
        app/date.cpp
        app/transaction.cpp
        app/money_goal.cpp
        app/task.cpp
        app/budget.cpp
        app/task_tracker.cpp
        app/calendar_event.cpp
        app/calendar.cpp
        app/roadmap_node.cpp
        app/roadmap.cpp
        app/habit.cpp
        app/habit_tracker.cpp
        app/workout_session.cpp
        app/fitness_tracker.cpp
        app/lifetracker.cpp
        # Tema 2: exceptii + ierarhia de alerte (reminder_rule si derivatele ei)
        app/lt_exceptions.cpp
        app/reminder_rule.cpp
        app/budget_alert.cpp
        app/deadline_alert.cpp
        app/habit_alert.cpp
        app/reminder_center.cpp
)

# NOTE: Add all defined targets (e.g. executables, libraries, etc. )
# NOTE: RUN_SANITIZERS is optional, if it's not present it will default to true
set_compiler_flags(RUN_SANITIZERS ${USE_ASAN} TARGET_NAMES ${MAIN_EXECUTABLE_NAME} ${MAIN_LIBRARY_NAME})

###############################################################################

# Configure includes

# app/ needs no include dir of its own: it inherits include/ by linking the library
# (the domain headers (date.h, budget.h, ...) live in app/, so that dir is exposed by the library too)
target_include_directories(${MAIN_LIBRARY_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/app>
)
target_include_directories(${MAIN_LIBRARY_NAME} PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

# use SYSTEM so cppcheck and clang-tidy do not report warnings from these directories
# vendored external deps should be private by default
# a public dep is an interface commitment, shared by app/ and tests/
target_include_directories(${MAIN_LIBRARY_NAME} SYSTEM PUBLIC
    ${EXT_PUBLIC_DIR}
)
target_include_directories(${MAIN_LIBRARY_NAME} SYSTEM PRIVATE
    ${EXT_PRIVATE_DIR}
)

# target_include_directories(${MAIN_EXECUTABLE_NAME} SYSTEM PRIVATE ${<SomeLib>_SOURCE_DIR}/include)

###############################################################################

# Compile definitions

# project root (where the data/ folder lives) - the executable changes the working directory here at startup
target_compile_definitions(${MAIN_EXECUTABLE_NAME} PRIVATE LT_ROOT_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

###############################################################################

# Link libraries

target_link_libraries(${MAIN_EXECUTABLE_NAME} PRIVATE ${MAIN_LIBRARY_NAME})

# target_link_directories(${MAIN_EXECUTABLE_NAME} PRIVATE ${<SomeLib>_BINARY_DIR}/lib)
# target_link_libraries(${MAIN_EXECUTABLE_NAME} <SomeLib>)

if(Qt6_FOUND)
    message(STATUS "Building Life Tracker with Qt6 Emerald GUI + TUI fallback")
    target_compile_definitions(${MAIN_EXECUTABLE_NAME} PRIVATE USE_QT_GUI)
    target_link_libraries(${MAIN_EXECUTABLE_NAME} PRIVATE Qt6::Widgets)
elseif(Qt5_FOUND)
    message(STATUS "Building Life Tracker with Qt5 Emerald GUI + TUI fallback")
    target_compile_definitions(${MAIN_EXECUTABLE_NAME} PRIVATE USE_QT_GUI)
    target_link_libraries(${MAIN_EXECUTABLE_NAME} PRIVATE Qt5::Widgets)
else()
    message(STATUS "Qt not found - Building Life Tracker with Emerald Terminal UI (TUI)")
endif()

###############################################################################

# copy binaries to "bin" folder; these are uploaded as artifacts on each release
# DESTINATION_DIR is set as "bin" in cmake/Options.cmake
install(TARGETS ${MAIN_EXECUTABLE_NAME} DESTINATION ${DESTINATION_DIR})
install(TARGETS ${MAIN_LIBRARY_NAME} ARCHIVE DESTINATION lib)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION include
    FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
)
if(APPLE)
    install(FILES ${CMAKE_SOURCE_DIR}/launcher.command DESTINATION ${DESTINATION_DIR})
endif()

# the domain headers live in app/ (CI copies artifacts/bin/* and artifacts/include/. after
# "cmake --install build --prefix artifacts")
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/app/
    DESTINATION include
    FILES_MATCHING
        PATTERN "*.h"
)
install(FILES app/tastatura.txt DESTINATION ${DESTINATION_DIR})
# PROGRAMS (not FILES) keeps the executable bit of the launcher
install(PROGRAMS launcher.command DESTINATION ${DESTINATION_DIR})
install(DIRECTORY DESTINATION lib)

copy_files(FILES tastatura.txt DIRECTORY assets COPY_TO_DESTINATION TARGET_NAME ${MAIN_EXECUTABLE_NAME})
# copy_files(FILES tastatura.txt config.json DIRECTORY images sounds COPY_TO_DESTINATION TARGET_NAME ${MAIN_EXECUTABLE_NAME})
# copy_files(DIRECTORY images sounds COPY_TO_DESTINATION TARGET_NAME ${MAIN_EXECUTABLE_NAME})

###############################################################################

# Tests; BUILD_TESTING is declared in cmake/Options.cmake, include(CTest) enables it. No framework on main

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
