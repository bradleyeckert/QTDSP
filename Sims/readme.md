# Simulations

## Asymptotic chirp

Another possibility for the exponential chirp is an upward chirp starting at 0 and rising asymptotically to a final frequency, as the signal emerges from infinite time. It seems the spectrum should be 1/F. To test this, a signal is generated in `fft.c`. The signal and its spectrum are output in csv format, suitable for a spreadsheet or GNUplot.

- Frequency = F0 * (1 - e^-kt)
- Amplitude = e^-kt
