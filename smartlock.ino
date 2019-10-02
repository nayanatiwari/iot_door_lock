//Nayana Tiwari, Comal Virdi, Samay Nathani, Stephan Patch
#include <CPutil.h>
#include <NewPing.h>
#include <NewTone.h>
#include <ServoTimer2.h>

#define maxDistance 200
const int doorStatusButtonPin = 12;
const int userButtonPin = 11;
const int buzzerPin = 10;
const int trigPin = 9;
const int echoPin = 8;
const int ledPin = 49;
const int led2Pin = 50;

ServoTimer2 doorServo;
ServoTimer2 lockServo;

NewPing sonar(trigPin, echoPin, maxDistance);
Button doorStatusButton(doorStatusButtonPin);
Button userButton(userButtonPin);
Led led(ledPin);
Led led2(led2Pin);

//enum statement for changing the LEDs
enum{STARTUP, UNLOCKED, LOCKED};

//Global Variables for RFID
int ValidRFIDTag[14] = {2,48,56,48,48,52,53,68,54,54,49,70,65,3};

int ScannedRFIDTag[14] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int RFIDDecision = -1;
int RFIDReadVariable = 0;

void setup()
{
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  
  doorServo.attach(7);
  lockServo.attach(6);
  
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial2.begin(9600); //This is the RFID Scanner
  
  delay(500);
  
  changeLeds(LOCKED);
}

void loop()
{
  control();
}

void control()
{
  if(checkRFID())
  {
    doorOpen();
    led.off();
    while (!verification2())
    {
      led2.on();
    }
    led2.off();
    led.on();
  }
  else if(userButton.wasPushed())
  {
    doorOpen();
    while (!verification1())
    {
      led.on();
    }
    led.off();
  }
}
 // verification 1 code
int verification1()
{
  static MSTimer timer;
  enum {START, YES_RFID, NO_RFID, NO_RFID_CHECK};
  static int state = START;
  int returnValue = false;
  
  switch(state)
  {
    case START:
      if(verifyRFID())
      {
        led.on();
        state = YES_RFID;
      }
      else
      {
        state = NO_RFID;  
      }
    break;
      
    case YES_RFID:
      doorClose(true);
      returnValue = true;
      state = START;
    break;
    
    case NO_RFID:
      timer.set(300000);
      NewTone(buzzerPin, 1088);
      Serial.println("User has left home WITHOUT KEY!"); // Mimics Message to caregiver
      doorClose(false);
      state = NO_RFID_CHECK;
    break;
    
    case NO_RFID_CHECK:
      if (checkRFID() || timer.done())
      {
        noNewTone(buzzerPin);
        lock();
        state = YES_RFID;
      }
    break;
  }
 return returnValue;
}

// verification 2 code
int verification2()
{
  static MSTimer timer;
  enum {START, CHECKBUTTON, DOORCLOSE};
  static int state = START;
  int returnValue = false;
 
  switch(state)
  {
    case START:
      timer.set(300000);     
      state = CHECKBUTTON;
    break;

    case CHECKBUTTON:
      if(userButton.wasPushed())
      { 
        state = START;
        returnValue = true;
        doorClose(true);
        led2.off();
      }
      else if(timer.done())
      {
       timer.set(3000);
       NewTone(buzzerPin, 1088);
       state = DOORCLOSE;
      }
    break;

    case DOORCLOSE:
      if(timer.done())
      {
        noNewTone(buzzerPin);
        doorClose(true);
        returnValue = true;
      }
    break; 
  }
 return returnValue;
}

void ultrasonicSensor(int passedLockFlag)
{
  enum{DOOR_CLEAR,DOOR_BLOCKED};
  static int state = DOOR_CLEAR;
  
  switch(state)
  {
    case DOOR_CLEAR:
      if(checkDoorway())
      {
        opened();
        NewTone(buzzerPin,1088);
        state = DOOR_BLOCKED;
      }
    break;
    case DOOR_BLOCKED:
      if(!checkDoorway())
      {
        noNewTone(buzzerPin);
        state = DOOR_CLEAR;
        doorClose(passedLockFlag);
        Serial.println("blocked");
      }
    break;
  }
}

