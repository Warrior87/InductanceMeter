
#define DEBUG

#ifdef DEBUG
  #define DEBUG_PRINT(x)  Serial.print (x)
  #define DEBUG_PRINTLN(x)  Serial.println (x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

byte drivePin = 13;
byte freqPin = 2;

const unsigned int overflowLimit = 0;
unsigned int overflows = 0;
float hertz;
byte error = 0;

void setup() {
  pinMode(drivePin, OUTPUT);
  digitalWrite(drivePin, LOW);
  pinMode(freqPin, INPUT);
  #ifdef DEBUG
    Serial.begin(115200);
  #endif
  TCCR1A = 0;                           /*reset the registers controlling TIMER1*/
  TCCR1B = 0;  
  TCCR1C = 0; 
  TCNT1 = 0;                            /*set the counter to 0, just in case*/
  TIMSK1 = _BV(TOIE1);                  /*this will enable the interrupt*/

  DEBUG_PRINTLN("starting test");
  delay(1000);
  digitalWrite(drivePin, HIGH);
  delay(5);
  digitalWrite(drivePin, LOW);
  delayMicroseconds(100);
  while(error == 0){
    hertz = getCounts();
    DEBUG_PRINTLN(hertz);
  }  
  if(error == 1){
    DEBUG_PRINTLN("error");
  }
  DEBUG_PRINTLN("no error");
}

void loop() {
  //leave empty for now, see how many oscillations we can get
  
}

unsigned long getCounts()
{
  unsigned int tempTimer;
  unsigned long _counts;
  float _microSeconds;
  float _hertz;
  
  TCCR0A = 0;                                                 /*disable timer0*/
  TCCR0B = 0;

  //make sure we catch a rising edge                              
  TCCR1B = 1;                                                     /*start timer1*/
  DEBUG_PRINTLN("while pin is high");
  while((PIND & 0b00000100) && (overflows <= overflowLimit));      /*while pin is high (wait for pin to go low)*/  
  TCCR1B = 0;                                                     /*stop timer1*/
  if(overflows >= overflowLimit){
    error = 1;
    DEBUG_PRINTLN("error set");
  }
  TCNT1 = 0;                                                      /*reset timer1*/  
  overflows = 0;                                                  /*reset overflows count*/
  TCCR1B = 1;                                                     /*start timer1*/
  DEBUG_PRINTLN("while pin is low");
  while((!(PIND & 0b00000100)) && (overflows <= overflowLimit));   /*while pin is low (wait for the pin to go high)*/
  TCCR1B = 0;                                                     /*stop timer1*/
  if(overflows >= overflowLimit){
    error = 1;
    DEBUG_PRINTLN("error set");
  }
  TCNT1 = 0;                                                      /*reset timer1*/  
  TCCR1B = 1;                                                     /*since pin is high, start timer1*/
  overflows = 0;                                                  /*reset overflows count*/

  //grab a frequency reading
  while((PIND & 0b00000100) && (overflows < overflowLimit));      /*while pin is high (wait for pin to go low)*/
  while((!(PIND & 0b00000100)) && (overflows < overflowLimit));   /*while pin is low (wait for the pin to go high)*/
  TCCR1B = 0;                                                     /*stop timer1*/

  TCCR0A = 0x03;                                                  /*restart timer0*/
  TCCR0B = 0x03;

  tempTimer = TCNT1;                                             /*get timer results*/
  _counts = ((((unsigned long)overflows) << 16) | (unsigned long)tempTimer);
  TCNT1 = 0;                                                      /*reset timer1*/ 
  overflows = 0;                                                  /*reset overflows count*/
   _microSeconds = (float)_counts / ((float)F_CPU / 1000001.0);    /*calculate microseconds, this extra bit corrects error*/
   _hertz = 1.0 / (_microSeconds / 1000000.0);

  return _hertz;
}

ISR(TIMER1_OVF_vect)                                        /*timer 1 overflow interrupt function*/  
{  
  overflows++;
  //Serial.println(overflows);
}
