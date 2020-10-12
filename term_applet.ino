#include "RingBufCPP.h"
#include "SPI.h"

const size_t LINE_BUF_SIZE = 64;
const size_t MAX_LINES = 4;
RingBufCPP<char, LINE_BUF_SIZE> line;
RingBufCPP<size_t, MAX_LINES> cmd_len;

bool error_flag;
byte go;
size_t nr_bytes;

//pin out definitions
const int ss_pin = 10;
const int ad5293rdy = 8;

//decoder pins
const int G1 = 4;
const int A = 7;
const int B = 6;
const int C = 5;

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
  size_t len, len2=0;
  bool twocmds = false;
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
  String scmd = cmd;
  String subzero = scmd.substring(0,4);
  String subzero3 = scmd.substring(0,3);

  //string cmp with commands

  if(subzero == "pot ") {
    Serial.println("Got a pot command");
    int val = scmd.substring(4).toInt();
    Serial.print("- setting wiper to: ");
    Serial.println(val);
    ad5293potWrite(val);

  } else if(subzero3 == "on ") {
    Serial.print("Turn ON command for LED");
    int val = scmd.substring(4).toInt();
    Serial.println(val);
    turn_on_led(val);

  } else if(strcmp(cmd, "off") == 0) {
    Serial.printl("Turn OFF all LEDs");
    digitalWrite(G1, LOW);

  } else if(strcmp(cmd, "potenable") == 0) {
    Serial.println("Enabling write to ad5293!");
    ad5293EnableWrite();

  } else if(strcmp(cmd, "potread") == 0) {
    Serial.println("Read wiper cmd");
    int wiper = ad5293readWiper();

  } else if(strcmp(cmd, "whoami") == 0) {
    Serial.println("I am the LED controller arduino board!");

  }
  go--;

}


void setup() {
  // put your setup code here, to run once:
  go =  0;
  error_flag = false;

  //SPI setup for AD5293
  pinMode(ss_pin, OUTPUT);//~SYNC, SS
  digitalWrite(ss_pin, HIGH);
  pinMode(ad5293rdy, INPUT); //RDY
  pinMode(11, OUTPUT); //MOSI
  pinMode(12, INPUT); //MISO
  pinMode(13, OUTPUT); //SCK

  //SN74LS138D decoder pins
  pinMode(G1, OUTPUT);
  pinMode(C, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(A, OUTPUT);

  digitalWrite(G1, LOW); //start with all LEDs off

  Serial.begin(115200);

  //ad5293EnableWrite();
}

void loop() {
  // put your main code here, to run repeatedly:
  while ( Serial.available() > 0)
    read_byte(Serial.read());

  if (go > 0)
    try_execute();

}

//helper functions
void turn_on_led(int lidx){
  switch (lidx){
    case 1:
      didigitalWrite(G1, HIGH);
      didigitalWrite(C, LOW);
      didigitalWrite(B, LOW);
      didigitalWrite(A, LOW);
      break;
    case 2:
      didigitalWrite(G1, HIGH);
      didigitalWrite(C, LOW);
      didigitalWrite(B, LOW);
      didigitalWrite(A, HIGH);
      break;
    case 3:
      didigitalWrite(G1, HIGH);
      didigitalWrite(C, LOW);
      didigitalWrite(B, HIGH);
      didigitalWrite(A, LOW);
      break;
    case 4:
      didigitalWrite(G1, HIGH);
      didigitalWrite(C, LOW);
      didigitalWrite(B, HIGH);
      didigitalWrite(A, HIGH);
      break;
    case 5:
      didigitalWrite(G1, HIGH);
      didigitalWrite(C, HIGH);
      didigitalWrite(B, LOW);
      didigitalWrite(A, LOW);
      break;
    case 6:
      didigitalWrite(G1, HIGH);
      didigitalWrite(C, HIGH);
      didigitalWrite(B, LOW);
      didigitalWrite(A, HIGH);
      break;
    case 7:
      didigitalWrite(G1, HIGH);
      didigitalWrite(C, HIGH);
      didigitalWrite(B, HIGH);
      didigitalWrite(A, LOW);
      break;
    case 8:
      didigitalWrite(G1, HIGH);
      didigitalWrite(C, HIGH);
      didigitalWrite(B, HIGH);
      didigitalWrite(A, HIGH);
      break;
  }
}

//AD5293 rheostat programming functions
void ad5293EnableWrite() {
  SPI.beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE1));

  digitalWrite(ss_pin, LOW);
  //is a delay necessary here ? probably..
  //delay(100); //100ms delay.. could also monitor the RDY pin ?
  SPI.transfer(0x18); //Command 4: 0001 10xx
  SPI.transfer(0x02); //XXXX X01X
  digitalWrite(ss_pin, HIGH);

  SPI.endTransaction();
}

void ad5293potWrite(unsigned int val) {
  SPI.beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE1));

  digitalWrite(ss_pin, LOW);

  //val >> 8 2 higher bits of data + command
  byte hibyte = (val >> 8) + 0x04; // 0x04 is command 1
  byte lobyte = val & 0xFF;
  SPI.transfer(hibyte);
  SPI.transfer(lobyte);
  digitalWrite(ss_pin, HIGH);
  SPI.endTransaction();
}

int ad5293readWiper() {
  unsigned int res = 0;

  SPI.beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE1));
  digitalWrite(ss_pin, LOW);

  byte hibyte = 0x08;
  byte lobyte = 0x00;
  SPI.transfer(hibyte);
  SPI.transfer(lobyte);
  delay(2); //wait for response to be ready, alternatively can also monitor the ready pin.
  Serial.print("Rdy pin: ");
  Serial.println(digitalRead(ad5293rdy));
  res = SPI.transfer(0);
  Serial.print("Res: ");
  Serial.println(res);
  //res = res & 0x03; //only the last 2 bits of hi byte
  //res = (res << 8) || SPI.transfer(0);
  res = SPI.transfer(0);
  //Serial.print("Res: ");
  Serial.println(res);

  digitalWrite(ss_pin, HIGH);
  SPI.endTransaction();

  return res;
}
