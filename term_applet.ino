#include "RingBufCPP.h"
#include "AFMotor.h"
#include "SPI.h"

const size_t LINE_BUF_SIZE = 64;
const size_t MAX_LINES = 4;
RingBufCPP<char, LINE_BUF_SIZE> line;
RingBufCPP<size_t, MAX_LINES> cmd_len;

//pump motor init (DC)
AF_DCMotor pump(1); //1Khz default pwm on channel 1

bool error_flag;
byte go;
size_t nr_bytes;

const int ss_pin = 10;

void read_byte(char c) {
  if (c == '\n') {
    go++;
    nr_bytes = line.numElements();
    cmd_len.add(nr_bytes);
    //Serial.println("Got a new line!");
  } else {
    line.add(c);
  }
}

void try_execute() {
  char bite;
  size_t len;
  cmd_len.pull(&len);
  //Serial.print("cmd length: ");
  //Serial.println(len);
  char *cmd = new char[len + 1];

  for (int i = 0; i < len; i++) {
    line.pull(&bite);
    cmd[i] = bite;
    //Serial.println(cmd[i]);
  }
  cmd[len] = '\0'; //strings end with \0
  //string cmp with commands
  if(len < 4) {
    int speed = atoi(cmd);
    if (speed > -1 && speed < 256){
        Serial.print("Setting motor speed to: ");
        Serial.println(speed);
        pump.setSpeed(speed);
    }
  }
  else if (strcmp(cmd, "fill") == 0) {
    pump.run(FORWARD);
    Serial.println("Look at me I'm filling the sphere!");
  } else if (strcmp(cmd, "empty") == 0) {
    pump.run(BACKWARD);
    Serial.println("Look at me I'm emptying the sphere!");
  } else if (strcmp(cmd, "stop") == 0) {
    pump.run(RELEASE);
    Serial.println("Look at me I'm stopping the pump");
  }
  go--;

}


void setup() {
  // put your setup code here, to run once:
  go =  0;
  error_flag = false;

  //SPI setup for AD5293
  pinMode(ss_pin, OUTPUT);

  Serial.begin(115200);

  ad5293EnableWrite();
}

void loop() {
  // put your main code here, to run repeatedly:
  while ( Serial.available() > 0)
    read_byte(Serial.read());

  if (go > 0)
    try_execute();

}


//AD5293 rheostat programming functions
void ad5293EnableWrite() {
  SPI.beginTransaction(SPISettings(SPI_CLOCK_DIV4, MSBFIRST, SPI_MODE1));

  digitalWrite(ss_pin, LOW);
  //is a delay necessary here ? probably..
  delay(100); //100ms delay.. could also monitor the RDY pin
  SPI.transfer(0x18); //Command 4: 0001 10xx
  SPI.transfer(0x02); //XXXX X01X
  digitalWrite(ss_pin, HIGH);

  SPI.endTransaction();
}
