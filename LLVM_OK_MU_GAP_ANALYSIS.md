# LLVM Gap Analysis For `examples/ok.mu`

## Scope
This document answers: what is still missing in [main.cpp](main.cpp) to compile [examples/ok.mu](examples/ok.mu) into an executable using LLVM, based on [mu_real_spec.md](mu_real_spec.md).

## Current State (Observed)
- Frontend implemented in one file: lexer + parser + AST printer in [main.cpp](main.cpp).
- Compiler entry point loads a fixed file path (`examples/ok.mu`), parses it, prints AST, then prints token stream in [main.cpp](main.cpp#L1453).
- Parsing pipeline exists: `parse_program` in [main.cpp](main.cpp#L1404), `parse_decl` in [main.cpp](main.cpp#L1268), `parse_expression` in [main.cpp](main.cpp#L1194), `parse_type` in [main.cpp](main.cpp#L906).
- No LLVM IR construction or object emission calls are present in [main.cpp](main.cpp).
- CMake already finds and links LLVM in [CMakeLists.txt](CMakeLists.txt#L11) and [CMakeLists.txt](CMakeLists.txt#L36).

## Feature Coverage For `ok.mu`

### Syntax that currently parses
- Global and local declarations with `::`, `:=`, and typed `:` forms in [main.cpp](main.cpp#L1268).
- Struct declarations and fields in [main.cpp](main.cpp#L1277).
- Function declarations and return types in [main.cpp](main.cpp#L1290).
- Pointer type syntax (`*mut T`) in [main.cpp](main.cpp#L908).
- Array/slice type syntax (`[3] i32`, `[..] i32`, `[]i32`) in [main.cpp](main.cpp#L926).
- `for i : expr { ... }` loop form in [main.cpp](main.cpp#L1221).
- Calls, member access, indexing, assignment ops in [main.cpp](main.cpp#L1085).
- Struct-style literals `. { ... }` via `parse_struct_literal` in [main.cpp](main.cpp#L1002).

### What is still missing for executable generation
The parser success is not enough. The following compiler stages are still missing in [main.cpp](main.cpp):

1. Semantic analysis / type checking
- No symbol table (scopes for globals, params, locals, fields).
- No type resolver (`i32`, `f32`, user structs, pointer/slice/array function types).
- No validation for field access (`p.pos.x`, `arr.count`) and call signatures (`length(v)`, `print(len)`).
- No mutability enforcement (`::` immutable vs `:=` mutable).
- No conversion/coercion checks (int/float arithmetic, return type compatibility).
- No check for unresolved symbols like `sqrt` and `print` (currently only parsed).

2. Lowered typed IR or direct typed AST walk
- Current AST nodes are untyped and text-oriented (see `AstNode` in [main.cpp](main.cpp#L578)).
- Need typed value category info (lvalue/rvalue, pointer base, aggregate layout info).
- Need explicit representation for declaration storage and temporaries.

3. LLVM IR generation
- No `LLVMContext`, `Module`, `IRBuilder`, function/type maps, or basic-block management in [main.cpp](main.cpp).
- No codegen for:
  - function prototypes/definitions
  - local/global variable allocation
  - arithmetic/comparison
  - struct literals and aggregate stores
  - pointer/member/index address computation (`gep`)
  - loops (`for`) and control flow blocks
  - return statements
- No target data layout usage for struct field offsets and ABI correctness.

4. Runtime/intrinsics bridge for `ok.mu`
`ok.mu` requires at least these callable symbols:
- `print(...)` used for scalar output and per-element printing in [examples/ok.mu](examples/ok.mu#L58).
- `sqrt(f32)` used in [examples/ok.mu](examples/ok.mu#L26).

Missing choices to implement:
- Either lower `print` to `printf` (with format selection by type), or provide your own C runtime helpers.
- Either map `sqrt` to LLVM intrinsic / libm symbol and link correctly.

5. Object emission + native linking
- No pass from LLVM module to `.o` file.
- No target-machine setup (`InitializeNativeTarget`, triple/cpu/features, relocation model).
- No link step to executable (invoke system linker or driver command).
- No output artifact naming, temp-file management, or error handling for external tool failures.

6. Driver behavior
- `main()` currently hardcodes input path in [main.cpp](main.cpp#L1456).
- Need CLI options:
  - input file
  - output executable path
  - optional `--emit-ir`, `--emit-obj`, `--run`
- Need mode separation: parse-only vs full compile.

## Spec-to-Implementation Delta Relevant To `ok.mu`
Based on [mu_real_spec.md](mu_real_spec.md), and what `ok.mu` uses now:

- Needed immediately for `ok.mu` executable:
  - structs with fields/defaults
  - pointers (`*mut`) and field mutation through references
  - function definitions and calls
  - array types and indexing
  - for-range form currently written as `0..arr.count-1`
  - basic numeric expressions
  - extern/builtin interop (`print`, `sqrt`)

- Not required to run `ok.mu` first pass (can defer):
  - traits/impl
  - enums/union/match
  - comptime execution (`comp`) semantics
  - optional types and unwrap operators full semantics
  - UFCS lowering rules

## Concrete Work Left In `main.cpp` (Implementation Checklist)

### Phase 1: Add a semantic pass (must-do before LLVM)
- Add plain structs (no classes) for symbol/type metadata.
- Implement scope stack for name resolution.
- Implement `resolve_type(node)` and `check_expr(node)` free functions.
- Annotate AST node ids with resolved types in side tables.
- Validate all declarations and expressions in `ok.mu`.

Suggested structs:
- `TypeInfo { kind, bit_width, is_signed, pointee, fields, element_type, count, params, ret }`
- `SymbolInfo { name, type_id, is_mutable, storage_kind, llvm_value_ptr }`
- `SemaState { vectors/maps for types, symbols, scope stack, errors }`

### Phase 2: LLVM context and module setup
- Add `CodegenState` struct with:
  - context/module/builder pointers
  - target machine/data layout
  - maps from type ids and symbols to LLVM objects
- Build module triple + data layout once.
- Emit declarations for runtime functions (`print_*`, `sqrtf`/intrinsic).

### Phase 3: Emit LLVM IR for AST forms used by `ok.mu`
- Globals: const and mutable variable declarations.
- Struct definitions: create named LLVM struct types and field index maps.
- Functions: prototype then body emission.
- Expressions:
  - literals, identifiers
  - binary arithmetic and assignment family
  - member/index access as addresses + loads/stores
  - call lowering
  - struct/array literals with aggregate initialization
- Statements:
  - declaration statements
  - return
  - for-loop lowering to basic blocks and branch edges

### Phase 4: Produce object + executable
- Initialize LLVM native target and asm printer.
- Emit object file from module.
- Invoke linker (or clang) to produce final executable.
- Link against `libm` if needed for `sqrt` symbol strategy.

### Phase 5: Driver and diagnostics
- Replace hardcoded source path in [main.cpp](main.cpp#L1456) with CLI args.
- Keep parse dump behind debug flag, not default.
- On semantic/codegen/link errors, print source locations and stop with non-zero code.

## Minimal "Get `ok.mu` Running" Plan (Recommended Order)
1. Semantic pass with enough typing for numeric, pointers, structs, arrays, calls.
2. Codegen for globals, functions, expressions, return, for-loop.
3. Builtin runtime bridge for `print` and `sqrt`.
4. Object emission + link.
5. CLI + artifact output handling.

## Acceptance Criteria
`ok.mu` is considered supported when all are true:
- `mucc examples/ok.mu -o ok` produces a native executable.
- Executable runs without compiler crashes.
- `length(v)` and `print(len)` execute correctly.
- Struct field mutation through pointer (`p_ptr.pos.x = 10.0`) works.
- Array indexing inside loop works for `print_array`.

## Important Notes
- Although LLVM is linked in CMake, that only makes APIs available; backend logic is still absent from [main.cpp](main.cpp).
- Current parser behavior proves syntactic coverage, not semantic correctness or executable generation.
