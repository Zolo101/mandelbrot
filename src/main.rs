use std::{
    fs::File,
    io::{self, Write},
};

const WIDTH: u16 = 1920;
const HEIGHT: u16 = 1280;

const MAX_ITER: usize = 100;

const OFFSET_X: f64 = 0.228155493653962;
const OFFSET_Y: f64 = 1.115142508039937;
const SCALE: f64 = 64.0;

const BFH_SIZE: u32 = 14;
const DIB_SIZE: u32 = 12;

const GRID_SIZE: usize = (WIDTH as usize) * (HEIGHT as usize);
const PIXEL_SIZE: u32 = (GRID_SIZE * 3) as u32;
const PALETTE_SIZE: usize = MAX_ITER * 3;

const HEADER_SIZE: u32 = BFH_SIZE + DIB_SIZE;
const SIZE: u32 = BFH_SIZE + DIB_SIZE + PIXEL_SIZE;

fn compute_palette(palette: &mut [u8; PALETTE_SIZE]) {
    for i in 0..MAX_ITER {
        let x: usize = i * i;
        let v: u8 = (128 - x) as u8;

        palette[i * 3] = v;
        palette[i * 3 + 1] = 255 - v;
        palette[i * 3 + 2] = 255 - (v / 2);
    }
}

fn save_to_image(filename: &str, grid: &mut Box<[f64; GRID_SIZE]>) -> io::Result<()> {
    let mut file = File::create(filename)?;

    // Bitmap file header
    _ = file.write(b"BM");
    _ = file.write(&SIZE.to_le_bytes());
    _ = file.write(b"\x00\x00\x00\x00");
    _ = file.write(&HEADER_SIZE.to_le_bytes());

    // DIB header
    _ = file.write(&DIB_SIZE.to_le_bytes());
    _ = file.write(&WIDTH.to_le_bytes());
    _ = file.write(&HEIGHT.to_le_bytes());
    // PLANES
    const PLANES: u16 = 1;
    _ = file.write(&PLANES.to_le_bytes());
    // DEPTH
    const DEPTH: u16 = 24;
    _ = file.write(&DEPTH.to_le_bytes());

    // Pixel array
    let mut pixels = [0; PIXEL_SIZE as usize];

    // Palette
    let mut palette = [0; PALETTE_SIZE];
    compute_palette(&mut palette);
    // println!("{:?}", grid);

    for v in 0..GRID_SIZE {
        let value = grid[v] as usize - 1;
        pixels[v * 3] = palette[value * 3];
        pixels[v * 3 + 1] = palette[value * 3 + 1];
        pixels[v * 3 + 2] = palette[value * 3 + 2];
    }

    _ = file.write(&pixels);
    Ok(())
}

fn compute_grid(grid: &mut Box<[f64; GRID_SIZE]>) {
    for x in 0..WIDTH {
        for y in 0..HEIGHT {
            let index: usize = (x as usize) + (y as usize * WIDTH as usize);
            let s_x: f64 = x as f64 / WIDTH as f64 / SCALE;
            let s_y: f64 = y as f64 / HEIGHT as f64 / SCALE;
            let c_r: f64 = (s_x * 3.47) - OFFSET_X;
            let c_i: f64 = (s_y * 2.24) - OFFSET_Y;
            let mut z_r: f64 = 0.0;
            let mut z_i: f64 = 0.0;
            let mut iter: usize = 0;
            while (z_r * z_r) + (z_i * z_i) <= 4.0 && iter < MAX_ITER {
                let z_r_temp: f64 = z_r * z_r - z_i * z_i + c_r;
                z_i = 2.0 * z_r * z_i + c_i;
                z_r = z_r_temp;
                iter += 1;
            }

            // printf("coord (%d, %d)\n", x, y);
            // printf("other (%d, %d)\n\n", x, index);
            grid[index] = iter as f64;
        }
    }
}

fn main() {
    let mut grid = Box::new([0.0; GRID_SIZE]);
    compute_grid(&mut grid);

    match save_to_image("image.bmp", &mut grid) {
        Ok(_) => println!("Done!"),
        Err(_) => println!("Error saving file!"),
    }
}
