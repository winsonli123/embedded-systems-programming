// //////////////////////////////////////////////
// Embedded System Programming (CS 33301-001)
// Project 3.1
// Kent State University
// Dept. of Computer Science
// Professor Jungyoon Kim
// Winson Li
//
// Write a program with appropriate comments. Input switch should be working with the external
// interrupt. HW Connection: Using External Interrupt 0 for input switch (normal state is high and pushed
// state is low) and no need to add an external LED.
//
// 1. Initial state: Internal LED OFF
// Using a Timer Interrupt (Any Timer with Timer Interrupt), write a program for blinking LED every one
// second. In order to make an accurate one second timer, serial communication function can be used.
// Setting Registers of Timer, LED toggle and Serial data to your email address.
//
// LED status is changed every one second. It toggles On and Off.
// No External Interrupt 0 for part 1, pull-up resistor (normal high, pushed low) is not needed in part 1, only board is needed, no wiring or input switch 
// ///////////////////////////////////////////////

// global variables
unsigned char *portB;
int LED_State = 0;
unsigned char *TCCR1A_ESP; // 0x80
unsigned char *TCCR1B_ESP; // 0x81
unsigned char *TCNT1H_ESP; // 0x85
unsigned char *TCNT1L_ESP; // 0x84
unsigned char *OCR1AH_ESP; // 0x89
unsigned char *OCR1AL_ESP; // 0x88
unsigned char *TIMSK1_ESP; // 0x6F

void setup()
{// initialize timer1
   Serial.begin(9600); // for debugging and printing email in serial montor, 9600 baud
   
  // Disable interrupts using the SREG
  unsigned char *ptrSREG;
  ptrSREG = (unsigned char *) 0x5F;
  *ptrSREG = 0x00; // b 0xxx xxxx, 0 disables interrupts
  
  unsigned char *portDDRB;
  portDDRB = (unsigned char *) 0x24; // Port B Data Direction Regrister 
  portB = (unsigned char *) 0x25; // Port B Data Regrister

  // Timer Counter Control Regrister, important
  unsigned char *TCCR1A_ESP;
  TCCR1A_ESP = (unsigned char *) 0x80;
  unsigned char *TCCR1B_ESP;
  TCCR1B_ESP = (unsigned char *) 0x81;
  // Timer/Counter Register, the actual timer value is stored here
  unsigned char *TCNT1H_ESP;
  TCNT1H_ESP = (unsigned char *) 0x85;
  unsigned char *TCNT1L_ESP;
  TCNT1L_ESP = (unsigned char *) 0x84;
  unsigned char *OCR1AH_ESP;
  // Output Compare Regrister
  OCR1AH_ESP = (unsigned char *) 0x89;
  unsigned char *OCR1AL_ESP;
  OCR1AL_ESP = (unsigned char *) 0x88;
  unsigned char *TIMSK1_ESP;
  // Timer/Counter Interrupt Mask Register, this enable/disable timer interrupts
  TIMSK1_ESP = (unsigned char *) 0x6F;

  *portDDRB = 0x20; // B5 is Internal LED 
  
  *TCCR1A_ESP = 0x00;
  *TCCR1B_ESP = 0x00;
  *TCNT1H_ESP = 0x00;
  *TCNT1L_ESP = 0x00;
  *OCR1AH_ESP = 0xF4;  // (16,000,000/256)/1 = 62500 = 0xF424
  *OCR1AL_ESP = 0x24;  // compare match register 16MHz/256/1Hz
  *TCCR1B_ESP |= 0x0C; // 1100, CTC mode (Clear Timer on Compare Match) - BIT3
                       // 256 prescaler BIT2
  *TIMSK1_ESP |= 0x02; // enable timer compare interrupt - BIT1- Timer/Counter1, Output Compare A Match Interrupt Enable
  
  *ptrSREG |= 0x80; // b 1000 0000, 1 enables interrupts 
  //  Serial.println("Setup");
}

ISR(TIMER1_COMPA_vect) // timer compare interrupt service routine
{
// toggle for PB5
  if(LED_State == 0)
  {
  *portB |= 0x20; // turns LED ON
  LED_State = 1;
 // Serial.println("ON");
  }else
  {
   *portB &= ~0x20; // turns LED OFF
  LED_State = 0;
 // Serial.println("OFF");
  }
  Serial.println("wli21@kent.edu"); // prints to show LED toggle in serial
}

void loop(){
  // your program here…
}
