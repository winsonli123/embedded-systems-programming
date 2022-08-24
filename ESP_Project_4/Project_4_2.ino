// //////////////////////////////////////////////
// Embedded System Programming (CS 33301-001)
// Project 4.2
// Kent State University
// Dept. of Computer Science
// Professor Jungyoon Kim
// Winson Li
//
// Write a program with appropriate comments. Input switch should be working with the external interrupt.
// HW Connection: Using External Interrupt 0 for input switch (normal state is high and pushed state is low)
// and no need to add an external LED.
//
// 2. Initial state: internal LED OFF
// a. When the pushed button is pushed. -> ADC value of temperature sensor. When your ADC value
// is over 50, blinking LED every one second.
// b. When the pushed button is pushed. -> ADC value of A1 pin. When your ADC value (temperature) is
// over 50, blinking LED every one second.
// c. When the pushed button is pushed. -> ADC value of GND without external connection. No need to control LED.
// d. When the pushed button is pushed. -> ADC value of A7 without external connection. No need to control LED.
// … (Repeat a – d)
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
int LED_State = 1; // Initial State LED OFF
int LED_Control = 0; // Initial State LED OFF
int ADC_Mode = 0; 
unsigned char *portB;
unsigned char *TCCR1A_ESP; // 0x80
unsigned char *TCCR1B_ESP; // 0x81
unsigned char *TCNT1H_ESP; // 0x85
unsigned char *TCNT1L_ESP; // 0x84
unsigned char *OCR1AH_ESP; // 0x89
unsigned char *OCR1AL_ESP; // 0x88
unsigned char *TIMSK1_ESP; // 0x6F
unsigned char *ptrSREG;    // 0x5F

// Initialization
void setup(){
  // Serial Communication
   Serial.begin(9600); // for debugging and printing email in serial montor, 9600 baud
   
  // initialize timer1
  // Timer Counter Control Regrister, important
  unsigned char *portDDRB;
  portDDRB = (unsigned char *) 0x24;
  portB = (unsigned char *) 0x25;
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

  // Kick off the first ADC
  readFlag = 0;
  // Set ADSC in ADCSRA (0x7A) to start the ADC conversion
  *ADCSRA_ESP |=B01000000;
 
  // Intitalize Interrupt0
  // Enable the pull-up resistor on PD2
  // using the Port D Data Register (PORTD)
  unsigned char *portD;
  portD = (unsigned char *) 0x2B;
  *portD |= 0x04;

  // Configure external interrupt 0 to generate an interrupt request on rising logical change
  // using External Interrupt Control Register A (EICRA)
  unsigned char *AT328_EICRA;
  AT328_EICRA = (unsigned char *) 0x69;
  *AT328_EICRA |= 0x03;   // b 0000 0011 --> b xxxx xx11 --> Rising Edge for INT 0

  // Enable external interrupt 0
  // using the External Interrupt Mask Register (EIMSK)
  unsigned char *AT328_EIMSK;
  AT328_EIMSK = (unsigned char *) 0x3D;
  *AT328_EIMSK |= 0x01; // enable INT0

  // Enable interrupts using SREG
  unsigned char *ptrSREG;
  ptrSREG = (unsigned char *) 0x5F;
  *ptrSREG |= 0x80; // b 1000 0000 == 0x80 --> b 1xxx xxxx
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

  // Must read low first
  unsigned char cl = *ADCL_ESP;
  unsigned char ch = *ADCH_ESP;
  *ADCL_ESP = 0x00;
  *ADCH_ESP = 0x00;
  analogVal = cl | (ch << 8);  // ADCL 0x78 ADCH 0x79
  //analogVal = ADCL | (ADCH << 8);  // ADCL 0x78 ADCH 0x79

  // Not needed because free-running mode is enabled.
  // Set ADSC in ADCSRA (0x7A) to start another ADC conversion
  *ADCSRA_ESP |= B01000000;
}

ISR(TIMER1_COMPA_vect) // timer compare interrupt service routine
{
  Serial.println("wli21@kent.edu:");
  Serial.println(analogVal);   // To check the ADC values on the computer (serial monitor)
  //Serial.println(ADC_Mode); // reccomend have on
// toggle for PB5
if (analogVal > 50 && LED_Control == 1){
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
  {*portB &= ~0x20; // turns LED OFF
  }
}

ISR(INT0_vect){ // external interrupt service routine
  unsigned char *portPinD;
  portPinD = (unsigned char *) 0x29;
  unsigned char *portB;
  portB = (unsigned char *) 0x25;
  
  // Read PD2 using the Port D Pin Input Register (PIND)
  if (!(*portPinD & 0x04)){ // due to pull-up, released button is high
     // Serial.println("PUSH"); // Helps to have this active since switch button sometimes regristered multiple times instead of as one press
    ADC_Mode = (ADC_Mode + 1) % 4;
    // clears the right four bits to ensure MUX changes start fresh
   *ADMUX_ESP &= B11110000;
    switch(ADC_Mode) {
    case(1): 
    // Set MUX3-0 to 1000 in ADMUX (0x7C) to read from AD8 (Internal Temperator Sensor)
    // Do not set above 15! You will overrun other parts of ADMUX. A full
    // list of possible inputs is available in Table 24-4 of the ATMega328
    // datasheet
      *ADMUX_ESP |= B00001000; // Binary equivalent
      LED_Control = 1;
      break;
      
    case(2):
    // Set MUX3-0..0001 in ADMUX (0x7C) to read from AD1
      *ADMUX_ESP |= B00000001; // Binary equivalent
      break;

    case(3):
      // Set MUX3-0..1111 in ADMUX (0x7C) to read from GND, analog value should be 0
      *ADMUX_ESP |= B00001111; // Binary equivalent
      LED_Control = 0;
      break;

    case(0):
       // Set MUX3-0..0111 in ADMUX (0x7C) to read from AD7
      *ADMUX_ESP |= B00000111; // Binary equivalent
      break;
      }
    }
}
