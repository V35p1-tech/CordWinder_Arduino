#include "Arduino.h"
#include "AccelStepper.h"

//CHANGELOG
  // V3 - limit switches handling
  // V4 - removing of speed potentiometer becouse of mechanic failure 
  // V5 - anit clog limit switch 

// Stepper setup

int  IN_Pot1_Speed_PIN     = A0;
int  IN_Pot2_WorkWidth_PIN  = A1;
int  IN_btn_Start = 5;
int  IN_btn_Homing = 6; 
int  IN_LimitSwitch = 2;
int  IN_LimitSwitch2 = 7;
int  IN_LimitSwitch_Clog = 8;


const int OUT_btn_Start_LED = 6;
const int OUT_Stepper_step  = 3;
const int OUT_Stepper_dir   = 4;

AccelStepper stepperMotor(1, OUT_Stepper_step, OUT_Stepper_dir);

bool homingMoveFromSensor;
bool homingTrigger;
bool limitSwitch_MoveFromSensor;
bool limitSwitch2_MoveFromSensor;
bool limitSwitch_Clog_Triggered;
bool SYS_homing = false; 
bool SYS_drive_homed = false; 
bool SYS_WorkPermission = false;
int  SYS_Speed; 
int  SYS_WorkWidth;
int  btnStart_lastState;
bool rtBtnStart;
bool ftBtnStart;


//FBs
  // Scaling
      int scalingAnalog(int input, int newMinValue, int newMaxValue) {
        // analog read = 1023 <-> 0
        int scaled_value = 0;
        int output = 0;
        scaled_value = analogRead(input);
            // Parametry do normalizacji
          int minValue = 1;  // Minimalna wartość danych
          int maxValue = 1023;  // Maksymalna wartość danych

          // Normalizacja
          float normalizedValue = float(scaled_value - minValue) / float(maxValue - minValue); // Percent value?

          // Skalowanie do oczekiwanego zakresu (np. 0-100)
          output = newMinValue + (float(1 - normalizedValue) * (newMaxValue - newMinValue));

        return output;
      }

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(OUT_Stepper_step,      OUTPUT);
  pinMode(OUT_Stepper_dir,       OUTPUT);
  pinMode(OUT_btn_Start_LED,     OUTPUT);  
  pinMode(LED_BUILTIN,           OUTPUT);
  pinMode(IN_btn_Start,          INPUT_PULLUP);
  pinMode(IN_btn_Homing,         INPUT_PULLUP);
  pinMode(IN_LimitSwitch,        INPUT_PULLUP);
  pinMode(IN_LimitSwitch2,       INPUT_PULLUP);
  pinMode(IN_LimitSwitch_Clog,   INPUT_PULLUP);

  // pinMode(2, OUTPUT);  // WTF ???? !!!! <-------

  SYS_Speed     = scalingAnalog(IN_Pot1_Speed_PIN, 75, 800);
  SYS_WorkWidth = scalingAnalog (IN_Pot2_WorkWidth_PIN, 100, 600);
  stepperMotor.setMaxSpeed(2000);
}

