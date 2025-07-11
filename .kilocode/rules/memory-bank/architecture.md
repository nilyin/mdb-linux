# System Architecture

The system is composed of several distinct modules, each with a specific responsibility. The modules are orchestrated by the main build system defined in the root `CMakeLists.txt`.

## Core Modules

- **`mdv_core`**: The central module that likely orchestrates the other components and contains the primary business logic of the database.
- **`mdv_storage`**: The storage engine layer. As per `tech.md`, this is an extensible backend that currently uses LMDB. This module handles the physical storage and retrieval of data.
- **`mdv_net`**: Manages the peer-to-peer networking. According to `brief.md`, this includes the gossip protocol for node discovery and data synchronization.
- **`mdv_platform`**: Provides platform-specific abstractions to ensure cross-platform compatibility (Linux, Android, etc.).
- **`mdv_crypto`**: Handles encryption and cryptographic operations. `tech.md` mentions AES encryption.
- **`mdv_types`**: Defines the core data structures and types used throughout the system.

## API and Client Modules

- **`mdv_api`**: Defines the public API for interacting with the database.
- **`mdv_client`**: A client implementation that uses the `mdv_api` to communicate with a medveddb instance.
- **`mdv_bindings`**: Generates language bindings, specifically for Java (JNI) using SWIG, as noted in `tech.md`.

## Service and Testing

- **`mdv_service`**: Implements a standalone network server, allowing the database to run as a service.
- **`mdv_tests`**: Contains the test suite for the project to ensure code quality and correctness.

## Third-party Dependencies

- **`mdv_thirdparty`**: Contains all third-party libraries like LMDB, AES, etc.

## Build and Deployment

- **`CMakeLists.txt`**: The root build file defines how all modules are compiled and linked.
- **`docker/`**: Contains Docker configurations for containerized deployment.
- **`assets/`**: Contains assets for packaging, such as DEB package scripts and systemd service files. It also has server configuration file: `assets/conf/medved.conf`

This modular architecture allows for clear separation of concerns and facilitates maintenance and extension of the system.