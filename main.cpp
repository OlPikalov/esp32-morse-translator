#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <map>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;
const int buttonPinRed = 4;   // Red button: changes mode / restart
const int buttonPin = 15;     // White button: records short/long signals (Morse input)
int buzzer = 5;               // Buzzer: plays Morse code sounds
int led = 18;                 // LED: indicates Bluetooth connection
unsigned long  secondCounter, startTime = 0, unpressedTime = 0, unpressedTimeCounter;
int buttonStateRed;
String inputChar = "";        // Current Morse character being typed
String Sentence = "";         // Whole translated sentence
bool newChar = false, newWord = false, modeCheck;

// Morse code lookup table
const std::map<String, char> Morse = {
  {". ___", 'A'},
  {"___ . . .", 'B'},
  {"___ . ___ .", 'C'},
  {"___ . .", 'D'},
  {".", 'E'},
  {". . ___ .", 'F'},
  {"___ ___ .", 'G'},
  {". . . .", 'H'},
  {". .", 'I'},
  {". ___ ___ ___", 'J'},
  {"___ . ___", 'K'},
  {". ___ . .", 'L'},
  {"___ ___", 'M'},
  {"___ .", 'N'},
  {"___ ___ ___", 'O'},
  {". ___ ___ .", 'P'},
  {"___ ___ . ___", 'Q'},
  {". ___ .", 'R'},
  {". . .", 'S'},
  {"___", 'T'},
  {". . ___", 'U'},
  {". . . ___", 'V'},
  {". ___ ___", 'W'},
  {"___ . . ___", 'X'},
  {"___ . ___ ___", 'Y'},
  {"___ ___ . .", 'Z'},

  {"___ ___ ___ ___ ___", '0'},
  {". ___ ___ ___ ___", '1'},
  {". . ___ ___ ___", '2'},
  {". . . ___ ___", '3'},
  {". . . . ___", '4'},
  {". . . . .", '5'},
  {"___ . . . .", '6'},
  {"___ ___ . . .", '7'},
  {"___ ___ ___ . .", '8'},
  {"___ ___ ___ ___ .", '9'}
};

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  SerialBT.begin("ESP32_Morse");   // Start Bluetooth with device name
  Serial.begin(115200);
  pinMode(buttonPin, INPUT);
  pinMode(buttonPinRed,INPUT_PULLUP);
  pinMode(buzzer,OUTPUT);
  pinMode(led,OUTPUT);
  modeCheck = false;
  // Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println("Unable to find OLED display");
    for(;;); 
  }
  display.clearDisplay();    
  display.setTextColor(SSD1306_WHITE);
  display.display();  
}

// Translate Morse string to character
char Translate(String inputChar){
  char firstCharacter = inputChar[0];
  for (const auto& pair : Morse) {
    if(pair.first[0] != firstCharacter) continue; 
    if(pair.first == inputChar) return pair.second;
  }
  return '?';  // Unknown Morse sequence
}

// Reverse translation: character -> Morse string
String ReverseTranslate(char incoming){
  for (const auto& pair : Morse) {if(pair.second == incoming) return pair.first;}
  return " ";
}

// Update OLED with current input and sentence
void UpdateDisplay(){
  display.clearDisplay();
  display.setCursor(0,0); 
  display.println(inputChar);
  display.setCursor(0,16); 
  display.println(Sentence);
  display.setCursor(0,48); 
  display.println("Hold red button to restart");
  display.display(); 
}

void loop() {
  delay(500);
  buttonStateRed = digitalRead(buttonPinRed);
  modeCheck = true;
  // ----------- MAIN MENU MODE (waiting for input) -----------
  while(buttonStateRed != LOW){
    digitalWrite(led, SerialBT.hasClient() ? HIGH : LOW); // LED shows BT connection state
    display.clearDisplay();
    display.setCursor(0,0); 
    display.println("Press red button to start or send text via bluetooth");
    display.display();

    // If text is received via Bluetooth, convert to Morse sounds
    if (SerialBT.available() && digitalRead(buttonPinRed) == HIGH) {
      String incoming = SerialBT.readString();
      display.clearDisplay();
      display.setCursor(0,0); 
      display.print("Currently being translated: " + incoming);
      display.display();
      for (int i = 0; i < incoming.length(); i++) {
        char in = incoming[i];
        String code = ReverseTranslate(in);
        for (int j = 0; j < code.length(); j++) {
          char cd = code[j];
          if (cd == '.') {              // Short beep for dot
            tone(buzzer, 1000);
            delay(300);
            noTone(buzzer);
          } else if (cd == '_') {       // Long beep for dash
            tone(buzzer, 1000);
            delay(1000);
            noTone(buzzer);
            j += 2; // Skip extra underscores
          }
          delay(100); 
        }
        delay(1000); 
        if (in == ' ') {
          delay(2000);  // Extra delay for spaces
        }
      }
      noTone(buzzer); 
    }
    buttonStateRed = digitalRead(buttonPinRed);
  }
  // ----------- MORSE INPUT MODE -----------
  display.clearDisplay();
  display.setCursor(0,0); 
  display.println("Prepare");
  display.display();
  delay(2000);
  buttonStateRed = digitalRead(buttonPinRed);
  digitalWrite(led, SerialBT.hasClient() ? HIGH : LOW);

  while(buttonStateRed != LOW){
    // Detect button press length to distinguish dot/dash
    startTime = millis();
    while(digitalRead(buttonPin) == LOW) {
      newChar = true;
      newWord = true;
      secondCounter = millis() - startTime;
      UpdateDisplay();
      digitalWrite(led, SerialBT.hasClient() ? HIGH : LOW);
      if (SerialBT.available() && modeCheck == true) {
        SerialBT.println("This option is unavailable right now, please change mode on the translator");
        modeCheck = false;
      }
    }
    // Add dot or dash depending on press length
    secondCounter = millis() - startTime;
    if(secondCounter > 0){
      if(inputChar.isEmpty())inputChar += (secondCounter < 200 && secondCounter > 0) ? "." : "___";
      else inputChar += (secondCounter < 200 && secondCounter > 0) ? " ." : " ___";
    }
    UpdateDisplay();
    // Detect pauses to decide if character or word ended
    unpressedTime = millis();
    digitalWrite(led, SerialBT.hasClient() ? HIGH : LOW);
    while(digitalRead(buttonPin) != LOW && digitalRead(buttonPinRed) != LOW){
      if (SerialBT.available() && modeCheck == true) {
        SerialBT.println("This option is unavailable right now, please change mode on the translator");
        modeCheck = false;
      }
      unpressedTimeCounter = millis() - unpressedTime;
      UpdateDisplay();

      // If enough time passed → translate Morse to letter
      if(unpressedTimeCounter > 1000 && newChar == true){
          Sentence += Translate(inputChar);
          inputChar.clear();
          newChar = false;
      }
      // Longer pause → new word
      if(unpressedTimeCounter > 3000 && newWord == true) {
        Sentence += ' ';
        newWord = false;
      }
      digitalWrite(led, SerialBT.hasClient() ? HIGH : LOW);
      delay(50);
    }
    // Red button handling: restart or send sentence
    if (digitalRead(buttonPinRed) == LOW) {
      unsigned long pressStart = millis();
      while (digitalRead(buttonPinRed) == LOW) {
        if (millis() - pressStart > 3000) ESP.restart(); // Hold red button → restart ESP
      }
      if (millis() - pressStart < 1000) {
        SerialBT.println("Translated sentence: " + Sentence); // Short press → send via Bluetooth
        Sentence.clear();
      }
    }
  }
}
