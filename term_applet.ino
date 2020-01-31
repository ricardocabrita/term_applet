#include "RingBufCPP.h"

const size_t LINE_BUF_SIZE = 64;
const size_t MAX_LINES = 4;
RingBufCPP<char, LINE_BUF_SIZE> line;
RingBufCPP<size_t, MAX_LINES> cmd_len;

bool error_flag;
byte go;
size_t nr_bytes;

//motor cmd functions
int mv_forward();
int mv_backward();

int mv_forward() {
  Serial.println("Look at me i'm filling the sphere!");
  return 1;
}

int mv_backward() {
  Serial.println("Look at me i'm emptying the sphere!");
  return 1;
}


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
  if (strcmp(cmd, "fill") == 0) {
    mv_forward();
  } else if (strcmp(cmd, "empty") == 0) {
    mv_backward();
  }
  go--;

}


void setup() {
  // put your setup code here, to run once:
  go =  0;
  error_flag = false;
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  while ( Serial.available() > 0)
    read_byte(Serial.read());

  if (go > 0)
    try_execute();

}
