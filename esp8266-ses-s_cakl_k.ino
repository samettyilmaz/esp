#include <Timer.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// callback function definition
typedef void (*CallBackFunction)();


// pins
//#define buzzer 9
#define LED_PIN 17
#define ANALOG_PIN 2
#define myPeriodic 10 //in sec | Thingspeak pub is 15sec
#define ONE_WIRE_BUS 4  // DS18B20 on arduino pin2 corresponds to D4 on physical board



OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
float prevTemp = 0;
const char* server = "api.thingspeak.com";
String apiKey ="XP20LNZ251CNDER6";
const char* MY_SSID = "AndroidAP"; 
const char* MY_PWD = "rbei1701";
int sent = 0;
// timer
Timer t;

// declaring functions
bool detectClap(int threshold = 300);
bool onClapDetected(CallBackFunction callBack, int threshold = 300);
int getAnalogData(int sample = 7);
void startTimer();
void stopTimer();
void ledON();
void ledOFF();
void ledToggle();
void clap();
void Tweet();
String buildTweetString();
String buildTweetString2();
bool detectSOS();
float temp;
// declaring global variables
int analog_value = 0; // analog value
bool led_on; // led light status; false:OFF, true:ON
bool timer_running = false;
int timeOutEvent;
int clapCount = 0;
int clapWaitingTime = 3000; // ms

// Tweet Settings
String API = "0OQN4WB7GCLCQBGY";
const char* ssid = "AndroidAP";
const char* password = "rbei1701";
WiFiClient client;

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  ledOFF();
}

// the loop function runs over and over again forever
void loop() {
  analog_value = getAnalogData();
  if (timer_running) {
    onClapDetected(clap);
    detectSOS();
  } else if (detectClap()) {
    startTimer();
    clap();
    delay(100);
  }
  t.update();

  
  //char buffer[10];
  DS18B20.requestTemperatures(); 
  temp = DS18B20.getTempCByIndex(0);
  //String tempC = dtostrf(temp, 4, 1, buffer);//handled in sendTemp()
  Serial.print(String(sent)+" Temperature: ");
  Serial.println(temp);
  detectSOS2();
  //if (temp != prevTemp)
  //{
  //sendTeperatureTS(temp);
  //prevTemp = temp;
  //}
  
  sendTeperatureTS(temp);
  int count = myPeriodic;
  while(count--)
  delay(1000);


  
}

void startTimer() {
  timer_running = true;
  timeOutEvent = t.every(clapWaitingTime, stopTimer);
  Serial.println("Timer has been STARTED.");
}

void stopTimer() {
  clapCount = 0;
  timer_running = false;
  Serial.println("Timer has been STOPPED.");
  t.stop(timeOutEvent);
}

bool onClapDetected(CallBackFunction callBack, int threshold) {
  bool detected = detectClap(threshold);
  if (detected) {
    callBack();
  }

  return detected;
}

bool detectClap(int threshold) {
  //analog_value = analogRead(ANALOG_PIN);
  if (analog_value >= threshold) {

    return true;
  } else {

    return false;
  }
}

int getAnalogData(int sample) {
  int a = 0, t;
  for (int i; i < sample; i++) {
    t = analogRead(ANALOG_PIN);
    if (t > a) {
      a = t;
    }
  }
  return a;
}

// led-on
void ledON() {
  led_on = true;
  digitalWrite(LED_PIN, LOW);
}

// led-off
void ledOFF() {
  led_on = false;
  digitalWrite(LED_PIN, HIGH);
}

// led toggle
void ledToggle() {
  if (led_on == true) {
    ledOFF();
  } else {
    ledON();
  }
}

void clap() {
  Serial.println("LIGHT ON!");
  ledON();
  clapCount++;
  delay(100);
  ledOFF();
  // reinitialize timer
  t.stop(timeOutEvent);
  timeOutEvent = t.every(clapWaitingTime, stopTimer);
}

void Tweet() {
  // connect to wifi
  WiFi.begin(ssid, password);

  // allow time to make connection
  while (WiFi.status() != WL_CONNECTED)
    delay(500);

  // if connection to thingspeak.com is successful, send your tweet!
  if (client.connect("184.106.153.149", 80))
  {
    client.print("GET /apps/thingtweet/1/statuses/update?key=" + API + "&status=" + buildTweetString() + " HTTP/1.1\r\n");
    client.print("Host: api.thingspeak.com\r\n");
    client.print("Accept: */*\r\n");
    client.print("User-Agent: Mozilla/4.0 (compatible; esp8266 Lua; Windows NT 5.1)\r\n");
    client.print("\r\n");
  }
}

void Tweet2() {
  // connect to wifi
  WiFi.begin(ssid, password);

  // allow time to make connection
  while (WiFi.status() != WL_CONNECTED)
    delay(500);

  // if connection to thingspeak.com is successful, send your tweet!
  if (client.connect("184.106.153.149", 80))
  {
    client.print("GET /apps/thingtweet/1/statuses/update?key=" + API + "&status=" + buildTweetString2() + " HTTP/1.1\r\n");
    client.print("Host: api.thingspeak.com\r\n");
    client.print("Accept: */*\r\n");
    client.print("User-Agent: Mozilla/4.0 (compatible; esp8266 Lua; Windows NT 5.1)\r\n");
    client.print("\r\n");
  }
}

void sendTeperatureTS(float temp)
{  
   WiFiClient client;
  
   if (client.connect(server, 80)) { // use ip 184.106.153.149 or api.thingspeak.com
   Serial.println("WiFi Client connected ");
   
   String postStr = apiKey;
   postStr += "&field1=";
   postStr += String(temp);
   postStr += "\r\n\r\n";
   
   client.print("POST /update HTTP/1.1\n");
   client.print("Host: api.thingspeak.com\n");
   client.print("Connection: close\n");
   client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
   client.print("Content-Type: application/x-www-form-urlencoded\n");
   client.print("Content-Length: ");
   client.print(postStr.length());
   client.print("\n\n");
   client.print(postStr);
   delay(1000);
   
   }//end if
   sent++;
 client.stop();
}//end send

bool detectSOS() {
  if (clapCount >= 5) {
    Serial.println("A Tweet is being sent..");
    Tweet();
    stopTimer();
    return true;
  } else {
    return false;
  }
}

bool detectSOS2() {
  if (temp>= 30) {
    Serial.println("A Tweet is being sent..");
    Tweet2();
    return true;
  } else {
    return false;
  }
}

String buildTweetString() {
  return "bebek%20uyandı!%20acele%20et!!";
}

String buildTweetString2() {
  return "Oda%20sıcaklığı%20çok%20yüksek!!";
}



