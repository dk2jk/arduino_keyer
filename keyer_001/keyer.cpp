#include "keyer.h"
#include <arduino.h>
#include "timer.h"
#include "pins.h"

// 2015 dk2jk

//Class
Timer    timer;

/* voreinstellungen fuer den Keyer ( constructor).
 * trotz der internen Pullupwiderstaende sollten externe
 * Pullups vorgesehen werden ( ~ 10kOhm )
 */
Keyer::Keyer( unsigned char  wpm,
              unsigned char xdelay_start, unsigned char xdelay_end)
{
 
  pinMode(PIN_KEYER_DID, INPUT);           // set pin to input
  pinMode(PIN_KEYER_DAH, INPUT);           // set pin to input


//  pinMode(PIN_KEYER_DID, INPUT_PULLUP);
//  pinMode(PIN_KEYER_DAH, INPUT_PULLUP);

  //default-werte
  t_tx_delay_start = xdelay_start;
  t_tx_delay_end   = xdelay_end;
  update_wpm( wpm);
  state       = ruhelage;
}

void Keyer::update_wpm(unsigned char wpm)
{
  t_dit  = 1200 / wpm;
}

/* ** DER eigentliche KEYER **
 * Ereignisgesteuerter Keyer; nur 'wenn etwas passiert'
 * -und NUR dann- , wird je nach Zustand (state) eine neue Aktion gestartet.
 */
void Keyer::run()
{
  switch ( state)
  {
    case ruhelage:
      /* warten, bis taste betaetigt wird
       * wenn ja, so startet die einschaltverzoegerung.
       * tx wird freigegeben, aber noch nicht getastet
       */
      if ( dit_paddle() ||  dah_paddle() ) // ereignis 1: dit- oder dah-taste betaetigt
      {
        timer.interval(t_tx_delay_start);
        state = tx_freigabe;
      }
      break;

    case tx_freigabe:
      /* einschaltverzoegerung abwarten
       * dann pruefen, ob dit oder dah taste gedrueckt wurde
       * falls taste zu kurz betaetigt wurde, wieder
       * in ruhelage zurueckkehren
       */
      if ( timer.event() ) {
        if (dit_paddle()) {       // ereignis 1: dit taste betaetigt
          timer.interval(t_dit);
          state = dit_ein;
        }
        else if (dah_paddle() ) { // ereignis 2: dah taste betaetigt
          timer.interval( 3 * (int) t_dit );
          state = dah_ein;
        }
        else {                    // ereignis 3: keine taste betaetigt
          state = ruhelage;
        }
        break;

      case dit_ein:
        // sender ist getastet fuer eine zeiteinheit
        if ( timer.event() ) // ereignis 1: zeit ist um
        {
          timer.interval( t_dit );
          state = dit_zwischenraum;
        }
        break;

      case dit_zwischenraum:
        /* sender ist aus fuer eine zeiteinheit
         * dadurch, dass jetzt dah-taste zuerst abgefragt wird,
         * kommen beim druecken beider tasten wechselnd dits und dahs heraus
         */
        if ( timer.event() )
        {
          if (dah_paddle()) {       // ereignis 1: dah-taste gedrueckt
            timer.interval( 3 * t_dit );
            state = dah_ein;
          }
          else  if (dit_paddle()) { // ereignis 2: dit-taste gedrueckt
            timer.interval(t_dit);
            state = dit_ein;
          }
          else {                    // ereignis 3: keine taste gedrückt
            timer.interval(t_tx_delay_end);
            state = tx_delay;
          }
        }
        break;

      case dah_ein:
        // sender ist getastet fuer 3 zeiteinheiten
        if ( timer.event() )       // ereignis 1: zeit ist um
        {
          timer.interval( t_dit );
          state = dah_zwischenraum;
        }
        break;

      case dah_zwischenraum:
        /* sender ist aus fuer eine zeiteinheit
         * dadurch, dass jetzt die dit-taste zuerst abgefragt wird,
         * kommen beim druecken beider tasten wechselnd dits und dahs heraus
         */
        punkt_speicher = dit_paddle(); // dit wird schon waehrend der pause registriert
        if ( timer.event() )
        {
          if ( dit_paddle() || punkt_speicher )
          {
            punkt_speicher = 0;
            timer.interval(t_dit );
            state = dit_ein;
          }
          else if (dah_paddle()) {
            timer.interval( 3 * t_dit );
            state = dah_ein;
          }
          else { //keine taste gedrückt
            timer.interval(t_tx_delay_end);
            state = tx_delay;
          }
        }
        break;

      case tx_delay:
        /* der sender ist aus, jedoch noch freigegeben
         * falls in der  ausschaltverzoegerung eine taste
         * betaetigt wird, bleibt der sender freigegeben
         * und fast sofort wieder mit dit oder dahs weitergemacht
         */
        if ( timer.event() ) { // ereignis 1: nichts mehr los...
          //ganz aus
          state = ruhelage;
        }
        else {
          if ( dit_paddle() || dah_paddle()) { // ereignis 2: eine taste gedrueckt
            timer.interval(1);                 //sehr kleine verzoegerung
            state = tx_freigabe;
          }
        }
        break;
      }
  }   // </switch>
}     // </run()>



/*
 * die zustaende (state) sind so konstruiert, dass bit0
 * txFreigabe ist und bit 1 txEin ist.
 */
bool Keyer::txEin() {
  return (state & 2) > 0 ;
}

bool Keyer::txFreigabe() {
  return (state & 1) > 0 ;
}

/*
 * ports abfragen; low-aktiv == true
 */
inline static bool dit_paddle() {
  return (digitalRead(PIN_KEYER_DID) == LOW);
}

inline static bool dah_paddle() {
  return (digitalRead(PIN_KEYER_DAH) == LOW) ;
}



















