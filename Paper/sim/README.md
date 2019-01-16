# Simulation in C

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

## Compiling `sim.exe`

That's the easy part. All source files are in the same folder. 
Compile them as a console application using any C compiler.
I used Code::Blocks.
