# Testele traiesc in build/tests (asa se asteapta si CI-ul)
add_executable(${TESTS_EXECUTABLE_NAME} test_model.cpp)
target_link_libraries(${TESTS_EXECUTABLE_NAME} PRIVATE ${MAIN_LIBRARY_NAME})

# aceleasi flaguri/sanitizere ca la biblioteca si executabil (altfel MSVC da LNK2038 cu ASan: biblioteca ASan + teste fara ASan)
set_compiler_flags(RUN_SANITIZERS ${USE_ASAN} TARGET_NAMES ${TESTS_EXECUTABLE_NAME})

add_test(NAME model_tests COMMAND ${TESTS_EXECUTABLE_NAME})
install(TARGETS ${TESTS_EXECUTABLE_NAME} RUNTIME DESTINATION bin)

# Tema 2: teste pentru ierarhia de alerte si pentru exceptii
add_executable(${TESTS_EXECUTABLE_NAME}_tema2 test_tema2.cpp)
target_link_libraries(${TESTS_EXECUTABLE_NAME}_tema2 PRIVATE ${MAIN_LIBRARY_NAME})
set_compiler_flags(RUN_SANITIZERS ${USE_ASAN} TARGET_NAMES ${TESTS_EXECUTABLE_NAME}_tema2)
add_test(NAME tema2_tests COMMAND ${TESTS_EXECUTABLE_NAME}_tema2)
