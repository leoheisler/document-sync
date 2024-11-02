# DocumentSync - A Distributed File Sharing and Synchronization Service
## Description:
This project consists of implementing a service similar to Dropbox, enabling automatic file  <br> 
sharing and synchronization between different devices of the same user <br>

To achieve this, this project makes use of:
- Threads;
- Processes;
- Communication using sockets;
- Process synchronization using mutexes and semaphores.

## Build Tutorial
To build and run this project, you can choose one of the following options:

### Option 1: Execute the Build Script
1. Navigate to the project root directory;
2. Make the script executable by running 'chmod +x build.sh';
3. Execute the script './build.sh'.

### Option 2: Execute the commands manually
1. Navigate to the project root directory;
2. Create the build directory if it doesn't exist with 'mkdir -p build';
3. Navigate to the build directory and build the project with 'cd build ; cmake ..; make'

### Running the project
1. Navigate to the build directory
2. FOR CLIENT: Execute './myClient <'username'> <'server_ip_address'> <'port'>'<br>
   FOR SERVER: Execute './myServer'
