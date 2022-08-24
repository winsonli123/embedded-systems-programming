// //////////////////////////////////////////////
// Embedded System Programming (CS 33301-001)
// Project 3.2
// Kent State University
// Dept. of Computer Science
// Professor Jungyoon Kim
// Winson Li
//
// Write a program with appropriate comments. Input switch should be working with the external
// interrupt. HW Connection: Using External Interrupt 0 for input switch (normal state is high and pushed
// state is low) and no need to add an external LED.
// 2. Initial state: Internal LED OFF, Starting State at Rest
// Using a Timer Interrupt (Any Timer with Timer Interrupt.
// In order to make an accurate one second timer, serial communication function can be used.
// Setting Registers of Timer, LED blink and Serial data to your email address.
//
// a. When the button is pushed. -> toggle LED every one second
// b. When the button is pushed. -> toggle LED every two seconds
// c. When the button is pushed. -> toggle LED every three seconds
// d. When the button is pushed. -> toggle LED every four seconds
// … (Repeat a – d)
//
// LED status is changed everytime the set timer interval is reached. It toggles On and Off.
// ///////////////////////////////////////////////

// global variables
bool toggle_LED_State = 0;
int timer_State = 0;

unsigned char *TCCR1A_ESP; // 0x80
unsigned char *TCCR1B_ESP; // 0x81
unsigned char *TCNT1H_ESP; // 0x85
unsigned char *TCNT1L_ESP; // 0x84
unsigned char *OCR1AH_ESP; // 0x89
unsigned char *OCR1AL_ESP; // 0x88
unsigned char *TIMSK1_ESP; // 0x6F
unsigned char *ptrSREG;    // 0x5F

int main(void) {
  Serial.begin(9600); // for debugging and printing email in serial montor, 9600 baud
  
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
  
  // Configure PB5 as an output
  // using the Port B Data Direction Register (DDRB)
  unsigned char *portDDRB;
  portDDRB = (unsigned char *) 0x24;
  unsigned char *portB;
  portB = (unsigned char *) 0x25;

  *portDDRB |= 0x20;  // PORTB bit 5 as output (bit --> 1) --> b 0010 0000 --> b xx1x xxxx

  // Enable interrupts
  // using SREG
  unsigned char *ptrSREG;
  ptrSREG = (unsigned char *) 0x5F;
  *ptrSREG |= 0x80;
  
    while (1){
    // Nothing to do here
    // All work is done in the ISR
  }
}

void timer(int timer_State) {
  // initialize timer1 and changes OCR1A based on desired time toggle interval aka timer_State
  *ptrSREG = 0x00; // b 0xxx xxxx, 0 disables interrupts
  
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

  *TCCR1A_ESP = 0x00;
  *TCCR1B_ESP = 0x00;
  *TCNT1H_ESP = 0x00;
  *TCNT1L_ESP = 0x00;
  // OCR1AH and OCR1AL are missing here and instead in the switch case. It doesn't jump to the Timer OCR1A settings. Therefore, the initial state/rest state of the LED is OFF.
  
  switch(timer_State) {
    // compare match register 16MHz/256/1Hz
    // LED toggles every 1 second, (16000000/1024)/1hz = 15625 = 0x3D09
    case(1): 
      *OCR1AH_ESP = 0x3D;
      *OCR1AL_ESP = 0x09;
      break;
      
    // LED toggles every 2 second, (16000000/1024)/0.5hz = 31250 = 0x7A12
    case(2): 
      *OCR1AH_ESP = 0x7A;
      *OCR1AL_ESP = 0x12;
      break;

    // LED toggles every 3 second, (16000000/1024)/0.3333hz = approx 46880 = 0xB720
    case(3): 
      *OCR1AH_ESP = 0xB7;
      *OCR1AL_ESP = 0x20;
      break;

    // LED toggles every 4 second, (16000000/1024)/0.25hz = 62500 = 0xF424
    case(0):
      *OCR1AH_ESP = 0xF4;
      *OCR1AL_ESP = 0x24;
      break;
      }
  
  *TCCR1B_ESP |= 0x0D;  // 1101, CTC mode (Clear Timer on Compare Match) - BIT3
                        // 1024 prescaler, BIT2 and BIT0
  *TIMSK1_ESP |= 0x02;  // enable timer compare interrupt - BIT1- Timer/Counter1, Output Compare A Match Interrupt Enable

  *ptrSREG |= 0x80;     // b 1000 0000, 1 enables interrupts 
}

ISR(INT0_vect){ // external interrupt service routine
  unsigned char *portPinD;
  portPinD = (unsigned char *) 0x29;
  unsigned char *portB;
  portB = (unsigned char *) 0x25;
  
  // Read PD2 using the Port D Pin Input Register (PIND)
  if (*portPinD & 0x04){ // when button is pressed, change timer_State up 1 second
    Serial.println("PUSH"); // Helps with testing since switch button sometimes regristered multiple times instead of as one press
    timer_State = (timer_State + 1) % 4;
    }
  timer(timer_State);
}

ISR(TIMER1_COMPA_vect) { // timer compare interrupt service routine
  unsigned char *portB;
  portB = (unsigned char *) 0x25;
  // toggle for PB5
  if (toggle_LED_State == 0) {
    *portB &= ~0x20;
    toggle_LED_State = 1;
    }
  else {
    *portB |= 0x20;
    toggle_LED_State = 0;
    }
  Serial.println("wli21@kent.edu");
  Serial.println(timer_State);
}
