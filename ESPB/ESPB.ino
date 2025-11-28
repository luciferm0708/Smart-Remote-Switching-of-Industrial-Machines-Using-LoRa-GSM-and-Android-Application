#include <SPI.h>
#include <Wire.h>
#include <LoRa.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//OLED DEFINE BEGIN
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
//OLED END

//LORA DEFINE BEGIN
#define NSS 4
#define RST 15
#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>
#include <ArduinoJson.h>

#define DIO0 2
#define LORA_FREQ 433E6
//LORA DEFINE END

//DEFINE SIM800L BEGIN
#define RX2 16
#define TX2 17
#define SerialAT Serial2
#define APN "wap"
#define USER ""
#define PASS ""
//RELAY DEFINE BEGIN
#define relay_pin 32
//SSR DEFINE BEGIN
#define SSR_PIN 25

const char* server = "myesp32test.loca.lt"; 
const int serverPort = 80;

// PHP script paths
const char* updatePath = "/iot/update.php";
const char* readPath   = "/iot/read.php";
String receivedMsg;

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

String buff;
//DEFINE SIM800L END


bool relayState = LOW;
//RELAY DEFINE END

bool ssrState = LOW;
//SSR DEINE END

bool busy = false;

void setup() {
  Serial.begin(9600);
  /**/
  while (!Serial);

  pinMode(relay_pin, OUTPUT);
  digitalWrite(relay_pin, relayState);

  pinMode(SSR_PIN, OUTPUT);
  digitalWrite(SSR_PIN, ssrState);

  // OLED INIT
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED failed!");
    while (1);
  }
  //OLED INIT END

  //LORA INIT
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("LoRa Receiver Ready");
  display.display();

  LoRa.setPins(NSS, RST, DIO0);
  LoRa.setSPIFrequency(1E6); 
  LoRa.setTxPower(20);      

  if (!LoRa.begin(LORA_FREQ)) {
    Serial.println("LoRa init failed!");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("LoRa Init Failed");
    display.display();
    while (1);
  }

  Serial.println("LoRa initialized");
  display.setCursor(0, 10);
  display.println("Listening...");
  display.display();
  //LORA INIT END

  //SIM800L INIT BEGIN
  Serial.println("Booting...");

  SerialAT.begin(9600, SERIAL_8N1, RX2, TX2);
  delay(3000);

  pinMode(relay_pin, OUTPUT);
  pinMode(SSR_PIN, OUTPUT);
  digitalWrite(relay_pin, relayState);
  digitalWrite(SSR_PIN, ssrState);

  // Initialize modem
  Serial.println("Initializing Modem...");
  if (!modem.restart()) {
    Serial.println("Failed to restart modem");
  } else {
    Serial.println("Modem restarted successfully");
  }
  modem.sendAT("+CMGF=1"); // SMS Text mode 
  modem.sendAT("+CNMI=1,2,0,0,0"); // New SMS notification to serial 
  Serial.println("SIM800L ready to receive SMS.");
  Serial.println("Connecting to network...");
  if (!modem.waitForNetwork()) {
    Serial.println("Network failed!");
    while (1);
  }

  Serial.println("Connecting to GPRS...");
  if (!modem.gprsConnect(APN, USER, PASS)) {
    Serial.println("GPRS Connect Failed!");
    while (1);
  }
  Serial.println("GPRS Connected!");
  //SIM800L END
}

void loop() {
  // int packetSize = LoRa.parsePacket();

  // if (packetSize) {
  //   String received = "";
  //   while (LoRa.available()) {
  //     received += (char)LoRa.read();
  //   }
  //   received.trim();

  //   if (received == "DC-ON") {
  //     relayState = HIGH;
  //   } else if (received == "DC-OFF") {
  //     relayState = LOW;
  //   } else if(received == "AC-ON"){
  //     ssrState = HIGH;
  //   } else if(received == "AC-OFF") {
  //     ssrState = LOW;
  //   }

  //   Serial.print("Received: ");
  //   Serial.println(received);

  //   display.clearDisplay();
  //   display.setCursor(0, 0);
  //   display.println("Received:");
  //   display.println(received);
  //   display.setCursor(0, 40);
  //   display.print("RSSI: ");
  //   display.println(LoRa.packetRssi());
  //   display.display();
  // }
  handleLoRa();
  digitalWrite(relay_pin, relayState);
  digitalWrite(SSR_PIN, ssrState);
    // ---------- Read SMS ----------
  checkSMS();

  static unsigned long lastHttp = 0;
  if (millis() - lastHttp >= 8000) {
      readCommands();
      sendStatus();
      lastHttp = millis();
  }
}

