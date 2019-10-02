
#include "Arduino.h"
#include <stdio.h>
#include <stdarg.h>
#include "CPutil.h"

#define __VERSION "CPE123_Fall1_18 v2.6"
#define __UPDATED "Dec 2017"

void printLibVersion(Stream & aStream)
{
	char buf[100];
	sprintf(buf, "Library Version: %s Last Updated: %s\n", __VERSION, __UPDATED);
	aStream.print(buf);
}

void setupMessage(const char * file, const char * message, Stream & aStream)
{
	// Pass in __FILE__
	aStream.print("Begin Program: ");
    aStream.println(file);     // prints out the file I am running
	aStream.print("Compiled: ");
    aStream.println(__TIMESTAMP__); // prints out the time you compiled the sketch
	if (message != NULL)
	{
		aStream.println(message);
	}
  
 }

 
void serialPrintf(const char * format, ...)
{
  char buffer[MAX_PRINTF];
  
  va_list args;
  va_start(args, format);
  vsprintf(buffer,format, args);
  Serial.print(buffer);
  va_end (args);
}

void serialPrintf(Stream & aStream, const char * format, ...)
{
  char buffer[MAX_PRINTF];
  
  va_list args;
  va_start(args, format);
  vsprintf(buffer,format, args);
  aStream.print(buffer);
  va_end (args);
}

//#endif

void printVolts(int aPin, char * voltageBuf, Stream & aStream)
{
  // To be used to monitor your battery.  
  
  // http://forum.arduino.cc/index.php/topic,10058.0.html (mem)
  // Code and comment below taken from the forum.arduino.cc link above.
  // Username mem from London

  // The battery voltage on the divider ranges from 0 to 10 volts.
  // The voltage on the arduino pin ranges from 0 to 5 volts (assuming two 4.7k resistors)
  // So an analogRead value of 1024 =  10 volts on the  divider
  // Therefore one bit is equal to 10v divided by 1024 (This is where the 102 comes from)

  if (aPin != 255)
  {
    int value = analogRead(aPin);
    aStream.print("voltage is: ");
    aStream.print(value / 102, DEC);    // print the integer value of volts
    aStream.print(".");                 // decimal point
    aStream.println(value % 102, DEC); // hundredths of a volt
	
	if (voltageBuf != NULL)
	{
		sprintf(voltageBuf, "%d.%d", value / 102, value % 102 );
	}
	
  }
}



//*************************************************** MSTimer
//
// Put your MSTimer code here

// This is a non-blocking delay 
// Eg. 
// MSTimer delayit;
// delayit.set(1000);
// if (delayit.delay() == true) ....

MSTimer::MSTimer()
{
 // initalize all class variables
  msDuration = 0;
  start_time = 0;
  msDelayState = MSTIMER_DONE;
  
}

MSTimer::MSTimer(uint32_t ms_duration)
{
  reset(ms_duration);
}

MSTimer MSTimer::operator=(const uint32_t ms_duration)
{
	reset(ms_duration);
}

void MSTimer::set(uint32_t ms_duration)
{
  reset(ms_duration);
}

void MSTimer::reset(uint32_t ms_duration)
{
  msDuration = ms_duration;
  start_time = millis();
  msDelayState = DELAYING;
}

unsigned long MSTimer::remaining()
{
	unsigned long timeLeft = msDuration - (millis() - start_time);
	
	if (timeLeft < 0)
		timeLeft = 0;		
	
	return timeLeft;
}

int MSTimer::done()
{
  int returnValue = true;
  
  switch (msDelayState)
  {
    case MSTIMER_DONE:
      returnValue = true;
    break;
    
    case DELAYING:
      if ((millis() - start_time) < msDuration)
      {
        returnValue = false;
      }
      else
      {
        returnValue = true;
        msDuration = 0;
        msDelayState = MSTIMER_DONE;
      }
     break;
     
     default:
       break;
  }
        
  return returnValue;
}


//**************************************************************
//
// Led code (you can use my LED code but you will need to implement the 
// blink() method.

Led::Led(int ledPin)
{
	// initalize all class variables
	initLed(ledPin);
  
}

Led::Led()
{
	initLed(-1);  

}

void Led::initLed(const int aPin)
{
	pin = aPin;
	durationOn = 0;
	shortDurationOff = 0;
	longDurationOff = 0;
	numRepeatBlinks = 0;
	repeatCount = 0;
	ledDelay.set(0);
	blinkState = BLINKING_OFF;
	
	initPin();
}

void Led::initPin()
{

	if (pin != -1)
	{
		pinMode(pin, OUTPUT);
		digitalWrite(pin, LOW);
	}
}

Led Led::operator=(const uint8_t aPin)
{
	initLed(aPin);
}

void Led::on()
{
	if (pin != -1)
	{
		blinkState = BLINKING_OFF;
		digitalWrite(pin, HIGH);
	}
}

void Led::ledSetOn()
{
	if (pin != -1)
	{
		digitalWrite(pin, HIGH);
	}
}

void Led::off()
{
	blinkState = BLINKING_OFF;
 	if (pin != -1)
	{
		digitalWrite(pin, LOW);
	}
}

