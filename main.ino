#include <Servo.h>

const int SensorL = A0;
const int SensorR = A1;
const int ServoLSpeed = 5;
const int ServoRSpeed = 6;

Servo servoLeft, servoRight; //for defining and working with servomotors, from Servo.h library
char CommandCheck, LastCommand, LClawStatus = 'O', RClawStatus = 'O'; //O - open, C - closed, H - halfway/neither
int posL = 0, posR = 0, LASTposR, LASTposL, INITIALposR = 90, INITIALposL = 90, COMMANDposR = 180, COMMANDposL = 0; //for claw control, at R=180 and L=0 each will turn 90 degree
static int force1 = 0, force2 = 0;
int COMMANDforce1 = 403, COMMANDforce2 = 403; //for grip strength and object detection
// ~10N = 460
// ~7N = 400
// ~4N = 310
int ClawSpeedDegree = 1, ClawSpeedDelay = 100; //claw movement parameters: degree turn per cycle speed (recommended 1) and milisecond delay (recommended 20-100) of each cycle step

void setup()
{
  Serial.begin(9600);
  servoLeft.attach(9); //defining servo pins
  servoRight.attach(10);
  pinMode(SensorL, INPUT);
  pinMode(SensorR, INPUT);
  pinMode(ServoLSpeed, OUTPUT);
  pinMode(ServoRSpeed, OUTPUT);
  analogWrite(ServoLSpeed, 255);
  analogWrite(ServoRSpeed, 255);
  //setting claws to open position
  Serial.println("Opening claws on start-up");
  servoLeft.write(INITIALposL);
  servoRight.write(INITIALposR);
  LASTposR = INITIALposR;
  LASTposL = INITIALposL;
  delay(500);
}

void readforce()
{
  //reading sensors through ADC
  force1 = analogRead(A0);
  force2 = analogRead(A1);
  //Serial.println(force1);
  //Serial.println(force2);
}

