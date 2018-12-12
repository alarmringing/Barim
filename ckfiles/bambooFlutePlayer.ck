Flute flute => PoleZero f => JCRev r => dac;
.75 => r.gain;
.05 => r.mix;

// Tunable Parameters
0.75 => external float finalGain;
61 => external int note => int previousNote;
0.883938 => external float jetDelay;
0.827470 => external float jetReflection;
0.762780 => external float endReflection;
0.25 => external float noiseGain;
0.914347 => external float pressure;
1 => external float vibratoFreq;
0 => external float vibratoGain;
// infinite time-loop
while( true )
{
    finalGain => r.gain;
    jetDelay => flute.jetDelay;
    jetReflection => flute.jetReflection;
    endReflection => flute.endReflection;
    noiseGain => flute.noiseGain;
    pressure => flute.pressure;
    if (previousNote != note) {
        <<< "finalGain is ", finalGain >>>;
        play(note, 1);
        note => previousNote;
    }
    10::ms => now;
}

// basic play function (add more arguments as needed)
fun void play( float note, float velocity )
{
    // start the note
    Std.mtof( note ) => flute.freq;
    velocity => flute.noteOn;
}