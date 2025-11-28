#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NSS 4
#define RST 15
#define DIO0 2
#define LORA_FREQ 433E6

#define BTN_PIN0 12
#define LED_PIN0 33

#define BTN_PIN1 32
#define LED_PIN1 5
void buttonGreenState();
void buttonRedState();

bool prevBtn0State = HIGH;
bool prevBtn1State = HIGH;
unsigned long lastDebounceTime0 = 0;
const unsigned long debounceDelay0 = 50;
unsigned long lastDebounceTime1 = 0;
const unsigned long debounceDelay1 = 50;

bool led0State = false;
bool led1State = false;
void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED initialization failed");
    while (1);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("OLED Initialized");
  display.display();
  delay(1000);

  pinMode(BTN_PIN0, INPUT_PULLUP);
  pinMode(LED_PIN0, OUTPUT);
  digitalWrite(LED_PIN0, LOW);
  pinMode(BTN_PIN1, INPUT_PULLUP);
  pinMode(LED_PIN1, OUTPUT);
  digitalWrite(LED_PIN1, LOW);

  Serial.println("LoRa Sender:");
  LoRa.setPins(NSS, RST, DIO0);
  LoRa.setSPIFrequency(20000000);
  LoRa.setTxPower(20);

  if (!LoRa.begin(LORA_FREQ)) {
    Serial.println("Starting LoRa failed!");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("LoRa failed!");
    display.display();
    while (1);
  } else {
    Serial.print("LoRa initialized with frequency ");
    Serial.println(LORA_FREQ);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("LoRa Ready");
    display.print("Freq: ");
    display.println((int)LORA_FREQ);
    display.display();
    delay(1500);
  }
}

void loop() {
  buttonGreenState();
  buttonRedState();
}
void buttonGreenState() {
    int reading0 = digitalRead(BTN_PIN0);
  
    if (reading0 != prevBtn0State) {
      lastDebounceTime0 = millis();
    }

    if ((millis() - lastDebounceTime0) > debounceDelay0) {
      static bool stable0State = HIGH;
      if (reading0 == LOW && stable0State == HIGH) {

        led0State = !led0State;
        digitalWrite(LED_PIN0, led0State ? HIGH : LOW);
        
        String msg0 = led0State ? "DC-ON" : "DC-OFF";
        LoRa.beginPacket();
        LoRa.print(msg0);
        LoRa.endPacket();
        Serial.println("Sent: " + msg0);

        display.clearDisplay();
        display.setCursor(0, 2);
        display.setTextSize(1);
        display.println("LoRa Sender");
        display.print("Sent: ");
        display.println(msg0);
        display.display();
      }
      stable0State = reading0;
    }
    prevBtn0State = reading0;
}


void buttonRedState() {
    int reading1 = digitalRead(BTN_PIN1);
    
    if (reading1 != prevBtn1State) {
      lastDebounceTime1 = millis();
    }

    if ((millis() - lastDebounceTime1) > debounceDelay1) {
      static bool stable1State = HIGH;
      if (reading1 == LOW && stable1State == HIGH) {

        led1State = !led1State;
        digitalWrite(LED_PIN1, led1State ? HIGH : LOW);
        
        String msg1 = led1State ? "AC-ON" : "AC-OFF";
        LoRa.beginPacket();
        LoRa.print(msg1);
        LoRa.endPacket();
        Serial.println("Sent: " + msg1);

        display.clearDisplay();
        display.setCursor(0, 2);
        display.setTextSize(1);
        display.println("LoRa Sender");
        display.print("Sent: ");
        display.println(msg1);
        display.display();
      }
      stable1State = reading1;
    }
    prevBtn1State = reading1;
}

