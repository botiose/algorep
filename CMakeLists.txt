cmake_minimum_required(VERSION 3.10)
project(algorep)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

set(SERVER_SRC
  src/server.cc
  src/node.cc
  src/messenger.cc
  src/leader-election.cc
  )

set(CLIENT_SRC
  src/client.cc
  )

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  set_source_files_properties(${SRC}
    PROPERTIES COMPILE_FLAGS "-Wall -Wextra -pedantic -Werror -g"
    )
endif ()

add_executable(server
  ${SERVER_SRC}
  )

add_executable(client
  ${CLIENT_SRC}
  )

target_include_directories(client PUBLIC
  "${PROJECT_SOURCE_DIR}/src/include"
  )

target_include_directories(server PUBLIC
  "${PROJECT_SOURCE_DIR}/src/include"
  )

# CROSSCOMPILE CLIENT TARGET
add_custom_target(crosscompile-client
  COMMAND ./../../etc/script/cross-compile.sh client
  )

# CROSSCOMPILE SERVER TARGET
add_custom_target(crosscompile-server
  COMMAND ./../../etc/script/cross-compile.sh server
  )

# CLIENT RUN TARGET
add_custom_target(run-client
  COMMAND printf %37sOUTPUT%37s\\\\n | tr " " "="
  COMMAND ./../../etc/script/launch-clients.sh
  COMMAND printf %80s\\\\n | tr " " "="
  DEPENDS client crosscompile-client
  )

# SERVER RUN TARGET
add_custom_target(run-server
  COMMAND printf %37sOUTPUT%37s\\\\n | tr " " "="
  COMMAND ./../../etc/script/launch-server.sh
  COMMAND printf %80s\\\\n | tr " " "="
  DEPENDS server crosscompile-server
  )

# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)
add_subdirectory(test/extern/googletest EXCLUDE_FROM_ALL)
add_subdirectory(test EXCLUDE_FROM_ALL)
