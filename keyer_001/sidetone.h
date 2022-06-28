#include <arduino.h>

class Sidetone{

public:
  Sidetone();
  void start(char pin , int x_frequency=650, bool xenable=true);
  void update(boolean on);
  int frequency;
  boolean enable; 
private:
  char    _pin;
   

};

