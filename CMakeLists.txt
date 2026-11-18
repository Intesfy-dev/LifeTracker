cmake_minimum_required(VERSION 3.16)
project(life_tracker CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Widgets QUIET)
if(NOT Qt6_FOUND)
  find_package(Qt5 COMPONENTS Widgets QUIET)
endif()

set(CORE_SOURCES
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
)

set(APP_SOURCES
  app/main.cpp
  app/account_manager.cpp
  app/terminal_ui.cpp
  app/gui_window.cpp
)

include(CTest)

# Logica de domeniu (fara UI) - folosita de aplicatie si de teste
add_library(lt_core STATIC ${CORE_SOURCES})
target_include_directories(lt_core PUBLIC app)

add_executable(life_tracker ${APP_SOURCES})
target_link_libraries(life_tracker PRIVATE lt_core)
# Radacina proiectului (unde e folderul data/) - executabilul face chdir aici la pornire
target_compile_definitions(life_tracker PRIVATE LT_ROOT_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

if(Qt6_FOUND)
  message(STATUS "Building Life Tracker with Qt6 Emerald GUI + TUI fallback")
  target_compile_definitions(life_tracker PRIVATE USE_QT_GUI)
  target_link_libraries(life_tracker PRIVATE Qt6::Widgets)
elseif(Qt5_FOUND)
  message(STATUS "Building Life Tracker with Qt5 Emerald GUI + TUI fallback")
  target_compile_definitions(life_tracker PRIVATE USE_QT_GUI)
  target_link_libraries(life_tracker PRIVATE Qt5::Widgets)
else()
  message(STATUS "Qt not found — Building Life Tracker with Emerald Terminal UI (TUI)")
endif()

if(BUILD_TESTING)
  add_executable(life_tracker_tests tests/test_model.cpp)
  target_link_libraries(life_tracker_tests PRIVATE lt_core)
  add_test(NAME model_tests COMMAND life_tracker_tests)
endif()

# CI-ul copiaza artifacts/bin/* dupa "cmake --install build --prefix artifacts"
install(TARGETS life_tracker RUNTIME DESTINATION bin)
install(DIRECTORY app/ DESTINATION include FILES_MATCHING PATTERN "*.h")
install(DIRECTORY DESTINATION lib)
