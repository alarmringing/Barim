Flute flute => PoleZero f => JCRev r => dac;
.75 => r.gain;
.05 => r.mix;

// Tunable Parameters
1 => int alreadyBlowing;
1 => external int noteOff;
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
    finalGain => r.gain;
    jetDelay => flute.jetDelay;
    jetReflection => flute.jetReflection;
    endReflection => flute.endReflection;
    noiseGain => flute.noiseGain;
    pressure => flute.pressure;
    Std.mtof( note ) => flute.freq;
    if (noteOff > 0 && alreadyBlowing == 1) {
        <<< "Stopped blowing" >>>;
        velocity => flute.noteOff;
        0 => alreadyBlowing;
    }
    else if(noteOff == 0 
        && alreadyBlowing == 0){
        <<< "started blowing" >>>;
        velocity => flute.noteOn;
        1 => alreadyBlowing;
    }
    10::ms => now;
}