// ---------- Read relay/SSR command from read.php ----------
void readCommands() {
  if (!client.connect(server, serverPort)) {
    Serial.println("Read connection failed");
    return;
  }

  client.print(String("GET ") + readPath + " HTTP/1.1\r\n" +
               "Host: " + server + "\r\n" +
               "Connection: close\r\n\r\n");

  String line;
  String jsonReply;
  bool headersEnded = false;
  unsigned long timeout = millis();

  while (client.connected() && millis() - timeout < 5000) {
    while (client.available()) {
      line = client.readStringUntil('\n');

      if (!headersEnded) {
        // empty line indicates end of headers
        if (line == "\r" || line.length() == 0) {
          headersEnded = true;
        }
      } else {
        // this is JSON body
        jsonReply += line;
      }
    }
  }
  client.stop();

  // Parse JSON
  DynamicJsonDocument doc(256);
  DeserializationError error = deserializeJson(doc, jsonReply);
  if (!error) {
    String dc = doc["relay"] | "";
    String ac = doc["ssr"]   | "";

    relayState = (dc == "DC-ON") ? HIGH : LOW;
    ssrState   = (ac == "AC-ON") ? HIGH : LOW;

    digitalWrite(relay_pin, relayState);
    digitalWrite(SSR_PIN, ssrState);

    Serial.println("DC: " + String(relayState ? "ON" : "OFF") +
                   " | AC: " + String(ssrState ? "ON" : "OFF"));
  } else {
    Serial.println("Failed to parse JSON");
    Serial.println(jsonReply);  // Print raw reply for debugging
  }
}

// ---------- Send current status to update.php ----------
void sendStatus() {
  if (!client.connect(server, serverPort)) {
    Serial.println("Update connection failed");
    return;
  }

  String url = String(updatePath) + "?relay=" + (relayState ? "DC-ON" : "DC-OFF")
               + "&ssr=" + (ssrState ? "AC-ON" : "AC-OFF");

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + server + "\r\n" +
               "Connection: close\r\n\r\n");

  while (client.connected()) {
    while (client.available()) {
      String line = client.readStringUntil('\n');
      Serial.println(line);
    }
  }
  client.stop();
}

void processSMS(String msg) {
  msg.trim();
  msg.replace("\r", "");
  msg.replace("\n", "");
  msg.replace("\"", "");
  msg.toUpperCase();

  Serial.println("Cleaned SMS: [" + msg + "]");

  bool changed = false;

  if (msg.indexOf("DC-ON") != -1) {
    relayState = HIGH;
    changed = true;
  }
  else if (msg.indexOf("DC-OFF") != -1) {
    relayState = LOW;
    changed = true;
  }
  else if (msg.indexOf("AC-ON") != -1) {
    ssrState = HIGH;
    changed = true;
  }
  else if (msg.indexOf("AC-OFF") != -1) {
    ssrState = LOW;
    changed = true;
  }

  digitalWrite(relay_pin, relayState);
  digitalWrite(SSR_PIN, ssrState);

  if (changed) {
    updateServerFromSMS();
  }

  if (msg == "STATUS") {
    sendSMSAuto("+880XXXXXXXXXX", "ESP32 is online");
  }
}
 
// ---------- SEND SMS USING TinyGSM ---------- // 
void sendSMSAuto(String number, String text) { 
  Serial.println("Sending SMS to " + number + "..."); 
  if (modem.sendSMS(number, text)) { 
    Serial.println("SMS sent successfully!"); 
  } else { 
    Serial.println("SMS send failed!"); 
  } 
}

void checkSMS() {
  while (SerialAT.available()) {
    String line = SerialAT.readStringUntil('\n');
    line.trim();

    if (line.startsWith("+CMT:")) {

      // Wait until full SMS arrives
      unsigned long start = millis();
      String smsText = "";

      while (millis() - start < 2000) {  // wait up to 2 seconds
        if (SerialAT.available()) {
          smsText += SerialAT.readString();
        }
      }

      smsText.trim();
      Serial.println("SMS Received: [" + smsText + "]");
      processSMS(smsText);
    }
  }
}

void updateServerFromSMS() {
  if (!client.connect(server, serverPort)) {
    Serial.println("SMS update server connect failed");
    return;
  }

  String url = String(updatePath) +
               "?relay=" + (relayState ? "DC-ON" : "DC-OFF") +
               "&ssr="   + (ssrState   ? "AC-ON" : "AC-OFF");

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + server + "\r\n" +
               "Connection: close\r\n\r\n");

  while (client.connected()) {
    while (client.available()) {
      client.readStringUntil('\n');  // ignore response
    }
  }
  client.stop();
  Serial.println("Server updated from SMS.");
  delay(500);
}
void handleLoRa() {
  if (busy) return;

  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    busy = true;
    String received = "";
    while (LoRa.available()) {
      received += (char)LoRa.read();
    }
    received.trim();
    handleRelay(received);
    busy = false;
  }
}

void handleRelay(String received) {
  bool changed = false;
  if (received == "DC-ON") {
    relayState = HIGH;
    changed = true;
  } else if (received == "DC-OFF") {
    relayState = LOW;
    changed = true;
  }
  else if (received == "AC-ON") {
    ssrState = HIGH;
    changed = true;
  }
  else if (received == "AC-OFF") {
    ssrState = LOW;
    changed = true;
  }
  
  if (changed) {
    digitalWrite(relay_pin, relayState);
    digitalWrite(SSR_PIN, ssrState);
    updateServerFromLoRa();
  }
}

void updateServerFromLoRa() {
  if (!client.connect(server, serverPort)) {
    Serial.println("LoRa update server connect failed");
    return;
  }
  String url = String(updatePath) +
               "?relay=" + (relayState? "DC-ON":"DC-OFF") + 
               "&ssr=" + (ssrState? "AC-ON" : "AC-OFF");
  client.print(String("GET ") + url + " HTTP/1.1\r\n" + 
               "Host: " + server + "\r\n" +
               "Connection: close\r\n\r\n");
  while(client.connected()) {
    while(client.available()) {
      client.readStringUntil('\n');
    }
  }
  client.stop();
  Serial.println("Server updated from LoRa");
}