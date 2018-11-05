#include <ESP8266WiFi.h>
#include <WifiUDP.h>
#include <ESP8266HTTPClient.h>
#include <String.h>
#include <Wire.h>
#include <NTPClient.h>
#include <Time.h>
#include <TimeLib.h>
#include <math.h>
#include <Timezone.h>
#include <Adafruit_LEDBackpack.h>
#include <Adafruit_GFX.h>
#include <ArduinoJson.h>
#include "SevenSegmentTM1637.h"
#include "SevenSegmentExtended.h"
#include <TM1637Display.h>

// Define NTP properties
#define NTP_OFFSET   60 * 60      // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "ca.pool.ntp.org"  // change this to whatever pool is closest (see ntp.org)

// Set up the NTP UDP client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);
const byte PIN_CLK = 16;   // define CLK pin (any digital pin)
const byte PIN_DIO = 5;   // define DIO pin (any digital pin)
TM1637Display    dateDisplay(PIN_CLK, PIN_DIO);
// Create a display object
Adafruit_7segment   disp = Adafruit_7segment();
Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();

const char* ssid = "ATT3Y3V8j4";   // insert your own ssid 
const char* password = "327d796p5c6n";              // and password
String date;
String t;
const char * days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"} ;
const char * months[] = {"Jan", "Feb", "Mar", "Apr", "May", "June", "July", "Aug", "Sep", "Oct", "Nov", "Dec"} ;
const char * ampm[] = {"AM", "PM"} ;

