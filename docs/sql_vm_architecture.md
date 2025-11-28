flowchart TB
    subgraph SQL_VM["SQL Virtual Machine"]
        direction TB
        P[Parser] -->|AST| O[Optimizer]
        O -->|Logical Plan| C[Code Generator]
        C -->|Bytecode| E[Execution Engine]
        E --> S[Storage Interface]
        
        subgraph Execution_Engine
            direction LR
            I[Interpreter] --> C[Cache]
            J[JIT Compiler] --> N[Native Code]
        end
    end
    
    Client -->|SQL Queries| SQL_VM
    SQL_VM -->|Results| Client
    S -->|Data Access| Storage[(LMDB)]
    Storage -->|Sharded| S1[Shard 1]
    Storage -->|Sharded| S2[Shard 2]
```

### Component Responsibilities
1. **Parser**:
   - Converts SQL to Abstract Syntax Tree (AST)
   - File: `src/sql/parser.c`

2. **Optimizer**:
   - Transforms logical plan
   - Applies rewrite rules
   - File: `src/sql/optimizer.c`

3. **Code Generator**:
   - Produces bytecode for VM
   - File: `src/sql/codegen.c`

4. **Execution Engine**:
   - Interprets bytecode
   - JIT compiles hot paths
   - File: `src/sql/execution.c`