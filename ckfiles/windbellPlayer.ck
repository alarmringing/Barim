fun void flute(float gain, float rate) {
    "../audio/windbell.wav" => string fileName;
    me.sourceDir() + fileName => string filePath;
    SndBuf buf => dac;
    filePath => buf.read;
    gain => buf.gain;
    rate => buf.rate;
    
    5::second => now;
}

0.8 => float gain;
0.9 => float rate;
flute(Std.atof(me.arg(0)), Std.atof(me.arg(1)));

/*
    "../audio/windbell.wav" => string fileName;
    me.sourceDir() + fileName => string filePath;
    SndBuf buf => dac;
    filePath => buf.read;
    gain => buf.gain;
    rate => buf.rate;

    5::second => now;
*/