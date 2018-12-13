/***************************************************
  This is an example for the Adafruit VS1053 Codec Breakout

  Designed specifically to work with the Adafruit VS1053 Codec Breakout
  ----> https://www.adafruit.com/products/1381

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/

// Libraries
#include <SoftwareSerial.h>
#include <math.h>
#include <Adafruit_VL53L0X.h>
#include <LiquidCrystal.h>

//********************************************************
// Initializing rangesensor objects
Adafruit_VL53L0X lox1 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox2 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox3 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox4 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox5 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox6 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox7 = Adafruit_VL53L0X();

//********************************************************
// MusicShield definitions
// define the pins used
#define VS1053_RX  2 // This is the pin that connects to the RX pin on VS1053

#define VS1053_RESET 9 // This is the pin that connects to the RESET pin on VS1053
// If you have the Music Maker shield, you don't need to connect the RESET pin!

// If you're using the VS1053 breakout:
// Don't forget to connect the GPIO #0 to GROUND and GPIO #1 pin to 3.3V
// If you're using the Music Maker shield:
// Don't forget to connect the GPIO #1 pin to 3.3V and the RX pin to digital #2

// See http://www.vlsi.fi/fileadmin/datasheets/vs1053.pdf Pg 31
#define VS1053_BANK_DEFAULT 0x00
#define VS1053_BANK_DRUMS1 0x78
#define VS1053_BANK_DRUMS2 0x7F
#define VS1053_BANK_MELODY 0x79

// See http://www.vlsi.fi/fileadmin/datasheets/vs1053.pdf Pg 32 for more!
#define VS1053_GM1_OCARINA 80

#define MIDI_NOTE_ON  0x90
#define MIDI_NOTE_OFF 0x80
#define MIDI_CHAN_MSG 0xB0
#define MIDI_CHAN_BANK 0x00
#define MIDI_CHAN_VOLUME 0x07
#define MIDI_CHAN_PROGRAM 0xC0

//shutdown pins to initialize the range sensors

int shdn1 = 6;
int shdn2 = 7;
int shdn3 = 8;
int shdn4 = 9;
int shdn5 = 10;
int shdn6 = 11;
int shdn7 = 12;

SoftwareSerial VS1053_MIDI(0, 2); // TX only, do not use the 'rx' side
// on a Mega/Leonardo you may have to change the pin to one that
// software serial support uses OR use a hardware serial port!

//********************************************************
//Lcd stuff
const int rs = 47, en = 46, d2 = 2, d3 = 3, d4 = 4, d5 = 5; // Defines LCD ports
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);   // Creates an LCD object


//********************************************************
// 7 rotarty encoders init



//********************************************************
//Own global variables

// Rotary encoder stuff
unsigned long currentTime;
unsigned long loopTime;
const int pinA[] = {22, 24, 26, 28, 30, 32, 34, 36}; //Pins for the rotary encoders of the rangesensors
const int pinB[] = {23, 25, 27, 29, 31, 33, 35, 37};

const int buttons[] = {38, 39, 40, 41, 42, 43, 44, 45}; //Pins for the buttons of the rotary encoders of the rangesensors
bool check = LOW;    // check for buttonstate
unsigned long lastButtonState = LOW;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;


unsigned char encoder_A[8];
unsigned char encoder_B[8];
unsigned char encoder_A_prev[8];
unsigned char encoder_B_prev[8];


// Arrays to store in the range measured by the range sensors
int lastOctaves[] = {0, 0, 0, 0, 0, 0, 0};
int octaves[] = {0, 0, 0, 0 , 0 , 0, 0};

// Define chords
int chords[21][4] = {{45, 49, 52, 57}, // A
  {45, 48, 52, 57}, // Am
  {45, 49, 52, 55}, // Asev
  {47, 51, 54, 59}, // B
  {47, 50, 54, 59}, // Bm
  {47, 51, 54, 57}, // Bsev
  {48, 52, 55, 60}, // C
  {48, 51, 55, 60}, // Cm
  {48, 52, 55, 58}, // Csev
  {50, 54, 57, 62}, // D
  {50, 53, 57, 62}, // Dm
  {50, 54, 57, 60}, // Dsev
  {52, 56, 59, 64}, // E
  {52, 55, 59, 64}, // Em
  {52, 56, 59, 62}, // Esev
  {53, 57, 60, 65}, // F
  {53, 56, 60, 65}, // Fm
  {53, 57, 60, 63}, // Fsev
  {55, 59, 62, 67}, // G
  {55, 58, 62, 67}, // Gm
  {55, 59, 62, 65}
}; //Gsev

String chordNames[] = {"A", "Am", "Asev",
                       "B", "Bm", "Bsev",
                       "C", "Cm", "Csev",
                       "D", "Dm", "Dsev",
                       "E", "Em", "Esev",
                       "F", "Fm", "Fsev",
                       "G", "Gm", "Gsev"
                      };

int chordNumber[] = {0, 0, 0, 0, 0, 0, 0}; // saves which cords corresponds to which range sensors. The number is the number of the chord in the list

// Set button (choose instrument)
//const int buttonPin = 52;
//int buttonState = 0;


void setup() {
  //********************************************************
    // Range Sensor setup
    pinMode(shdn1, OUTPUT);
    pinMode(shdn2, OUTPUT);
    pinMode(shdn3, OUTPUT);
    pinMode(shdn4, OUTPUT);
    pinMode(shdn5, OUTPUT);
    pinMode(shdn6, OUTPUT);
    pinMode(shdn7, OUTPUT);
  
  
  
    // Reset sensors
    digitalWrite(shdn1, LOW);
    digitalWrite(shdn2, LOW);
    digitalWrite(shdn3, LOW);
    digitalWrite(shdn4, LOW);
    digitalWrite(shdn5, LOW);
    digitalWrite(shdn6, LOW);
    digitalWrite(shdn7, LOW);
  
    delay(10);
  
  
    // Get the sensors out of reset mode
    digitalWrite(shdn1, HIGH);
    digitalWrite(shdn2, HIGH);
    digitalWrite(shdn3, HIGH);
    digitalWrite(shdn4, HIGH);
    digitalWrite(shdn5, HIGH);
    digitalWrite(shdn6, HIGH);
    digitalWrite(shdn7, HIGH);
  
    // Initialize sensors
    digitalWrite(shdn2, LOW);
    digitalWrite(shdn3, LOW);
    digitalWrite(shdn4, LOW);
    digitalWrite(shdn5, LOW);
    digitalWrite(shdn6, LOW);
    digitalWrite(shdn7, LOW);
  
    Serial.println("Adafruit VL53L0X test");
    if (!lox1.begin(0x30)) {
      Serial.println(F("Failed to boot VL53L0X - sensor 1"));
      while (1);
    }
  
    digitalWrite(shdn2, HIGH);
    if (!lox2.begin(0x31)) {
      Serial.println(F("Failed to boot VL53L0X - sensor 2"));
      while (1);
    }
  
    digitalWrite(shdn3, HIGH);
    if (!lox3.begin(0x32)) {
      Serial.println(F("Failed to boot VL53L0X - sensor 3"));
      while (1);
    }
  
    digitalWrite(shdn4, HIGH);
    if (!lox4.begin(0x33)) {
      Serial.println(F("Failed to boot VL53L0X - sensor 4"));
      while (1);
    }
  
    digitalWrite(shdn5, HIGH);
    if (!lox5.begin(0x34)) {
      Serial.println(F("Failed to boot VL53L0X - sensor 5"));
      while (1);
    }
  
    digitalWrite(shdn6, HIGH);
    if (!lox6.begin(0x35)) {
      Serial.println(F("Failed to boot VL53L0X - sensor 6"));
      while (1);
    }
  
    digitalWrite(shdn7, HIGH);
    if (!lox7.begin(0x36)) {
      Serial.println(F("Failed to boot VL53L0X - sensor 7"));
      while (1);
    }

  //********************************************************
  // Start midi, lcd and Serial monitor

  Serial.begin(9600);
  lcd.begin(16, 2);  // Begin LCD
  lcd.setCursor(1,0);
  lcd.print("Welcome to PIE!");
  lcd.setCursor(1,1);
  lcd.print("Starting up...");


  VS1053_MIDI.begin(31250); // MIDI uses a 'strange baud rate'

  //********************************************************
  // MusicShield stuff setup
  pinMode(VS1053_RESET, OUTPUT);
  digitalWrite(VS1053_RESET, LOW);
  delay(10);
  digitalWrite(VS1053_RESET, HIGH);
  delay(10);

  midiSetChannelBank(0, VS1053_BANK_MELODY);
  midiSetInstrument(0, 83);
  midiSetChannelVolume(0, 127);

  // All the possible instruments
  //  const char instrumentNames[] = {
  //    "Grand Piano", "Bright Piano", "Elec Grand Piano", "Honky-Tonk Piano", "Electric Piano 1", "Electric Piano 2", "Harpsicord", "Clavi",
  //    "Celesta", "Glockenspiel", "Music Box", "Vibraphone", "Marimba", "Xylophone", "Tubular Bells", "Dulcimer",
  //    "Drawbar Organ", "Percussive Organ", "Rock Organ", "Church Organ", "Reed Organ", "Accordian", "Harmonica", "Tango Accordian",
  //    "Acous Gtr (Nylon)", "Acous Gtr (Steel)", "Elec Gtr (Jazz)", "Elec Gtr (Clean)", "Elec Gtr (Muted)", "Overdriven Gtr", "Distorted Gtr", "Gtr Harmonics",
  //    "Acoustic Bass", "Elec Bass (Finger)", "Elec Bass (Pick)", "Fretless Bass", "Slap Bass 1", "Slap Bass 2", "Synth Bass 1", "Synth Bass 2",
  //    "Violin", "Viola", "Cello", "Contrabass", "Tremelo Strings", "Pizzicato Strings", "Orchestral Harp", "Timpani",
  //    "String Ensemb 1", "String Ensemb 2", "Synth Strings 1", "Synth Strings 2", "Choir Aahs", "Voice Oohs", "Synth Voice", "Orchestra Hit",
  //    "Trumpet", "Trombone", "Tuba", "Muted Trumpet", "French Horn", "Brass Section", "Synth Brass 1", "Synth Brass 2",
  //    "Soprano Sax", "Alto Sax", "Tenor Sax", "Baritone Sax", "Oboe", "English Horn", "Bassoon", "Clarinet",
  //    "Piccolo", "Flute", "Recorder", "Pan Flute", "Blown Bottle", "Shakuhachi", "Whistle", "Ocarina",
  //    "Square Lead", "Saw Lead", "Calliope Lead", "Chiff Lead", "Charang Lead", "Voice Lead", "Fifths Lead", "Bass & Lead",
  //    "New Age Pad", "Warm Pad", "Polysynth Pad", "Choir Pad", "Bowed Pad", "Metallic Pad", "Halo Pad", "Sweep Pad",
  //    "Rain FX", "Sound Track FX", "Crystal FX", "Atmosphere FX", "Brightness FX", "Goblins FX", "Echoes FX", "Sci-Fi FX",
  //    "Sitar", "Banjo", "Shamisen", "Koto", "Kalimba", "Bag Pipe", "Fiddle", "Shanai",
  //    "Tinkle Bell", "Agogo", "Pitched Percusssion", "Wood Block", "Taiko Drum", "Melodic Drum", "Synth Drum", "Reverse Cymbal",
  //    "Gtr Fret Noise", "Breath Noise", "Seashore", "Bird Tweet", "Telephone Ring", "Helicopter", "Applause", "Gunshot"
  //  };
  //********************************************************
  // Rotary encoder setup
  for (int i = 0; i < 7; i++) {
    pinMode(pinA[i], INPUT);
    pinMode(pinB[i], INPUT);
  }

  currentTime = millis();
  loopTime = currentTime;
}
//********************************************************

void loop() {
  int count = 0;
  lcd.clear();

  for (int i = 0; i < 8; i++) {
    bool buttonState = digitalRead(buttons[i]);  // If clicked, buttonState is high
    while(digitalRead(buttons[i])){
      
    }
    // Serial.println(buttonState);
    while (buttonState == HIGH) {
      delay(30);
      int reading = digitalRead(buttons[i]);
      if (reading == LOW) {
        check = HIGH;   // does a check if the button goes low at least once
        //Serial.println("check");
        delay(10);
      }
      currentTime = millis();  // Saves the time
      // Serial.println("Check");
      if (currentTime >= (loopTime + 5)) {
        encoder_A[i] = digitalRead(pinA[i]); // Read encoder pins
        encoder_B[i] = digitalRead(pinB[i]);
        if ((!encoder_A[i]) && (encoder_A_prev[i])) {
          // A has gone from high to low
          //Serial.println("test");
          if (encoder_B[i] && encoder_B_prev[i]) {
            // B is high so clockwise
            count--;
            lcd.setCursor(0,1);
            lcd.print("             ");
            if (i != 7) {  // If the button is not the instrument choser
              if (count < 0) { //reset count
                count = 20;     // reset to 21 for 1-7
              }
            }
            else {
              if (count < 0) {  // reset to 127 for 8
                count = 127;
              }
            }
          }
        }
        if ((!encoder_B[i]) && (encoder_B_prev[i])) {
          // A has gone from high to low
          if (encoder_A[i] && encoder_A_prev[i]) {
            // B is high so clockwise
            count++;
            lcd.setCursor(0,1);
            lcd.print("             ");
            if (i != 7) {
              if (count > 20) { //reset count
                count = 0; // reset if bigger than 21 for 1-7
              }
            }
            else {
              if (count > 127) {
                count = 0;  // reset if bigger than 127 for 8
              }
            }
          }
        }
      }
      lcd.setCursor(1,0);
      if (i != 7) {
        lcd.print("Sensor:");
        lcd.setCursor(9,0);
        lcd.print(i+1);                       // prints the sensor number
        lcd.setCursor(1,1);
        lcd.print(chordNames[count]) ;       // print the name of the chord on the LCD
        chordNumber[i] = count;               // Stores number of the chord for the specific sensor
        Serial.println(i+1);
        Serial.println(chordNames[count]);    
      }
      else {
        lcd.println(count);
        midiSetInstrument(0, count);
      }
      encoder_A_prev[i] = encoder_A[i];     // Store value of A for next time
      encoder_B_prev[i] = encoder_B[i];


      loopTime = currentTime;  // Updates loopTime
      if (digitalRead(buttons[i]) == HIGH && check == HIGH) { // if it detects a click again, it sets buttonState low
        buttonState = LOW; // Go out of the loop
        delay(30);
        while (digitalRead(buttons[i])){
        }
      }
    }
  }


  // Measure the range sensors
  VL53L0X_RangingMeasurementData_t measure1;
  VL53L0X_RangingMeasurementData_t measure2;
  VL53L0X_RangingMeasurementData_t measure3;
  VL53L0X_RangingMeasurementData_t measure4;
  VL53L0X_RangingMeasurementData_t measure5;
  VL53L0X_RangingMeasurementData_t measure6;
  VL53L0X_RangingMeasurementData_t measure7;

  // Not sure if this is needed
  //  lox1.rangingTest(&measure1, false); // pass in 'true' to get debug data printout!
  //  lox2.rangingTest(&measure2, false); // pass in 'true' to get debug data printout!
  //  lox3.rangingTest(&measure3, false); // pass in 'true' to get debug data printout!
  //  lox4.rangingTest(&measure4, false); // pass in 'true' to get debug data printout!
  //  lox5.rangingTest(&measure5, false); // pass in 'true' to get debug data printout!
  //  lox6.rangingTest(&measure6, false); // pass in 'true' to get debug data printout!
  //  lox7.rangingTest(&measure7, false); // pass in 'true' to get debug data printout!


  octaves[0] = round((measure1.RangeMilliMeter) / 100);
  octaves[1] = round((measure2.RangeMilliMeter) / 100);
  octaves[2] = round((measure3.RangeMilliMeter) / 100);
  octaves[3] = round((measure4.RangeMilliMeter) / 100);
  octaves[4] = round((measure5.RangeMilliMeter) / 100);
  octaves[5] = round((measure6.RangeMilliMeter) / 100);
  octaves[6] = round((measure7.RangeMilliMeter) / 100);

 // Play a chord
  for (int i = 0; i < 7; i++) {
    playChord(i);
  }
}


//delay(1000);
void playChord(int sensorNr) {
  if (octaves[sensorNr] == 81) {   // If the sensor sees nothing
    for (int i = 0; i < 4; i++) {
      midiNoteOff(0, chords[chordNumber[sensorNr]][i] + lastOctaves[0] * 12, 127);  // Put the notes off that is last played
    }

  }
  else if (octaves[sensorNr] != lastOctaves[sensorNr]) {   // If the sensor sees something else than last time
    for (int i = 0; i < 4; i++) {
      midiNoteOff(0, chords[chordNumber[sensorNr]][i] + lastOctaves[0] * 12, 127);  // Put last chord off 
      midiNoteOn(0, chords[chordNumber[sensorNr]][i] + octaves[0] * 12, 127);  // And play the new chord
    }
  }  
  lastOctaves[sensorNr] = octaves[sensorNr];     // Save the last cord played
}


// Midi music shield functions
void midiSetInstrument(uint8_t chan, uint8_t inst) {
  if (chan > 15) return;
  inst --; // page 32 has instruments starting with 1 not 0 :(
  if (inst > 127) return;

  VS1053_MIDI.write(MIDI_CHAN_PROGRAM | chan);
  VS1053_MIDI.write(inst);
}


void midiSetChannelVolume(uint8_t chan, uint8_t vol) {
  if (chan > 15) return;
  if (vol > 127) return;

  VS1053_MIDI.write(MIDI_CHAN_MSG | chan);
  VS1053_MIDI.write(MIDI_CHAN_VOLUME);
  VS1053_MIDI.write(vol);
}

void midiSetChannelBank(uint8_t chan, uint8_t bank) {
  if (chan > 15) return;
  if (bank > 127) return;

  VS1053_MIDI.write(MIDI_CHAN_MSG | chan);
  VS1053_MIDI.write((uint8_t)MIDI_CHAN_BANK);
  VS1053_MIDI.write(bank);
}

void midiNoteOn(uint8_t chan, uint8_t n, uint8_t vel) {
  if (chan > 15) return;
  if (n > 127) return;
  if (vel > 127) return;

  VS1053_MIDI.write(MIDI_NOTE_ON | chan);
  VS1053_MIDI.write(n);
  VS1053_MIDI.write(vel);
}

void midiNoteOff(uint8_t chan, uint8_t n, uint8_t vel) {
  if (chan > 15) return;
  if (n > 127) return;
  if (vel > 127) return;

  VS1053_MIDI.write(MIDI_NOTE_OFF | chan);
  VS1053_MIDI.write(n);
  VS1053_MIDI.write(vel);
}
