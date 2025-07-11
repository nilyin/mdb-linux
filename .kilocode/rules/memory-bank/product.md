# Product: MedvedDB

## Problem Solved

MedvedDB addresses the challenge of reliable data storage and synchronization in highly distributed, often-resourced-constrained environments like IoT and edge computing. Traditional databases often rely on centralized coordinators, which introduces a single point of failure and a performance bottleneck, making them unsuitable for large-scale, decentralized deployments.

## How It Works

MedvedDB is a peer-to-peer (P2P) database with no master node. Every node in the network is equal and can accept reads and writes.

1.  **Node Discovery**: When a new node comes online, it uses a gossip protocol to discover other nodes in the network and establish connections, forming a resilient mesh network.
2.  **Data Synchronization**: Data is synchronized across the network using a Write-Ahead Log (WAL). When a node receives a write, it records the change in its log and then propagates it to its peers. This ensures eventual consistency across the entire database.
3.  **Local Storage**: Each node uses a local storage engine (currently LMDB) for fast, transactional, local data access. The ACID properties of LMDB provide reliability at the node level.
4.  **Conflict Resolution**: (This part is an assumption, as it's not explicitly mentioned yet, but it's a critical part of a multi-master system. This is an area for further investigation.) The system needs a mechanism to resolve write conflicts that can occur when different nodes write to the same data concurrently.

## User Experience Goals

- **For Developers**: To provide a simple, lightweight, and embeddable database that "just works" in a distributed environment without complex configuration. The C API and Java bindings are intended to make integration straightforward.
- **For Operators**: To offer a fault-tolerant and scalable system that can be deployed as a standalone server or a service, with easy packaging (DEB/RPM/Docker) and minimal maintenance.