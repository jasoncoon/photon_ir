/*
Author: AnalysIR
Revision: 1.0

This code is provided to overcome an issue with Arduino IR libraries
It allows you to capture raw timings for signals longer than 255 marks & spaces.
Typical use case is for long Air conditioner signals.

You can use the output to plug back into IRremote, to resend the signal.

This Software was written by AnalysIR.

Usage: Free to use, subject to conditions posted on blog below.
Please credit AnalysIR and provide a link to our website/blog, where possible.

Copyright AnalysIR 2014

Please refer to the blog posting for conditions associated with use.
http://www.analysir.com/blog/2014/03/19/air-conditioners-problems-recording-long-infrared-remote-control-signals-arduino/

Connections:
IR Receiver      Arduino
V+          ->  +5v
GND          ->  GND
Signal Out   ->  Digital Pin 2
(If using a 3V Arduino, you may connect V+ to +3V)
*/

// From https://github.com/shirriff/Arduino-IRremote/blob/master/IRremoteInt.h

// Pulse parms are *50-100 for the Mark and *50+100 for the space
// First MARK is the one after the long gap
// pulse parameters in usec
#define NEC_HDR_MARK	9000
#define NEC_HDR_SPACE	4500
#define NEC_BIT_MARK	560
#define NEC_ONE_SPACE	1600
#define NEC_ZERO_SPACE	560
#define NEC_RPT_SPACE	2250

#define NEC_BITS 32

// Marks tend to be 100us too long, and spaces 100us too short
// when received due to sensor lag.
#define MARK_EXCESS 100

//you may increase this value on Arduinos with greater than 2k SRAM
#define maxLen 800

volatile unsigned int irBuffer[maxLen]; //stores timings - volatile because changed by ISR
volatile unsigned int irPointer = 0; //Pointer thru irBuffer - volatile because changed by ISR

#define TOLERANCE 25  // percent tolerance in measurements
#define LTOL (1.0 - TOLERANCE/100.)
#define UTOL (1.0 + TOLERANCE/100.)

#define USECPERTICK 50  // microseconds per clock interrupt tick

#define TICKS_LOW(us) (int) (((us)*LTOL/USECPERTICK))
#define TICKS_HIGH(us) (int) (((us)*UTOL/USECPERTICK + 1))

int MATCH(int measured, int desired) {return measured >= TICKS_LOW(desired) && measured <= TICKS_HIGH(desired);}
int MATCH_MARK(int measured_ticks, int desired_us) {return MATCH(measured_ticks, (desired_us + MARK_EXCESS));}
int MATCH_SPACE(int measured_ticks, int desired_us) {return MATCH(measured_ticks, (desired_us - MARK_EXCESS));}

// NECs have a repeat only 4 items long
long decodeNEC(int *irData) {
  long data = 0;
  int offset = 1;
  // Initial mark
  if (!MATCH_MARK(irData[offset], NEC_HDR_MARK)) {
    Serial.println("ERR No NEC Header");
    return -1;
  }
  offset++;
  // Check for repeat
  if (irPointer == 4 &&
    MATCH_SPACE(irData[offset], NEC_RPT_SPACE) &&
    MATCH_MARK(irData[offset+1], NEC_BIT_MARK)) {
    /*results->bits = 0;
    results->value = REPEAT;
    results->decode_type = NEC;*/
    Serial.println("NEC Repeat");
    return data;
  }
  if (irPointer < 2 * NEC_BITS + 4) {
    Serial.println("ERR not enough data");
    return -1;
  }
  // Initial space
  if (!MATCH_SPACE(irData[offset], NEC_HDR_SPACE)) {
    Serial.println("ERR No NEC Header");
    return -1;
  }
  offset++;
  for (int i = 0; i < NEC_BITS; i++) {
    if (!MATCH_MARK(irData[offset], NEC_BIT_MARK)) {
      Serial.println("ERR Expected a Mark");
      return -1;
    }
    offset++;
    if (MATCH_SPACE(irData[offset], NEC_ONE_SPACE)) {
      Serial.print("1");
      data = (data << 1) | 1;
    }
    else if (MATCH_SPACE(irData[offset], NEC_ZERO_SPACE)) {
      Serial.print("0");
      data <<= 1;
    }
    else {
      Serial.println("ERR Expected a Space");
      return -1;
    }
    offset++;
  }
  // Success
  /*results->bits = NEC_BITS; */ //32 bits
  return data;
  /*results->decode_type = NEC;*/
  /*return DECODED;*/
}

void rxIR_Interrupt_Handler() {
  if (irPointer > maxLen) return; //ignore if irBuffer is already full
  irBuffer[irPointer++] = micros(); //just continually record the time-stamp of signal transitions
}
