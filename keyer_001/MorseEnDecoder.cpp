/*          MORSE ENDECODER
 
 - Morse encoder / decoder classes for the Arduino.
 
 Copyright (C) 2010-2012 raron
 
 GNU GPLv3 license:
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
 
 Contact: raronzen@gmail.com
 Details: http://raronoff.wordpress.com/2010/12/16/morse-endecoder/
 
 TODO: (a bit messy but will remove in time as it (maybe) gets done)
 - Use micros() for faster timings
 - use different defines for different morse code tables, up to including 9-signal SOS etc
 - how to deal with different Morse language settings? Define's don't work with libraries in Arduino...
 - possibly combine binary tree with table for the last few signals, to keep size down.
 - UTF-8 and ASCII encoding
 - configurable setting or both simultaneous?
 - Speed auto sense? (would be nice).
 - e.g. average x nr. of shortest and also longest signals for a time?
 - All the time or just when asked to?
 - debounceDelay might interfere
 - Serial command parser example sketch (to change speed and settings on the fly etc) 
 
 
 History:
 2012.11.25 - raron: Implemented another type of binary tree and algorithms.
 morseSignalString is now backwards.
 2012.11.24 - AdH: wrapped enocer digitalWrite calls in virtual start_signal
 and stop_signal functions to make alternate output methods 
 easy via subclassing.
 2012.11.22 - Debugged the _underscore_ problem, it got "uppercased" to a
 question mark. Also, included ampersand (&)
 2012.11.20 - Finally moved table to PROGMEM! Cleaned up header comments a bit.
 2012.11.10 - Fixed minor bug: pinMode for the Morse output pin (thanks Rezoss!)
 2012.01.31 - Tiny update for Arduino 1.0. Fixed header comments.
 2010.12.06 - Cleaned up code a bit.
 Added the "MN digraph" ---. for alternate exclamation mark(!).
 Still encoded as the "KW digraph" -.-.-- though.
 2010.12.04 - Program changed to use (Decode and Encode) classes instead.
 2010.12.02 - Changed back to signed timers to avoid overflow.
 2010.11.30 - Morse punctuation added (except $ - the dollar sign).
 2010.11.29 - Added echo on/off command.
 2010.11.28 - Added simple Morse audio clipping filter + Command parser.
 2010.11.27 - Added Morse encoding via reverse-dichotomic path tracing.
 Thus using the same Morse tree for encoding and decoding.
 2010.11.11 - Complete Rewrite for the Arduino.
 1992.01.06 - My old rather unknown "Morse decoder 3.5" for Amiga 600.
 A 68000 Assembler version using a binary tree for Morse
 decoding only, of which this is based on.
 */

#include "MorseEnDecoder.h"

// Morse code binary tree table (dichotomic search table)

// ITU with most punctuation (but without non-english characters - for now)
const int morseTreeLevels = 6; // Minus top level, also the max nr. of morse signals
const int morseTableLength = pow(2,morseTreeLevels+1);
const char morseTable[] PROGMEM = 
" ETIANMSURWDKGOHVF*L*PJBXCYZQ!*54*3***2&*+****16=/***(*7***8*90*"
"***********?_****\"**.****@***'**-********;!*)*****,****:*******\0";

morseDecoder::morseDecoder()
{
  // Some initial values  
  debounceDelay = 2;
  setspeed(13);

  morseTablePointer = 0;

  morseKeyer = LOW;
  morseSignalState = LOW;
  lastKeyerState = LOW;

  gotLastSig = true;
  morseSpace = true;
  decodedMorseChar = '\0';

  lastDebounceTime = 0;
  markTime = 0;
  spaceTime = 0;
}

void morseDecoder::setspeed(int value)
{
  wpm = value;
  if (wpm <= 0) wpm = 1;
  dotTime = 1200 / wpm;
  dashTime = 3 * 1200 / wpm;
  wordSpace = 7 * 1200 / wpm;
}

boolean morseDecoder::available()
{
  if (decodedMorseChar) return true; 
  else return false;
}

char morseDecoder::read()
{
  char temp = decodedMorseChar;
  decodedMorseChar = '\0';
  return temp;
}

void morseDecoder::decode(bool input)
{
  currentTime = millis();

  // Read Morse signals
  {
    // Read the Morse keyer (digital)
    morseKeyer = input;

    // If the switch changed, due to noise or pressing:
    if (morseKeyer != lastKeyerState) lastDebounceTime = currentTime; // reset timer

    // debounce the morse keyer
    if ((currentTime - lastDebounceTime) > debounceDelay)
    {
      // whatever the reading is at, it's been there for longer
      // than the debounce delay, so take it as the actual current state:
      morseSignalState = morseKeyer;

      // differentiante mark and space times
      if (morseSignalState) markTime = lastDebounceTime; 
      else spaceTime = lastDebounceTime;
    }
  }

  // Decode morse code
  if (!morseSignalState)
  {
    if (!gotLastSig)
    {
      if (morseTablePointer < morseTableLength/2-1)
      {
        // if pause for more than half a dot, get what kind of signal pulse (dot/dash) received last
        if (currentTime - spaceTime > dotTime/2)
        {
          // if signal for more than 1/4 dotTime, take it as a morse pulse
          if (spaceTime-markTime > dotTime/4)
          {
            morseTablePointer *= 2;  // go one level down the tree
            // if signal for less than half a dash, take it as a dot
            if (spaceTime-markTime < dashTime/2)
            {
              morseTablePointer++; // point to node for a dot
              gotLastSig = true;
            }
            // else if signal for between half a dash and a dash + one dot (1.33 dashes), take as a dash
            else if (spaceTime-markTime < dashTime + dotTime)
            {
              morseTablePointer += 2; // point to node for a dash
              gotLastSig = true;
            }
          }
        }
      } 
      else { // error if too many pulses in one morse character
        //Serial.println("<ERROR: unrecognized signal!>");
        decodedMorseChar = '#'; // error mark
        gotLastSig = true;
        morseTablePointer = 0;
      }
    }
    // Write out the character if pause is longer than 2/3 dash time (2 dots) and a character received
    if ((currentTime-spaceTime >= (dotTime*2)) && (morseTablePointer > 0))
    {
      decodedMorseChar = pgm_read_byte_near(morseTable + morseTablePointer);
      morseTablePointer = 0;
    }
    // Write a space if pause is longer than 2/3rd wordspace
    if (currentTime-spaceTime > (wordSpace*2/3) && morseSpace == false)
    {
      decodedMorseChar = ' ';
      morseSpace = true ; // space written-flag
    }
  } 
  else {
    // while there is a signal, reset some flags
    gotLastSig = false;
    morseSpace = false;
  }

  // Save the morse keyer state for next round
  lastKeyerState = morseKeyer;
}


















