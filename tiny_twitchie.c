//Twitchie for AT tiny, by Raphael Abrams
//This code is protected by GPL rules. Please feel free tomodify it and use it in any way you wish, private or commercial. Any derivative works must contain attribution and also fall under the same license.

//This code makes a small robot wiggle and twitch in way both creepy and endearing. It's mostly random motion, but with certain constraints on that motion to add personality. There's lots of possible changes to make the codemoreto your liking. 



//Lets get the boring housekeeping stuff out of the way first.

// Set CPU clock speed. This doesn't actually set the speed that the chip run at, it tells the compiler what the speed is. If this doesn't match the actual speed of the chip, then all the delays will be wrong. The internal RC oscillator of the attiny85 runs at 8 MHz, so we'll use that number.
//(the UL at the end makes sure that the compiler sees the constant as an unsigned long number. Look up "datatypes" if that last bit doesn't make sense.
#define F_CPU 8000000UL


//Include the low level hardware definitions. This tells the compiler which pins are which, and also defines the words you can use to control those pins and special peripherals. 
#include <avr/io.h>

//Some other includes that you might want to bring in if you start to expand the code. We won't be using them now, so I commented them out.
//#include <util/delay.h>
//#include <stdlib.h>



///////////////////////////////////////////////////////////////////////////////////////////////////////////
//constants

//Some numbers are used over and over again in the code. Instead of writing them directly, 
//we create aliases for them using the "#define" key word. #define is not part of 
//the C language as such, but a pre-processor command. Anywhere in the code that the first 
//argument is typed, the pre-processor will replace that word with the contents of 
//the second argument. 
//So, for example as below, the word "LEDPIN" is really just the same thing as typing "3".
//If you ever decide to change which pin is used for the LED, you change the number "3" 
//to a number "2" then all the places that say "LEDPIN" will resolve to a two. 

//Here are names we are giving to physical pin numbers to make the code easier to read. 
#define LEDPIN 	3
#define SERVO1 	0
#define SERVO2 	1
#define SERVO3 	2

//Defines are useful for more than just pins! 
//Here we're using them to make it easier to read the code later on.
//Instead of testing to see if the robot is "1" or "0", we can test if it's "AWAKE" or "ASLEEP".
#define AWAKE  		1
#define ASLEEP 		0

////////////////////////////////////////////////////////////////////////////////////////////////////


//prototypes
//This is like a heads-up for the compiler to look out for user defined functions. 
//These can sometimes be kept in a separate .h file, 
//but for a program this small it's not really worth the trouble.

void delay_us(unsigned long reps);
unsigned char limited_random(unsigned char a, unsigned char b);
void servo(char pin, unsigned char angle);
void led_blink(void);
static inline void led_off(void); 
static inline void led_on(void); 




//main///////////////////////////////////////////////////////////////////////////////////////////////
int main(void) {

	//initialize the IO port pins
	//I like to use binary because it's easy to see right away which way pins are set. 
	//Setting a bit to zero makes its corresponding pin behave as an input, 
	//setting to one makes it an output.
	DDRB = 0b001111;
	//Make all the output pins go low:
	PORTB=0;
	//Now we're ready for some real stuff!


	//Infinite loop. The code does whatever is written between here 
	//and the corresponding curly brace  
	//until power is removed or the chip is reset.
	while(1){							

		//define a variable and fill it with a random number from 1 to 30.
		//this will determine how many times the loop is run before the rest period. 	
		unsigned char loop=limited_random(1,30);

		//start the main wiggling loop, using the number from just above:
		for(;loop>0;loop--){
			
			//how many times to call the servo routine
			char reps=limited_random(3,20); 

			//the angle for the servos.
			char x=limited_random(0,180);   
			char y=limited_random(0,180);	
			char z=limited_random(0,180);	
	
			//start the loop that sends a volley of pulses to the servos:
			for(;reps>0;reps--){
				//Actually send the servo pulses now:
				servo(SERVO1,x);	
				servo(SERVO2,y);
				servo(SERVO3,z);
				//Delay a little bit so the servo has 
				//time to execute the command before we send another one:
				delay_us(10000);	
			}
		}
		led_on();
		delay_us(limited_random(1,30)*1000);
		led_off();
	}
}


///////////////////////////////////////////////////////////////////////////////////////////
//Here is a user defined delay routine that takes less space than the standard library version.
//at very low values, maybe less than 10 or so, the accuracy degrades. This is a simple delay,
//and it simply stops all execution until it's done delaying.
void delay_us(unsigned long reps){  
volatile unsigned char x=0;
	for(;reps!=0;reps--){
		x++;
	}
}


//////////////////////////////////////////////////////////////////////////////////////////
//give the led a little blink. Good for getting the status of the robot, or just for fun.
void led_blink (void){

	//Light up the LED:
	led_on();

	//Wait a tiny fraction of a second:
	delay_us(1000);

	//Turn theLED back off:
	led_off();

	//Delay 1/10 second, in case the function gets called more than once:
	delay_us(10000);
}

/////////////////////////////////////////////////////////////////////////////////////////
//These two functions make the low level task of setting the LED pin on and off easier. 
//They use boolean logic to selectively switch just one bit in the byte that 
//controls the pin voltages.
static inline void led_on(void){ 
	PORTB |= (0x01<<LEDPIN);
}
static inline void led_off(void){
	PORTB &= (0xfe<<LEDPIN);
}

////////////////////////////////////////////////////////////////////////////////////////
//Time to talk to the servos!
//This function sends the necessary pulses to a servo to drive it to a desired angle.
//The same function is used for all the servos. 
//Servos take a positive pulse, measure its length, and use the duration of the pulse 
//to determine the angle they try to go to.
//Usually, it's about 1ms to 1.5ms for 0- 90 degrees. The actual pulse to angle 
//relationship varies from brand to brand, and between models as well.

//Every pulse you send the servo makes it try to get to the destination, 
//but for just a fraction of a second. 
//To actually get to your desired angle you need to keep sending pulses over and over at about 60 Hz.
//This function sends one pulse.

//This routine is optimized for hitec standard servos. 
//Give it a 0, the servo is going to go to 0 degrees.
//At 255, the servo will go to 180 degrees.
//127 move the servo to an angle of 90 degrees.
void servo(char pin, unsigned char angle){   

	//Start the pulse on the called pin
	PORTB |= (0x01<<pin);
	
	//Delay the minimum amount, about 1/1000 of a second:
	delay_us(1000);  

	//Now trim the total delay by adding a variable amount:
	for(;angle>0;angle--){
		delay_us(6);    
	}
	//All done delaying! 

	//Now stop the pulse on the called pin:
	PORTB &= (0xfe<<pin);
}


//////////////////////////////////////////////////////////////////////////////////////////////
//Here is a function to generate a random number. It uses the standard random number generator
//built in to the AVR libraries, but then makes some adjustments to be sure that the numbers 
//are in a useful range.
unsigned char limited_random(unsigned char a, unsigned char b){

	unsigned int temp;
	if(b<a){temp=a;a=b;b=temp;}
	if(b==a){return a;}
	return( rand() % ((b-a)+1)) +a;
}

