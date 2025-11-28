stateDiagram-v2
    [*] --> Absent
    Absent --> Present: add(timestamp)
    Present --> Removed: remove(timestamp)
    Removed --> Present: add(new_timestamp) if new_timestamp > removal_timestamp
```

### State Transitions Logic
1. **Add Operation**: 
   - When current timestamp > removal timestamp → Transition to Present
   - Else → Discard operation
2. **Remove Operation**:
   - Only valid if element exists → Transition to Removed
3. **Re-add Operation**:
   - Requires newer timestamp than removal → Transition back to Present