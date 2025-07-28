# Use the Debian Bookworm slim image as a base (supports JDK 21)
FROM debian:bookworm-slim
ARG NAME="medveddb-test"
ARG VERSION="1"
LABEL Name="$NAME" \
      Version="$VERSION"

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
    gcc \
    gcc-multilib \
    libc6-dev \
    libc6-dbg \
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

# Install debugging tools for C89 applications
RUN apt-get update && apt-get install -y --no-install-recommends \
    gdb \
    gdb-multiarch \
    valgrind \
    strace \
    ltrace \
    binutils \
    binutils-dev \
    libc6-dbg \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*


# configure SSH for communication with Visual Studio 
RUN apt-get update && apt-get install -y openssh-server
# Create the directory for SSH daemon to run
RUN mkdir -p /var/run/sshd
# Set the root password and modify SSH configuration
RUN echo 'root:root' | chpasswd \
    && sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config \
    && sed 's@session\s*required\s*pam_loginuid.so@session optional pam_loginuid.so@g' -i /etc/pam.d/sshd
    
RUN mkdir -p /app


# Set JAVA_HOME environment variable
ENV JAVA_HOME=/usr/lib/jvm/temurin-21-jdk-amd64

# Set the working directory
WORKDIR /app

# Copy the project files into the container
COPY . .

# Define the build command
CMD ["sh", "-c", "mkdir -p build && cd build && rm -rf CMakeCache.txt CMakeFiles && cmake .. && cmake --build . && ./mdv_tests/mdv_tests && /usr/sbin/sshd -D"]

EXPOSE 22