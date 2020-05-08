//Matthew Rinker, Emma Steinman, Colin Smith
//Water Meter Reader Code w/WiFi Functionality

// include the library code:
#include <LiquidCrystal.h>
#include "WiFiEsp.h"

// Emulate Serial on pins 9/10 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(9, 10); // RX, TX
#endif
//This is a backup for if you are not connected to Pins 0,1 on the Arduino

//WIFI Setup

//===========================================
//*****CHANGE THESE VALUES*****
//===========================================
char ssid[] = "ENTER WIFI NAME HERE";
char pass[] = "ENTER WIFI PASSWORD HERE";

//===========================================
//*****DO NOT MODIFY PAST THIS POINT*****
//===========================================


int status = WL_IDLE_STATUS;     // the Wifi radio's status
char devid[] = "v42FE15BC09B20df";  // THIS IS THE DEVICE ID FROM PUSHINGBOX
char arduinoID[] = "TestArduino"; //Device Id to record on google form


int del=1800000;  // Amount of miliseconds delay between posting to google docs.
int timefromlastsend = 0;
int starttime;

char postmsg[100];
int k = 0;
char server[] = "api.pushingbox.com";

// Initialize the Ethernet client object
WiFiEspClient client;

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
  starttime = millis(); //set a value for starttime
  //WIFI SETUP *taken from library example file*

// initialize serial for debugging
  Serial.begin(115200);
  // initialize serial for ESP module
  Serial1.begin(9600);
  // initialize ESP module
  WiFi.init(&Serial1);

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  // you're connected now, so print out the data
  Serial.println("You're connected to the network");
  
  printWifiStatus();


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

  
  //determine if it is time to send input to google form
  timefromlastsend+=(millis()-starttime);
  if(timefromlastsend>del){
  // Post to Google Form.............................................
  if (client.connect(server, 80)) 
  {
    k=0;
    Serial.println("connected");
    sprintf(postmsg,"GET /pushingbox?devid=%c&status1=%c&status2=%d HTTP/1.1",devid,arduinoID,gallonsUsed);  // NOTE** In this line of code you can see where the temperature value is inserted into the wed address. It follows 'status=' Change that value to whatever you want to post.
    client.println(postmsg);
    client.println("Host: api.pushingbox.com");
    client.println("Connection: close");
    client.println();

    Serial.println(postmsg);
    Serial.println("Host: api.pushingbox.com");
    Serial.println("Connection: close");
    Serial.println();
 
     delay(1000);
     client.stop();
  }
  delay(1000);
  
  if (!client.connected()) 
  {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
    k==1;
    return;
  }
  timefromlastsend=0;
  }
  starttime=millis();
}

void printWifiStatus()
{
  //Debug function to print WiFi information
  //*Taken from the example project in the library files*
  
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
