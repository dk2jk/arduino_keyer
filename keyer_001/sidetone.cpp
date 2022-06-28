#include "sidetone.h"
#include <arduino.h>

Sidetone::Sidetone(){
}
void Sidetone::start(char pin,int x_frequency, bool xenable){
  enable=xenable;
  _pin=pin;
  frequency=x_frequency;
}
void Sidetone::update(boolean x){
  if ( x & enable )
  {
    tone(_pin,frequency);
  }
  else 
  { 
    noTone(_pin); 
  }
}


