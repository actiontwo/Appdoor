//#include <Adafruit_Sensor.h>
//#include <DHT.h>

/*
  Wifi Request
  Chuong trinh se truy cap vao mang Wifi voi SSID va PASS như khai bao.
  Khi ket noi thanh cong, chung ta gui lenh Ping tu cua so COM PORT hoac tu Bluetooth
  chuong trình se gui Request den Host va nhan du lieu tra ve.
*/

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <SoftwareSerial.h>


#define KEY 2
#define P0 0
#define P12 12
#define P13 13
#define P14 14
#define P15 15
#define P16 16
#define TX 1
#define RX 3
#define ADC A0
#define rxPin 5
#define txPin 4

const char* ssid = "UIZA";
const char* password = "1qaz2wsx";
const char WiFiAPPSK[] = "sparkfun";


const char* host = "demo1374215.mockable.io";
const int httpsPort = 443;

//const int DHTPIN = 2;
//const int DHTTYPE = DHT11;
//DHT dht(DHTPIN, DHTTYPE);


SoftwareSerial NewSerial (rxPin, txPin);

char buf[10];
int Pin[8] = {P0, P12, P13, P14, P15, P16, TX, RX};
String Read;

WiFiServer server(80);
void setup() {
//  dht.begin(); 
  pinMode(KEY, OUTPUT);
  digitalWrite(KEY, LOW);

  pinMode(P0, OUTPUT);
  pinMode(P12,OUTPUT);
  pinMode(P13, OUTPUT);
  pinMode(P14, OUTPUT);
  pinMode(P15, OUTPUT);
  pinMode(P16, OUTPUT);
  
//  pinMode (2, INPUT);
  pinMode(A0, INPUT);

  Serial.begin(115200);


  delay(10);

  Serial.print("connecting to ");
  Serial.println(ssid);



  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
   
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

//  setupWiFi();
  server.begin();


}
void setupWiFi()
{
  WiFi.mode(WIFI_AP);

  // Do a little work to get a unique-ish name. Append the
  // last two bytes of the MAC (HEX'd) to "ThingDev-":
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  macID.toUpperCase();
  String AP_NameString = "Actiontwo-" + macID;

  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);

  for (int i=0; i<AP_NameString.length(); i++)
    AP_NameChar[i] = AP_NameString.charAt(i);
  
  WiFi.softAP(AP_NameChar, WiFiAPPSK);
}

void clientComming(){
// Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();

  // Match the request
  int val = -1; // We'll use 'val' to keep track of both the
                // request type (read/set) and value if set.
  if (req.indexOf("/led/0") != -1)
    val = 1; // Will write LED high
  else if (req.indexOf("/led/1") != -1)
    val = 0; // Will write LED low
  else if (req.indexOf("/read") != -1)
    val = -2; // Will print pin reads
  // Otherwise request will be invalid. We'll say as much in HTML

  // Set GPIO5 according to the request
//  if (val >= 0)
//    digitalWrite(LED_PIN, val);

  client.flush();

  // Prepare the response. Start with the common header:
  String s = "HTTP/1.1 200 OK\r\n";
  s += "Content-Type: text/html\r\n\r\n";
  s += "<!DOCTYPE HTML>\r\n<html>\r\n";
  // If we're setting the LED, print out a message saying we did
  if (val >= 0)
  {
    s += "LED is now ";
    s += (val)?"off":"on";
  }
  else if (val == -2)
  { // If we're reading pins, print out those values:
    s += "Analog Pin = ";
//    s += String(analogRead(ANALOG_PIN));
    s += "<br>"; // Go to the next line.
    s += "Digital Pin 12 = ";
//    s += String(digitalRead(DIGITAL_PIN));
  }
  else
  {
    s += "Invalid Request.<br> Try /led/1, /led/0, or /read.";
  }
  s += "</html>\n";

  // Send the response to the client
  client.print(s);
  delay(1);
  Serial.println("Client disonnected");

  // The client will actually be disconnected 
  // when the function returns and 'client' object is detroyed
}

void loop() {
//  float h = dht.readHumidity();
//  float t = dht.readTemperature();
//  Serial.println(t);
//  Serial.println();
//  
  clientComming();

  if (Serial.available())
    Read = Serial.readStringUntil('\n');
  else if (NewSerial.available())
    Read = NewSerial.readStringUntil('\n');
  else
    Read = "";
    
    
  int value = analogRead(A0);
  if(value){
    NewSerial.println("{\"soilMoisture\":\""+String(value,DEC)+"\"}");
  } 
//    
  if(Read =="") {
   delay(1000);  
   return; 
  }
  
  Serial.println(Read);
  
  String line;
  int isCommand =  Read.indexOf('command:');
  line = Read;
  
  if(isCommand !=- 1) {
    line.replace("command:","");
    Serial.println(line);    
    action(line);
    NewSerial.println("{\"ack:true,\"pin\":\""+line+"\"}");
    return;
  }       
  
  
  if (Read == "Ping") {
    WiFiClientSecure client;
    Serial.print("connecting to ");
    Serial.println(host);

    if (!client.connect(host, httpsPort)) {
      Serial.println("connection failed");
     
      return;
    }
    String url = "/test";
    Serial.print("requesting URL: ");
    Serial.println(url);

    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "User-Agent: BuildFailureDetectorESP8266\r\n" +
                 "Connection: close\r\n\r\n");

    Serial.println("request sent");
    
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
        Serial.println("headers received");
        NewSerial.println("headers received");
        break;
      }
    }
    String line = client.readStringUntil('\n');

    Serial.println("reply was:");
    Serial.println("==========");
    Serial.println(line);
    Serial.println("==========");
    Serial.println("closing connection");
    action(line);
  }
  delay(1000);
}

void action(String line){
  for (int i = 0; i < line.length(); i++) {
        if (line[i] == '1')
          digitalWrite(Pin[i], HIGH);
        else
          digitalWrite(Pin[i], LOW);
      }
}
