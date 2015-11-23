/*
*******************************************************************
****         Arduino sketch for EGEN310 MiniProject #2         ****
****      Salinity probe made of an anode and sensor to read   ****
****  voltage through ADC to measure conductivity. Serial I/O  ****
****  will be used to control sketch remotely and operate      ****
****  servos for mixer and probe movement, 12v DC fluid pump   ****
****  and all other code required for the EGEN310 assignment.  ****
****                                                           ****
****   This version of the code is intended to be used in      ****
****   conjunction with MakerPlot for serial control and       ****
****   communication with the microcontroller with MP GUI      ****
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
#define mlPerSecond 25.0                        // volume per second of pump output, used to calculate time pump runs for desired volume
const uint16_t PAUSE = 300;                     // delay period between readings in milliseconds
const uint8_t PROBEPIN = A0;                    // input pin for probe to ADC
const uint8_t TEMPPIN = A3;                     // input pin for thermometer to ADC
const uint8_t PUMPPIN = 9;                      // digital pin for pump MOSFET
const uint8_t MIXERPIN = 3;                     // PWM pin for mixer servo
const uint8_t PROBEANODE = 12;                  // digital pin for probe on/off transistor *** active low, PNP transistor ***
const uint16_t ADCMIN = 500;                    // ADC count minimum used for mapping count to percent salt, determined from calibration experiment
const uint16_t ADCMAX = 911;                    // ADC count maximum used for mapping count to percent salt, determined form calibration experiment

//  ----- Variables  -----  //
float rolling;                                  // analog reading rolling avg, mainly used for comparison to calculated mean
float percentSalt;                              // final calculation of percent salt by weight
float mean;                                     // mean for std. dev. calculation or store info from stats library
float stdDev;                                   // store info from stats library or just use print statements to view in monitor
int readings[samples];                          // ***  samples needs to be a define or this array will not be global  ***
float voltage[samples];                         // raw ADC data converted to voltage in millivolts
float temperatureC[samples];                    // temperatures calculated from voltages in Celsius
float temperatureF[samples];                    // temperatures calculated from Celsius in Fahrenheit

//  ----- Inits -----  //
Servo mixerServo;                               // servo object for mixer motor
Statistics stats(samples);                      // create instance of Stats object for standard dev, mean etc. calculations

void setup(){
	pinMode(PUMPPIN, OUTPUT);
	pinMode(PROBEANODE, OUTPUT);
	digitalWrite(PUMPPIN, LOW);
	digitalWrite(PROBEANODE, HIGH);             // set pin high since using a PNP transistor this circuit is active low, may not need this since we are using a pull-up resistor
	mixerServo.attach(MIXERPIN);
	mixerServo.write(90);                       // initialize continuous rotation servo to stopped position
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
	printMenu();
}  // end setup()

void loop(){
	char menuSelection;
	if (Serial.available() > 0){
		menuSelection = Serial.read();
		switch(menuSelection){
			case 'm':
			runMixer();
			flushBuffer();
			break;
			case 'p':
			dispenseSolution();
			flushBuffer();
			break;
			case 's':
			salinityTest();
			flushBuffer();
			break;
			case 'd':
			plotData();
			flushBuffer();
			break;
			default:
			Serial.println("Invalid entry, try again.");
			printMenu();
			flushBuffer();
			break;
		}  // end switch
	}  // end if serial.avail()
}  // end loop()


// function to print menu/choices while using terminal to control sketch
void printMenu(){
	Serial.println("Please make a selection from the following:");
	Serial.println("     Enter 'm' to start mixer,");
	Serial.println("     Enter 'p' to enter desired volume and start pump,");
	Serial.println("     Enter 's' to measure salinity,");
	Serial.println("     Enter 'd' to display data on plot.");
}  //  end printMenu()

// run mixer servo for a period of time
void runMixer(){
	//Serial.println("Just a stub need to finish runMixer()");
	mixerServo.write(0);                        // continuous rotation servo: 0=full speed one direction, 90=stopped, 180=full speed opposite direction
	Serial.println("Press any key to stop mixer motor.");
	flushBuffer();
	while (Serial.available() == 0){
	}
	mixerServo.write(90);                       // turn off servo
	flushBuffer();
	printMenu();
}  // end runMixer()

// run pump to dispense desired volume
void dispenseSolution(){
	//Serial.println("Just a stub, need to finish dispenseSolution.");
	String userInput;
	float volume;
	char check = 'n';                           // used for incoming byte to verify volume input, initialize to anything but 'y'
	
	while (check != 'y'){
		Serial.setTimeout(5000);
		Serial.println("Enter desired volume to dispense in milliliters:");
		flushBuffer();
		while (Serial.available() == 0){
		}
		userInput = Serial.readStringUntil('\n');    //  ***  be sure to have terminal program set to send new line/LF char  ***
		volume = userInput.toFloat();
		//volume = Serial.parseFloat();
		Serial.print("You have entered: ");
		Serial.print(volume);
		Serial.println(" milliliters.");
		Serial.println("Press 'y' to confirm volume, any other key to re-enter volume desired.");
		while (Serial.available() == 0){
		}
		check = Serial.read();
		Serial.print("Check byte is: ");
		Serial.println(check);
	}
	
	if (check == 'y'){                          // redundant check
		int period = (volume/mlPerSecond) * 1000; 
		digitalWrite(PUMPPIN, HIGH);
		delay(period);
		digitalWrite(PUMPPIN, LOW);
	}else{
		Serial.println("Exiting dispenseSolution()");
	}
	
	printMenu();
	
}  // end dispenseSolution()

// gather salinity probe readings
void salinityTest(){
	Serial.println("Just a stub, need to finish salinityTest()");
	digitalWrite(PROBEANODE, LOW);              // turn on power to anode side of probe, using PNP transistor = active low
	buildDataSet(PROBEPIN);
	digitalWrite(PROBEANODE, HIGH);             // turn off power to anode
	rolling = calculateRollingAvg();
	if (rolling < ADCMIN){
		percentSalt = 0.0;
	}else if(rolling > (ADCMIN -1) && rolling < (ADCMAX + 1)){
		percentSalt = map(rolling, ADCMIN, ADCMAX, 0, 26);
	}else{
		percentSalt = 26.0;
	}
	// ***  need to finish data processing and add result to variable to be plotted  ***
	printMenu();
}  // end salinityTest()

// plot all data to time plot
//  all serial output needs to be formatted for makerPlot
void plotData(){
	Serial.println("Just a stub, need finish plotData");
	salinityTest();
	Serial.println(percentSalt);
}  // end plotData()

//  function to fill array with raw ADC readings
void buildDataSet(int inputPin){                //  could combine all these functions into one
	                                            //  but having separate arrays we can manipulate
	                                            //  data in more ways for analysis
	// fill array with readings from ADC
	for (int i; i < samples; i++){
		readings[i] = analogRead(inputPin);
		delay(PAUSE);
	}
}  // end getDataSet()

// function to calculate rolling average of ADC counts
// be sure to call buildDataSet() before calling this function
float calculateRollingAvg(){                    //  ***  maybe change this to accept an array as a parameter???  ***
	float average = ((float)readings[0] + (float)readings[1]) / 2;
	for (int i = 2; i < samples; i++){
		average = (average + (float)readings[i]) / 2;
	}
	return average;
}  //  end calculateRollingAvg()

//function to calculate rolling average from an array
float rollingAvgFromArray(float theArray[]){    // will automatically pass by reference, no need for &
	float average = (theArray[0] + theArray[1]) / 2;
	
	for (int i = 2; i < sizeof(theArray); i++){
		average = (average + theArray[i]) / 2;
	}
	return average;
}  // end of rollingAvgFromArray()


// function to clear serial output buffer
// since arduino 1.0 serial.flush() acts differently
void flushBuffer(){
	Serial.flush();
	while (Serial.available())
		Serial.read();
}  // end flushBuffer()

// function to convert single ADC count to millivolt value
float rawToVoltage(float count){
	/*Serial.println("Converting ADC counts to millivolts.....");
	float volts = 0.0;
	for (int i; i < samples; i++){
		volts = (float)readings[i] * arefVoltage;
		volts /= 1024.0;
		voltage[i] = volts;
	}*/
	float volts = 0.0;
	volts = count * arefVoltage;
	volts /= 1024.0;
	return volts;
}  // end rawToVoltage()