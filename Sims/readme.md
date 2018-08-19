# Simulations

## Asymptotic chirp

Another possibility for the exponential chirp is an upward chirp starting at 0 and rising asymptotically to a final frequency, as the signal emerges from infinite time. To test this, a signal is generated in `fft.c`. The signal and its spectrum are output in csv format, suitable for a spreadsheet or GNUplot.

- Frequency = F0 * (1 - e^-kt)
- Amplitude = e^-kt

You can compile and run `fft.c` and plot the results, but it's not very interesting. The spectrum is roughly flat from DC to F0, like a white noise spectrum lopped off at F0. That the spectrum is white is the interesting part.
