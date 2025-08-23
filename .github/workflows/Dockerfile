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
    
# Install Android SDK command-line tools and NDK (needed for Android cross builds)
ENV ANDROID_HOME=/usr/local/lib/android/sdk
ENV ANDROID_SDK_ROOT=/usr/local/lib/android/sdk
ENV PATH=$PATH:$ANDROID_HOME/cmdline-tools/latest/bin:$ANDROID_HOME/platform-tools

RUN apt-get update && apt-get install -y --no-install-recommends unzip zip ninja-build && \
    mkdir -p $ANDROID_HOME && \
    cd /tmp && \
    # Download Android command-line tools. Update URL if a newer archive is preferred.
    wget -q https://dl.google.com/android/repository/commandlinetools-linux-9477386_latest.zip -O commandlinetools.zip && \
    mkdir -p $ANDROID_HOME/cmdline-tools && \
    unzip -q commandlinetools.zip -d /tmp && \
    # Move into the expected cmdline-tools/latest layout used by sdkmanager
    mkdir -p $ANDROID_HOME/cmdline-tools/latest && \
    mv /tmp/cmdline-tools/* $ANDROID_HOME/cmdline-tools/latest/ && \
    rm -rf /tmp/commandlinetools.zip /tmp/cmdline-tools && \
    # Accept licenses and install required sdk packages (platform-tools, platform API 23, NDK, CMake, build-tools)
    yes | $ANDROID_HOME/cmdline-tools/latest/bin/sdkmanager --sdk_root=$ANDROID_HOME --licenses && \
    # Use a specific NDK version (newer sdkmanager may not provide 'ndk-bundle'); 'ndk;VERSION' is more reliable.
    $ANDROID_HOME/cmdline-tools/latest/bin/sdkmanager --sdk_root=$ANDROID_HOME "platform-tools" "platforms;android-23" "ndk;25.2.9519653" "cmake;3.22.1" "build-tools;30.0.3" && \
    # Create legacy ndk-bundle symlink for tools/cmake invocations that expect $ANDROID_HOME/ndk-bundle
    if [ ! -d "$ANDROID_HOME/ndk-bundle" ]; then \
      ndkdir=$(ls -d $ANDROID_HOME/ndk/* 2>/dev/null | head -n1 || true); \
      if [ -n "$ndkdir" ]; then ln -s "$ndkdir" $ANDROID_HOME/ndk-bundle; fi; \
    fi && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

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
    
RUN mkdir -p /app


# Set JAVA_HOME environment variable
ENV JAVA_HOME=/usr/lib/jvm/temurin-21-jdk-amd64

# Set the working directory
WORKDIR /app
