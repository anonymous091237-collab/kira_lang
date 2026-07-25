# kira_lang

A compiled language that lowers source code through a custom binary IR format down to LLVM IR, then to a native executable via Clang.

> **Note:** Semantic analysis is not yet implemented.

---

## Dependencies

- A C++ compiler — project is configured for **GCC**
- **Clang** — required to compile the final `.ll` file into an executable

---

## Project Structure

```
kira_lang/
├── generate_IR/   # parses .k source and emits binary IR
├── compile_IR/    # compiles binary IR to LLVM IR (.ll) and invokes Clang
├── view_IR/       # disassembles binary IR to human-readable text
└── include/       # shared headers
```

---

## Building

Run from the project root:

```bash
cmake -B build
cmake --build build
```

Each tool outputs its executable into its own `build/` subfolder.

---

## Usage

### 1 — Generate IR from source

```bash
path_to_project/kira_lang/generate_IR/build/main.exe  <k_build/result>  <source.k>
```

| Argument | Description |
|---|---|
| `k_build/result` | Output path prefix — produces `resultIR.bin` |
| `source.k` | Path to the `.k` source file |

---

### 2 — Compile IR to executable

```bash
path_to_project/kira_lang/compile_IR/build/kllvm.exe  <k_build/resultIR.bin>  <executable_name>
```

| Argument | Description |
|---|---|
| `resultIR.bin` | Binary IR file produced by step 1 |
| `executable_name` | Name for the output executable (no extension) |

---

### 3 — View IR (optional)

Disassemble the binary IR into human-readable text:

```bash
path_to_project/kira_lang/view_IR/build/main.exe  <k_build/resultIR.bin>  [output_file]
```

`output_file` is optional — omit it to print to stdout.

---

## Example

Compile `test/testcode.k` and run the result:

```bash
# Step 1 — generate IR
path_to_project/kira_lang/generate_IR/build/main.exe  test/k_build/result  test/testcode.k

# Step 2 — compile to executable
path_to_project/kira_lang/compile_IR/build/kllvm.exe  test/k_build/resultIR.bin  app

# Step 3 — run
test/k_build/app
```