void setup () 
{
  Serial.begin(115200); // most ESP-01's use 115200 but this could vary
  disp.begin(0x72);   // default display for the LED backpack
  alpha4.begin(0x70);  // pass in the address
  
  notime();         // set the display to 8888 with no colon
  timeClient.begin();   // Start the NTP UDP client
  // Connect to wifi
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.print(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi at ");
  Serial.print(WiFi.localIP());
  Serial.println("");
   const char* welcomeMsg = "HSV WX    .";
    ledprint(welcomeMsg);
    delay(2000);
      dateDisplay.setBrightness(0x0f);

}

void loop() 
{
  if (WiFi.status() == WL_CONNECTED) //Check WiFi connection status
  {   
    date = "";  // clear the variables
    t = "";
    
    // update the NTP client and get the UNIX UTC timestamp 
    timeClient.update();
    unsigned long epochTime =  timeClient.getEpochTime();

    // convert received time stamp to time_t object
    time_t local, utc;
    utc = epochTime;

    // Then convert the UTC UNIX timestamp to local time
    TimeChangeRule usEDT = {"CDT", Second, Sun, Mar, 2, -360};  //UTC - 5 hours - change this as needed
    TimeChangeRule usEST = {"CST", First, Sun, Nov, 2, -420};   //UTC - 6 hours - change this as needed
    Timezone usCentral(usEDT, usEST);
    local = usCentral.toLocal(utc);

    // now format the Time variables into strings with proper names for month, day etc
    date += days[weekday(local)-1];
    date += ", ";
    date += months[month(local)-1];
    date += " ";
    date += day(local);
    date += ", ";
    date += year(local);

    // format the time to 12-hour format with AM/PM and no seconds
    t += hourFormat12(local);
    t += ":";
    if(minute(local) < 10)  // add a zero if minute is under 10
      t += "0";
    t += minute(local);
    t += " ";
    t += ampm[isPM(local)];

  
    // Display the date and time
    Serial.println("");
    Serial.print("Local date: ");
    Serial.print(date);
    Serial.println("");
    Serial.print("Local time: ");
    Serial.print(t);

    // print the date and time on the Display
    
    disp.print(hourFormat12(local) * 100 + minute(local)); // I like to see it as a 12 hour clock
    disp.drawColon(true);
    disp.writeDisplay();
    String displayDay;
    String displayMonth;
    String displayDate;
    if(day(local) < 10){
      displayDay += "0";
      displayDay += String(day(local));
    }
    else{
      displayDate += String(day(local));
    }
    if(month(local) < 10){
      displayMonth += "0";
      displayMonth += String(month(local));
    }
    else{
      displayMonth += String(month(local));
    }
    displayDate += displayMonth;
    displayDate += displayDay;
    Serial.println("10.04");

byte  rawData;
//  if(displayMonth < 10){
//     rawData[0] = dateDisplay.encode(' ');
//  rawData[1] = dateDisplay.encode(displaybuffer[0]) | B10000000;
//  }
//  else{
//  rawData[0] = dateDisplay.encode(displaybuffer[0]);
//  rawData[1] = dateDisplay.encode(displaybuffer[1]) | B10000000;
//  }
//  if(displayDay < 10){
//      rawData[2] = dateDisplay.encode(' ') ;
//      rawData[3] = dateDisplay.encode(displaybuffer2[0]) ;
//  }
//  else{
//  rawData[2] = dateDisplay.encode(displaybuffer2[0]) ;
//  rawData[3] = dateDisplay.encode(displaybuffer2[1]); 
//  }
//  
//  dateDisplay.printRaw(rawData);
    dateDisplay.showNumberDecEx(153, 1, true, 3, 1); 
    delay(2000);
    int k;
    for(k=0; k <= 4; k++) {
    dateDisplay.showNumberDecEx(0, (0x80 >> k), true);
    delay(2000);
    }
 // dateDisplay.print(displayDate); 


    }
  else // attempt to connect to wifi again if disconnected
  {
    WiFi.begin(ssid, password);

    delay(1000);
  }
   printWeather();
}
void printWeather(){
    HTTPClient http;  //Object of class HTTPClient
    http.begin("http://api.openweathermap.org/data/2.5/weather?zip=35749,us&APPID=6c0656abd51a51e96a36b06b774aa90c&units=imperial");
    int httpCode = http.GET();
    //Check the returning code                                                                  
    if (httpCode > 0) {
// Parsing
const size_t bufferSize = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + 2*JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(12);
      DynamicJsonBuffer jsonBuffer(bufferSize);
      JsonObject& root = jsonBuffer.parseObject(http.getString());
      // Parameters
     JsonObject& main = root["main"];
  float main_temp = main["temp"]; // 85.77
  int temp = round(main_temp);
  char displaybuffer[4] = {' ', ' ', ' ', ' '};
  dtostrf(temp,2,2,displaybuffer); //convert double to char array

  alpha4.writeDigitAscii(0, displaybuffer[0]);
  alpha4.writeDigitAscii(1, displaybuffer[1]);
  alpha4.writeDigitRaw(2, 0x283);
  alpha4.writeDigitAscii(3, 'F');
    alpha4.writeDisplay();
    delay(1000);

    JsonObject& weather0 = root["weather"][0];
    
   // const char* weather0_main = weather0["main"]; // "Clouds"
   // ledprint(weather0_main);
   // delay(500);
    
   
        const char* weather0_description = weather0["description"]; // "broken clouds"
        ledprint(weather0_description);
        delay(500);
        
        int clouds_all = root["clouds"]["all"];
        if(clouds_all > 9){
          const char* skyCondition = "SKY COVERAGE";
          ledprint(skyCondition);
          delay(300);
          dtostrf(clouds_all,2,2,displaybuffer); //convert double to char array
          alpha4.writeDigitAscii(0, displaybuffer[0]);
          alpha4.writeDigitAscii(1, displaybuffer[1]);
          alpha4.writeDigitAscii(2, displaybuffer[2]);
          alpha4.writeDigitAscii(3, '%');
          alpha4.writeDisplay();
          delay(1000);
        }

        const char* wind = "WIND";
          ledprint(wind);
          delay(500);
        int windDir = root["wind"]["deg"];
        dtostrf(windDir,3,3,displaybuffer); //convert double to char array
        if(windDir > 0 && windDir < 90){
          alpha4.writeDigitRaw(0, 0xC03); //arrow up right
        }
         else if (windDir == 90){
          alpha4.writeDigitRaw(0, 0x940); //arrow right
        }
        else if (windDir > 90 && windDir < 180){
          alpha4.writeDigitRaw(0, 0x210C); //arrow down right
        }
        else if (windDir == 180){
          alpha4.writeDigitRaw(0, 0x523); //arrow down
        }
        else if (windDir > 180 && windDir < 270){
          alpha4.writeDigitRaw(0, 0xC18); //arrow down left
        }
        else if (windDir == 270){
          alpha4.writeDigitRaw(0, 0x2480); //arrow left
        }
        else if (windDir > 270 && windDir < 360){
          alpha4.writeDigitRaw(0, 0x2121); //arrow up left
        }
        else if (windDir == 360){
          alpha4.writeDigitRaw(0, 0x28DC); //arrow up
        }


        if (windDir < 100 && windDir > 9){
        alpha4.writeDigitAscii(1, '0');
        alpha4.writeDigitAscii(2, displaybuffer[0]);
        alpha4.writeDigitAscii(3, displaybuffer[1]);
        }
        else if(windDir < 10){
        alpha4.writeDigitAscii(1, '0');
        alpha4.writeDigitAscii(2, '0');
        alpha4.writeDigitAscii(3, displaybuffer[0]);
        }
        else if(windDir > 100) {
        alpha4.writeDigitAscii(1, displaybuffer[0]);
        alpha4.writeDigitAscii(2, displaybuffer[1]);
        alpha4.writeDigitAscii(3, displaybuffer[2]);
        }
        else{
        alpha4.writeDigitAscii(0, 'C');
        alpha4.writeDigitAscii(1, 'A');
        alpha4.writeDigitAscii(2, 'L');
        alpha4.writeDigitAscii(3, 'M');
        }
        alpha4.writeDisplay();
        delay(1500);
          
        float windSpeedFloat = root["wind"]["speed"];
        int windSpeed = round(float(windSpeedFloat * 0.868976));
        if(windSpeed > 0){
        dtostrf(windSpeed,2,2,displaybuffer); //convert double to char array
          alpha4.writeDigitAscii(0, '@');
          alpha4.writeDigitAscii(1, ' ');
          if(windSpeed < 10){
            alpha4.writeDigitAscii(2, '0');
            alpha4.writeDigitAscii(3, displaybuffer[0]);
          }
          else {
            alpha4.writeDigitAscii(2, displaybuffer[0]);
            alpha4.writeDigitAscii(3, displaybuffer[1]);
          }
        alpha4.writeDisplay();
          
        int gusts = root["wind"]["gust"];
        if(gusts > 0)
        { delay(1000);
          const char* gust = "PEAK GUST";
          ledprint(gust);
          delay(300);
          float gustFloat = root["wind"]["gust"];
          int gustSpeed = round(float(gustFloat * 0.868976));
          dtostrf(gustSpeed,2,2,displaybuffer); //convert double to char array
          alpha4.writeDigitAscii(0, ' ');
          alpha4.writeDigitAscii(1, ' ');
          alpha4.writeDigitAscii(2, displaybuffer[0]);
          alpha4.writeDigitAscii(3, displaybuffer[1]);
          alpha4.writeDisplay();
        }
        delay(1500);
    }
    dtostrf(temp,2,2,displaybuffer); //convert double to char array
    alpha4.writeDigitAscii(0, displaybuffer[0]);
    alpha4.writeDigitAscii(1, displaybuffer[1]);
    alpha4.writeDigitRaw(2, 0x283);
    alpha4.writeDigitAscii(3, 'F');
    alpha4.writeDisplay();
    delay(5000);
    }
    http.end();   //Close connection
}

void ledprint(const char* str)
{
  char displaybuffer[4] = {' ', ' ', ' ', ' '};

  for (int i = 0; i < strlen(str); i++)      //for each character in str
  {
    // scroll down display
  displaybuffer[0] = toupper(displaybuffer[1]);
  displaybuffer[1] = toupper(displaybuffer[2]);
  displaybuffer[2] = toupper(displaybuffer[3]);
  displaybuffer[3] = toupper(str[i]);
  alpha4.writeDigitAscii(0, displaybuffer[0]);
  alpha4.writeDigitAscii(1, displaybuffer[1]);
  alpha4.writeDigitAscii(2, displaybuffer[2]);
  alpha4.writeDigitAscii(3, displaybuffer[3]);
  alpha4.writeDisplay();  //write to the display.
  delay(300);
  } 
}

void printLeading0(int n)
{
   if(n < 10) Serial.print("0");
   Serial.print(n);
}

void notime(void)
{
   disp.drawColon(false);
   disp.print(8888);
   disp.writeDisplay();
}
