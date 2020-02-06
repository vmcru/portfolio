//libraries
#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal.h>
#include <OneWire.h>

//initialisation of the lcd
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
//temperature sensor
int DS18S20_Pin = 2; //DS18S20 Signal pin on digital 2

//Temperature chip i/o
OneWire ds(DS18S20_Pin); // on digital pin 2

// define variablesfor the lcd and keypad
bool backLightOn = 1;
int lcd_key = 0;
int adc_key_in = 0;

//define variables for the use of peltier and the fan 
int peltier = 3; //The N-Channel MOSFET is on digital pin 3
int power = 0; //Power level from 0 to 99%
int peltier_level = map(power, 0, 99, 0, 255); //This is a value from 0 to 255 that actually controls the MOSFET

// define constants
const int backLight = 10; 
const int pirPin = 16;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5
#define beeper A1
#define shortBeep 100
#define longBeep  500

void setup (void) {
  Serial.begin(9600);           //display info in console
  pinMode(backLight, OUTPUT);   //set pin to control backlight of the display
  digitalWrite(backLight, LOW); // turn backlight off  
  pinMode(beeper, OUTPUT);
  digitalWrite(beeper, LOW); 
// test the beeper sound by oscillating the driving signal at high frequency
   for( int j=0; j<2400; j++ ){
      digitalWrite(beeper, HIGH);
      delayMicroseconds(208);
      digitalWrite(beeper, LOW);
      delayMicroseconds(208);
   }
   
  Wire.begin();
  digitalWrite(backLight, HIGH); // turn backlight on
}

void loop (void) {
  float temperature = getTemp(); //will take about 750ms to run
  Serial.println(temperature);
  digitalTempDisplay(temperature); // update temperature

  for (int i = 0; i < 10000; i++)
  {
    button_loop(); //check for button pushed
  }
}

void printDigits(byte digits)
{
  // utility function for digital clock display: prints preceding colon and leading 0
  lcd.print(":");
  if(digits < 10)
    lcd.print('0');
  lcd.print(digits,DEC);
}

void digitalTempDisplay(float temperature)
{ 
  //sound alarm
  //setOffAlarm();
  int temp_int = temperature;
  int temp_dec = 100 * (temperature - temp_int);
  lcdClear();
  lcd.print("Temp:");
  lcd.print(temp_int,DEC);
  lcd.print(".");
  lcd.print(temp_dec,DEC); 
  //printDigits(temperature);
 
}

void button_loop()
{
  int button = read_LCD_buttons();
  if (button == btnSELECT)
  {
    timedBeep(shortBeep,1); 
    //selectMenu();
  }
}



// read the buttons
int read_LCD_buttons()
{
  adc_key_in = analogRead(0);      // read the value from the sensor
  // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
  // we add approx 50 to those values and check to see if we are close
  if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
  if (adc_key_in < 50)   return btnRIGHT; 
  if (adc_key_in < 195)  return btnUP;
  if (adc_key_in < 380)  return btnDOWN;
  if (adc_key_in < 555)  return btnLEFT;
  if (adc_key_in < 790)  return btnSELECT;  
  return btnNONE;  // when all others fail, return this...

}

void timedBeep(int beepTime, int beepCount)
{
  for (int i = 0; i < beepCount; i ++)
  {
    digitalWrite(beeper, HIGH);
    delay(beepTime);
    digitalWrite(beeper, LOW);
    delay(beepTime);
  }
}

void lcdClear(){
  // 8/1/2012 Fixed default day and hour settings on set date/time
  //lastDay = 0;
  //lastMinute = 0;
  //resetClock = true;
  lcd.clear();
  lcd.begin(16,2);
  lcd.setCursor(0,0); 
}

float getTemp(){
 //returns the temperature from one DS18S20 in DEG Celsius

 byte data[12];
 byte addr[8];

 if ( !ds.search(addr)) {
   //no more sensors on chain, reset search
   ds.reset_search();
   return -1000;
 }

 if ( OneWire::crc8( addr, 7) != addr[7]) {
   Serial.println("CRC is not valid!");
   return -1000;
 }

 if ( addr[0] != 0x10 && addr[0] != 0x28) {
   Serial.print("Device is not recognized");
   return -1000;
 }

 ds.reset();
 ds.select(addr);
 ds.write(0x44,1); // start conversion, with parasite power on at the end
 
 delay(750); // Wait for temperature conversion to complete

 byte present = ds.reset();
 ds.select(addr);  
 ds.write(0xBE); // Read Scratchpad

 
 for (int i = 0; i < 9; i++) { // we need 9 bytes
  data[i] = ds.read();
 }
 
 ds.reset_search();
 
 byte MSB = data[1];
 byte LSB = data[0];

 float tempRead = ((MSB << 8) | LSB); //using two's compliment
 float TemperatureSum = tempRead / 16;
 
 return TemperatureSum;
 
}

