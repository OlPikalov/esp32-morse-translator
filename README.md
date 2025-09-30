
ESP32 Morse Code Translator is a project that allows users to input Morse code using a physical button and receive real-time translation on an OLED display. The system also supports Bluetooth communication, enabling users to send text to the ESP32 and hear it played back as Morse code through a buzzer.
The device operates in two modes:
- Main Menu Mode – waits for a red button press or Bluetooth input. If text is received via Bluetooth, it is converted into Morse signals and played through the buzzer.
- Morse Input Mode – activated by pressing the red button. Users input Morse code using short and long presses of the white button. The system detects pauses to determine when a character or word ends. Translated text is displayed and can be sent back via Bluetooth or the device can be restarted by holding the red button.

The OLED screen displays the current Morse input, the full translated sentence, and instructions. The LED indicates Bluetooth connection status. Timing logic distinguishes between dots, dashes, character breaks, and word breaks. The Morse code is mapped to letters A–Z and digits 0–9.
This project uses the Adafruit SSD1306 and GFX libraries for display control and the native BluetoothSerial library for wireless communication. It’s ideal for learning Morse code, experimenting with embedded systems, or building interactive communication tools.
