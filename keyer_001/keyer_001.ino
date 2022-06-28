
#include ".\timer.h"
#include "keyer.h"
#include "pins.h"
#include "MorseEnDecoder.h"

// Classes
Keyer        keyer; 
Timer        timer_poti( 100 );
morseDecoder decoder;
// </classes>


void setup() {
  // pins auf Ausgang schalten, low aktiv
  //digitalWrite(PIN_TX_KEY,    HIGH); // tx_key
  pinMode(     PIN_TX_KEY,    OUTPUT);
 // digitalWrite(PIN_TX_ENABLE, HIGH); // tx_enable
  pinMode(     PIN_TX_ENABLE, OUTPUT);
  pinMode(     PIN_SIDETONE,  OUTPUT); // ton ausgang

  Serial.begin(9600);
    Serial.println(__FILE__";" __DATE__";" __TIME__);
}

void loop() {
  ////////////// keyer  ///////////////////////
 keyer.run();                                     //  keyer aktionen
  digitalWrite(PIN_TX_ENABLE, !keyer.txFreigabe()); //  tx freigabe schalten
  digitalWrite(PIN_TX_KEY, !keyer.txEin());         //  tx schalten
  if(  keyer.txEin() )   {     tone(PIN_SIDETONE, 650) ;   }
  else                   {     noTone(PIN_SIDETONE);       }// mithoerton

  if (timer_poti.event())  // alle 100 ms
  { 
    long wpm = (long)analogRead(PIN_POTI); //0..1013  // poti speed
    wpm =  map(wpm, 0, 1023, 30, 10); // Bereich 0 bis 1013 in y bis x wandeln
    keyer.update_wpm(wpm);  // speed einstellen fuer keyer
        decoder.setspeed(wpm);  // speed einstellen fuer decoder ( der kann es nicht selbst messen )
  }

  // decoder auf serielle schnittstelle
  decoder.decode(keyer.txEin());  // decoder arbeiten lassen
  if ( decoder.available() )      // hat er was erkannt ?
  { char x = decoder.read();      // dann lesen
    Serial.print(x);              // und auf UART ausgeben
  }

}// loop()






