# Quantum Time Domain Signal Processing

Hi, I'm Brad Eckert. I code and I mess with electronics.
I also think a lot about the nature of "reality".

I have developed a theory for signal-processing artifacts of quantum time.
Quantum time is a property of consciousness spiraling into relative time using
a 2D time model.
This is a consequence of the eternal (outside of time) divine nature of life
and its quantum entanglement with the material world.

The signal artifacts look just like random noise, but they aren't necessarily.
With the appropriate DSP, they are mappable onto (potentially) detectable signals.
Such an algorithm could provide the long-sought *way in* that skips all of the esoterica.
The digital "Philosopher's Stone" of alchemy.

## The paper

"Quantum Time Signals in Living Beings" is a paper that I wrote to explore
algorithms for implementing QTDSP (Quantum Time Digital Signal Processing) codecs.

It's in TeX format, perfect for repositories like Github. This is truly Open Science.
If you want to add to the paper or make corrections,
you can create a branch and generate a pull request.
Welcome to the next step in human evolution.

## Research Status

The `/sim` folder contains C99 code (a generic console app) to implement
the algorithm and generate a visual image.
Test chirps are shown in the paper, illustrating what might be expected.
Proof of real-world signal has not been found, yet.
Let's build good tools, like Galileo built telescopes.
Then anyone can search the bazillions of existing noisy data streams or new ones.

## To Do

The image of a test chirp with positive R has a slope to it.
It's caused by the display position being dependent on F1 instead of F0.
F1's horizontal position in the image moves with R.
The current correction function isn't perfect but it's close enough.
I don't seem to understand the math correctly.

The implementation could be much faster because it's amenable to GPU implementation.
The multiple-R warp transforms and FFTs are both parallelizable in CUDA or OpenCL.
The next app should use a GPU. A GUI with slider controls would be nice.

Peak detection should be implemented so as to reduce the amount of data to
a few data points that mean something.
For example, if peaks keep occuring at a particular R value, that R value should stand out.
An image analog of an audio VU meter (peak detect) should help.

A GUI version of the app should be created and built in a dedicated repository.
