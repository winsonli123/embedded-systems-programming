// //////////////////////////////////////////////
// Embedded System Programming (CS 33301-001)
// Project 1.1
// Kent State University
// Dept. of Computer Science
// Professor Jungyoon Kim
// Winson Li
//
// Assignment for Part 1:
// Using Port B bit 3 for input switch and Port D bit 5 for output LED
// Initial state: LED ON
// After 1 second - Push button -> LED OFF
// After 1 second - Push again -> LED ON
// After 1 second - Push again -> LED OFF
// Note: Project 1.1 --> You can use either External LED only or use both external and internal LEDs. 
// //////////////////////////////////////////////

#define BIT3_MASK 0x08 // Bit 3, binary is 0000 1000
#define BIT5_MASK 0x20 // Bit 5, binary is 0010 0000

#define PORTB_MASK 0x25 // Port B Data Regrister, 0010 0101
#define PORTD_MASK 0x2B // Port D Data Regrister, 0010 1011
#define DDRB_MASK 0x24  // Port B Data Direction Regrister, 0010 0100
#define DDRD_MASK 0x2A  // Port D Data Direction Regrister, 0010 1010
#define PINB_MASK 0x23  // Port B Input Pin Address, 0010 0011

// Determine MyDelay function and declare int switch_State
void MyDelay(unsigned long mSec);
int switch_State;

// The setup function runs once when you press reset or power the board
void setup() {
  switch_State = 1; // 0 is OFF and 1 is ON, this sets up the intial state of the LED as ON
  unsigned char *portDDRB;
  unsigned char *portDDRD;
  portDDRB = (unsigned char *) DDRB_MASK; // 0x24 0010 0100
  portDDRD = (unsigned char *) DDRD_MASK; // 0x2A 0010 1010
  *portDDRB &= ~BIT3_MASK;  // Input Switch, configure bit 3 (PB3, D11) as an input, 0010 0100 &= 1111 0111 -> 0010 0100, since we want DDRB3 written as '0' (XXXX 0XXX) to config PORTB3 as input pin, we use &= and ~
  *portDDRB |= BIT5_MASK;   // Internal LED, configure bit 5 (PB5, D13) as an output 0010 0100 |= 0010 0000 -> 0010 0100, since we want DDRB5 written as '1' (XX1X XXXX) to config PORTB5 as output pin, we use |=
  *portDDRD |= BIT5_MASK;   // External LED, configure bit 5 (PD5, D5) as an output 0010 1010 |= 0010 0000 -> 0010 1010, since we want DDRD5 written as '1' (XX1X XXXX) to config PORTD5 as ouput pin, we use |=
  Serial.begin(9600); // for debugging in serial montor, 9600 baud
}

// the loop function runs over and over again forever
void loop() {
  // Serial.println("START"); // for debugging, shows when loop starts
  unsigned char *portPinB; 
  portPinB = (unsigned char *) PINB_MASK; //0x23 0010 0011, Port B input pin address
  unsigned char *portB;
  unsigned char *portD;
  portB = (unsigned char *) PORTB_MASK; //0x25 is 0010 0101, Port B data regrister
  portD = (unsigned char *) PORTD_MASK; //0x2B is 0010 1011, Port D data regrister

// a toggle via switch_State was needed since without it the LED will only be ON for the short blip of when button is pressed  
// this detects the input from the switch (PB3, D11) and when the regrister matches aka when the switch is preseed, it toggles the switch state to 1 or 0 mean LED ON or OFF
  if (((*portPinB) & BIT3_MASK) == 0x08) // if (0010 0011 & 0000 1000 == 0000 1000), portPinB becomes 0x08 when switch/button is pressed
  {
       switch_State = !switch_State; // flips the switch state, 1 becomes 0 while 0 becomes 1
       Serial.println(switch_State); // for debugging, prints 1 or 0 in monitor when switch is pressed
  }
  
  if (switch_State == 1) // when switch state is 1, turn LEDs ON 
  {
       Serial.println("ON");
       *portB |= BIT5_MASK; // 0010 0101 |= 0010 0000 -> 0010 0101, turns ON internal LED (PB5, D13)
       *portD |= BIT5_MASK; // 0010 1011 |= 0010 0000 -> 0010 1011, turns ON external LED (PD5, D5)
       // we want bit 5 of both port regristers to have the result of 1 (XX1X XXX) for it to turn ON which is why we use an OR bit-wise operator and why we use BIT5_MASK
       MyDelay(100); 
       // delays accepting of swithc input press, gives an short interval of time in which pressing the switch doesn't change anything
       // this helps prevent a single switch press accidently resulting in multiple input presses, can be set greater for longer delay interval
  }else // when switch is 0, turn LEDs OFF
  {
       Serial.println("OFF");
       *portB &= ~BIT5_MASK; // 0010 0101 &= 1101 1111 -> 0010 0101, turns OFF internal LED (PB5, D13)
       *portD &= ~BIT5_MASK; // 0010 1011 &= 1101 1111 -> 0010 1011, turns OFF external LED (PD5, D5)
       // we want bit 5 of both port regristers to have the result of 0 (XX0X XXX) for it to turn OFF which is why we use an AND bit-wise operator and why we use the inverse(~) of BIT5_MASK
       MyDelay(100); 
  }
  // Serial.println("END"); // for debugging, shows when loop ends
}

// defines function MyDelay that's responsible for delay
// experimentation via serial monitor + timestamps has found MyDelay(500) actually results in ~1 second delay in real-world use
void MyDelay(unsigned long mSec)
{
  volatile unsigned long i;
  unsigned long endT = 1000 * mSec;

  for (i = 0; i < endT; i++);
}
