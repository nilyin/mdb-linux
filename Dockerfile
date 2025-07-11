# Use the Debian Bullseye slim image as a base
FROM debian:bullseye-slim

# Install necessary build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /app

# Copy the project files into the container
COPY . .

# Define the build command
CMD ["sh", "-c", "mkdir -p build && cd build && cmake .. && cmake --build . && ./mdv_tests/mdv_tests"]