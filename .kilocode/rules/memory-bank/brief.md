Building a peer-to-peer distributed database system designed for IoT and edge computing environments. It provides automatic data synchronization across hundreds of thousands of nodes without requiring dedicated master nodes or coordinators.

**Core Architecture:**
- **Backend Wrapper**: Currently supports LMDB storage engine, with SQLite integration planned
- **P2P Mesh Network**: Fully distributed topology with automatic node discovery via gossip protocol
- **Synchronization Engine**: Provides eventual consistency with Write-Ahead Log (WAL) based data synchronization with ACID compliance due to LMDB
- **Network Components**: Includes Network Listener, Topology Tracker, Data Synchronizer, and Data Fetcher/Committer modules

**Key Features:**
- **Lightweight**: 1MB footprint with fast local read/write operations
- **Cross-platform**: Linux, Android support with C bindings and Java compatibility  
- **Real-time**: Low-latency data streaming for connected devices and vehicles
- **Scalable**: Handles massive IoT deployments (automotive sensors generating 3-40 Gbits/hour)
- **Fault-tolerant**: No single point of failure, automatic peer discovery and synchronization
