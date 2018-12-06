"../audio/windbell.wav" => string fileName;
me.sourceDir() + fileName => string filePath;
SndBuf buf => dac;
filePath => buf.read;
0.8 => external float gain;
0.9 => external float rate;

gain => buf.gain;
rate => buf.rate;

2::second => now;