void loop() {
   // put your main code here, to run repeatedly:

    // Report IOs states to 
      // read values from IOs
        SYS_Speed = 47; //=scalingAnalog(IN_Pot1_Speed_PIN, 50, 870);
        SYS_WorkWidth = scalingAnalog (IN_Pot2_WorkWidth_PIN, 50, 715);
        Serial.println();
        Serial.print("Speed: POT1    "); 
        Serial.print(analogRead(IN_Pot1_Speed_PIN));
        Serial.print("    VAR:    "); 
        Serial.print(SYS_Speed);
        Serial.println();
        Serial.print("WorkWidth: POT2  ");
        Serial.print(analogRead(IN_Pot2_WorkWidth_PIN));
        Serial.print("     VAR:  ");
        Serial.print(SYS_WorkWidth);        
        Serial.println();
        Serial.print("BTN START    "); 
        Serial.print(digitalRead(IN_btn_Start));
        Serial.println();
        Serial.print("BTN HOME  ");
        Serial.print(digitalRead(IN_btn_Homing)); 
        Serial.println();
        Serial.print("LS1:  ");
        Serial.print(digitalRead(IN_LimitSwitch));
        Serial.println();
        Serial.print("LS2:  ");
        Serial.print(digitalRead(IN_LimitSwitch2));        
        Serial.println();
        Serial.print("LS Clog:  ");
        Serial.print(digitalRead(IN_LimitSwitch_Clog));        
        Serial.println();
        Serial.print("  !!!  MOTOR POS:  ");
        Serial.print(stepperMotor.currentPosition());
        Serial.print("  to go:  ");     
        Serial.print(stepperMotor.distanceToGo());
        Serial.println();
        Serial.print("WORK PERMISSION:    ");
        Serial.print(SYS_WorkPermission);

        if (digitalRead(IN_btn_Start) == btnStart_lastState) {
            rtBtnStart = false; 
            ftBtnStart = false;          
        }

        if (digitalRead(IN_btn_Start) == 0 && btnStart_lastState == 1) {
            rtBtnStart = true; 
            ftBtnStart = false; 
            btnStart_lastState = 0;
        }
        if (digitalRead(IN_btn_Start) == 1 && btnStart_lastState == 0) {
            rtBtnStart = false; 
            ftBtnStart = true; 
            btnStart_lastState = 1;
        }        


  //~~~~  Limit Switch handling  ~~~~
        //if (digitalRead(IN_btn_Start) == 0 && (digitalRead(IN_LimitSwitch)==0) && (digitalRead(IN_LimitSwitch)==0)) {
        if (rtBtnStart == true && (digitalRead(IN_LimitSwitch)==0) && (digitalRead(IN_LimitSwitch)==0)) {  
          SYS_WorkPermission = true;
        }  
        else if (digitalRead(IN_LimitSwitch)==1) {
          SYS_WorkPermission = false;
          limitSwitch_MoveFromSensor = true;
          stepperMotor.setCurrentPosition(0);
          stepperMotor.stop();

        }
        else if (digitalRead(IN_LimitSwitch2)==1) {
          SYS_WorkPermission = false;
          limitSwitch2_MoveFromSensor = true;
          stepperMotor.setCurrentPosition(0);
          stepperMotor.stop();

        }
        else if (ftBtnStart == true) {
          SYS_WorkPermission = false;
          stepperMotor.stop();
        }  
      // Move from sensor
        if (limitSwitch_MoveFromSensor == true) {
              while(stepperMotor.currentPosition() != 50) {
                  stepperMotor.setSpeed(20);
                  stepperMotor.runSpeed();
              }
              if (stepperMotor.currentPosition() > 49) {
                limitSwitch_MoveFromSensor = false;
                stepperMotor.setCurrentPosition(0);
              }
        }

        if (limitSwitch2_MoveFromSensor == true) {
              while(stepperMotor.currentPosition() != -50) {
                  stepperMotor.setSpeed(-20);
                  stepperMotor.runSpeed();
              }
              if (stepperMotor.currentPosition() < -49) {
                limitSwitch2_MoveFromSensor = false;
                stepperMotor.setCurrentPosition(700);  // <-- WRITE HERE ACCURATE POSITION OF HEAD
              }
        }
        

  //~~~~ WORK ~~~~
    if (SYS_WorkPermission == true) {
        // Set the current position to 0:
        stepperMotor.setCurrentPosition(0);

        // Run the motor forward at 200 steps/second until the motor reaches 400 steps (2 revolutions):
        while(stepperMotor.currentPosition() != SYS_WorkWidth && SYS_WorkPermission == true)
        {
          if (digitalRead(IN_LimitSwitch)==1) {
            stepperMotor.stop();
            SYS_WorkPermission = false;
          }
          else if (digitalRead(IN_LimitSwitch2)==1) {
            stepperMotor.stop();
            SYS_WorkPermission = false;
          }
          else {
          stepperMotor.setSpeed(SYS_Speed);
          stepperMotor.runSpeed();
          }
        }

        delay(750);

        // Reset the position to 0:
        stepperMotor.setCurrentPosition(0);

        // Run the motor backwards at 600 steps/second until the motor reaches -200 steps (1 revolution):
        while(stepperMotor.currentPosition() != -SYS_WorkWidth && SYS_WorkPermission == true) 
        {
              if (digitalRead(IN_LimitSwitch)==1) {
                stepperMotor.stop();
                SYS_WorkPermission = false;
              }
              else if (digitalRead(IN_LimitSwitch2)==1) {
                stepperMotor.stop();
                SYS_WorkPermission = false;
              }
              else {
                stepperMotor.setSpeed(-SYS_Speed);
                stepperMotor.runSpeed();
              }
        }

        delay(750);

        Serial.print("  ***RUN*** ");  
                              
    }

  // !!! HOMING !!!  
    if (digitalRead(IN_btn_Homing)==0 && digitalRead(IN_btn_Start)==1) {
        homingTrigger = true;
    }
    if (homingTrigger == true) {
        while(digitalRead(IN_LimitSwitch) != 1){
        stepperMotor.setSpeed(-50);
        stepperMotor.runSpeed();
        }
        if (digitalRead(IN_LimitSwitch)==1) {
          stepperMotor.stop();
          stepperMotor.setCurrentPosition(0);
          homingTrigger = false;
          homingMoveFromSensor = true;
        }
        if (homingTrigger == false && homingMoveFromSensor == true) {
          while(stepperMotor.currentPosition() != 25) {
              stepperMotor.setSpeed(100);
              stepperMotor.runSpeed();
          }
          if (stepperMotor.currentPosition() >24) {
            homingMoveFromSensor = false;
            stepperMotor.setCurrentPosition(0);  
          }

        }
    }
 



}
