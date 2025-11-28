flowchart LR
    A[Conflict Detected] --> B[Check Vector Clock]
    B --> C{Clock Comparison}
    C -->|A > B| D[Apply Change A]
    C -->|B > A| E[Apply Change B]
    C -->|Concurrent| F[Invoke Custom Resolver]
    F --> G[Merge Operations]
    F --> H[Select Newest]
```

### Conflict Resolution Types
| Type | Implementation File | Logic |
|------|---------------------|-------|
| Priority-based | `mdv_storage/conflict_priority.c` | Highest priority node wins |
| Content-aware | `mdv_storage/conflict_semantic.c` | Field-level merge for JSON |
| Hybrid | `mdv_storage/conflict_hybrid.c` | Priority + semantic rules |
| Last-write-wins | `mdv_storage/conflict_lww.c` | Highest timestamp wins |