/* Main Arduino sketch for Nybble, the walking robot kitten.
   Updates should be posted on GitHub: https://github.com/PetoiCamp/OpenCat

   Rongzhong Li
   Aug. 27, 2018
   Copyright (c) 2018 Petoi LLC.

   This sketch may also includes others' codes under MIT or other open source liscence.
   Check those liscences in corresponding module test folders.
   Feel free to contact us if you find any missing references.

   The MIT License

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
#define MAIN_SKETCH
#include "WriteInstinct/OpenCat.h"

#include <I2Cdev.h>
#include <MPU6050_6Axis_MotionApps20.h>


#define PACKET_SIZE 42
#define OVERFLOW_THRESHOLD 128

//#if OVERFLOW_THRESHOLD>1024-1024%PACKET_SIZE-1   // when using (1024-1024%PACKET_SIZE) as the overflow resetThreshold, the packet buffer may be broken
// and the reading will be unpredictable. it should be replaced with previous reading to avoid jumping
#define FIX_OVERFLOW
//#endif
#define HISTORY 2
int8_t lag = 0;
float ypr[3];         // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector
float yprLag[HISTORY][2];

MPU6050 mpu;
#define OUTPUT_READABLE_YAWPITCHROLL
// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[PACKET_SIZE]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorFloat gravity;    // [x, y, z]            gravity vector

// ================================================================
// ===               INTERRUPT DETECTION ROUTINE                ===
// ================================================================

volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
void dmpDataReady() {
  mpuInterrupt = true;
}

char token;
#define CMD_LEN 10
char *lastCmd = new char[CMD_LEN];
char *newCmd = new char[CMD_LEN];
byte newCmdIdx = 0;
byte hold = 0;
int8_t offsetLR = 0;
bool checkGyro = true;
int8_t countDown = 0;

//ESTHER
long duration;
int distance;

const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
unsigned int sampleAux1=0, sampleAux2=0, sampleMic1=0, sampleMic2=0;
int enviarVect[3];
//FIN ESTHER    

uint8_t timer = 0;
#define SKIP 2
#ifdef SKIP
byte updateFrame = 0;
#endif
byte firstMotionJoint;
byte jointIdx = 0;


unsigned long usedTime = 0;

void checkBodyMotion()  {
  if (!dmpReady) return;
  // wait for MPU interrupt or extra packet(s) available
  //while (!mpuInterrupt && fifoCount < packetSize) ;
  if (mpuInterrupt || fifoCount >= packetSize)
  {
    // reset interrupt flag and get INT_STATUS byte
    mpuInterrupt = false;
    mpuIntStatus = mpu.getIntStatus();

    // get current FIFO count
    fifoCount = mpu.getFIFOCount();
    //PTL(fifoCount);
    // check for overflow (this should never happen unless our code is too inefficient)
    if ((mpuIntStatus & 0x10) || fifoCount > OVERFLOW_THRESHOLD) { //1024) {
      // reset so we can continue cleanly
      mpu.resetFIFO();
      // otherwise, check for DMP data ready interrupt (this should happen frequently)

      // -- RzLi --
#ifdef FIX_OVERFLOW
#ifdef DEVELOPER
      PTLF("FIFO overflow! Using last reading!");
#endif
      lag = (lag - 1 + HISTORY) % HISTORY;
#endif
      // --
    }
    else if (mpuIntStatus & 0x02) {
      // wait for correct available data length, should be a VERY short wait
      while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

      // read a packet from FIFO
      mpu.getFIFOBytes(fifoBuffer, packetSize);

      // track FIFO count here in case there is > 1 packet available
      // (this lets us immediately read more without waiting for an interrupt)
      fifoCount -= packetSize;

#ifdef OUTPUT_READABLE_YAWPITCHROLL
      // display Euler angles in degrees
      mpu.dmpGetQuaternion(&q, fifoBuffer);
      mpu.dmpGetGravity(&gravity, &q);
      mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
#ifdef MPU_YAW180
      ypr[2] = -ypr[2];
#else
      ypr[1] = -ypr[1] ;
#endif
#endif
      /*PT(ypr[1] * degPerRad);
        PTF("\t");
        PTL(ypr[2] * degPerRad);*/
      // overflow is detected after the ypr is read. it's necessary to keep a lag recrod of previous reading.  -- RzLi --