void doorClose(int lockFlag)
{
  closed();
  while(!doorStatusButton.wasPushed())
  {
    ultrasonicSensor(lockFlag);
  }
  if(lockFlag)
  {
    lock();
    changeLeds(LOCKED);
  }
}

void doorOpen()
{
  unlock();
  changeLeds(UNLOCKED);
  delay(250);
  opened();
}


int checkRFID()
{
  int returnValue = false;
  int i = 0;
  RFIDDecision = -1;

  if(Serial2.available() > 0)
  {
    delay(100);

    for(i = 0;i < 14;i++)
    {
      RFIDReadVariable = Serial2.read();
      ScannedRFIDTag[i] = RFIDReadVariable;
    }

    Serial2.flush();
    
    //Flush the Serial Buffer ALOT
    for(i = 0;i < 10;i++)
    {
      while(Serial2.available() > 0)
      {
        Serial2.read();
        delay(1);
      }
      delay(1);
    }

    checkTag();
  }

  if(RFIDDecision > 0)
  {
    returnValue = true;
    RFIDDecision = -1;
  }
  else if(RFIDDecision == 0)
  {
    returnValue = false;
    RFIDDecision = -1;
  }
  
  return returnValue;
}

int verifyRFID()
{
  int returnValue = false;
  int i = 0;
  RFIDDecision = -1;
  
  static MSTimer timer;
  timer.set(60000);
  
  while(!timer.done() && RFIDDecision == -1)
  {
    if(Serial2.available() > 0)
    {
      delay(100);

      for(i = 0;i < 14;i++)
      {
        RFIDReadVariable = Serial2.read();
        ScannedRFIDTag[i] = RFIDReadVariable;
      }
      
      Serial2.flush();
      
      //Flush the Serial Buffer ALOT
      for(i = 0;i < 10;i++)
      {
        while(Serial2.available() > 0)
        {
          Serial2.read();
          delay(1);
        }
        delay(1);
      }
        
      checkTag();
    }
  }

  if(RFIDDecision > 0)
  {
    returnValue = true;
    RFIDDecision = -1;
  }
  else if(RFIDDecision == 0)
  {
    returnValue = false;
    RFIDDecision = -1;
  }
  
  return returnValue;
}

void checkTag()
{
  //Note: -1 is No Read Attempt, 0 is Read but Not Validated, 1 is Valid
  RFIDDecision = 0;

  if(compareTag(ScannedRFIDTag,ValidRFIDTag) == true)
  {
    RFIDDecision++;
  }
}

int compareTag(int readDigit[14], int validDigit[14])
{
  int returnFlag = false;
  int digit = 0;
  int i = 0;

  for(i = 0; i < 14; i++)
  {
    if(readDigit[i] == validDigit[i])
    {
      digit++;
    }
  }

  if (digit == 14)
  {
    returnFlag = true;
  }

  return returnFlag;
}

//Code to communicate to the second board to change the LED States
void changeLeds(int newState)
{
  if(newState == STARTUP)
  {
    Serial1.println(STARTUP);
  }
  else if(newState == UNLOCKED)
  {
    Serial1.println(UNLOCKED);
  }
  else if(newState == LOCKED)
  {
    Serial1.println(LOCKED);
  }
}

//proximity sensor code
int checkDoorway()
{
  int value = 0;
  
  value =sonar.ping_cm();
  if (value > 10 && value < 30) // When in use in a real doorway, it would check value > 20 && value < 180
  {
    return true;
  }
  else
  {
    return false;
  }
}

// Servo lock and unlock code for lock servo + close and open for the door servo
void unlock()
{
  lockServo.write(1500);
  Serial.println("unlock");
}

void lock()
{
  lockServo.write(2250);
  Serial.println("locked");
}
void closed()
{
  doorServo.write(750);
  Serial.println("closed");
}
void opened()
{
  doorServo.write(1750);
  Serial.println("opened");
}