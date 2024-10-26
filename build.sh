#!/bin/bash

# Create a build directory if it doesn't exist
mkdir -p build

# Navigate to the build directory and build the project
cd build ; cmake ..

# Run the project
make run ; cd ..