#ifdef FIX_OVERFLOW
      for (byte g = 0; g < 2; g++) {
        yprLag[lag][g] = ypr[g + 1];
        ypr[g + 1] = yprLag[(lag - 1 + HISTORY) % HISTORY][g] ;
      }
      lag = (lag + 1) % HISTORY;
#endif
      // --
      //deal with accidents
      if (fabs(ypr[1])*degPerRad > LARGE_PITCH) {
#ifdef DEVELOPER
        PT(ypr[1] * degPerRad);
        PTF("\t");
        PTL(ypr[2] * degPerRad);
#endif
        if (!hold) {
          token = 'k';
          strcpy(newCmd, ypr[1]*degPerRad > LARGE_PITCH ? "lifted" : "dropped");
          newCmdIdx = 1;
        }
        hold = 10;
      }
      // recover
      else if (hold) {
        if (hold == 10) {
          token = 'k';
          strcpy(newCmd, "balance");
          newCmdIdx = 1;
        }
        hold --;
        if (!hold) {
          char temp[CMD_LEN];
          strcpy(temp, newCmd);
          strcpy(newCmd, lastCmd);
          strcpy(lastCmd, temp);
          newCmdIdx = 1;
          meow();
        }
      }
      //calculate deviation
      for (byte i = 0; i < 2; i++) {
        RollPitchDeviation[i] = ypr[2 - i] - motion.expectedRollPitch[i];
        //PTL(RollPitchDeviation[i]);
        RollPitchDeviation[i] = sign(ypr[2 - i]) * max(fabs(RollPitchDeviation[i]) - levelTolerance[i], 0);//filter out small angles
      }
      //PTL(jointIdx);
    }
  }
}

void setup() {
  pinMode(BUZZER, OUTPUT);
  // join I2C bus (I2Cdev library doesn't do this automatically)
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
  Wire.begin();
  //Wire.setClock(400000);
  TWBR = 24; // 400kHz I2C clock (200kHz if CPU is 8MHz)
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
  Fastwire::setup(400, true);
#endif

  Serial.begin(57600);
  Serial.setTimeout(10);
  while (!Serial);
  // wait for ready
  while (Serial.available() && Serial.read()); // empty buffer
  delay(100);
  PTLF("\n* Start *");
  PTLF("Initialize I2C");
  PTLF("Connect MPU6050");
  mpu.initialize();
  //do
  {
    delay(500);
    // verify connection
    PTLF("Test connection");
    PTL(mpu.testConnection() ? F("MPU successful") : F("MPU failed"));//sometimes it shows "failed" but is ok to bypass.
  } //while (!mpu.testConnection());

  // load and configure the DMP
  do {
    PTLF("Initialize DMP");
    devStatus = mpu.dmpInitialize();
    delay(500);
    // supply your own gyro offsets here, scaled for min sensitivity

    for (byte i = 0; i < 4; i++) {
      PT(EEPROMReadInt(MPUCALIB + 4 + i * 2));
      PT(" ");
    }
    PTL();
    mpu.setZAccelOffset(EEPROMReadInt(MPUCALIB + 4));
    mpu.setXGyroOffset(EEPROMReadInt(MPUCALIB + 6));
    mpu.setYGyroOffset(EEPROMReadInt(MPUCALIB + 8));
    mpu.setZGyroOffset(EEPROMReadInt(MPUCALIB + 10));
    // make sure it worked (returns 0 if so)
    if (devStatus == 0) {
      // turn on the DMP, now that it's ready
      PTLF("Enable DMP");
      mpu.setDMPEnabled(true);

      // enable Arduino interrupt detection
      PTLF("Enable interrupt");
      attachInterrupt(INTERRUPT, dmpDataReady, RISING);
      mpuIntStatus = mpu.getIntStatus();

      // set our DMP Ready flag so the main loop() function knows it's okay to use it
      PTLF("DMP ready!");
      dmpReady = true;

      // get expected DMP packet size for later comparison
      packetSize = mpu.dmpGetFIFOPacketSize();
    } else {
      // ERROR!
      // 1 = initial memory load failed
      // 2 = DMP configuration updates failed
      // (if it's going to break, usually the code will be 1)
      PTLF("DMP failed (code ");
      PT(devStatus);
      PTLF(")");
      PTL();
    }
  } while (devStatus);

  //opening music
#if WALKING_DOF == 8
  playMelody(MELODY);
#endif


  assignSkillAddressToOnboardEeprom();
  PTL();

  // servo
  { pwm.begin();

    pwm.setPWMFreq(60 * PWM_FACTOR); // Analog servos run at ~60 Hz updates
    delay(200);

    //meow();
    strcpy(lastCmd, "rest");
    motion.loadBySkillName("rest");
    for (int8_t i = DOF - 1; i >= 0; i--) {
      pulsePerDegree[i] = float(PWM_RANGE) / servoAngleRange(i);
      servoCalibs[i] = servoCalib(i);
      calibratedDuty0[i] =  SERVOMIN + PWM_RANGE / 2 + float(middleShift(i) + servoCalibs[i]) * pulsePerDegree[i]  * rotationDirection(i) ;
      //PTL(SERVOMIN + PWM_RANGE / 2 + float(middleShift(i) + servoCalibs[i]) * pulsePerDegree[i] * rotationDirection(i) );
      calibratedPWM(i, motion.dutyAngles[i]);
      delay(100);
    }
   // randomSeed(analogRead(0));//use the fluctuation of voltage caused by servos as entropy pool
    shutServos();
    token = 'd';
  }
  beep(30);

/*  pinMode(BATT, INPUT);
  pinMode(VCC, OUTPUT);
  pinMode(TRIGGER, OUTPUT); // Sets the trigPin as an Output
  pinMode(ECHO, INPUT); // Sets the echoPin as an Input
  digitalWrite(VCC, HIGH);*/
/*  int t = 0;
  int minDist, maxDist;
 while (0) {//disabled for now. needs virtual threading to reduce lag in motion.
    calibratedPWM(0, -10 * cos(t++*M_PI / 360));
    calibratedPWM(1, 10 * sin(t++ * 2 * M_PI / 360));
    digitalWrite(TRIGGER, LOW);
    delayMicroseconds(2);

    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(TRIGGER, HIGH);
    digitalWrite(BUZZER, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER, LOW);
    digitalWrite(BUZZER, LOW);

    // Reads the echoPin, returns the sound wave travel time in microseconds

    long duration = pulseIn(ECHO, HIGH, farTime);

    // Calculating the distance

    int distance = duration * 0.034 / 2; // 10^-6 * 34000 cm/s

    // Prints the distance on the Serial Monitor
    Serial.print("Distance: ");
    Serial.print(distance == 0 ? LONGEST_DISTANCE : distance);
    Serial.println(" cm");
  }*/
   
  //ESTHER
  pinMode(TRIGGER, OUTPUT); // Sets the trigPin as an Output
  pinMode(ECHO, INPUT); // Sets the echoPin as an Input

  //FIN ESTHER    
  
  meow();
}

