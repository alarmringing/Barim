"../audio/gamelangong.wav" => string fileName;

me.sourceDir() + fileName => string filePath;
SndBuf buf => dac;
filePath => buf.read;
0.8 => buf.gain;

15::second => now;