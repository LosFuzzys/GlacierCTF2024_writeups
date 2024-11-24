const std = @import("std");
const Notes = @import("notes.zig");

const CommandType = enum {
    List,
    View,
    Exit,
    None,
};

const Command = struct {
    type: CommandType,
    query: []const u8,
};

const commands = [_]Command{
    .{ .type = CommandType.List, .query = "list" },
    .{ .type = CommandType.View, .query = "view" },
    .{ .type = CommandType.Exit, .query = "exit" },
};

const stdin = std.io.getStdIn().reader();
const stdout = std.io.getStdOut().reader();

fn printMenu() void {
    std.debug.print("Commands:\n", .{});
    for (commands) |command| {
        std.debug.print("- {s}\n", .{command.query});
    }

    std.debug.print("> ", .{});
}

fn getCommand() !Command {
    var command_query_buffer: [100]u8 = undefined;
    const command_query = try stdin.readUntilDelimiterOrEof(&command_query_buffer, '\n') orelse "";
    const command_query_stripped = std.mem.trim(u8, command_query, "\n\r");
    const command: Command = x: {
        for (commands) |command| {
            if (std.mem.eql(u8, command.query, command_query_stripped)) {
                break :x command;
            }
        }
        break :x .{ .type = CommandType.None, .query = "" };
    };

    return command;
}

fn substitute(buf: [*]u8, content: []const u8) void {
    @memcpy(buf[0..content.len], content);
    Notes.substitute(&buf[0]);
}

noinline fn list() void {
    for (Notes.notes.entries) |note| {
        var title_buffer: [0x100]u8 = std.mem.zeroes([0x100]u8);
        substitute(&title_buffer, note.title);
        std.debug.print("{s}\n", .{title_buffer});
    }
}

noinline fn view() void {
    var query_buffer: [100]u8 = undefined;

    std.debug.print("Note title: ", .{});
    const title = stdin.readUntilDelimiterOrEof(&query_buffer, '\n') catch "" orelse "";
    const title_stripped = std.mem.trim(u8, title, "\n\r");

    for (Notes.notes.entries) |note| {
        var title_buffer: [0x100]u8 = std.mem.zeroes([0x100]u8);
        substitute(&title_buffer, note.title);
        if (std.mem.eql(u8, title_buffer[0..title_stripped.len], title_stripped)) {
            var content_buffer: [0x1000]u8 = std.mem.zeroes([0x1000]u8);
            substitute(&content_buffer, note.content);
            std.debug.print("{s}\n", .{content_buffer});
            break;
        }
    }
}

pub fn main() !void {
    while (true) {
        printMenu();
        const command = try getCommand();
        switch (command.type) {
            CommandType.List => list(),
            CommandType.View => view(),
            CommandType.Exit => {
                return;
            },
            CommandType.None => {
                std.debug.print("Invalid command\n", .{});
            },
        }
    }
}
