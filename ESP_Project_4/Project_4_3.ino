// //////////////////////////////////////////////
// Embedded System Programming (CS 33301-001)
// Project 4.3
// Kent State University
// Dept. of Computer Science
// Professor Jungyoon Kim
// Winson Li
//
// Fast PWM output at the OC0A pin (port D pin 6) with maximal period and select the duty cycle based on
// your student ID (last two digits). For example, if your student id is 3455 1234, your duty cycle should be
// 34%. Connect your A0 pin to port D pin 6 and extract the ADC values for at least 5 seconds. The results
// need to be plotted in excel and submit and excel file with the plot result and number recordings.
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
unsigned char *TCCR0A_ESP; // 0x44
unsigned char *TCCR0B_ESP; // 0x45
unsigned char *TCNT0_ESP; // 0x46
unsigned char *OCR0A_ESP; // 0x47
unsigned char *OCR0B_ESP; // 0x48
unsigned char *TIMSK0_ESP; // 0x6E

// Initialization
void setup(){
  // Serial Communication
   Serial.begin(9600);

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
 
  // Set MUX3..0 in ADMUX (0x7C) to read from AD8 (Internal temp)
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

    // Timer Counter Control Regrister, important
  unsigned char *TCCR0A_ESP;
  TCCR0A_ESP = (unsigned char *) 0x44;
  unsigned char *TCCR0B_ESP;
  TCCR0B_ESP = (unsigned char *) 0x45;
  // Timer/Counter Register, the actual timer value is stored here
  unsigned char *TCNT0_ESP;
  TCNT0_ESP = (unsigned char *) 0x46;
  // Output Compare Regrister
  unsigned char *OCR0A_ESP;
  OCR0A_ESP = (unsigned char *) 0x47;
  unsigned char *OCR0B_ESP;
  OCR0B_ESP = (unsigned char *) 0x48;
  unsigned char *TIMSK0_ESP;
  // Timer/Counter Interrupt Mask Register, this enable/disable timer interrupts
  TIMSK0_ESP = (unsigned char *) 0x6E;
  
  *TCCR0A_ESP = 0x00;
  *TCCR0B_ESP = 0x00;
  *TCNT0_ESP = 0x00;
  *OCR0A_ESP = 0xFF; // max 
  *OCR0B_ESP = 0xC4; // C4 = 196/255 which is 77%, 810998677, outputs to OC0A PD6
  *TCCR0A_ESP |= B10100011; // Fast PWM, Clear OC0B when upcounting and set downcounting
  *TCCR0B_ESP |= B00001001; // prescaler 1
  *TIMSK0_ESP |= 0x04; // enable timer compare interrupt - BIT2- Timer/Counter, Output Compare B Match Interrupt Enable
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

  //unsigned char cl = *ADCL_ESP;
  //unsigned char ch = *ADCH_ESP;
  // Must read low first
  analogVal = ADCL | (ADCH << 8);  // ADCL 0x78 ADCH 0x79

  // Serial Comm to be learned.
  // To check the ADC values on the computer (serial monitor)
  Serial.print("wli21@kent.edu:");
  Serial.println(analogVal);
  
  // Not needed because free-running mode is enabled.
  // Set ADSC in ADCSRA (0x7A) to start another ADC conversion
  *ADCSRA_ESP |= B01000000;
}
