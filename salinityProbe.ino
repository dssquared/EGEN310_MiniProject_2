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
#define mlPerSecond 25.0                        // volume per second of pump output, used to calculate time pump runs for desired volume
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
	pinMode(PUMPPIN, OUTPUT);
	pinMode(PROBEANODE, OUTPUT);
	digitalWrite(PUMPPIN, HIGH);
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
}  // end setup()

void loop(){
	dispenseSolution();
}  // end loop()


// run mixer servo for a period of time
void runMixer(){
	Serial.println("Just a stub need to finish runMixer()");
	mixerServo.write(45);                       // continuous rotation servo: 0=full speed one direction, 90=stopped, 180=full speed opposite direction
	Serial.println("Press any key to stop mixer motor.");
	Serial.flush();
	while (Serial.available() == 0){
	}
	mixerServo.write(90);                       // turn off servo
	Serial.flush();                             // ***  need to flush or get rid of entered key press  ***
}  // end runMixer()

// run pump to dispense desired volume
void dispenseSolution(){
	//Serial.println("Just a stub, need to finish dispenseSolution.");
	String userInput;
	float volume;
	char check = 'n';                           // used for incoming byte to verify volume input
	
	while (check != 'y'){
		Serial.setTimeout(8000);
		Serial.println("Enter desired volume to dispense in milliliters:");
		Serial.flush();
		while (Serial.available() == 0){
		}
		userInput = Serial.readStringUntil('\n');
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
	
}  // end dispenseSolution()

// gather salinity probe readings
void salinityTest(){
	Serial.println("Just a stub, need to finish salinityTest()");
}  // end salinityTest()

// plot all data to time plot
void plotData(){
	Serial.println("Just a stub, need finish plotData");
}  // end plotData()

//  function to fill array with raw ADC readings
void buildDataSet(int inputPin){                //  could combine all these functions into one
	                                            //  but having separate arrays we can manipulate
	                                            //  data in more ways for analysis
	Serial.print("Gathering data and compiling sample set of ");
	Serial.print(samples);
	Serial.println(" ADC counts");
	Serial.print("and a delay of ");
	Serial.print(pause);
	Serial.println(" milliseconds between readings.");
	Serial.print("ADC reference voltage of ");
	Serial.print(arefVoltage);
	Serial.println(" volts.");
	// fill array with readings from ADC
	for (int i; i < samples; i++){
		readings[i] = analogRead(inputPin);
		delay(pause);
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
float rollingAvgFromArray(float theArray[]){
	float average = (theArray[0] + theArray[1]) / 2;
	for (int i = 2; i < sizeof(theArray); i++){
		average = (average + theArray[i]) / 2;
	}
	return average;
}  // end of rollingAvgFromArray()

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