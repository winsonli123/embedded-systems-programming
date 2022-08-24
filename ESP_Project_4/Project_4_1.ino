// //////////////////////////////////////////////
// Embedded System Programming (CS 33301-001)
// Project 4.1
// Kent State University
// Dept. of Computer Science
// Professor Jungyoon Kim
// Winson Li
//
// 1. Initial state: Internal LED OFF
// Using an ADC Interrupt (10-bit ADC), write a program for blinking LED every one second when you ADC
// value (A0 pin) is over 500. You can test this function with 3.3V pin and 0V pin. In order to check the ADC
// value of A0 Pin, serial communication function can be used. Setting Registers of ADC, LED blink and
// Serial data to your email address.
//
// No need for Interrupt 0 stuff nor pull-up resistor since only A0 wire and internal LED is needed on board.
// //////////////////////////////////////////////

// High when a value is ready to be read
volatile int readFlag;

// Value to store analog result
volatile int analogVal;

unsigned char *ADMUX_ESP;
unsigned char *ADCSRA_ESP;
unsigned char *ADCSRB_ESP;
unsigned char *ADCL_ESP;
unsigned char *ADCH_ESP;

// global variables
unsigned char *portB;
int LED_State = 1;
unsigned char *TCCR1A_ESP; // 0x80
unsigned char *TCCR1B_ESP; // 0x81
unsigned char *TCNT1H_ESP; // 0x85
unsigned char *TCNT1L_ESP; // 0x84
unsigned char *OCR1AH_ESP; // 0x89
unsigned char *OCR1AL_ESP; // 0x88
unsigned char *TIMSK1_ESP; // 0x6F


// Initialization
void setup(){
  // Serial Communication
   Serial.begin(9600); // for debugging and printing email in serial montor, 9600 baud

  // Registers' Address Setting
  ADMUX_ESP = (unsigned char*) 0x7c;
  ADCSRA_ESP = (unsigned char*) 0x7a;
  ADCSRB_ESP = (unsigned char*) 0x7b;
  ADCL_ESP = (unsigned char*) 0x78;
  ADCH_ESP = (unsigned char*) 0x79;

  // clear ADLAR in ADMUX (0x7C) to right-adjust the result
  // ADCL will contain lower 8 bits, ADCH upper 2 (in last two bits)
  *ADMUX_ESP &= B11011111; // 0x7c
 
  // Set REFS1..0 in ADMUX (0x7C) to change reference voltage to the
  // proper source (01)
  *ADMUX_ESP |= B01000000;
 
  // Clear MUX3..0 in ADMUX (0x7C) in preparation for setting the analog
  // input (A0)
  *ADMUX_ESP &= B11110000;
 
  // Set MUX3..0 in ADMUX (0x7C) to read from AD0
  // Do not set above 15! You will overrun other parts of ADMUX. A full
  // list of possible inputs is available in Table 24-4 of the ATMega328
  // datasheet
  *ADMUX_ESP |= B00000000; // Binary equivalent
 
  // Set ADEN in ADCSRA (0x7A) to enable the ADC.
  // Note, this instruction takes 12 ADC clocks to execute
  *ADCSRA_ESP |= B10000000; // 0x7a
 
  // Set ADATE in ADCSRA (0x7A) to enable auto-triggering.
  *ADCSRA_ESP |= B00100000;
 
  // Clear ADTS2..0 in ADCSRB (0x7B) to set trigger mode to free running.
  // This means that as soon as an ADC has finished, the next will be
  // immediately started.
  *ADCSRB_ESP &= B11111000;
 
  // Set the Prescaler to 128 (16000KHz/128 = 125KHz)
  // Above 200KHz 10-bit results are not reliable.
  *ADCSRA_ESP |= B00000111;
 
  // Set ADIE in ADCSRA (0x7A) to enable the ADC interrupt.
  // Without this, the internal interrupt will not trigger.
  *ADCSRA_ESP |= B00001000;
 
  // Enable interrupts using SREG
  unsigned char *ptrSREG;
  ptrSREG = (unsigned char *) 0x5F;
  *ptrSREG |= 0x80; // b 1000 0000 == 0x80 --> b 1xxx xxxx
    
  // Kick off the first ADC
  readFlag = 0;
  // Set ADSC in ADCSRA (0x7A) to start the ADC conversion
  *ADCSRA_ESP |=B01000000;

  // initialize timer1
  // Disable interrupts using the SREG
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

// Processor loop
void loop(){

  // Check to see if the value has been updated
  if (readFlag == 1){
    // Perform whatever updating needed   
    readFlag = 0; 
  }
   
}

// Interrupt service routine for the ADC completion
ISR(ADC_vect){

  // Done reading
  readFlag = 1;

  // BONUS POINTS
  // By using cl and ch instead, it is being done via the data registers instead of the built in arduino AVR installed values
  // The cl and ch represent the values in the data register of ADCL_ESP and ADCH_ESP respectively. 
  // From which data register address to read was set via ADCL_ESP = (unsigned char*) 0x78 and ADCH_ESP = (unsigned char*) 0x79 already in setup().
  // ADCL_ESP and ADCH_ESP are the registers that store the analog values like the AVR installed values of ADCL and ADCH did. 
  // The analog value is 10 bits. cl (aka ADCL_ESP) and ADCL store the right 8 bits. ch (aka ADCH_ESP) and ADCH represents the left remaining 2 bits.
  // By using | (OR bitwise) it's combining the values of cl and ch together to one 10 bit value so it can be stored as the int variable analogVal. 
  // The << 8 was to shift the bits of ADCL left 8 bits so they occupy the two left bits. 
  // Must read low first
  unsigned char cl = *ADCL_ESP;
  unsigned char ch = *ADCH_ESP;
  *ADCL_ESP = 0x00;
  *ADCH_ESP = 0x00;
  analogVal = cl | (ch << 8);  // ADCL 0x78 ADCH 0x79
  // analogVal = ADCL | (ADCH << 8);  // ADCL 0x78 ADCH 0x79
  Serial.println("wli21@kent.edu:");
  Serial.println(analogVal);   // To check the ADC values on the computer (serial monitor)

  // Not needed because free-running mode is enabled.
  // Set ADSC in ADCSRA (0x7A) to start another ADC conversion
  *ADCSRA_ESP |= B01000000;
}

ISR(TIMER1_COMPA_vect) // timer compare interrupt service routine
{
// toggle for PB5
if (analogVal > 500){ // LED toggle/blink when over 500
  if(LED_State == 0)
  {
  *portB |= 0x20; // turns LED ON
  LED_State = 1;
  //Serial.println("ON");
  }else
  {
   *portB &= ~0x20; // turns LED OFF
  LED_State = 0;
  //Serial.println("OFF");
  }
  }else
  {*portB &= ~0x20; 
  // turns LED OFF when analog value is under 500, added so that when you move the wire, it doesn't get stuck on ON after toggle, keeps it OFF when not over 500
  // can remove this else part if you want it to be stuck on the toggled State till blinking resumes
  }
}
