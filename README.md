# Parallel Actors

Parallel Actors is a server-client application for the training of self-learning navigation machines.

The server acts as a central map and manages client movement and vision updates.

Clients connect to the server, which provides each client with its own position, destination, and vision data. The client then uses this information to determine its next move, and sends this to the server.

The server will also inform the client if it has collided, arrived at its destination, or timed-out (no next-move packet within a given time step).

This project has two main branches - master, and MPICH. The master branch implements a serial server, while the MPICH branch uses MPICH to split the server into an arbitrary number of map segments managed by separate processes.

## Installation

This project has the following dependencies
- **C++**
- **SFML** (for the GUI).
- **MPICH** (for the MPICH branch **ONLY** - you will need to enable multi-threading during your MPICH configuration)

Installation is simple -

1) Install all dependencies
2) Clone the repository onto your machine
3) Navigate to the repository folder
4) Enter `make`

## Usage

- `./server` to launch the server
- `./client` to launch a client. The default Client will simply move towards its destination, regardless of obstacles or other Actors.

The server and the clients, by default, communicate over the loopback interface on port 3000.

#### Helper Scripts
To use the helper scripts, you will need to ensure that *kill\_clients.sh* and *launch\_clients.sh* are executable. You can do this using `chmod +x <scriptname>`.

*kill\_clients.sh* is used to pkill all running clients. Be careful - the script will kill any non-protected process with 'client' in the process name.

Use this script with `./kill_clients.sh`

*launch\_clients.sh* will launch \<*number*> clients in the background. Useful for testing your own Client code or to see how clients interact on the map.

Use this script with `./launch_clients.sh <number>`

For example, to launch 100 clients, type `./launch_clients.sh 100`

## Contributing

1. Fork it!
2. Create your feature branch: `git checkout -b my-new-feature`
3. Commit your changes: `git commit -am 'Add some feature'`
4. Push to the branch: `git push origin my-new-feature`
5. Submit a pull request :D
