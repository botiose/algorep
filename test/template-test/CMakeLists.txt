enable_testing()

set(SRC
  ${CMAKE_CURRENT_LIST_DIR}/template.cc
  )

add_executable(template-test
  ${SRC}
  )

target_include_directories(template-test PUBLIC
  "${PROJECT_SOURCE_DIR}/src/include"
  )

target_link_libraries(template-test PUBLIC gtest gtest_main)

add_test(
  NAME template-test
  COMMAND template-test
  )

