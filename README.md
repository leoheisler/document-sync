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
1. Install docker: use this link to install docker engine in your ubuntu machine in the easiest way possible -- https://docs.docker.com/engine/install/ubuntu/ ;
2. Navigate to the project root and run 'docker build -t docsync-image .', this command build the image used for the containers in the application, is the same image for all the different agents (server, backup server and clients);
3. To run the container for an agent type: 'docker run -it --name ***container-name*** --network docksync-network --rm docsync-image', the ***container-name*** is self explanatory, give a name that represents the part that the agent will take on you execution (server, backup server, client); 
4. Now inside the container, for each different agent:
   For Client: Execute: './myClient ***client-username*** ***primary-server-container-name*** ***primary-server-port***' <br>
   For Primary Server: './myServer' or './myServer ***port-for-server-to-run***' <br>
   For Backup Servers: './myServer ***primary-server-container-name*** ***primary-server-port***' <br>
