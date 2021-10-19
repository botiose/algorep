cmake_minimum_required(VERSION 3.10)
project(algorep)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

set(SERVER_SRC
  src/server-main.cc
  src/node.cc
  src/messenger.cc
  src/message.cc
  src/message-info.cc
  src/message-receiver.cc
  src/receiver-manager.cc
  src/manager/consensus-manager.cc
  src/manager/election-manager.cc
  src/manager/client-manager.cc
  src/manager/failure-manager.cc
  src/manager/repl-manager.cc
  )

set(CLIENT_SRC
  src/client-main.cc
  src/client.cc
  src/messenger.cc
  src/message.cc
  src/message-info.cc
  )

set(CONN_SRC
  src/main.cc
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

set(COMMON_INCLUDES
  "${PROJECT_SOURCE_DIR}/extern/json"
  "${PROJECT_SOURCE_DIR}/src/include"
  "${PROJECT_SOURCE_DIR}/src/include/manager"
  )

target_include_directories(client PUBLIC
  ${COMMON_INCLUDES}
  )

target_include_directories(server PUBLIC
  ${COMMON_INCLUDES}
  )

# CROSSCOMPILE CLIENT TARGET
add_custom_target(crosscompile-client
  COMMAND ./../../etc/script/cross-compile.sh client
  )

# CROSSCOMPILE SERVER TARGET
add_custom_target(crosscompile-server
  COMMAND ./../../etc/script/cross-compile.sh server
  )

add_custom_target(etc-link
  [ ! -d etc ] && ln -s ${PROJECT_SOURCE_DIR}/etc ${PROJECT_BINARY_DIR}/etc ||
  exit 0
  )

add_custom_target(clear-repl
  > etc/repl-msg.txt
  DEPENDS etc-link
  )

# CLIENT RUN TARGET
add_custom_target(run-client
  COMMAND printf %37sOUTPUT%37s\\\\n | tr " " "="
  COMMAND ./../../etc/script/launch-clients.sh
  COMMAND printf %80s\\\\n | tr " " "="
  DEPENDS client crosscompile-client etc-link clear-repl
  )

# SERVER RUN TARGET
add_custom_target(run-server
  COMMAND printf %37sOUTPUT%37s\\\\n | tr " " "="
  COMMAND ./../../etc/script/launch-server.sh
  COMMAND printf %80s\\\\n | tr " " "="
  DEPENDS server crosscompile-server etc-link clear-repl
  )

add_subdirectory(test EXCLUDE_FROM_ALL)
