/**********************************************************
  Libraries
**********************************************************/
//DHT11
#include <DHT.h>

//esp
#include <ESP8266WiFi.h>

//bmp180
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>

//oled
#include <SSD1306Wire.h>

//blynk
#include <BlynkSimpleEsp8266.h>



/**********************************************************
  Variables Declaration
**********************************************************/
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);

// replace with your channel’s thingspeak API key
String apiKey = "xxxxxxxxxxxxxxxx;                    //fill in the api key from thingspeak
const char* ssid = "xxxxxxxxxxxxxxx";                 //fill in your wifi name
const char* password = "xxxxxxxxxxxxxxxXXX";         //fill in your wifi password
const char* server = "api.thingspeak.com";

char auth[] = "xxxxxxxxxxxxxxxxxxxxxx";     //Put your blynk token

#define DHTPIN 2              // what pin we’re connected to
DHT dht(DHTPIN, DHT11, 2);

WiFiClient client;

int sensorPin = A0; // input for LDR and air quality sensor
int enable1 = 15;   // enable reading LDR
int enable2 = 13;   // enable reading Air Quality sensor
int sensorValue1 = 0;  // variable to store the value coming from sensor LDR
int sensorValue2 = 0;  // variable to store the value coming from sensor Air Quality sensor


const int I2C_DISPLAY_ADDRESS = 0x3C;   // oled i2c address
const int SDA_PIN = 4;
const int SCL_PIN = 5;
SSD1306Wire display(I2C_DISPLAY_ADDRESS, SDA_PIN, SCL_PIN);



void setup()
{
  // declare the enable1 and enable2 as an OUTPUT:
  pinMode(enable1, OUTPUT);
  pinMode(enable2, OUTPUT);
  pinMode(sensorPin, INPUT);

  Serial.begin(115200);
  delay(10);

  dht.begin();

  WiFi.begin(ssid, password);
  Blynk.begin(auth, ssid, pass);                    //Connect the blynk to esp8266 board

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  Serial.print("..........");
  Serial.println();
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
  Serial.println("WiFi connected");
  Serial.println();

  display.init();
  display.clear();
  display.flipScreenVertically();
  display.display();

  Serial.println("Initiating Sensor Test");
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "Major Project");
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 29, "Test 1");
  display.setFont(ArialMT_Plain_10);
  display.display();
  delay(3000);
}

void loop()
{
  //--------------------------DHT11-------------------------
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  double gamma = log(h / 100) + ((17.62 * t) / (243.5 + t));
  double dp = 243.5 * gamma / (17.62 - gamma);

  if (isnan(h) || isnan(t))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Temperature: "); Serial.print(t); Serial.println(" *C");
  Serial.print("Humidity: "); Serial.print(h); Serial.println(" %");
  Serial.print("Dew point: "); Serial.print(dp); Serial.println(" *C");


  //--------------------------BMP180------------------------
  if (!bmp.begin())
  {
    Serial.print("Failed to read from BMP sensor!!");
    while (1);
  }

  sensors_event_t event;
  bmp.getEvent(&event);
  float temperature;
  bmp.getTemperature(&temperature);
  float seaLevelPressure = 1015;

  Serial.print("Pressure: "); Serial.print(event.pressure); Serial.println(" hPa/mBar");
  Serial.print("Temperature: "); Serial.print(temperature); Serial.println(" *C");
  Serial.print("Altitude: "); Serial.print(bmp.pressureToAltitude(seaLevelPressure, event.pressure)); Serial.println(" m");


  //--------------------------LDR-------------------------
  digitalWrite(enable1, HIGH);
  sensorValue1 = analogRead(sensorPin);
  sensorValue1 = constrain(sensorValue1, 0, 30);
  sensorValue1 = map(sensorValue1, 0, 30, 1023, 0);
  Serial.print("Light intensity: "); Serial.println(sensorValue1);

  if (sensorValue1 > 500)
  {
    Serial.println("---> The room is well-lit");
  }
  else
    Serial.println("---> There is less light in the room");

  digitalWrite(enable1, LOW);
  delay(100);


  //--------------------------Air Quality Sensor-------------------------
  digitalWrite(enable2, HIGH);
  delay(500);
  String aqi;
  sensorValue2 = analogRead(sensorPin);
  //sensorValue2 = constrain(sensorValue2, 150, 440);
  //sensorValue2 = map(sensorValue2, 150, 440, 1023, 0);

  if (sensorValue2 <= 50)
  {
    aqi = "Low";
  }
  else if (sensorValue2 > 50 && sensorValue2 <= 70)
  {
    aqi = "Moderate";
  }
  else if (sensorValue2 > 70 && sensorValue2 <= 90)
  {
    aqi = "High";
  }
  else
    aqi = "Very High";

  Serial.print("Air Quality: "); Serial.println(sensorValue2);
  Serial.print("Air Quality Ratio: "); Serial.println(aqi);
  delay(100);

  digitalWrite(enable2, LOW);


  //--------------------------oled-----------------------------
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);

  String w = "*********WELCOME*********";
  String temp = "T=" + String(temperature) + "*C , ";
  String hum = "H=" + String(h) + "%";
  String dewp = "DP=" + String(dp) + "*C , ";
  String pr = "P=" + String(event.pressure) + "mBar/hPa ";
  String al = "A=" + String(bmp.pressureToAltitude(seaLevelPressure, event.pressure)) + "m";  // insert pressure at sea level
  String aq = "AQI=" + aqi;
  //String s= "S="+ String(bmp.readSealevelpressure(30))+" Pa";     // or insert your atltitude

  display.drawString(0, 5, w);
  display.drawString(0, 13, temp);
  display.drawString(60, 13, hum);
  display.drawString(0, 26, dewp);
  display.drawString(65, 26, al);
  display.drawString(0, 39, pr);
  display.drawString(0, 52, aq);
  //display.drawString(0, 38, s);       // if you want to calculate sealevelpressure

  // write the buffer to the display
  display.display();
  delay(100);


  //--------------------------blynk-------------------------
  Blynk.run();                                      //Run the Blynk
  Blynk.virtualWrite(V0, t);
  Blynk.virtualWrite(V1, h);
  Blynk.virtualWrite(V2, dp);
  Blynk.virtualWrite(V3, event.pressure);
  Blynk.virtualWrite(V4, bmp.pressureToAltitude(seaLevelPressure, event.pressure));
  Blynk.virtualWrite(V5, sensorValue1);
  Blynk.virtualWrite(V6, sensorValue2);
  if (q > 70)
  {
    Blynk.notify("The Quailty of air is bad !!");   //Blynk will send notify if q>70
  }
  if (t > 40)
  {
    Blynk.notify("The Temperature is too high");    //Blynk will send notify if t>40
  }


  //--------------------------thingspeak-------------------------
  if (client.connect(server, 80))
  { // "184.106.153.149" or api.thingspeak.com
    String postStr = apiKey;
    postStr += "&field1=";
    postStr += String(t);
    postStr += "&field2=";
    postStr += String(h);
    postStr += "&field3=";
    postStr += String(dp);
    postStr += "&field4=";
    postStr += String(event.pressure);
    /*postStr +="&field5=";
      postStr += String(temperature);*/
    postStr += "&field5=";
    postStr += String(bmp.pressureToAltitude(seaLevelPressure, event.pressure));
    postStr += "&field6=";
    postStr += String(sensorValue1);
    postStr += "&field7=";
    postStr += String(sensorValue2);
    postStr += "\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n\n\n\n\n\n\n");
    client.print(postStr);
  }
  client.stop();

  // thingspeak needs minimum 15 sec delay between updates
  delay(20000);
}
