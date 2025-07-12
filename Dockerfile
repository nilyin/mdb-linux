# Use the Debian Bookworm slim image as a base (supports JDK 21)
FROM debian:bookworm-slim

# Install necessary build dependencies and tools
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    wget \
    curl \
    netcat-openbsd \
    ca-certificates \
    gnupg \
    doxygen \
    graphviz \
    flex \
    bison \
    libpcre3-dev \
    libpcre2-dev \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Install Eclipse Temurin JDK 21
RUN wget -O - https://packages.adoptium.net/artifactory/api/gpg/key/public | apt-key add - \
    && echo "deb https://packages.adoptium.net/artifactory/deb $(awk -F= '/^VERSION_CODENAME/{print$2}' /etc/os-release) main" | tee /etc/apt/sources.list.d/adoptium.list \
    && apt-get update \
    && apt-get install -y temurin-21-jdk \
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

# Install debugging tools
RUN apt-get update && apt-get install -y --no-install-recommends \
    gdb \
    valgrind \
    strace \
    ltrace \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Set JAVA_HOME environment variable
ENV JAVA_HOME=/usr/lib/jvm/temurin-21-jdk-amd64

# Set the working directory
WORKDIR /app

# Copy the project files into the container
COPY . .

# Define the build command
CMD ["sh", "-c", "mkdir -p build && cd build && cmake .. && cmake --build . && ./mdv_tests/mdv_tests"]