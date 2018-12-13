#include <Encoder.h>
Encoder myEnc(3,2);
unsigned long initialTime = 0;
unsigned long previousTime = 0;
unsigned long prevTime = 0;
unsigned long nowTime = 0;
long initialReading = 0;
long previousReading = 0;
float errorAccum = 0.01;
int potReading = 0;
int PWM = 0;
float encPerMS = 0;
float encDiff = 0;
float timeDiff = 0;
float revPerS = 0;
float revsDesired = 0.5;
float error = 0;
float intError = 0;
float DesiredV = 0;

const int buttonSnare = 7;
const int buttonBass = 8;
const int solenoidSnare = 6;
const int solenoidBass = 9;
const int buttonLoop = 4;
bool snareFlag;
bool bassFlag;
bool recording;
bool buttonLoopState;
bool loopFlag;
bool prevLoopFlag;
bool snareState;
bool bassState;
int bpm = 100;
long temp = (1000.0 * 60.0 / bpm);
int mspb = int(temp)*2;
int bassArray[40];
int snareArray[40];
int bassIndex = 0;
int snareIndex = 0;
int snareHits = 0;
int bassHits = 0;
unsigned long t;
int recordingStartTime;
int recordingFinishTime;
int snareArrayParsed[1] = {-1};
int bassArrayParsed[1] = {-1};
long snareTime = 0;
long bassTime = 0;

unsigned long currentTime;
unsigned long loopTime;
const int pin_A = 12;  // pin 12
const int pin_B = 13;  // pin 11
const int button = 3;
unsigned char encoder_A;
unsigned char encoder_B;
unsigned char encoder_A_prev=0;
unsigned char encoder_B_prev=0;
int motorSpeed = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(buttonSnare,INPUT);
  pinMode(buttonBass,INPUT);
  pinMode(buttonLoop,INPUT);
  pinMode(solenoidSnare,OUTPUT);
  pinMode(solenoidBass,OUTPUT);
  pinMode(A0,INPUT);
  pinMode(11,OUTPUT);
  pinMode(12,INPUT);
  pinMode(13,INPUT);
  Serial.begin(9600);
}

void loop() {
  
  t = millis();
  initialReading = myEnc.read();
  snareFlag = digitalRead(buttonSnare);
  bassFlag = digitalRead(buttonBass);
  loopFlag = digitalRead(buttonLoop);

  // Checks for the button press
  if(loopFlag && prevLoopFlag && !recording && !buttonLoopState){
    recording = true;
    mspb = 1000 / (revPerS+0.00001); // converts the motor's speed to a millisecond to beats
    recordingStartTime = ((initialReading / 1120)+1) * 1120;
    bassHits = 0;
    snareHits = 0;
    buttonLoopState = true;
    delay(40);
  }

  // Checks for the button to be pressed again to end the loop
  if(loopFlag && prevLoopFlag && recording && !buttonLoopState){
    recording = false; 
    recordingFinishTime = ((initialReading / 1120)+1)*1120;
    // converts all early hits to the end
    for (int i = 0; i < snareHits; i++){
      if (snareArray[i] < 0){
        snareArray[i] = (recordingFinishTime-recordingStartTime)+snareArray[i];
      }
    for (int i = 0; i < bassHits; i++){
      if(bassArray[i] < 0){
        bassArray[i] = (recordingFinishTime-recordingStartTime)+bassArray[i];
      }
    }
    delay(40);
    }
    bassIndex = 0;
    snareIndex = 0;
    buttonLoopState = true;
  }
  // Senses for the button to be unpressed to allow it to be pressed again
  if(buttonLoopState && !loopFlag && !prevLoopFlag){
    buttonLoopState = false;
    delay(40);
  }

  // Recording portion
  if(recording){

    //If the snare drum is hit, turn the snare drum solenoid off to hit it  and record the point in the motor's revolution when it was hit
    if(snareFlag){
      digitalWrite(solenoidSnare,LOW);
      if(!snareState){
        snareArray[snareIndex] = initialReading-recordingStartTime;
        snareIndex++;
        snareHits++;
      }
      snareState = true;
      delay(20);
    }
    // turn the snare drum on to pull the hitter back in if the button is not hit
    else{
      digitalWrite(solenoidSnare,HIGH);
      snareState = false;
    }
    // same thing as above but for the bass drum
    if(bassFlag){
      digitalWrite(solenoidBass,LOW);
      if(!bassState){
        bassArray[bassIndex] = initialReading-recordingStartTime;
        Serial.println(bassArray[bassIndex]);
        bassIndex++;
        bassHits++;
      }
      bassState = true;
      delay(20);
    }
    else{
      digitalWrite(solenoidBass,HIGH);
      bassState = false;
    }
  }

  // playing the looping functionality
  else{
    // if one of the hits in the array is within 10 of the current encoder reading, hit the drum
    snareFlag = false;
    for(int i = 0; i < snareHits; i++){
      if(snareArray[i] < initialReading % (recordingFinishTime - recordingStartTime) && initialReading % (recordingFinishTime - recordingStartTime) < snareArray[i] + 10){
        snareFlag = true;
        snareTime = t;
      }
    }
    if(snareFlag || t - snareTime < 70){
      digitalWrite(solenoidSnare,LOW);
    }
    else{
      digitalWrite(solenoidSnare,HIGH);
    }
    // same as above but for bass drum
    bassFlag = false;
    for(int i = 0; i < bassHits; i++){
      if(bassArray[i] < initialReading % (recordingFinishTime - recordingStartTime) && initialReading % (recordingFinishTime - recordingStartTime) < bassArray[i] + 10){
        bassFlag = true;
        bassTime = t;
      }
    }
    if(bassFlag || t - bassTime < 70){
      digitalWrite(solenoidBass,LOW);
    }
    else{
      digitalWrite(solenoidBass,HIGH);
    }
  }  
  // updates the speed of the motor and maps it to a revolutions per second
  initialTime = millis();
  if(initialTime - previousTime > 300){
    initialReading = myEnc.read();
    encDiff = initialReading - previousReading;
    timeDiff = initialTime - previousTime;
    encPerMS = encDiff / timeDiff;
    revPerS = encPerMS * 1000 / 1120;
    previousReading = initialReading;
    previousTime = initialTime;
  }
  // changes the speed of the motor
  getCount();
  PWM = motorSpeed;
  Serial.println(PWM);
  analogWrite(11,PWM);
  prevLoopFlag = loopFlag;
}
void getCount(){
  // changes the speed of the motor based on the rotary encoder clicks
  currentTime = millis();
  if(currentTime >= (loopTime + 5)){
    // 5ms since last check of encoder = 200Hz  
    encoder_A = digitalRead(pin_A);    // Read encoder pins
    encoder_B = digitalRead(pin_B);   
    if((!encoder_A) && (encoder_A_prev)){
      // A has gone from high to low 
      if(encoder_B && encoder_B_prev) {
        // B is high so clockwise
        if(motorSpeed > 5){
          motorSpeed -= 5;
        }
      }
    }
    if((!encoder_B) && (encoder_B_prev)){
      // A has gone from high to low 
      if(encoder_A && encoder_A_prev) {
        // B is high so clockwise
        if(motorSpeed < 200){
          motorSpeed += 5;
        }
      }     
    }   
    encoder_A_prev = encoder_A;     // Store value of A for next time
    encoder_B_prev = encoder_B;     
   
    loopTime = currentTime;  // Updates loopTime
  }
}