void loop() {

 /* float voltage = analogRead(BATT);
  if (voltage <
#ifdef NyBoard_V0_1
      650
#else
      300
#endif
     ) { //give the cat a break when voltage drops after sprint
    //adjust the thresholds according to your batteries' voltage
    //if set too high, Nybble will keep crying.
    //If too low, Nybble may faint due to temporary voltage drop
    PTL(voltage);//relative voltage
    meow();
  }
  else {*/
    newCmd[0] = '\0';
    newCmdIdx = 0;
    // MPU block
/*#ifdef GYRO //if opt out the gyro, the calculation can be really fast
    if (checkGyro && countDown == 0)
      checkBodyMotion();
#endif*/
    // accident block
    //...
    //...
    //for obstacle avoidance and auto recovery

    // input block
    //else if (t == 0) {
    if ( Serial.available() > 0) {
      token = Serial.read();
      newCmdIdx = 3;
    }
    //}
    if (newCmdIdx) {
      //PTL(token);
      beep(newCmdIdx * 4);
      // this block handles argumentless tokens
      switch (token) {
        case '1':{
            //check US ESTHER
      // Clears the trigPin
      digitalWrite(TRIGGER, LOW);
      delayMicroseconds(2);
      // Sets the trigPin on HIGH state for 10 micro seconds
      digitalWrite(TRIGGER, HIGH);
      delayMicroseconds(10);
      digitalWrite(TRIGGER, LOW);
      // Reads the echoPin, returns the sound wave travel time in microseconds
      duration = pulseIn(ECHO, HIGH);
      // Calculating the distance
      distance = duration * 0.034 / 2;
      // Prints the distance on the Serial Monitor
   /*   Serial.print("Distance: ");
      Serial.println(distance);*/

      //MICROS
  sampleMic1 = 0;
  sampleMic2 = 0;
   unsigned long startMillis= millis();  // Start of sample window

   // collect data for 50 mS
   while (millis() - startMillis < sampleWindow)
   {
      sampleAux1 = analogRead(MIC1);
      sampleAux2 = analogRead(MIC2);
      if (sampleAux1 > sampleMic1)
      {
        sampleMic1 = sampleAux1;
      }
      if (sampleAux2 > sampleMic2)
      {
        sampleMic2 = sampleAux2;
      } 
   }
  /*Serial.print("Mic 1: ");
  Serial.println(sampleMic1);
  Serial.print("Mic 2: ");
  Serial.println(sampleMic2); */

enviarVect[0] = distance;
enviarVect[1] = sampleMic1;
enviarVect[2] = sampleMic2;
          for(int i = 0; i < 3; i++)
{
  Serial.println(enviarVect[i]);
} 
          break;
          }
         case '2':{
          meow();
          meow();
          meow();
          meow();
          break;
          }
          case '3':{
         beep(255,255);
         beep(0,255);
         beep(255,255);
         beep(0,255);
         beep(255,255);
         beep(0,255);
         beep(255,255);
         beep(0,255);
         beep(255,255);
         beep(0,255);
          break;
          }
        
        case 'h': {
            PTLF("* Help Info *");// print the help document. not implemented on NyBoard Vo due to limited space
            break;
          }
        case 'd': {
            motion.loadBySkillName("rest");
            transform( motion.dutyAngles);
            PTLF("shut down servos");
            shutServos();
            break;
          }

        case 's': {
            PTLF("save calibration");
            saveCalib(servoCalibs);
            break;
          }
        case 'a': {
            PTLF("abort calibration");
            for (byte i = 0; i < DOF; i++) {
              servoCalibs[i] = servoCalib( i);
            }
            break;
          }

        // this block handles array like arguments
        case 'i': //indexed joint motions: joint0, angle0, joint1, angle1, ...
        case 'l': //list of all 16 joint: angle0, angle2,... angle15
          //case 'o': //for melody
          {
            String inBuffer = Serial.readStringUntil('~');
            int8_t numArg = inBuffer.length();
            char* list = inBuffer.c_str();
            if (token == 'i') {
              for (int i = 0; i < numArg; i += 2) {
                calibratedPWM(list[i], list[i + 1]);
              }
            }
            else if (token == 'l') {
              allCalibratedPWM(list);
            }
            break;
          }
        case 'j': { //show the list of current joint anles
            printList((int8_t*)currentAng);
            break;
          }
        case 'c': //calibration
        case 'm': //move joint to angle
        case 'u': //meow (repeat, increament)
        case 'b': //beep(tone, duration): tone 0 is pause, duration range is 0~255
          {
            int8_t target[2] = {};
            String inBuffer = Serial.readStringUntil('\n');
            byte inLen = 0;
            strcpy(newCmd, inBuffer.c_str());
            char *pch;
            pch = strtok (newCmd, " ,");
            for (byte c = 0; pch != NULL; c++)
            {
              target[c] = atoi(pch);
              pch = strtok (NULL, " ,");
              inLen++;
            }
            if (token == 'c') {
              //PTLF("calibrating [ targetIdx, angle ]: ");
              if (strcmp(lastCmd, "c")) { //first time entering the calibration function
                motion.loadBySkillName("calib");
                transform( motion.dutyAngles);
              }
              if (inLen == 2)
                servoCalibs[target[0]] = target[1];
              PTL();
              for (byte i = 0; i < DOF; i++) {
                PT(i);
                PT(",\t");
              }
              PTL();
              printList(servoCalibs);
              yield();
            }
            else if (token == 'm') {
              //SPF("moving [ targetIdx, angle ]: ");
              motion.dutyAngles[target[0]] = currentAng[target[0]] = target[1];
            }
            else if (token == 'u') {
              meow(target[0], 0, 50, 200, 1 + target[1]);
            }
            else if (token == 'b') {
              beep(target[0], (byte)target[1]);
            }
            PT(token);
            printList(target, 2);
            if (token == 'c' || token == 'm') {
              int duty = SERVOMIN + PWM_RANGE / 2 + float(middleShift(target[0])  + servoCalibs[target[0]] + motion.dutyAngles[target[0]]) * pulsePerDegree[target[0]] * rotationDirection(target[0]);
              pwm.setPWM(pin(target[0]), 0,  duty);
            }
            break;
          }
        default: if (Serial.available() > 0) {
            String inBuffer = Serial.readStringUntil('\n');
            strcpy(newCmd, inBuffer.c_str());
          }
      }
      while (Serial.available() && Serial.read()); //flush the remaining serial buffer in case the commands are parsed incorrectly
      //check above
      if (strcmp(newCmd, "") && strcmp(newCmd, lastCmd) ) {
        //      PT("compare lastCmd ");
        //      PT(lastCmd);
        //      PT(" with newCmd ");
        //      PT(token);
        //      PT(newCmd);
        //      PT("\n");
        if (token == 'w') {}; //some words for undefined behaviors

        if (token == 'k') { //validating key

          motion.loadBySkillName(newCmd);

         char lr = newCmd[strlen(newCmd) - 1];
         offsetLR = (lr == 'L' ? 15 : (lr == 'R' ? -15 : 0));

          //motion.info();
          timer = 0;
          if (strcmp(newCmd, "balance") && strcmp(newCmd, "lifted") && strcmp(newCmd, "dropped") )
            strcpy(lastCmd, newCmd);

#ifdef POSTURE_WALKING_FACTOR
          postureOrWalkingFactor = (motion.period == 1 ? 1 : POSTURE_WALKING_FACTOR);
#endif
          // if posture, start jointIdx from 0
          // if gait, walking DOF = 8, start jointIdx from 8
          //          walking DOF = 12, start jointIdx from 4
          firstMotionJoint = (motion.period == 1) ? 0 : DOF - WALKING_DOF;
          transform( motion.dutyAngles,  1, firstMotionJoint);
          jointIdx = DOF;
         // jointIdx = firstMotionJoint;
          if (!strcmp(newCmd, "rest")) {
            shutServos();
            token = 'd';
          }
        }
        else {
          lastCmd[0] = token;
          memset(lastCmd + 1, '\0', CMD_LEN - 1);
        }
      }
    }

    //motion block
    {

   /*
   if (token == 'k') {
      // if (lastCmd[0] == 'm' && lastCmd[1] == 'r')
#ifndef HEAD  //skip head
      if (jointIdx == 0)
        jointIdx = 2;
#endif
#ifndef TAIL  //skip tail
      if (jointIdx == 2)
        jointIdx = 4;
#endif
      if (motion.period != 1) {//skip non-walking DOF
        if (jointIdx < 4)
          jointIdx = 4;

      }
#if WALKING_DOF==8 //skip shoulder roll 
      if (jointIdx == 4)
        jointIdx = 8;
#endif
      int dutyIdx = timer * WALKING_DOF + jointIdx - firstMotionJoint;
      calibratedPWM(jointIdx, motion.dutyAngles[dutyIdx] );
      jointIdx++;
      delayMicroseconds(1000);
      if (jointIdx == DOF) {
        jointIdx = 0;
#ifdef SKIP
        if (updateFrame++ == SKIP) {
          updateFrame = 0;
#endif
          timer = (timer + 1) % motion.period;
#ifdef SKIP
        }
#endif
      }
    }*/

   
      if (token == 'k') {
        if (jointIdx == DOF) {
#ifdef SKIP
          if (updateFrame++ == SKIP) {
            updateFrame = 0;
#endif
            // timer = (timer + 1) % motion.period;
            timer++;
           if (timer == motion.period) {
              timer = 0;
              if (countDown == 0)
                checkGyro = true;
              else countDown--;
            }
#ifdef SKIP
          }
#endif
          jointIdx =
#ifdef HEAD  //start from head
            0;
#else
#ifdef TAIL
            2;
#else
            DOF - WALKING_DOF;
#endif
#endif
        }

      if (jointIdx < firstMotionJoint && motion.period > 1) {
          calibratedPWM(jointIdx, (jointIdx != 1 ? offsetLR : 0) //look left or right
                        + 10 * sin (timer * (jointIdx + 2) * M_PI / motion.period) //look around
#ifdef GYRO
                        + (checkGyro ? adjust(jointIdx) : 0)
#endif
                       );
        }
       else if (jointIdx >= firstMotionJoint) {
       //delayMicroseconds(1500);
         int dutyIdx = timer * WALKING_DOF + jointIdx - firstMotionJoint;
          calibratedPWM(jointIdx, motion.dutyAngles[dutyIdx]
#ifdef GYRO
                        + adjust(jointIdx)
#endif;
                       );
        }
        jointIdx++;
      }
    }
}
  
