
// Writing by Hugh Smith - 9/2018
//  version 2.6
//
// Timer, LED, Button and support functions 

#ifndef CPUTIL_H
#define CPUTIL_H

#include "Arduino.h"

#ifndef uint8_t
#define uint8_t unsigned char
#endif

#define MAX_PRINTF 150

// Also defined below: print2(x, y); print3(x,y,z); print4(a,b,c,d);
// support functions 
void printLibVersion(Stream & aStream = Serial);
void setupMessage(const char * file, const char * message = NULL, Stream & aStream = Serial);
void serialPrintf(const char * format, ...);
void serialPrintf(Stream & aStream, const char * format, ...);
void printVolts(int aPin, char * voltageBuf = NULL, Stream & aStream = Serial);


class MSTimer {
 
 private:
	// MSTimer enums
	enum {MSTIMER_DONE, DELAYING};

	unsigned long msDuration;
	unsigned long start_time;
	int msDelayState;
	void reset(uint32_t ms_duration);

  public: 
	MSTimer();
	MSTimer(uint32_t ms_duration);
	MSTimer operator=(const uint32_t ms_duration);
	void set(uint32_t ms_duration);
	unsigned long remaining();
	int done();

};



class Led {
  private:
  
	// Led enums 
	enum {ON_BLINK, OFF_BLINK_SHORT, OFF_BLINK_LONG, BLINKING_OFF};
	
    unsigned char pin;
	
    unsigned long durationOn;
	unsigned long shortDurationOff;
	unsigned long longDurationOff;
    int blinkState;
	int numRepeatBlinks;
	int repeatCount;
    MSTimer ledDelay;
		        
    // only used internally
	void initPin();
	void ledSetOn();
 
  public:
    Led(const int ledPin);
	Led();
	void initLed(const int aPin);

    void on();
    void off();
    void update();
	void numberedBlinkOn(int repeatsOn = 2, uint32_t durationOn = 500, uint32_t shortDurationOff = 200, uint32_t longDurationOff = 1000);
    void blinkOn(unsigned long aDuration = 1000);
	Led operator=(const uint8_t aPin);
	
};




class Button
{
		
 private:
	// Button enums
	enum {PUSH_START, PUSH_WAITING};
	enum {DUR_START, DUR_WAITING, DUR_TIME};
 	

	int pin;

	// isPushed variables
	int pushState;
	MSTimer pushTimer;

	// duration variables
	int durState;
	unsigned long durStart;
	MSTimer durTimer;

	public:
	Button(int buttonPin);
	int wasPushed(int pushWait = 500);
	int isCurrentlyPushed();
	
	// allows for led, no led, delayTime, no delayTime, I give, I don't
	// know how to do a default value for Led & aLed... so need all 3 methods
	void waitOnPush(Led &  aLed, int delayTime = 0);
	void waitOnPush(int delayTime);
	void waitOnPush(Led * waitLed = NULL, int delayTime = 0);
		
	unsigned long duration();

};
	
	
// printing multiple data items (I could collapse these into one function except for the newline feature)

template<typename T1, typename T2> void printPair(const T1 aVar1, const T2 aVar2, int newLine = true); 
template<typename T1, typename T2> void printPair(const T1 aVar1, const T2 aVar2, int newLine)
{
	printPair(Serial, aVar1, aVar2, newLine);
	
}

template<typename T1, typename T2> void printPair(Stream & aStream, const T1 aVar1, const T2 aVar2, int newLine = true);
template<typename T1, typename T2> void printPair(Stream & aStream, const T1 aVar1, const T2 aVar2, int newLine)
{
	aStream.print(aVar1);
	aStream.print(aVar2);
	if (newLine)
		aStream.println();
}

template<typename T1, typename T2> void print2(T1 aVar1, T2 aVar2, int newLine = true);
template<typename T1, typename T2> void print2(T1 aVar1, T2 aVar2, int newLine)
{
	printPair(Serial, aVar1, aVar2, newLine);
}

template<typename T1, typename T2, typename T3> void print3(T1 aVar1, T2 aVar2, T3 aVar3, int newLine = true);
template<typename T1, typename T2, typename T3> void print3(T1 aVar1, T2 aVar2, T3 aVar3, int newLine)
{
	printPair(Serial, aVar1, aVar2, false);
	printPair(Serial, aVar3, "", newLine);
}

template<typename T1, typename T2, typename T3, typename T4> void print4(const T1 aVar1, const T2 aVar2, const T3 aVar3, const T4 aVar4, int newLine = true);
template<typename T1, typename T2, typename T3, typename T4> void print4(const T1 aVar1, const T2 aVar2, const T3 aVar3, const T4 aVar4, int newLine)
{
	printPair(Serial, aVar1, aVar2, false);
	printPair(Serial, aVar3, aVar4, newLine);
}


#endif

