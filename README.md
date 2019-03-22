# Quantum Time Domain Signal Processing

Hi, I'm Brad Eckert. I code and mess with electronics.
I also think a lot about the structure of reality.

I have developed a theory for signal-processing artifacts of quantum time.
Quantum time is a property of consciousness spiraling into relative time using
a 2D time model.
This is a consequence of the eternal (outside of time) divine nature of life
and its entanglement in the material world.

The signal artifacts look just like noise, but they are not.
With the appropriate DSP, they are mappable onto detectable signals.
Such an algorithm could provide the long-sought *way in* that skips all of the esoterica.
The digital "Philosopher's Stone" of alchemy.

## The paper

"Quantum Time Signals in Living Beings" is a paper that I wrote to explore
algorithms for implementing QTDSP codecs.

It's in TeX format, perfect for repositories like Github. This is truly Open Science.
If you want to add to the paper or make corrections,
you can create a branch and generate a pull request.
Welcome to the next step in human evolution.

## Research Status

The `/sim` folder contains C code (generic console app) to implement
the algorithm and generate a visual image.
A test chirp with negative R is shown in the paper, illustrating what a noisy signal looks like.

## To Do

The image of a test chirp with positive R has a slope to it that should be corrected.
It's caused by the display position being dependent on F1 instead of F0.
F1's horizontal position in the image moves with R.

The implementation could be much faster because it's amenable to GPU implementation.
The multiple-R warp transforms and FFTs are both parallelizable in CUDA or OpenCL.
The next app should use a GPU. A GUI with slider controls would be nice.

Peak detection should be implemented so as to reduce the amount of data to
a few data points that mean something.
For example, if peaks keep occuring at a particular R value, that R value should stand out.
An image analog of an audio VU meter (peak detect) should help.