void Led::blinkOn(unsigned long aDuration)
{	
	if (pin != -1)
	{
		// this is a on then off then on... same duration blink
		numberedBlinkOn(1, aDuration, aDuration, aDuration);
	}
}

void Led::numberedBlinkOn(int repeatsOn, uint32_t aDurationOn, uint32_t aShortDurationOff, uint32_t aLongDurationOff)
{
	if (pin == -1)
	{
		return;
	}
	
	// Store the info needed to define the on/off cycle including repeat count and 
	// the timing info 
	durationOn = aDurationOn;
	shortDurationOff = aShortDurationOff;
	longDurationOff = aLongDurationOff;
	numRepeatBlinks = repeatsOn;  // how many on per off 

	// now setup the state stuff for the update function
	ledDelay.set(durationOn);
	repeatCount = 1;		// how many ons have happend before off (counter)
	ledSetOn();
	blinkState = ON_BLINK;
}


void Led::update()
{
	if (pin == -1)
	{
		return;
	}
		
	switch (blinkState)
	{
		case ON_BLINK:
		  if (ledDelay.done() == true && repeatCount < numRepeatBlinks)
		  {
			ledDelay.set(shortDurationOff);
			digitalWrite(pin, LOW);
			repeatCount++;
			blinkState = OFF_BLINK_SHORT;
		  }
		  else if (ledDelay.done() == true && repeatCount >= numRepeatBlinks)
		  {
			  ledDelay.set(longDurationOff);
			  repeatCount = 1;
			  digitalWrite(pin, LOW);
			  blinkState = OFF_BLINK_LONG;
			  
		  }
		break;

		case OFF_BLINK_SHORT:
		  if (ledDelay.done() == true)
		  {
			ledDelay.set(durationOn);
			ledSetOn();
			blinkState = ON_BLINK;
		  }
		break;

		case OFF_BLINK_LONG:
			if (ledDelay.done() == true)
			{
				ledDelay.set(durationOn);
				ledSetOn();
				repeatCount = 1;
				blinkState = ON_BLINK;
			}

		break;

		case BLINKING_OFF:
		  // nothing since the led should already be off
		  // We get here when off() is called.
		break;

		default:
		  break;
	}
}
      

//***********************************************************
//
// Button code

Button::Button(int buttonPin)
{
  // initalize all class variables
  pin = buttonPin;
  pinMode(pin, INPUT_PULLUP); 

  pushState = PUSH_START;
  pushTimer.set(0);
 
  durState = 0;
  durStart = 0;
  durTimer.set(0);
 
}

int Button::wasPushed(int pushWait)
{
  // returns true if button pushed and then ignores button for a period of time
  
  int returnValue = false;
  
  switch (pushState)
  {
    case PUSH_START:
      if (digitalRead(pin) == LOW)
      {
        pushTimer.set(pushWait);
        returnValue = true;
        pushState = PUSH_WAITING;
      }
    break;
   
   case PUSH_WAITING:
    if (pushTimer.done() == true)
    {
      returnValue = false;
      pushState = PUSH_START;
    }
   break;
    
    default: 
      break;
  }
  
  return (returnValue);
}

int Button::isCurrentlyPushed()
{
	// Replace isPushed() with this function is the future (Fall 2016)
	int returnValue = false;
	
	if (duration() > 0)
	{
		returnValue = true;
	}
	
	return returnValue;
}

void Button::waitOnPush(Led & aLed, int delayTime)
{
	// allows pass by reference of the led 
	waitOnPush(&aLed, delayTime);

}

void Button::waitOnPush(int delayTime)
{
	// allows pass by reference of the led 
	waitOnPush(NULL, delayTime);

}

void Button::waitOnPush(Led * waitLed, int delayTime)
{
	const char preMessage[] = "Press button to continue...";
	const char postMessage[] = "Starting...";

	Serial.println(preMessage);	
		
	if (waitLed != NULL)
	{
		waitLed->blinkOn(500);
	}
				  
	while (wasPushed() == false)
	{ 
		// looping here
		if (waitLed != NULL)
		{
			waitLed->update();
		}
	}

	Serial.println(postMessage);
		
	if (waitLed != NULL)
	{
		waitLed->on();
	}
	
	if (delayTime > 0)
	{
		delay(delayTime);
	}
}

unsigned long Button::duration()
{
	// debounces button for 200 ms
  int returnValue = 0;
  
  switch (durState)
  {
    case DUR_START:
      if (digitalRead(pin) == LOW)
      {
        durStart = millis();
        durTimer.set(200);
        returnValue = 0;
        durState = DUR_WAITING;
      }
     break;
     
     case DUR_WAITING:
       if (digitalRead(pin) == HIGH)
       {
         returnValue = 0;
         durState = DUR_START;
       }
       else if (durTimer.done() == true)
       {
         returnValue = millis() - durStart;
         durState = DUR_TIME;
       }
      break;
      
      case DUR_TIME:
        if (digitalRead(pin) == HIGH)
        {
          returnValue = 0;
          durState = DUR_START;
        }
        else // digitalRead low
        {
          returnValue = millis() - durStart;
          durState = DUR_TIME;
        }
       break;
       
       default:
         break;
  }
  
  return (returnValue);
}  
  
  