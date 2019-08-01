# C apps

Include all files in this folder (and FFT subfolder) but only include
one of the _app.c files because they contain the main function.
Compile them as a console application using any C compiler.
I used Code::Blocks.

## sim_app.c

- Generate exponential chirp `X` with time offset `t0`.
- Dump `X` to a 1D CSV file, `X.txt`.
- Time-warp it onto `Y` to produce a constant frequency.
- Dump `Y` to a 1D CSV file, `Y.txt`.
- Apply a Hann window and in-place FFT (Im=0) on `U`.
- Dump `U` to a 2D CSV file, `U.txt`. Output is in rectangular format.
- Time-warp the FFT result (magnitudes squared) to produce `W`.
- Dump `W` to a 1D CSV file, `W.txt`.
- Accumulate `W` in `V`.
- Increment `t0` and repeat.

The individual W results are output to a CSV file so you can compare peak widths and time offsets.

## sa_app.c

This app uses tools in the `sim` folder to convert a single channel of
data to a panoramic display of Time vs R.
The app creates a BMP file from command line parameters and a data file.

The maximum |R| is 0.78.
The data may be reduced by a decimation factor of 1 or more.

The BMP internally is a 2D array of floats that start out at 0.
Points are added to the array at polar coordinates by weighting the four closest
pixels appropriately and adding power data.
Upon output to a BMP file, it is raster-scanned and converted to 24-bit format
using a color scheme to create a "heat map" from log power.

Command line arguments:

- -b <filename> Set bmp output filename
- -i <filename> Set input filename
- -m Set new decimation factor, range = 1 to 100
- -o Set offset to starting point in file, must be >=0
- -N Set new N, must be a power of 2 between 128 and 16384
- -r Set new Rmin
- -R Set new Rmax
- -g Set new gamma, range 0.001 to 1.0
- -x Set new X step size, range 4 to N
- -f Set floor of heatmap
- -c Set ceiling of heatmap
- -h Set BMP height to multiple of 4
- -w Set BMP width
- -v Set verbose mode: dumps extra stuff
- -a Enable autowidth
- -F Set floor relative to ceiling in dB
- -M Set color scheme to monochrome
- -z Enable extra filtering (not working)

