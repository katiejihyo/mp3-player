# mp3-player
Skeleton code for Arduino Nano based mp3 player. Track and Artist names are generalized as placeholders but must be manually edited via code (then uploaded to the Arduino) when changed on the flash drive. 

Naming convention for audio files must abide by the following pattern and stored via the root folder:

0001.mp3
0002.mp3
0003.mp3
...
0015.mp3

## Wiring Reference
Power core (PowerBoost 1000C): LiPo battery → PowerBoost JST jack (stays plugged in always); charge via microUSB; Vs/5V out → 5V rail; GND → GND rail; EN → slide switch center pin.

Power switch (SPDT slide): center pin → PowerBoost EN; one outer pin → GND rail; other outer pin empty.

Arduino Nano power: 5V pin → 5V rail (not VIN); GND → GND rail.

DFPlayer Mini: VCC → 5V rail; GND → GND rail; TX → Nano D2; RX → Nano D3 (via 1kΩ resistor in-line); DAC_L → jack left; DAC_R → jack right; DAC_GND → jack sleeve/ground.

OLED (SSD1306, I2C @ 0x3C): VCC → 5V rail; GND → GND rail; SDA → Nano A4; SCL → Nano A5.

Control buttons (each: one leg to Nano pin, other leg to GND rail): Play/Pause → D4; Next → D5; Previous → D6.

Headphone jack: left → DFPlayer DAC_L; right → DFPlayer DAC_R; sleeve/ground → DFPlayer DAC_GND.

