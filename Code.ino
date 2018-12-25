/***********************************************************************
						Includes
***********************************************************************/
#include "TimerOne.h"

/***********************************************************************
						Variables
***********************************************************************/
// Variables Led Display
#define DataPin = 6;
#define ClockPin = 5;
#define LatchPin = 7;
#define Dig0Pin = 8;
#define Dig1Pin = 9;
#define Dig2Pin = 10;
#define Dig3Pin = 11;
volatile long Value[4] = {1,2,3,4};
volatile byte Pointer = 0;
//const int SegData[]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};  // Common Kathode
const int SegData[]={0x40,0x79,0x24,0x30,0x19,0x12,0x02,0x78,0x00,0x10};  // Common Anode + decimal points

// Variables timer
volatile unsigned int counter = 0;
volatile byte CurrentSecond = 0;
volatile byte CurrentMinute = 0;
volatile byte StartSecond = 30;
volatile byte StartMinute = 1;
const byte MAX_SECOND = 60;
const byte MAX_MINUTE = 60;

// Variables Button
#define ButtonStopPin = 1;
#define ButtonStartPin = 0;
#define ButtonResetPin = 2;
#define ButtonDownPin = 3;
#define ButtonUpPin = 4;

// Variables output
#define LedBankPin = A5;
#define BuzzerPin = A4;

// Flags
volatile byte isRunning = 0;
volatile int StopButtonState = LOW;
volatile int StartButtonState = LOW;
volatile int ResetButtonState = LOW;
volatile int UpButtonState = LOW;
volatile int DownButtonState = LOW;

/***********************************************************************
						Code
***********************************************************************/

/*
 * Initialization routine
 */
void setup() {
	// Initialization 4 digits 7 segment display
	pinMode(DataPin, OUTPUT);
	pinMode(ClockPin, OUTPUT);
	pinMode(LatchPin, OUTPUT);  
	pinMode(Dig0Pin, OUTPUT);  
	pinMode(Dig1Pin, OUTPUT);  
	pinMode(Dig2Pin, OUTPUT);
	pinMode(Dig3Pin, OUTPUT); 
	Serial.println(F("IO LedBank is initialized"));

	// Initialization I/O
	pinMode(ButtonStopPin, INPUT);
	pinMode(ButtonStartPin, INPUT);
	pinMode(ButtonResetPin, INPUT);
	pinMode(ButtonUpPin, INPUT);
	pinMode(ButtonDownPin, INPUT);
	pinMode(LedBankPin, OUTPUT);
	pinMode(BuzzerPin, OUTPUT);
	Serial.println(F("I/O is initialized"));

	// Initialization of interrupt timer
	Timer1.initialize(5000);  // Intterrupt every 10ms
	Timer1.attachInterrupt(TimerInterrupt);
	
	CurrentMinute = StartMinute;
	CurrentSecond = StartSecond;
}

/*
 * Main routine
 */
void loop(){
	ReadButtons();  // Read input variable and set outputs
	delay(150);
	setTime();
	Value[0] = CurrentMinute / 10;
	Value[1] = CurrentMinute % 10;
	Value[2] = CurrentSecond / 10;
	Value[3] = CurrentSecond % 10;
}

/***********************************************************************
						Functions
***********************************************************************/
/*
 * This function is called to read in the IO statements en do the necessary actions
 */
void ReadButtons(){
	// Read the button states
	StopButtonState = digitalRead(ButtonStopPin);
	StartButtonState = digitalRead(ButtonStartPin);
	ResetButtonState = digitalRead(ButtonResetPin);
	UpButtonState = digitalRead(ButtonUpPin);
	DownButtonState = digitalRead(ButtonDownPin);

	// For the start button
	if(StartButtonState == HIGH){
		if(isRunning == 0){
			isRunning = 1;
			CurrentSecond = StartSecond;
			CurrentMinute = StartMinute;
			digitalWrite(LedBankPin, HIGH);
		}
	}

	// For the stop button
	if(StopButtonState == HIGH && isRunning == 1){
		digitalWrite(LedBankPin, LOW);
		isRunning = 0;
	}

	// For the reset button
	if(ResetButtonState == HIGH){
		if(isRunning == 0){
			digitalWrite(LedBankPin, LOW);
			CurrentSecond = StartSecond;
			CurrentMinute = StartMinute;
		}
	}

	// For the Up button
	if(UpButtonState == HIGH){
		if(isRunning == 0){
			if(StartMinute == MAX_MINUTE){
				StartSecond = 0;
				StartMinute = 60;
			}else{
				if(StartSecond >= MAX_SECOND-10){
					StartSecond = 0;
					StartMinute++;
					if(StartMinute >= 60){
						StartMinute = 60;
					}
				} else {
					StartSecond += 10;
				}
			}
			CurrentSecond = StartSecond;
			CurrentMinute = StartMinute;
		}
	}

	// For the Down button
	if(DownButtonState == HIGH){
		if(isRunning == 0){
			if(StartSecond == 0 && StartMinute == 0){
				StartSecond = 0;
				StartMinute = 0;
			} else {
				if(StartSecond == 0){
					StartSecond = 50;
					StartMinute--;
					if(StartMinute == 255){
						StartMinute = 0;
					}
				}else {
					StartSecond -= 10;
				}
			}
			CurrentSecond = StartSecond;
			CurrentMinute = StartMinute;
		}
	}
}

/*
 * This function is called by the timer interrupt
 */
void setTime(){
	if(isRunning == 1){
		if(counter >= 7){
			CurrentSecond--;
		if(CurrentMinute == 0 && CurrentSecond == 0){
			isRunning = 0;
			digitalWrite(LedBankPin, LOW);
			tone(BuzzerPin, 3000, 3000);
      }else if(CurrentSecond == 0){
				CurrentMinute--;
				CurrentSecond = 59;
			}
			counter = 1;
		} else {
			counter++;
		}
	}
}

/*
 * 
 */
void WriteDigit(volatile byte number){
	for(volatile byte i = 0; i < 8; i++){
		if((number & 0x80) == 0x80){ // Shift bit by bit data in shift register
			digitalWrite(DataPin, HIGH);
		} else {
			digitalWrite(DataPin, LOW);
		}
		number = number<<1;
		// Give Clock pulse
		digitalWrite(ClockPin, LOW);
		digitalWrite(ClockPin, HIGH);
	}
	// Latch the data
	digitalWrite(LatchPin, LOW);
	digitalWrite(LatchPin, HIGH);
}

/*
 * 
 */
void WriteLedBankDisplay(){
	switch(Pointer){
		case 0:
			digitalWrite(Dig3Pin, LOW);
			WriteDigit(SegData[Value[0]]);
			digitalWrite(Dig0Pin, HIGH);
			break;
    
		case 1:
			digitalWrite(Dig0Pin, LOW);
			WriteDigit(SegData[Value[1]]);
			digitalWrite(Dig1Pin, HIGH);
			break;
    
		case 2:
			digitalWrite(Dig1Pin, LOW);
			WriteDigit(SegData[Value[2]]);
			digitalWrite(Dig2Pin, HIGH);
			break;
    
		case 3:
			digitalWrite(Dig2Pin, LOW);
			WriteDigit(SegData[Value[3]]);
			digitalWrite(Dig3Pin, HIGH);
			break;

		default:
			break;
	}
}

/*
 * 
 */
void TimerInterrupt(){
  Pointer++;
  if(Pointer >=4){
	  Pointer = 0;
  }
  WriteLedBankDisplay();
  TCNT0=0xCC;
}

