const std = @import("std");

const WIDTH: u16 = 1920;
const HEIGHT: u16 = 1280;

const MAX_ITER: usize = 100;

const OFFSET_X: f64 = 0.228155493653962;
const OFFSET_Y: f64 = 1.115142508039937;
const SCALE: f64 = 64.0;

const BFH_SIZE: u32 = 14;
const DIB_SIZE: u32 = 12;

const GRID_SIZE: usize = @as(usize, WIDTH) * HEIGHT;
const PIXEL_SIZE: u32 = GRID_SIZE * 3;
const PALETTE_SIZE: usize = MAX_ITER * 3;

const HEADER_SIZE: u16 = BFH_SIZE + DIB_SIZE;
const SIZE: u32 = BFH_SIZE + DIB_SIZE + PIXEL_SIZE;

pub fn computePalette(palette: []u8) !void {
    for (0..MAX_ITER) |i| {
        const x: usize = i * i;
        const v: u8 = @truncate(128 -% x);

        palette[i] = v;
        palette[i] = 1 -% v;
        palette[i] = 1 -% (v / 2);
    }
}

pub fn saveToImage(filename: []const u8, grid: []f64) !void {
    const file = try std.fs.cwd().createFile(filename, .{ .read = true });
    defer file.close();

    const writer = file.writer();

    // Bitmap file header
    _ = try writer.write("BM");
    _ = try writer.writeInt(u32, SIZE, .little);
    _ = try writer.write("\x00\x00\x00\x00");
    _ = try writer.writeInt(u32, HEADER_SIZE, .little);

    // DIB header
    _ = try writer.writeInt(u32, DIB_SIZE, .little);
    _ = try writer.writeInt(u16, WIDTH, .little);
    _ = try writer.writeInt(u16, HEIGHT, .little);
    _ = try writer.writeInt(u16, 1, .little); // PLANES
    _ = try writer.writeInt(u16, 24, .little); // DEPTH

    // Pixel array
    var pixels = [_]u8{0} ** PIXEL_SIZE;

    // Palette
    var palette = [_]u8{0} ** PALETTE_SIZE;
    try computePalette(&palette);

    for (0..GRID_SIZE) |v| {
        const value = @as(u32, @intFromFloat(grid[v] - 1)); // - 1 for indexing
        const b = v * 3;
        pixels[b] = palette[value];
        pixels[b + 1] = palette[value + 1];
        pixels[b + 2] = palette[value + 2];
        // @memcpy(pixels[b], palette[value]);
    }

    _ = try writer.write(&pixels);
}

pub fn computeGrid(grid: []f64) !void {
    for (0..WIDTH) |x| {
        for (0..HEIGHT) |y| {
            const index: usize = x + (y * WIDTH);
            const sX: f64 = @as(f64, @floatFromInt(x)) / WIDTH / SCALE;
            const sY: f64 = @as(f64, @floatFromInt(y)) / HEIGHT / SCALE;
            const cR: f64 = (sX * 3.47) - OFFSET_X;
            const cI: f64 = (sY * 2.24) - OFFSET_Y;
            var zR: f64 = 0;
            var zI: f64 = 0;
            var iter: usize = 0;
            while ((zR * zR) + (zI * zI) <= 4 and iter < MAX_ITER) {
                const zRTemp = zR * zR - zI * zI + cR;
                zI = 2 * zR * zI + cI;
                zR = zRTemp;
                iter += 1;
            }

            // printf("coord (%d, %d)\n", x, y);
            // printf("other (%d, %d)\n\n", x, index);
            grid[index] = @as(f64, @floatFromInt(iter));
        }
    }
}

pub fn main() !void {
    const allocator = std.heap.page_allocator;

    const grid = try allocator.alloc(f64, GRID_SIZE);
    try computeGrid(grid);
    try saveToImage("image.bmp", grid);

    defer allocator.free(grid);
    std.debug.print("Done\n", .{});
}
