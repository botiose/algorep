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
  src/log-file-manager.cc
  src/manager/consensus-manager.cc
  src/manager/election-manager.cc
  src/manager/client-manager.cc
  src/manager/failure-manager.cc
  src/manager/repl-manager.cc
  )

set(CLIENT_COMMON_SRC
  src/message-receiver.cc
  src/receiver-manager.cc
  src/manager/repl-manager.cc
  src/client.cc
  src/messenger.cc
  src/message.cc
  src/message-info.cc
  )

set(CLIENT_SRC
  src/client-main.cc
  ${CLIENT_COMMON_SRC}
  )

set(SHUTDOWN_CLIENT_SRC
  src/shutdown-client-main.cc
  ${CLIENT_COMMON_SRC}
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

add_executable(shutdown-client
  ${SHUTDOWN_CLIENT_SRC}
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

target_include_directories(shutdown-client PUBLIC
  ${COMMON_INCLUDES}
  )

add_custom_target(etc-link
  [ ! -d etc ] && ln -s ${PROJECT_SOURCE_DIR}/etc ${PROJECT_BINARY_DIR}/etc ||
  exit 0
  )

add_custom_target(clear-repl-client
  COMMAND rm -rf etc/client && mkdir -p etc/client
  COMMAND echo 0 > etc/turn.txt
  DEPENDS etc-link
  )

add_custom_target(clear-repl-server
  COMMAND rm -rf etc/server && mkdir -p etc/server/log
  COMMAND > etc/server/repl.txt
  DEPENDS etc-link
  )

# CLIENT RUN TARGET
add_custom_target(run-client
  COMMAND printf %37sOUTPUT%37s\\\\n | tr " " "="
  COMMAND ./etc/script/launch-clients-no-dep.sh
  COMMAND tail -f etc/client/00/repl.txt | sed '/shutdown/ q' > /dev/null
  COMMAND printf %80s\\\\n | tr " " "="
  DEPENDS client etc-link clear-repl-client
  )

# SERVER RUN TARGET
add_custom_target(run-server
  COMMAND printf %37sOUTPUT%37s\\\\n | tr " " "="
  COMMAND ./../../etc/script/launch-server-no-dep.sh
  COMMAND printf %80s\\\\n | tr " " "="
  COMMAND printf %35sLOG-CKSUM%36s\\\\n | tr " " "="
  COMMAND cksum etc/server/log/* || exit 0
  COMMAND printf %80s\\\\n | tr " " "="
  DEPENDS server etc-link shutdown-client clear-repl-server
  )

# SERVER REPL TARGET
add_custom_target(repl-server
  COMMAND printf %35sREPL:SERVER%34s\\\\n | tr " " "="
  COMMAND ./etc/script/server-repl.sh
  COMMAND printf %80s\\\\n | tr " " "="
  DEPENDS etc-link
  )

# SERVER REPL TARGET
add_custom_target(repl-client
  COMMAND printf %35sREPL:CLIENT%34s\\\\n | tr " " "="
  COMMAND ./etc/script/client-repl.sh
  COMMAND printf %80s\\\\n | tr " " "="
  DEPENDS etc-link
  )

# DOCUMENTATION SYMBOLIC LINK
add_custom_target(doc-link
  [ ! -d doc ] && ln -s ${PROJECT_SOURCE_DIR}/doc ${PROJECT_BINARY_DIR}/doc ||
  exit 0
  )

# DOCUMENTATION GENERATE
add_custom_target(doc-gen
  COMMAND doxygen doc/Doxyfile 1> /dev/null
  DEPENDS doc-link
  )

# DOCUMENTATION OPEN
add_custom_target(doc-view
  # firefox --new-window html/index.html
  firefox html/index.html
  DEPENDS doc-gen
  )

