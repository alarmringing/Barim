Flute flute => PoleZero f => JCRev r => dac;
.75 => r.gain;
.05 => r.mix;

// Tunable Parameters
1 => external float filterRate;
1 => int alreadyBlowing;
0 => external int noteOn;
0.75 => external float finalGain;
61 => external int note => int previousNote;
0.616657 => external float jetDelay;
0.517472 => external float jetReflection;
0.380731 => external float endReflection;
0.25 => external float noiseGain;
0.914347 => external float pressure;
1 => external float vibratoFreq;
0 => external float vibratoGain;
1 => external float velocity;
// infinite time-loop
while( true )
{
    jetDelay => flute.jetDelay;
    jetReflection => flute.jetReflection;
    endReflection => flute.endReflection;
    noiseGain => flute.noiseGain;
    pressure => flute.pressure;
    vibratoGain => flute.vibratoGain;
    vibratoFreq => flute.vibratoFreq;
    Std.mtof( note ) => flute.freq;
    filterRate => f.blockZero;

    if (noteOn == 0) {
        velocity => flute.noteOff;
        0 => alreadyBlowing;
    }
    else if(noteOn > 0 
        && alreadyBlowing == 0){
        velocity => flute.noteOn;
        finalGain => r.gain;
        1 => alreadyBlowing;
    }

    10::ms => now;
}