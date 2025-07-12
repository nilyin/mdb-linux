# MedvedDB Future Architecture Plan (Updated)

## SQL Virtual Machine Module - Revised Design
```mermaid
flowchart LR
    Client --> SQL_VM["SQL VM (Local Only)"]
    SQL_VM --> Storage[(Local Storage)]
    Storage -->|Fully Replicated| Data[(Node Data)]
```

### Key Changes:
1. **Localized Execution**:
   - All queries execute against local storage only
   - No distributed query processing needed
   - Leverages database's built-in synchronization

2. **Simplified Architecture**:
   - Removed distributed query planner components
   - Eliminated network-aware optimization

3. **Updated Performance Targets**:
   - Point queries: <0.5ms latency (local)
   - Batch inserts: 2M rows/sec (local)
   - Analytical queries: 500K rows/sec/core

## Conflict Resolution System
(Remains unchanged from previous documentation)