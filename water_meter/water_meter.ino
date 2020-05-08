//Matthew Rinker, Emma Steinman, Colin Smith
//Water Meter Reader Code w/WiFi Functionality

// include the library code:
#include <LiquidCrystal.h>


// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
const int hallEffectPin = A5;

int sensorValue = 0;
int rollingArray[300];
float runningAverage, shortRunningAverage;
float gallonsUsed;
const float conversionToGalConstant = 0.03;
const float pulseThreshold = 3.2;
boolean highPulseActive = false;
int pulseCount = 0;


void setup() {

  //LCD SETUP
  
  //set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.print("Pulses: ");
  lcd.print(0);
  lcd.setCursor(0,1);
  lcd.print("Gallons: ");
  lcd.print(0);
  
}

void loop() {
  // set the cursor to column 0, line 1
  lcd.setCursor(0, 1);

  //initialize average values
  runningAverage = 0;
  shortRunningAverage = 0;



  //populate arrays to calculate averages
  for (int i = 0; i < 299; i++) {
    rollingArray[i] = rollingArray[i + 1];
    runningAverage = runningAverage + rollingArray[i];
  }
  for (int i = 270; i < 299;  i++) {
    shortRunningAverage = shortRunningAverage + rollingArray[i];
  }
  rollingArray[299] = analogRead(hallEffectPin);
  runningAverage = runningAverage + rollingArray[299];
  shortRunningAverage = shortRunningAverage + rollingArray[299];
  shortRunningAverage = shortRunningAverage / 30.000;
  runningAverage = runningAverage / 300.000;


  //calculate "flat value"
  float flatValue  =  shortRunningAverage - runningAverage;
  if (flatValue > pulseThreshold && highPulseActive == false) {
    highPulseActive = true;
    pulseCount++;
    lcd.setCursor(8,0);
    lcd.print(pulseCount);
    gallonsUsed += conversionToGalConstant * pulseCount;
    lcd.setCursor(9,1);
    lcd.print(gallonsUsed);
  }
  else if (flatValue < (-1 * pulseThreshold) && highPulseActive == true) {
    highPulseActive = false;
  }
}
