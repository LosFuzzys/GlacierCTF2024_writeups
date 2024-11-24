const std = @import("std");
const ArrayList = std.ArrayList;
const testing = std.testing;
const json = std.json;
const raw_notes = @embedFile("notes.json");

pub const Note = struct {
    title: []const u8,
    content: []const u8,
};

fn ComptimeSet(comptime T: type) type {
    return struct {
        const Self = @This();
        items: []const T = &.{},

        inline fn contains(comptime self: *Self, comptime item: T) bool {
            for (self.items) |i| {
                if (std.mem.eql(@TypeOf(i[0]), i, item)) {
                    return true;
                }
            }
            return false;
        }

        pub inline fn append(comptime self: *Self, comptime item: T) void {
            if (self.contains(item)) {
                return;
            }
            comptime self.items = self.items ++ &[_]T{item};
        }
    };
}

fn findSubstitutions(slice: []const u8, subs: *ComptimeSet([]const u8)) void {
    var curr = 0;
    while (curr < slice.len) {
        var start_idx = std.mem.indexOf(u8, slice[curr..], "@");
        if (start_idx == null) {
            break;
        }
        start_idx.? += curr;
        var end_idx = std.mem.indexOf(u8, slice[start_idx.? + 1 ..], "@");
        if (end_idx == null) {
            break;
        }
        end_idx = end_idx.? + start_idx.? + 1;
        subs.*.append(slice[start_idx.? + 1 .. end_idx.?]);
        curr = end_idx.? + 1;
    }
}

fn addZ(comptime length: usize, value: [length]u8) [length:0]u8 {
    var terminated_value: [length:0]u8 = undefined;
    terminated_value[length] = 0;
    @memcpy(&terminated_value, &value);
    return terminated_value;
}

pub const notes = x: {
    @setEvalBranchQuota(std.math.maxInt(u32));

    var buf: [0x300]u8 = undefined;
    var fba = std.heap.FixedBufferAllocator.init(&buf);

    // This just parses the notes json at compiletime.
    // All the code is mainly there, because we can only parse "well-defined" (known size) types at comptime.
    // See https://github.com/ziglang/zig/issues/1292

    const _Notes = struct { size: u32 };
    const res = json.parseFromSliceLeaky(_Notes, fba.allocator(), raw_notes, .{
        .ignore_unknown_fields = true,
    }) catch @compileError("Failed to parse json notes size!");

    const Notes = struct {
        size: u32,
        entries: [res.size]Note,
        enable_substitutions: bool = false,
    };

    // Note that unicode chars won't work, because they trigger a comptime allocation.
    const res2 = json.parseFromSliceLeaky(Notes, fba.allocator(), raw_notes, .{
        .ignore_unknown_fields = true,
    }) catch @compileError("Failed to parse json notes entries!");

    break :x res2;
};

export fn getSubstituteVariableOffset(buf: [*:0]u8, v: [*:0]u8) i32 {
    const buf_len = std.mem.indexOfSentinel(u8, 0, buf) - 1;
    const v_len = std.mem.indexOfSentinel(u8, 0, v) - 1;
    const buf_slice = buf[0..buf_len];
    for (0..buf_len) |j| {
        if (std.mem.eql(u8, buf_slice[j..(j + v_len)], v[0..v_len])) {
            return @intCast(j);
        }
    }
    return -1;
}

export fn subcpy(buf: [*:0]u8, sub: [*:0]u8) void {
    const sub_len = std.mem.indexOfSentinel(u8, 0, sub);
    for (0..sub_len) |i| {
        const c = sub[i];
        buf[i] = c;
    }
    buf[sub_len] = 0;
}

comptime {
    if (!notes.enable_substitutions) {
        asm (
            \\.global substitute;
            \\.type substitute, @function;
            \\substitute:
            \\movq %rdi, %rax
            \\ret
        );
    } else {
        const substitutions = x: {
            @setEvalBranchQuota(std.math.maxInt(u32));
            var buf: [0x10000]u8 = undefined;
            var fba = std.heap.FixedBufferAllocator.init(&buf);

            // Find all substritutions in Notes
            var subs = ComptimeSet([]const u8){};
            for (notes.entries) |note| {
                findSubstitutions(note.title, &subs);
                findSubstitutions(note.content, &subs);
            }

            const _Substitutions = struct {};
            var fields = @typeInfo(_Substitutions).@"struct".fields;

            // Now add all the substitution vars to the fields of the Substitutions struct
            for (subs.items) |substitution| {
                const field_name = addZ(substitution.len, substitution[0..].*);
                fields = fields ++ [_]std.builtin.Type.StructField{.{
                    .alignment = 0,
                    .default_value = null,
                    .is_comptime = false,
                    .name = &field_name,
                    .type = []const u8,
                }};
            }

            const Substitutions = @Type(.{ .@"struct" = .{
                .layout = .auto,
                .is_tuple = false,
                .fields = fields,
                .decls = &.{},
            } });
            const _Notes = struct {
                substitutions: Substitutions,
            };

            const res = json.parseFromSliceLeaky(_Notes, fba.allocator(), raw_notes, .{
                .ignore_unknown_fields = true,
            }) catch @compileError("Failed to parse substitutions!");

            break :x res.substitutions;
        };

        const pre =
            \\.global substitute;
            \\.type substitute, @function;
            \\substitute:
            \\pushq   %rbp
            \\movq    %rsp, %rbp
            \\subq    $0x50, %rsp
            \\movq    %rdi, -0x48(%rbp)
            \\
        ;
        var code: []const u8 = pre;
        for (@typeInfo(@TypeOf(substitutions)).@"struct".fields, 0..) |field, i| {
            //@compileLog(code);
            code = code ++ std.fmt.comptimePrint(
                \\substitute_{d}:
                //\\xorq    %r9, %r9
                //\\substitute_{d}_loop:
                \\movq    -0x48(%rbp), %rdi
                //\\addq    %r9, %rdi
                \\lea     sub_{s}, %rsi
                \\call    getSubstituteVariableOffset
                \\cmp     $0, %eax
                \\jl      substitute_{d}
                \\movq    -0x48(%rbp), %rdi
                \\addq    %rax, %rdi
                \\lea     sub_{s}_with, %rsi
                \\call    subcpy
                \\jmp     substitute_{d}
                \\sub_{s}: .asciz "{s}"
                \\sub_{s}_with: .asciz "{s}"
                \\
            , .{
                i,
                field.name,
                i + 1,
                field.name,
                i + 1,
                field.name,
                "@" ++ field.name ++ "@",
                field.name,
                @field(substitutions, field.name),
            });
        }

        asm (code ++
                std.fmt.comptimePrint(
                \\substitute_{d}:
                \\    addq    $0x50, %rsp
                \\    popq    %rbp
                \\    ret
            , .{
                @typeInfo(@TypeOf(substitutions)).@"struct".fields.len,
            }));
    }
}

pub extern fn substitute(slice: *const u8) void;

test "test" {
    _ = notes;
}
