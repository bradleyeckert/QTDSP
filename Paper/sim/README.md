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

What's done so far:

Functions for FFT and Hann window.
Main.c generates the chirp and W dump.

Next: W accumulation in V.
