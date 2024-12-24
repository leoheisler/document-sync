# Use the official Ubuntu 22.04 image as the base
FROM ubuntu:22.04

# Set environment variables to avoid prompts during installation
ENV DEBIAN_FRONTEND=noninteractive

# Install necessary dependencies: build-essential, CMake, g++, git, and others
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    g++ \
    git \
    bash \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory inside the container
WORKDIR /app

# Copy the entire project into the container
COPY . /app/project

# Create the necessary directories for the application (adjusted for the new directory structure)
RUN mkdir -p /app/project/src/server/userDirectories

# Clean up previous builds if any and create the build directory
RUN rm -rf /app/project/build && mkdir /app/project/build && cd /app/project/build && cmake .. && make

# Set the default command to run a bash shell (so the container stays interactive)
CMD ["bash"]