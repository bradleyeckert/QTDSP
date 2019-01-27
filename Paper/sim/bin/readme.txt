Data is from http://www.tcts.fpms.ac.be/~devuyst/Databases/DatabaseSpindles/
Sleep Spindles database

excerpt2.txt is a waveform of 30 minutes at 200 SPS, 360000 samples

Hypnogram_excerpt.txt: is a textual file containing, in first line, the name "[hypnogram]" followed by one column of integer. These numerical values correspond to the sleep stage (one value per 5 sec) annotated by the expert according to the Rechtschaffen and Kales criteria.
5=wake			green
4=REM stage		yellow
3=sleep stage S1	red
2=sleep stage S2	magenta
1=sleep stage S3	cyan
0=sleep stage S4	white
-1=sleep stage movement
-2 or -3 =unknow sleep stage
 
Hypnogram_excerpt2.txt has 360 entries. Each one is 1000 samples, or 5 seconds.

The data was converted to a WAV file by MATLAB script towav.m. You can listen to it since its sample rate is set at 2000 SPS. Waterfall spectral analysis shows that most of the bandwidth is below 30 Hz. The overall spectrum is pink from 15 down to 0.7 Hz. There is a periodic note high in harmonics around 1.2 Hz. This is probably heartbeat.

