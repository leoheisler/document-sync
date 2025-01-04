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

## **Notes**
- Ensure the `docksync-network` Docker network exists. Create it if necessary:
  ```bash
  docker network create docksync-network
  ```
- Replace placeholders (`<...>`) with appropriate values for your setup.

--- 

### 1. **Install Docker**
Follow the instructions in the [Docker Engine Installation Guide for Ubuntu](https://docs.docker.com/engine/install/ubuntu/) to set up Docker on your machine.

### 2. **Build the Docker Image**
Navigate to the project root directory and build the Docker image:

```bash
docker build -t docsync-image .
```

This image will be used for all application agents (server, backup server, and clients).

### 3. **Run a Container**
To run a container for an agent, use the following command:

```bash
docker run -it --name <container-name> --network docksync-network --rm docsync-image
```

Replace `<container-name>` with a name that represents the agent (e.g., server, backup server, or client).

---

## **Agent Commands**

After entering the container, execute the following commands based on the agent type:

- **Client:**
  ```bash
  ./myClient <client-username> <primary-server-container-name> <primary-server-port>
  ```

- **Primary Server:**
  ```bash
  ./myServer
  ```
  Or, to specify a custom port:
  ```bash
  ./myServer <port-for-server-to-run>
  ```

- **Backup Server:**
  ```bash
  ./myServer <primary-server-container-name> <primary-server-port>
  ```

---

## Copying Files from a Container to Host Using `docker cp`

This method allows you to copy a file or directory from a container to your host machine.

### Steps:

1. Identify the running container's ID or name:

   ```bash
   docker ps

2. Use the docker cp command to copy the file:

   ```bash
   docker cp <container_name_or_id>:<path_in_container> <path_on_host>

### Examples:

To copy a file from the container my_container to your host machine:

   ```bash
   docker cp my_container:/path/to/file /host/path

Kenji Example
   ```bash
   docker cp backup_server:/app/project/src/server/userDirectories/sync_dir_kenji/test.png /home/kenji/Desktop/teste.png




