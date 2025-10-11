# Introduction
A simple cross-platform TCP server that communicates with a client in the same network.
This code was developed in C++ using Qt library in order to control a robot (server) by a smartphone. Server accepts sockets from the client and acts accordingly.

# Dependencies
Qt 5.x

# Usage
When the server starts, any incoming socket data will be printed in the Qt console and an acknowledgement will be sent back to the client.
It can be tested with [this free TCP packet sender software](https://www.packetsender.com).
  
