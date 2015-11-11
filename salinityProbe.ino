/*
*******************************************************************
****         Arduino sketch for EGEN310 MiniProject #2         ****
****      Salinity probe made of an anode and sensor to read   ****
****  voltage through ADC to measure conductivity. Serial I/O  ****
****  will be used to control sketch remotely and operate      ****
****  servos for mixer and probe movement, 12v DC fluid pump   ****
****  and all other code required for the EGEN310 assignment.  ****
*******************************************************************
by David Schwehr Novembr 2015 (for Group D5)
   github: dssquared
*/

#include <Servo.h>
#include <Statistics.h>
#include <IntStatistics.h>

//  ----- Defines and Constants  -----  //
#define samples 30                              // number of readings to include in standard deviation calc also length of data array
#define arefVoltage 5.0                         // external reference voltage, 1.1v internal, or default 5v  *** besure to change analogRef()  ***
const uint16_t pause = 300;                     // delay period between readings in milliseconds
const uint8_t PROBEPIN = A0;                    // input pin for probe to ADC
const uint8_t TEMPPIN = A3;                     // input pin for thermometer to ADC
const uint8_t PUMPPIN = 3;                      // digital pin for pump MOSFET
const uint8_t MIXERPIN = 9;                     // PWM pin for mixer servo
const uint8_t PROBEANODE = 8;                   // digital pin for probe on/off transistor *** active low, PNP transistor ***

//  ----- Variables  -----  //
volatile uint16_t ADCcount;                     // analog read values  ***  may not use  ***
float rolling;                                  // analog reading rolling avg, mainly used for comparison to calculated mean
float mean;                                     // mean for std. dev. calculation or store info from stats library
float variance;                                 // variance for std dev. calculation or store info from stats library
float stdDev;                                   // store info from stats library or just use print statements to view in monitor
int readings[samples];                          // ***  samples needs to be a define or this array will not be global  ***
float voltage[samples];                         // raw ADC data converted to voltage in millivolts
float temperatureC[samples];                    // temperatures calculated from voltages in Celsius
float temperatureF[samples];                    // temperatures calculated from Celsius in Fahrenheit

//  ----- Inits -----  //
Servo mixerServo;                               // servo object for mixer motor
Statistics stats(samples);                      // create instance of Stats object for standard dev, mean etc. calculations

void setup(){
	mixerServo.attach(MIXERPIN);
	Serial.begin(9600);
	analogReference(DEFAULT);                   // initialize reference voltage   *** be sure to set #define arefVoltage accordingly  ***
	                                            // DEFAULT = 5v, INTERNAL = 1.1v, EXTERNAL = external reference on AREF pin
												
	// initialize arrays
	for (int i; i < samples; i++){
		readings[i] = 0;
		voltage[i] = 0.0;
		temperatureC[i] = 0.0;
		temperatureF[i] = 0.0;
	}
}  // end setup()

void loop(){
	
}  // end loop()
