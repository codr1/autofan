#include <DHT.h>
#include <DHT_U.h>

#include <Adafruit_Sensor.h>


/*************************************************
Automatic Temperature Based Fan Control

Video at https://youtu.be/zq1ni1V_5Iw

Based on the project by MHUM-01 example code by Daniel Jay 

Read the current ambient temperature & humidity 
air levels along with the moisture water content

Adjusts the speed of a potentiometer controlled fan to ensure
optimal operating conditions for a media cabinet by optimizing
its speed (and noise level).

   Copyright (C) 2016  Vess Bakalov

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    
*************************************************/


#define DHTTYPE DHT11   // DHT 11


int dhtPin = 10;
int moisturePin = A0;    // select the input pin for the potentiometer
int moistureValue = 0;  // variable to store the value coming from the sensor

int fanControl = 9; //PWM Pin with the fan attached to it
float minTemperature = 80;  // The lowest temperature at which to activate the fan
float maxTemperature = 96;  // The temperature at which the fan will get to full speed.
float tempIncrement = 256.0 / ( maxTemperature - minTemperature );
float fanPower = 0;
float oldFanPower = 0;

int smoothPeriod = 10;  // the amount of time we need to wait before chaning direction of speed (accelerate or decelerate)
int speedingUp = 0;     // counter that keeps the amount of time before we are allowed to slow down
int slowingDown = 0;    // counter that keeps the amount of time before we are allowed to speed up



DHT dht(dhtPin, DHTTYPE);

void setup() {
  // declare the ledPin as an OUTPUT:
   Serial.begin(9600);  
   dht.begin();

   pinMode( fanControl, OUTPUT );
   
}
 
void loop() {

  float newFanPower;

  delay(1000);
   
  // read the value from the sensor:
  moistureValue = analogRead(moisturePin);              
  Serial.print("moisture value  = " );                       
  Serial.println(moistureValue);       


               // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  if( f > minTemperature ) {
    if( f > maxTemperature ) {
      newFanPower = 0;
    } else {
      newFanPower =   tempIncrement * ( maxTemperature - f ) ; 
      //Serial.println( / (maxTemperature - f ));
    }
  } else {
    newFanPower = 255 ;
  }

 
  // Before we write the final value, we want to prevent the system from continuously changing speed.
  // If the fan speeds up, it will be prevented from slowing down for at least 10 seconds.  It can speed up more whenever, though.
  // If the fan slows down, it will be prevented from speeding up for at least 10 seconds.  It can slow down more whenever

  //If any smoothing is alredy in effect, we need to decrement
  if( slowingDown > 0 ) {
    slowingDown--;
  }

  if( speedingUp > 0 ) {
    speedingUp--;
  }
  
  // Are we speeding up?
  if( oldFanPower > newFanPower ) { 
    // ...and we haven't slowed down recently
    if( slowingDown == 0 ) {
      Serial.println( "Speeding Up!" );
      fanPower = newFanPower;
      speedingUp = smoothPeriod;
      oldFanPower = fanPower;
    } else {
      Serial.print( "Smoothing: Seconds before I can speed up: " );
      Serial.println( slowingDown );
      fanPower = oldFanPower;
    }
  }

  // Are we slowing down?
  if( oldFanPower < newFanPower ) {
    // and we haven't sped up down recently
    if( speedingUp == 0 ) {
      Serial.println( "Slowing Down!" );
      fanPower = newFanPower;
      slowingDown = smoothPeriod;
      oldFanPower = fanPower;
    } else {
      Serial.print( "Smoothing: Seconds before I can slow down: " );
      Serial.println( speedingUp );
      fanPower = oldFanPower;
    }
  }

   
  // Are we just hangin'
  if( oldFanPower == newFanPower ) {
    fanPower = newFanPower;
    oldFanPower = fanPower;
  }

  

  analogWrite( fanControl, fanPower );

  // Since my fan is connected to 3.3 volt output, it can only slow down to 33%.
  // We only get to play with the power between 33 - 100
  float fanPercentage = (((255.0 - fanPower) / 255.0 ) * 66.6) + 33.3;
  
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");
  Serial.print("FanPower %: ");
  Serial.println(fanPercentage);
}
