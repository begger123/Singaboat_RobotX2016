#include <SPI.h>
#include <Wire.h>

#define pwr5volt 6  
#define slaveSelectPin 53
#define RC1 7
#define RC2 8
#define RC3 9
#define modePin 3


int ch1=1500; // Here's where we'll keep our channel values
int ch2=1500; 
int ch3=0;// channel 3 unused, can be used for switching between ROS command or RC command

int forward=1500;
int turn=1500;
int forwardROS=255/2;
int turnROS=255/2;

double right_calib=1;//value 10 default
double left_calib=1;


void receiveEvent(int howMany){
  int i=0;
  while(1<=Wire.available()){
    
    if (i==0){
      forwardROS= Wire.read();
    }
    else if(i==1){
      turnROS=Wire.read();
    }
    else if(i==2){
      right_calib=Wire.read();
    }
    else if(i==3){
      left_calib=Wire.read();
    }

    i=i+1;
  }
  
}

void setup() {
 
  SPI.begin(); //initialize SPI
  pinMode(pwr5volt,OUTPUT);
  pinMode(slaveSelectPin,OUTPUT);
  digitalWrite(pwr5volt,HIGH);

  //slave receiver of I2c
  Wire.begin(8); //8 is address of i2c
  Wire.onReceive(receiveEvent);

  pinMode(RC1, INPUT); // Set our input pins as such
  pinMode(RC2, INPUT);
  pinMode(RC3, INPUT);

  pinMode(modePin, OUTPUT);

  Serial.begin(57600); // Pour a bowl of Serial

}

void loop() {
  
  ch1 = pulseIn(RC1, HIGH, 25000); // Read the pulse width of 
  ch2 = pulseIn(RC2, HIGH, 25000); // each channel
  ch3 = pulseIn(RC3, HIGH, 25000); // ch3 unused at the moment
  
  if((ch3>1000) && (ch3<1300)){
    forward = map(ch1,1100,1900,-500,500); //map values from RC
    turn = map(ch2,1100,1900,-500,500);
    digitalWrite(modePin, HIGH);
  }
  else{
    forward=map(forwardROS, 0, 255, -500, 500); //value is -500 to 500
    turn=map(turnROS, 0, 255, -500, 500);
    digitalWrite(modePin, LOW);
  }


  unicycleRun(forward, turn);
  printValue();
  delay(100);
  
}

void unicycleRun(int forward, int turn){
  int Ur;
  int Ul;
  int digitalStepRight;
  int digitalStepLeft;
  
  Ur=right_calib*(forward-turn/2); //here turning in cw is positive
  Ul=left_calib*(forward+turn/2);
  //Ur & Ul ranges from -750 to 750 if right and left calibration factors = 1 
  Ur=-Ur;
  Ul=-Ul;
  
  //map Ur and Ul to digital potentiometer values 0-255, map back to 1000-2000
  Ur=map(Ur, -750, 750, 1000, 2000);
  Ul=map(Ul, -750, 750, 1000, 2000);

  digitalStepRight=mapToResistance(Ur);
  digitalStepLeft=mapToResistance(Ul);
  
  Serial.print("\nright: ");
  Serial.print(digitalStepRight);

  Serial.print("\nleft:");
  Serial.print(digitalStepLeft);
  setMotor(digitalStepRight, digitalStepLeft);
  
}

int mapToResistance(int input){
  
  int digitalStep;
  
  if (input<=2000&&input>1550)
  {
    digitalStep = map(input,1551,2000,80,0); //right (80-50) left(80-0)
  }
  else if (input<=1550&&input>1450)
  {
    digitalStep =120;
  }
  else if (input<=1450&&input>=1000)
  {
    digitalStep = map(input,1000,1450,255,175); 
  }
  
  return digitalStep;
}

void setMotor(int Right, int Left)
{
    digitalWrite(slaveSelectPin,LOW);//take SS pin to low to select the chip
    SPI.transfer(0);//adress of the first potentiometer 00 (motor select)
    SPI.transfer(Right);//second data is digital value
    digitalWrite(slaveSelectPin,HIGH);//take SS pin high to deselect chip

    delay(50);//wait
    
    digitalWrite(slaveSelectPin,LOW);
    SPI.transfer(1); //adress of second potentiometer 01
    SPI.transfer(Left);
    digitalWrite(slaveSelectPin,HIGH);
}

void printValue()
{ 

  //information to serial monitor
  Serial.print("\nCh1:"); // Print the value of 
  Serial.print(ch1);    // each channel

  Serial.print("\nCh2:");
  Serial.print(ch2);

  Serial.print("\nCh3:");
  Serial.print(ch3);


  Serial.print("\n\n");

  delay(100); // I put this here just to make the terminal window happier  
}
