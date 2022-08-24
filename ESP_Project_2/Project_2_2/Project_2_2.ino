// //////////////////////////////////////////////
// Embedded System Programming (CS 33301-001)
// Project 2.2
// Kent State University
// Dept. of Computer Science
// Professor Jungyoon Kim
// Winson Li
//
// HW Connection: Using External Interrupt 0 for input switch (normal state is high and pushed state is low) and Port C bit 3 for output LED (On is pin high and off is pin low)
// 2. initial state: internal LED ON and external LED OFF with optimal delayed value
// a. After 1 second - when the pushed button is off (rising edge). -> External LED ON with optimal delayed value and internal LED OFF
// b. After 1 second - Push (Falling edge) again -> External LED OFF and internal LED ON with optimal delayed value
// c. After 1 second - Push (Low status) again -> External LED ON with optimal delayed value and internal LED OFF
// d. After 1 second - Push (Any Change) again -> External LED OFF and internal LED ON with optimal delayed value
// … (Repeat a – d)
// //////////////////////////////////////////////

void MyDelay(unsigned long mSec);
int switch_State = 0;
int Interrupt_State = 0;

int main(void)
{
    unsigned char *portD;
    portD = (unsigned char *) 0x2B;

    // Enable the pull-up resistor on PD2 using the Port D
    // Data Register (PORTD)
    *portD |= 0x04; // b 0000 0100 

    // Configure external interrupt 0 to generate an interrupt request on any
    // logical change using External Interrupt Control Register A (EICRA)
    unsigned char *AT328_EICRA;
    AT328_EICRA = (unsigned char *) 0x69;

    // Enable external interrupt 0 using the External Interrupt Mask Register
    // (EIMSK)
    unsigned char *AT328_EIMSK;
    AT328_EIMSK = (unsigned char *) 0x3D;
    *AT328_EIMSK |= 0x01; // b 0000 0001 --> b xxxx xxx1 --> INT0 enable
    
    // Configure PC3 as an output using the Port C Data Direction Register
    // (DDRC)
    unsigned char *portDDRC;
    portDDRC = (unsigned char *) 0x27;
    *portDDRC |= 0x08;  // PORTC bit 3 as output (bit --> 1) --> b 0000 1000 --> b xx1x xxxx

    // Configure PB5 as an output using the Port B Data Direction Register
    // (DDRB)
    unsigned char *portDDRB;
    portDDRB = (unsigned char *) 0x24;
    *portDDRB |= 0x20;  // PORTB bit 5 as output (bit --> 1) --> b 0010 0000 --> b xx1x xxxx

    // Enable interrupts using SREG
    unsigned char *ptrSREG;
    ptrSREG = (unsigned char *) 0x5F;
    *ptrSREG |= 0x80; // b 1000 0000 == 0x80 --> b 1xxx xxxx

    Serial.begin(9600); // for debugging in serial montor, 9600 baud

    unsigned char *portC;
    portC = (unsigned char *) 0x28;

    unsigned char *portB;
    portB = (unsigned char *) 0x25;
    
    //sei();
    // Loop forever
    while (1)
    {
      switch(Interrupt_State){
      case 0:
      *AT328_EICRA |= 0x03;  // b 0000 0011 --> b xxxx xx11 --> Rising Edge for INT 0
      Serial.println("Rising");
      break;
      case 1:
      *AT328_EICRA |= 0x02;  // b 0000 0010 --> b xxxx xx10 --> Falling Edge for INT 0
      Serial.println("Falling");
      break;
      case 2:
      *AT328_EICRA |= 0x00;  // b 0000 0000 --> b xxxx xx00 --> Low status for INT 0
      Serial.println("Low");
      break;
      case 3:
      *AT328_EICRA |= 0x01;  // b 0000 0001 --> b xxxx xx01 --> Any change for INT 0 
      Serial.println("Any");
      break;
      }
     
    if (switch_State == 1)
    {
        // Set PC3 blink
        *portC |= 0x08; // Turns external LED ON
        // Serial.println("ON");
        MyDelay(5); // optimal delay value
        *portC &= ~0x08; // Turns external LED OFF
        // Serial.println("OFF");
        MyDelay(5);
    }
    else
    {
        // Set PB5 blink
        *portB |= 0x20; // Turns internal LED ON
        // Serial.println("ON");
        MyDelay(5);
        *portB &= ~0x20; // Turns internal LED OFF
        // Serial.println("OFF");
        MyDelay(5);
    }
      
    }
}



ISR(INT0_vect)
{
    unsigned char *portPinD;
    portPinD = (unsigned char *) 0x29; // 0000 0000
    int i;
    
    if (*portPinD & 0x04){
    switch_State = !switch_State;
    Serial.println("switch");
    Interrupt_State = (Interrupt_State + 1) % 4;
    }
}


// defines function MyDelay that's responsible for delay
void MyDelay(unsigned long mSec)
{
  volatile unsigned long i;
  unsigned long endT = 1000 * mSec;

  for (i = 0; i < endT; i++);
}
