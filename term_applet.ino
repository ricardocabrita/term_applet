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
int kk = 0;

int pins [3] = {10,13,11};

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

bool try_execute() {
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

  //string cmp with commands

  if (strcmp(cmd, "check") == 0) {
    Serial.println("Yes, the serial cmd line is working");
    return false;
  }else if (strcmp(cmd, "test") == 0){
    Serial.println("Testing pin..");
    return true;
  }
  go--;
  return false;
}


void setup() {
  // put your setup code here, to run once:
  go =  0;
  error_flag = false;

  //SPI setup for AD5293
  //pinMode(ss_pin, OUTPUT);
  //pinMode(ad5293rdy, INPUT);
  for (int i = 0; i < 5; i++) {
    pinMode(pins[i], OUTPUT);
  }

  Serial.begin(115200);

}

void loop() {
  // put your main code here, to run repeatedly:
  while ( Serial.available() > 0)
    read_byte(Serial.read());

  if (go > 0)
    if(try_execute()){
      digitalWrite(pins[kk], HIGH);
      Serial.print("Turning pin ");
      Serial.print(kk);
      Serial.println(" HIGH");
      delay(5000); //wait 5 seconds
      digitalWrite(pins[kk], LOW);
      kk++;
      if(kk == 5) kk = 0;
    }

}