void loop()
{

  readforce();
  
  //claw/sensor control
  CommandCheck = Serial.read();
  if (CommandCheck == '1' || CommandCheck == '2' || CommandCheck == '3' || CommandCheck == 'Q' || CommandCheck == 'W' || CommandCheck == 'E' || CommandCheck == 'A' || CommandCheck == 'S' || CommandCheck == 'D' )
  {
    LastCommand = CommandCheck;
  	Serial.println(LastCommand);
    switch (CommandCheck)
    {
      case '1': //LIGHT GRIP
      {
        COMMANDforce1 = 310, COMMANDforce2 = 310;
        Serial.println("Weak grip");
        break;
      };
      case '2': //MEDIUM GRIP
      {
        COMMANDforce1 = 403, COMMANDforce2 = 403;
        Serial.println("Moderate grip");
        break;
      };
      case '3': //MAX GRIP
      {
        COMMANDforce1 = 466, COMMANDforce2 = 466;
        Serial.println("Strong grip");
        break;
      };
      case 'Q': //CLOSE LEFT
      {
        if (LClawStatus=='C')
        Serial.println("Left claw already closed");
        else
        {
          CloseLToo:
          Serial.println("Closing left");
          Serial.println(LASTposL);
          for (int posL = LASTposL; posL >= COMMANDposL ; posL -= ClawSpeedDegree ) //instructed degree turn each
          {
            delay(ClawSpeedDelay);
            servoLeft.write(posL);
            LASTposL=posL;
            Serial.println(posL);
			readforce();
            if (force1>COMMANDforce1)
            {
            	if (force2>COMMANDforce2)
                {
                    Serial.println("Gripping");
                  	LClawStatus='H';
                  	RClawStatus='H';
                    return;
                }
                
                Serial.println("Left pressing");
                LClawStatus='H';
              	return;
            }
          }
          LClawStatus='C';
	  	  Serial.println(LClawStatus);
        }
        break;
      };
      case 'A': //OPEN LEFT
      {
        if (LClawStatus=='O')
        Serial.println("Left claw already open");
        else
        {
          OpenLToo:
          Serial.println("Opening left");
          Serial.println(LASTposL);
          //open left
          //COMMANDposR = 90, COMMANDposL = 0
          for (int posL = LASTposL; posL <= INITIALposL; posL += ClawSpeedDegree ) //instructed degree turn each
          {
            delay(ClawSpeedDelay);
            servoLeft.write(posL);
            LASTposL=posL;
            Serial.println(posL);
          }
          LClawStatus='O';
	  	  Serial.println(LClawStatus);
        }
        break;
      };
      case 'W': //CLOSE BOTH
      {

        if(LClawStatus=='C')
        {
          if(RClawStatus=='C')
          {
          Serial.println("Both claws already closed");
          return;
          }
          else
          {
          Serial.println("Left claw is already closed");
          goto CloseRToo;
          }
        }
        else
        if(RClawStatus=='C')
        {
          	Serial.println("Right claw is already closed");
        	goto CloseLToo;
        }
        else
        {
          //close both
          Serial.println("Closing claws");
          Serial.println(LASTposL);
          Serial.println(LASTposR);
          for (int posR = LASTposR, posL = LASTposL; posR <= COMMANDposR && posL >= COMMANDposL ; posL -= ClawSpeedDegree, posR += ClawSpeedDegree ) //instructed degree turn each
          {
            delay(ClawSpeedDelay);
            servoLeft.write(posL);
            servoRight.write(posR);
            LASTposR=posR;
            LASTposL=posL;
			Serial.println(posL);
          	Serial.println(posR);
			readforce();
            if (force1>COMMANDforce1 || posL <= COMMANDposL)
            {
              	if (posL <= COMMANDposL)
                {
          		LClawStatus='C';
	  	  		Serial.println(LClawStatus);
            	}
              	else
                LClawStatus='H'; 

                if (force2>COMMANDforce2)
                {
                    if (posR >= COMMANDposR)
               		{
          			RClawStatus='C';
	  	  			Serial.println(RClawStatus);
                    Serial.println("Both claws closed");
                    return;
            		}
                  	         	
                    Serial.println("Gripping");
                  	RClawStatus='H'; 
                    return;
                }
                
                Serial.println("Left pressing");
                goto CloseRToo;
            }
            else
            if (force2>COMMANDforce2 || posR >= COMMANDposR)
            {
                if (posR >= COMMANDposR)
                {
          		RClawStatus='C';
	  	  		Serial.println(RClawStatus);
                Serial.println("Right claw closed");
                return;
            	}
      
                Serial.println("Right pressing");
                RClawStatus='H'; 
                goto CloseLToo;
            }
          }
          LClawStatus='C';
          RClawStatus='C';
	  	  Serial.println(LClawStatus);
	  	  Serial.println(RClawStatus);
        }
        break;
      };
      case 'S': //OPEN BOTH
      {
        if(LClawStatus=='O')
        {
          if(RClawStatus=='O')
          Serial.println("Both claws already open");
          else
          {
          Serial.println("Left claw is already open");
          goto OpenRToo;
          }
        }
        else
        if(RClawStatus=='O')
        {
          	Serial.println("Right claw is already open");
        	goto OpenLToo;
        }
        else
        {
          Serial.println("Opening claws");
          Serial.println(LASTposL);
          Serial.println(LASTposR);
          //open both
          for (int posR = LASTposR, posL = LASTposL; posR >= INITIALposR && posL <= INITIALposL ; posL += ClawSpeedDegree, posR -= ClawSpeedDegree ) //instructed degree turn each
          {
            delay(ClawSpeedDelay);
            servoLeft.write(posL);
            servoRight.write(posR);
            LASTposR=posR;
            LASTposL=posL;
          	Serial.println(posL);
          	Serial.println(posR);
            if (posR <= INITIALposR)
            {
          	RClawStatus='O';
            goto OpenLToo;
            }
            else
            if (posL >= INITIALposL)
            {
          	LClawStatus='O';
            goto OpenRToo;
            }
          }
          LClawStatus='O';
          RClawStatus='O';
	  	  Serial.println(LClawStatus);
		  Serial.println(RClawStatus);
        }
        break;
      };
      case 'E': //CLOSE RIGHT
      {
        if (RClawStatus=='C')
        Serial.println("Right claw already closed");
        else
        {
          CloseRToo:
          Serial.println("Closing right");
          Serial.println(LASTposR);
          //close right
          for (int posR = LASTposR; posR <= COMMANDposR; posR += ClawSpeedDegree ) //instructed degree turn each
          {
            delay(ClawSpeedDelay);
            servoRight.write(posR);
			LASTposR=posR;
            Serial.println(posR);
			readforce();
            if (force2>COMMANDforce2)
            {
                if (force1>COMMANDforce1)
                {
                    Serial.println("Gripping");
                  	LClawStatus='H';
                  	RClawStatus='H';
                    return;
                }
                
                Serial.println("Right pressing");
              	RClawStatus='H';
                return;
            }
          }
          RClawStatus='C';
	  	  Serial.println(RClawStatus);
        }
        break;
      };
      case 'D': //OPEN RIGHT
      {
        if (RClawStatus=='O')
        Serial.println("Right claw already open");
        else
        {
          OpenRToo:
          Serial.println("Opening right");
          Serial.println(LASTposR);
          //open right
          for (int posR = LASTposR; posR >= INITIALposR; posR -= ClawSpeedDegree ) //instructed degree turn each
          {
            delay(ClawSpeedDelay);
            servoRight.write(posR);
            LASTposR=posR;
          	Serial.println(posR);
          }
          RClawStatus='O';
	 	  Serial.println(RClawStatus);
        }
        break;
      };
    }
  }
}
