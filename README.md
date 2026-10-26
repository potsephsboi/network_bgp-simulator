Network/BGP Simulator

Work in progress. C++ simulator with one router per autonomous system. 

Goal
Create a GUI where users can:
- Configure network topology.
- Press Run to start distributed routing and build/update routing tables.
- View any router’s routing table.
Implement some of the following:
Inter-AS policies, dynamic topology changes, and the actual BGP wire protocol

Current functionality
- CLI to request and display a router’s routing table by ID.
- Static, predefined topology.
- The routing logic follows a simplified, BGP-inspired path vector algorithm approach.

Architecture
The parent process creates starts a separate process for each router using fork()
and execv(). Each router maintains its own routing table and communicates with its
neighbors and the parent, through Unix-domain socket pairs.

Routers use poll() to wait for incoming messages. Dedicated parent–router sockets 
carry routing-table requests and responses, while the parent handles CLI input.