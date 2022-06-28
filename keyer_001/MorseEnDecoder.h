#ifndef MorseEnDecoder_H
#define MorseEnDecoder_H

#if (ARDUINO <  100)
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

class morseDecoder
{
  public:
    morseDecoder();
    void decode(bool input);
    void setspeed(int value);
    char read();
    boolean available();
    long debounceDelay;     // the debounce time. Keep well below dotTime!!
    boolean morseSignalState;  
  private:
    bool *morseIn;         // The Morse input logic state as pointer
    int morseTablePointer;
    int wpm;                // Word-per-minute speed
    long dotTime;           // morse dot time length in ms
    long dashTime;
    long wordSpace;
    boolean morseSpace;     // Flag to prevent multiple received spaces
    boolean gotLastSig;     // Flag that the last received morse signal is decoded as dot or dash
    boolean morseKeyer;
    boolean lastKeyerState;
    long markTime;          // timers for mark and space in morse signal
    long spaceTime;         // E=MC^2 ;p
    long lastDebounceTime;  // the last time the input pin was toggled
    long currentTime;       // The current (signed) time
    char decodedMorseChar;  // The last decoded Morse character
};



#endif


