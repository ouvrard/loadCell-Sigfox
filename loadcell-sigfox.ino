#include <SoftwareSerial.h>
#include "Akeru.h"

// Title: 	Cat food monitor - Load cell value over Sigfox network
// Autor: 	Aloys OUVRARD
// Mail: 	aloys.ouvrard_at_gmail.com
// Github: 	https://github.com/ouvrard

// This sketch is based on the Arduino cell amplifier sketch writtent by Christian Liljedahl (christian.liljedahl.dk)

// Set Akeru instance (modem initialization)
Akeru_ akeru;

// Load cells constants 
// Load is calculated by interpolation between two data pairs since the sensor is linear
// First data pair
const float loadA = 0;         // Load A
const int analogvalA = 618;    // Analog reading taken with load A on the load cell

// Second data pair
const float loadB = 500;       // Load B
const int analogvalB = 819;    // Analog reading taken with load B on the load cell

// Load variables
float tare;                    // Tare weight of the food container
float currentLoad = 0;         // Current weight on the load cell (food+container)
float load = 0;                // Food weight in the container
float lastLoad = 0;            // Last food weight send
float sensibility = 2;         // Load scale sensibility
const int nSamples = 30;       // Number of samples for the load value

// Time variables
unsigned long time = 0;                    // Current time initialization
const unsigned long timeoffset = 620000;   // Time offset to send data

// Initialization
void setup() {
  // Turn the LED on during the initialization process
  digitalWrite(13, HIGH);
  // Sigfox modem initilization
  akeru.begin();
  // Waiting for complete modem initialization
  delay(2000);
	
  // Tare the food container weight
  // BTW, It must be empty!
  tare = getLoad();

  // Turn the LED off after the initialization process
  digitalWrite(13, LOW);
}

// Core
void loop() {
  // Time to send data ?
  if(millis() > time + timeoffset){
    // Get the current weigth of the food in the container
    currentLoad = getLoad();
    load = currentLoad - tare;

    // Sometimes the load is negative (but close to zero)
    // This is caused by a bad calibration during the initialization process (maybe...)
    // TODO : fix it !
    if(load < 0)
      load = 0;

    // Set the tolerance of the load scale +/- 2 g
    if(abs(lastLoad - load) <= sensibility)
      load = lastLoad;

    // Turn the LED on during data sending
    digitalWrite(13, HIGH);
    
    // Send data to the Sigfox network
    akeru.send(&load, sizeof(load));

    // Turn the LED on during when data is send
    digitalWrite(13, LOW);

    // Store the current time
    time = millis();
  }
}

// Get the load from analog input
// Smooth the data : average value of n samples (n=nSamples)
float getLoad()
{
  int i;
  int total = 0;
  
  for (i = 0; i < nSamples; i++) {
    // Read the analog input (the sensor)
    total += analogRead(A0);
    delay(10);
  }

  return analogToLoad(total / nSamples);
}

// Convert analog value to load using the two interpolation pairs
float analogToLoad(float analogval){
  float load = mapfloat(analogval, analogvalA, analogvalB, loadA, loadB);
  
  return load;
}

// Overload Arduino Map function (standard arduino map function only uses int)
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  int v = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  
  if(v<0)
    return 0;
	
  return v;
}
