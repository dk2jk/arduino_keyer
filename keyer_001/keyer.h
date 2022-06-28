#ifndef _keyer_h
#define _keyer_h

#include <arduino.h>
inline static bool dit_paddle() ;
inline static bool dah_paddle();

// Copyright (c) dk2jk
class Keyer {
    enum EKeyer {
      ruhelage = 0,
      tx_freigabe = 1,
      dit_zwischenraum = 9,
      dah_zwischenraum = 5,
      dah_ein = 7,
      dit_ein = 11,
      tx_delay = 17,
    };
    
  public:
    Keyer(  unsigned char wpm = 18,
            unsigned char delay_start = 50,
            unsigned char delay_end = 500);
    void run();
    void update_wpm(unsigned char wpm);
    bool txEin();
    bool txFreigabe();

  private:
    EKeyer state;
    bool punkt_speicher;
    unsigned char  t_tx_delay_start;
    unsigned char  t_tx_delay_end;
    unsigned char  t_dit;
};

#endif

