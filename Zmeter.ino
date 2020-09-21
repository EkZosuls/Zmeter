/* Zmeter DC multimeter  
 *  Aleks Zosuls 2020
 *  
 *  !!DO NOT EXCEED THE MICROCONTROLLER CORE VOLTAGE WHEN USING THIS PROGRAM!!
 *  
 *  Use a simple command line interface to measure DC voltage and resistance with an Arduino IDE
 *  compatible device.
 *  This version is configured for Arduino Uno or Nano 5V
 *  
 *  COMMANDS: Type them into the serial monitor input. Case insensitive
 *  
 *  V: measure volts. This makes a voltage measurement across the "voltInputPinP" and "voltInputPinN". 
 *  The pins are variables you can set. The range is 0 volts to 3.3 or 5 volts on both pins. The value  
 *  can be negative or positive depending on the polarity of the leads but note the ground referenced 
 *  voltage range on each 'test lead' is 0 to core voltage (usually 3.3 or 5V).
 *  
 *  R: measure resistance. For this to work you need to connect a sense resistor to the core voltage
 *  pin (5V on the Uno). The other side of the resistor is connected to a test lead and an analog input
 *  pin specified by "rInputPin". A second test lead is connected to ground. 
 *  The two test leads measure resistance through a device such as 
 *  a resistor or strain gauge. The sense resistor needs to be specified with the processor directive
 *  SENSE_RES_OHMS. The accuracy of ths resistance measurement deteriorates as your device under test
 *  resistance gets a few decades away from the sense resistor. 
 *  
 *  S: pauses measurements. stops the data collection.
 *  
 *  Notes:
 *  The voltage range is only within the input range of the microprocessor pin. This is typically
 *  in the 0 to 5 volt range or 0 to 3.3 volt range depending on your device. Exceeding this 
 *  will probably ruin your device and may damage your computer and expose you to an 
 *  electrocution risk. BE CAREFUL!! Think and ask qualified people before acting.
 *  
 *  Note that there is a finite input resistance on the analog inputs of these microprocessors.
 *  This may result in accuracy issues when working with high resistance sources
 *  and resistance values (>100,000 ohms).
 *  
 *  MIT License

Copyright (c) 2020 Aleksandrs Zosuls

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

 *  
 */

 // this sets the average window length
#define AVG_WINDOW 10
//this defines a voltage sensing resistor value in Ohms
#define SENSE_RES_OHMS 1000
// the voltage for the resistor divider ohmmeter
#define SYSTEM_VOLTS 5.00

//user declared functions
float computeAvg(int *myArrg);
void collectPoints(int sampleInterval, int pinNum,int pinNumN, int *myArrg);

//variables
int dispRefresh = 450;  //how often the display updates. needs to be >100
int voltInputPinP = 0; //pin connected to voltmeter positive terminal
int voltInputPinN = 1; //pin connected to voltmeter positive terminal
int rInputPin = 2;  //pin connected to ohm meter
int sampleInterval = 1; // time between multiple samples
int ADCRaw[AVG_WINDOW] = {0};
int controlByte = 'V'; //
int controlFlag = 1;  // inital mode state
float ADCconversion = 0.00488;
float myAverage;
float voltage;
float ohms;

void setup() {
  Serial.begin(9600); //Init serial comms. The argument is the speed
  // it should match the serial terminal speed
  Serial.println("Zmeter Arduino multimeter");
  Serial.println("Defaults to measure voltage");
  Serial.println("Enter V to measure volts...");
  Serial.println("enter R to measure resistance...");
  Serial.println("enter S to pause measurements.");
  delay(2000); //delay for 2000 milliseconds or 2 seconds

}

void loop() {
  if (Serial.available() > 0) {
      controlByte = Serial.read(); 
      controlByte = toupper(controlByte); //make it upper case
      //UI check for commands then 
      switch (controlByte) {
        case 'V':
          Serial.println("Measuring Voltage");
          delay(1500);
          controlFlag = 1;
          break;
  
         case 'R':
           Serial.println("Measuring Resistance");
           delay(1500);
           controlFlag = 2;
           break;
  
         case 'S':
           Serial.println("Measuring Paused");
           controlFlag = 0;
           break;
      }
  } //end of if controlByte...
   
//Serial.println("blah");

if (controlFlag == 1) {  //run the VOltmeter
  collectPoints(sampleInterval, voltInputPinP,voltInputPinN, ADCRaw);
  myAverage = computeAvg(ADCRaw);
  voltage = myAverage * ADCconversion;
  Serial.println(voltage,4);
} //end of voltage measure

if (controlFlag == 2) { //run the ohmMeter

  collectPoints(sampleInterval, rInputPin, 'G', ADCRaw);
  myAverage = computeAvg(ADCRaw);
  voltage = myAverage * ADCconversion;
  ohms = SENSE_RES_OHMS/((SYSTEM_VOLTS / voltage) -1);
  if(ohms > (200.0 * SENSE_RES_OHMS)){  //we dont believe relatively high values
    Serial.println("Out of range high");
  }
  else {
    Serial.println(ohms, 1);
  }
}

delay(dispRefresh);  //main loop delay or refresh rate
}

float computeAvg(int *myArrg)
{  //compute the average of the array of points
  int accumulator = 0;
  float val;
  for (int j = 0; j < AVG_WINDOW; j++) {
  accumulator = accumulator + myArrg[j];
  }
  val = (float)accumulator / AVG_WINDOW;
  return val;
  
  }

  void collectPoints(int sampleInterval, int pinNumP, int pinNumN, int *myArrg)
  { // collect N points from 2 analog ins and take the difference.
    int i = 0;
    while(i < AVG_WINDOW) {
      if(pinNumN == 'G'){
        myArrg[i] = analogRead(pinNumP);
      }
      else{
        myArrg[i] = analogRead(pinNumP) - analogRead(pinNumN);
      }
      
      //i = (i + 1) % AVG_WINDOW;
      ++i;
      delay(sampleInterval);
  }
    }
    
