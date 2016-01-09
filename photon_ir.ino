#define LEDPIN D7
#define IRPIN D6

#include "ir.h"

long decodeIr() {
  if (irPointer) { //if a signal is captured
    detachInterrupt(IRPIN);//stop interrupts & capture until finshed here
    digitalWrite(LEDPIN, HIGH);//visual indicator that signal received

    int irData[irPointer];
    Serial.println();
    for (int i = 1; i < irPointer; i++) { //now dump the times
      int timing = irBuffer[i] - irBuffer[i - 1];
      irData[i-1] = timing;
      Serial.print(irData[i-1]);
      if (i + 1 < irPointer) Serial.print(F("\t"));
    }

    Serial.println();
    long necCode = decodeNEC(irData);
    if(necCode > 0) {
      /*Serial.println();*/
      Serial.print(F("NEC: "));
      Serial.println(necCode, HEX);
      Serial.print(F("NEC: "));
      Serial.println(necCode, BIN);
    }

    irPointer = 0;
    /*Serial.println();*/
    digitalWrite(LEDPIN, LOW);//end of visual indicator, for this time
    attachInterrupt(IRPIN, rxIR_Interrupt_Handler, CHANGE);//re-enable ISR for receiving IR signal

    return necCode;
  }

  return 0;
}

void setup() {
  Serial.begin(115200); //change BAUD rate as required
  pinMode(LEDPIN, OUTPUT);
  pinMode(IRPIN, INPUT);
  attachInterrupt(IRPIN, rxIR_Interrupt_Handler, CHANGE); //set up ISR for receiving IR signal
}

void loop() {
  long code = decodeIr();

  switch(code) {
    case 1:
      break;

    case 0:
    default:
      break;
  }
}
