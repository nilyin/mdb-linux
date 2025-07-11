# Use the Debian Bullseye slim image as a base
FROM debian:bullseye-slim

# Install necessary build dependencies and tools
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    wget \
    curl \
    netcat-openbsd \
    openjdk-11-jdk \
    doxygen \
    graphviz \
    flex \
    bison \
    libpcre3-dev \
    libpcre2-dev \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Install SWIG 4.3.1
RUN cd /tmp \
    && wget -q http://prdownloads.sourceforge.net/swig/swig-4.3.1.tar.gz \
    && tar -xzf swig-4.3.1.tar.gz \
    && cd swig-4.3.1 \
    && ./configure --prefix=/usr/local \
    && make -j$(nproc) \
    && make install \
    && cd / \
    && rm -rf /tmp/swig-4.3.1*

# Set JAVA_HOME environment variable
ENV JAVA_HOME=/usr/lib/jvm/java-11-openjdk-amd64

# Set the working directory
WORKDIR /app

# Copy the project files into the container
COPY . .

# Define the build command
CMD ["sh", "-c", "mkdir -p build && cd build && cmake .. && cmake --build . && ./mdv_tests/mdv_tests"]