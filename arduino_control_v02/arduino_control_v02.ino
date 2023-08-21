/*  Disclaimer
Verion 0.2
Vlad Himcinschi - 21.08.2023

This program is used for powering the Foucault Pendulum placed in the Johannes-Kepler-Gymnasium by Jan Mischke and Vlad Himcinschi.
This program checks its actual state every millisecond. Possible states are:
- DECIDING:     initial state, the coil is turned on (except for the first loop), deciding if the coil should be turned of when pendulum is nearby. 
                The program then changes to ACTUATING
- ACTUATING:    when the program has this state, it turns off the coil and puts itself into the WAITING state
- WAITING:      during this state, the pendulum is swinging away from the center of the coil, so it can't be pulled. the waiting time until pulling is turned on 
                is a quarter of the pendulums period - 500 ms. After this time the coil is turned on to pull the pendulum to its center
- PULLING:      to avoid pulling to little the pendulum is pulled for a quarter of the pendulums period. Atfer this time the state is set to DECIDING (to decide when to stop)
*/

#define h0 A0                               // hall sensor pins
#define h1 A1
#define h2 A2
#define h3 A3
const int relaisPin = 7;                    // pin of relais to control the pulling
const int pendulumperiod = 7769;            //  2 Pi * root (L / g)
// bool schub = true; use only for debuging

enum State {                                // possible states of the program while running to do everything on time
  DECIDING,
  ACTUATING,
  WAITING,
  PULLING
};

State currentState = DECIDING;                              // for init
unsigned long previousMillis = 0;                           // for init
const unsigned long pullduration = pendulumperiod/4 - 500;  // in ms, puffer after pulling to prevent pulling to early
const int tolerance = 10;                                   // the tolerance for the sensor input

void setup() {
  pinMode(h0, INPUT);
  pinMode(h1, INPUT);
  pinMode(h2, INPUT);
  pinMode(h3, INPUT);
  pinMode(relaisPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  int i0 = abs(analogRead(h0));                 // reading all the hall sensor data (only important sensor is sensor 0, the rest is for analyzing data)
  int i1 = abs(analogRead(h1));
  int i2 = abs(analogRead(h2));
  int i3 = abs(analogRead(h3));
  Serial.print(i0);                             // send it to Serial for Raspi
  Serial.print(" ");
  Serial.print(i1);
  Serial.print(" ");
  Serial.print(i2);
  Serial.print(" ");
  Serial.println(i3);                           
  unsigned long currentMillis = millis();       // get actual time in ms to decide the next step with the following switch

  switch (currentState) {
    case DECIDING:   // deciding if pulling is stopped
      if (abs(i0 - 512) > tolerance) {  //&& schub  !!only for debuging
        currentState = ACTUATING;
        previousMillis = currentMillis;
      } else {
        currentState = DECIDING;
      }
      break;

    case ACTUATING:   // stop the pull
      digitalWrite(relaisPin, LOW);
      currentState = WAITING;
      break;

    case WAITING:    // wait until the pendulum is allowed to be pulled again and then start the pull
      if (currentMillis - previousMillis >= pendulumperiod / 4 - 500) {
        digitalWrite(relaisPin, HIGH);
        currentState = PULLING;
        previousMillis = currentMillis;
      }
      break;

    case PULLING:     // wait for the puffer time after the pulling started to pass to prevent stop pulling to early
      if (currentMillis - previousMillis >= pullduration) {
        currentState = DECIDING;
      }
      break;
  }
}
