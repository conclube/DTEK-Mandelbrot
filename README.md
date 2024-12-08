# DTEK-Mandelbrot

## Authors
- Alexander Lorentzson (lorentzs@kth.se)
- Filip de Meyere (filipdm@kth.se)

This project also uses the `time4risc.zip` on canvas as a base template authored by Artur Podobas. 

## Configuration

#### Constraints
- Resolution can only be 64, 128 and 256.
- There are only three types of fractals M (mandelbrot), J (julia), S (sierpinski).

#### Formats
- `M;xmax;xmin;ymax;ymin;resolution;`
  - `xmax` - double
  - `xmin` - double
  - `ymax` - double
  - `ymin` - double
  - `resolution` - int 
 
- `J;xmax;xmin;ymax;ymin;real;imag;resolution;`
  - `xmax` - double
  - `xmin` - double
  - `ymax` - double
  - `ymin` - double
  - `real` - double
  - `imag` - double
  - `resolution` - int 

- `S;`
  - Unimplemented :c
 
- `#`
  - Terminate configuration.

#### Example
```
M;1;-1;1;-1;256;
M;1;-1;1;-1;128;
M;1;-1;1;-1;64;
J;2;-2;2;-2;0.45;0.1428;256;
S;
#
```

Here the julia fractal would correspond to switch 3 for example.


## Description
This project implements a simple fractal image generation implementation of mandelbrot sets, julia sets and sierpinski triangles.
Presently, only mandelbrot sets and julia sets are supported. (Running sierpinski will yield unimplemented errors.)

#### Mandelbrot Sets
Mandelbrot sets are defined with the recursive definition z_n = z_n-1 + c with the following contraints:
- z_0 := 0
- c := x + iy for all pixels x and y

We implement this by recursively checkig if the definition results in a cyclic sequence of values for any given (x,y), in such case (x,y) is convergent and should be painted in a distinct color.
If not, then the value diverges and (x,y) should be painted in a color to represent how quickly it diverges.

#### Julia Sets

Mandelbrot sets are defined with the recursive definition z_n = z_n-1 + c with the following contraints:
- z_0 := x + iy for all pixels x and y
- c := a + bi for some constant a and b 

We color the pixel depending on how quickly it diverges.

## Terminal Commands
- `module add dtekv` add dtekv toolchain.
- `module add riscv-gcc` add compiler.
- `jtagd --user-start` boot up the jtagd server that allows for communication between the computer and the chip.
- `dtekv-upload config.txt 0x200000` upload the default config to the chip at memory address 0x200000.
- `dtekv-download image.ppm 0x250000 <size>` download the image to image.ppm on the computer from the chip at memory address 0x250000, the size should be written in HEX prefixed with **0x**.
- `make` compile the program binaries.
- `dtekv-run main.bin` runs the program, if the program is already running then this will resume the program terminal (if you stepped out of it via C^).

## How to Run
1. Add required modules.
2. Compile the program.
3. Start the JTAGD server.
4. Upload configuration.
5. Run the program.
6. Once config is loaded, use switch 0-9 to select which fractal to generate, then press the button.
7. Once generated, download it with the given size prompted in the terminal. This will require stepping out of the program.
8. To generate another image, run the program again - you will be put back into the current instance, so just select another switch and generate ahead!
