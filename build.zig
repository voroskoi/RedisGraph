const std = @import("std");
const Str = []const u8;

pub fn build(b: *std.build.Builder) void {
    // Standard release options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall.
    const mode = b.standardReleaseOptions();
    
    var arena = std.heap.ArenaAllocator.init(std.testing.allocator);
    const files = getSourceFiles(arena.allocator(), "src") catch unreachable;
    defer arena.allocator().free(files);
    
    const lib = b.addSharedLibrary("redisgraph", "src/RG.h", .unversioned);
    lib.linkLibC();
    lib.addCSourceFiles(files, &[_][]const u8{});
    lib.addIncludePath("src");
    lib.addIncludePath("deps/xxHash");
    lib.addIncludePath("deps/rax");
    lib.addIncludePath("deps/libcypher-parser/lib/src");
    lib.addIncludePath("deps/RediSearch/src");
    lib.addIncludePath("deps/GraphBLAS/Include");

    lib.setBuildMode(mode);
    lib.install();

    // const main_tests = b.addTest("src/main.zig");
    // main_tests.setBuildMode(mode);

    // const test_step = b.step("test", "Run library tests");
    // test_step.dependOn(&main_tests.step);
}

fn getSourceFiles(allocator: std.mem.Allocator, comptime dirname: Str) ![]Str {
    const path = try std.fs.cwd().realpathAlloc(allocator, dirname);
    defer allocator.free(path);

    const source_dir = try std.fs.openDirAbsolute(
        path,
        std.fs.Dir.OpenDirOptions{ .iterate = true, .no_follow = true },
    );
    
    var ret = std.ArrayList(Str).init(allocator);

    var walker = try source_dir.walk(allocator);
    defer walker.deinit();

    while (walker.next() catch unreachable) |item| {
        if (item.kind != .File) continue;
        if (item.basename[item.basename.len-1] != 'c') continue;
        const file_name = walker.name_buffer.items;
        // std.debug.print("{s}\n", .{file_name});

        try ret.append(try std.fmt.allocPrint(allocator, "{s}/{s}", .{dirname, file_name}));
    }
    
    return ret.toOwnedSlice();
}
