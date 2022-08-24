// //////////////////////////////////////////////
// Embedded System Programming (CS 33301-001)
// Project 2.1
// Kent State University
// Dept. of Computer Science
// Professor Jungyoon Kim
// Winson Li
//
// HW Connection: Using External Interrupt 0 for input switch (normal state is high and pushed state is low) and Port C bit 3 for output LED (On is pin high and off is pin low)
// 1. Initial state: LED OFF -> External LED
// After 1 second - when the button is pushed. -> LED ON
// After 1 second - Push again -> LED OFF
// After 1 second - Push again -> LED ON
// //////////////////////////////////////////////

int switch_State = 0; // Sets the intial state for the External LED to be OFF

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
    *AT328_EICRA |= 0x01;  // b 0000 0001 --> b xxxx xx01 --> Any change for INT 0

    // Enable external interrupt 0 using the External Interrupt Mask Register
    // (EIMSK)
    unsigned char *AT328_EIMSK;
    AT328_EIMSK = (unsigned char *) 0x3D;
    *AT328_EIMSK |= 0x01; // b 0000 0001 --> b xxxx xxx1 --> INT0 enable
    
    // Configure PC3 as an output using the Port C Data Direction Register
    // (DDRC)
    unsigned char *portDDRC;
    portDDRC = (unsigned char *) 0x27; // Port C Direction Regrister
    *portDDRC |= 0x08;  // PORTC bit 3 (External LED) as output (bit --> 1) --> b 0000 1000 --> b xxxx 1xxx

    // Enable interrupts using SREG
    unsigned char *ptrSREG;
    ptrSREG = (unsigned char *) 0x5F;
    *ptrSREG |= 0x80; // b 1000 0000 == 0x80 --> b 1xxx xxxx

    Serial.begin(9600); // for debugging in serial montor, 9600 baud
    
    //sei();
    // Loop forever
    while (1)
    {
        // Nothing to do here
        // All work is done in the ISR
    }
}

ISR(INT0_vect)
{
    unsigned char *portPinD;
    portPinD = (unsigned char *) 0x29; // PortD Input Regrister
    unsigned char *portC;
    portC = (unsigned char *) 0x28; //  PortC Data Regrister

    if (*portPinD & 0x04){ // if switch button is pressed
    switch_State = !switch_State; // toggle the switch state
    Serial.println("switch"); // debugging for when switch is pressed
    }

    // Read PD2 using the Port D Pin Input Register (PIND)
    if (switch_State == 1)
    {
        // PD2 is high, so button is released
        // Set PB5 low using the Port B Data Register (PORTB)
        *portC &= ~0x08; // Turns LED OFF
        Serial.println("1");
    }
    else
    {
        // PD2 is low, so button is pressed
        // Set PB5 high using the Port B Data Register (PORTB)
        *portC |= 0x08; // Turns LED ON
         Serial.println("0");
    }
}
