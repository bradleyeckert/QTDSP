# Simulations

## Chirp

The `noisemodel.ods` spreadsheet (LibreOffice format) demonstrates the 1/F noise spectrum produced by summing several exponential chirps together. The spectrum of an individual chirp is fairly simple due to its exponential nature. The differentials needed by Taylor series expansion are trivial. You get a chirp spectrum that is itself exponential. Summing an exponentially distributed spectrum of chirps in the spreadsheet shows the 1/F sum.

Notice the flattening at the low end, caused by the limitation in how high the warp rate can go. The low end of the spectrum of a 1/F "noise" source may indicate this kind of bandwidth limit.

## Asymptotic chirp

Another possibility for the exponential chirp is an upward chirp starting at 0 and rising asymptotically to a final frequency, as the signal emerges from infinite time. To test this, a signal is generated in `fft.c`. The signal and its spectrum are output in csv format, suitable for a spreadsheet or GNUplot.

- Frequency = F0 * (1 - e^-kt)
- Amplitude = e^-kt

You can compile and run `fft.c` and plot the results, but it's not very interesting. The spectrum is roughly flat from DC to F0, like a white noise spectrum lopped off at F0. That the spectrum is white is the interesting part.
