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

## eeg_app.c

This app uses tools in the `sim` folder to convert a single channel of EEG or
other data to a panoramic display of Time vs R.
The number of pixels in a row of data is proportional to R, so the "low-R" end
is shrunk relative to the "high-R" end.
Rather than use a rectangular image, a polar format such as a semicircle is used.
The app creates a BMP file from command line parameters and a data file.

The maximum |R| is 0.78.
Allowing a step size of 0.0005 for R puts the BMP height at 1560 R values. 
Two R values per pixel are used to avoid dead spots in the BMP,
so the BMP size is 1560 x 780.
The data may be reduced by a decimation factor of 1 or more.
When not reduced, a 200 SPS signal produces an output of between 0 and 78 SPS.
Angular data uses two points per pixel.
An arc of 780pi pixels fits 4900 points, or 62 seconds of data at the highest R.
It takes about a minute of processing to run the gamut of R values.

The BMP internally is a 2D array of floats that start out at 0.
Points are added to the array at polar coordinates by weighting the four closest
pixels appropriately and adding power data.
Upon output to a BMP file, it is raster-scanned and converted to 24-bit format
using a color scheme to create a "heat map" from log power.
