/*
  Based on SerialPassthrough sketch example.
*/

#define ERRORSPER1000 10
#define TRANSDELAY 2

#define SERIALBAUD 19200
#define SERIALTIMEOUT 1

#define BUFSIZE 256

#define MAXITER 1

void setup() {
  Serial.begin(9600);
  Serial3.begin(SERIALBAUD);
  Serial3.setTimeout(SERIALTIMEOUT);
  Serial2.begin(SERIALBAUD);
  Serial2.setTimeout(SERIALTIMEOUT);
  randomSeed(analogRead(0));
}

char buffer[BUFSIZE];
unsigned int readlen;
unsigned int iterations;

void loop() {

  iterations = 0;
  while (1) {
    readlen = Serial3.readBytes(buffer,BUFSIZE);
    if (readlen <= 0) {
      break;
    }
    
    // Serial.write("1->2: ");
    // Serial.println(readlen);
    // Serial.write(buffer,readlen);
    
    if (TRANSDELAY>0) {
      delay(TRANSDELAY);
    }
    if (random(1000)<ERRORSPER1000) {
      unsigned int rind = random(BUFSIZE);
      buffer[rind] = 0;
    }
    
    Serial2.write(buffer,readlen);
    iterations += 1;
    if (iterations >= MAXITER) {
      break;
    }
    
  }

  iterations = 0;
  while (1) {
    readlen = Serial2.readBytes(buffer,BUFSIZE);
    if (readlen <= 0) {
      break;
    }
    
    // Serial.write("2->1: ");
    // Serial.println(readlen);
    // Serial.write(buffer,readlen);
    
    if (TRANSDELAY>0) {
      delay(TRANSDELAY);
    }
    if (random(1000)<ERRORSPER1000) {
      unsigned int rind = random(BUFSIZE);
      buffer[rind] = 0;
    }
    
    Serial3.write(buffer,readlen);
    iterations += 1;
    if (iterations >= MAXITER) {
      break;
    }
    
  }
  
}
