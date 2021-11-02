################################# DESCRIPTION ##################################

This project is an implementation of both a log replicating server and associate
client. Communication in between server nodes and the client programs is done
through the use of the Open MPI API https://www.open-mpi.org/.

#################################### CONFIG ####################################

The project was initially configured to make the execution of the server on
multiple machines as straightforward as possible. And so the scripts for
launching and cross-compiling the binaries were written with this in mind. These
scripts all read from a common config file located in 'etc/config.json'. The
file itself specifies the computer network in which the server and clients are
meant to run on as well as their respective cpu architecture. The following is a
example format for this file:

{
  "nodes": {
    "ARCHITECTURE": [
      {
        "address": "localhost",
        "serverProcessCount": 5,
        "clientProcessCount": 3,
        "projectPath": "PROJECT_PATH"
      }
    ]
  }
}

The number of server nodes and client processes to run is specified by modifying
the respective "serverProcessCount" and "clientProcessCount" fields to indicate
the number of processes.

#################################### BUILD #####################################

To generate the build directory use the following command from the root
directory with either the "debug" or "release" arguments to respectively build
to project in debug or release mode.

./etc/script/gen-build.sh [build/release]

If no argument is given the project will be build in debug mode by default.

The generated directory will contain a single subdirectory named after the
output of the 'lscpu' command. The usual build files will be located under this
one.

#################################### USAGE #####################################

The binary can be manually compiled with:

$ make

For communication in between the server and clients an ompi-server instance
needs to be running in the background, this one should be launched with:

$ ompi-server --no-daemonize --report-uri etc/urifile

The server/client can be run with:

$ make run-<server/client>

And their respective repl can be started with:

$ make repl-<server/client>

##################################### REPL #####################################

Both server and client repls support the following commands:

- start: starts all server nodes or all clients.

- shutdown: shuts down the all server nodes or all client.

- id,speed-<low/medium/high>: sets the respective speed the given server node or
  client.

- nodeId,crash: simulates a node crash on the server.

- nodeId,recovery: simulates a node recovery on the server.

The following is an example of valid repl inputs for the server:

> start
> 1,crash
> 2,crash
> 1,recover
> 3,speed-low
> 4,speed-medium
> 2,recover
> shutdown

################################ DOCUMENTATION #################################

The report can be found in doc/doc.pdf

Doxygen was used for the documentation. To create and open the documentation in
a firefox browser use:

$ make doc-view

To view class diagrams 'graphviz' is required. On a debian based OS this can be
installed with:

$ sudo apt install graphviz
