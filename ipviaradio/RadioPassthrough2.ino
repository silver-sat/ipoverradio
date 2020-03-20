
// Timeout in ms to wait for a serial message
#define SERIALBAUD 115200
#define SERIALTIMEOUT 5

// Timeout in ms to wait for a radio message
#define RADIOTIMEOUT 5

// Whether to output diagnotic messages on the serial console - set to 1 to see messages
#define CONSOLE 0

//
// Support for switchable console output and printf formatting
//

char outbuffer[256];
#define console(...) { if (CONSOLE) { sprintf(outbuffer, __VA_ARGS__); Serial.print(outbuffer); }; };

//
// Radio support and wrappers for basic functionality
//

#include <SPI.h>
#include <RH_RF95.h>

#define RFM95_CS 4
#define RFM95_RST 2
#define RFM95_INT 3
#define RF95_FREQ 434.0

// Singleton instance of the radio driver
RH_RF95 radio(RFM95_CS, RFM95_INT);

bool radioinit() {
  return radio.init();
}

bool setFrequency(float freq) {
  return radio.setFrequency(freq);
}

void setTxPower(int8_t power, bool useRFO) {
  radio.setTxPower(power, useRFO);
}

bool radiosend(const uint8_t *buffer, uint8_t length) {
  radio.send(buffer, length);
  radio.waitPacketSent();
  return true;
}

bool radiorecv(uint8_t *buffer, uint8_t* length) {
   if (radio.waitAvailableTimeout(RADIOTIMEOUT)) {
       if (radio.recv(buffer, length) && (*length) > 0) {
           return true;
       }
   }
   return false;
}

//
// Allow for easy redefinition of the relevant serial port
//

#define rPi Serial3

//
// Suport for blinking the board's LED, no use of delay!!!
//

#define LEDPIN LED_BUILTIN
#define LEDINT 100

unsigned long transtime = 0;
unsigned int ledstate = LOW;
unsigned int ledcount = 0;

void blinkit(int count=1) {
  ledcount += count;
  if (transtime == 0) {
    ledstate = HIGH;
    digitalWrite(LEDPIN,ledstate);
    transtime = (millis()+LEDINT);
  }
}

void blinker() {
  if (transtime > 0 && millis() > transtime) {
    if (ledstate == HIGH) {
      ledstate = LOW;
      digitalWrite(LEDPIN,ledstate);
      ledcount--;
      if (ledcount == 0) {
        transtime = 0;  
      } else {
        transtime = (millis()+LEDINT);
      }
    } else {
      ledstate = HIGH;
      digitalWrite(LEDPIN,ledstate);
      transtime = (millis()+LEDINT);
    }
  }
}

//
// Support for computing checksums 
//

unsigned long crc32b(uint8_t *buffer, int len) {
   int i, j;
   unsigned long byte, crc, mask;

   i = 0;
   crc = 0xFFFFFFFF;
   while (i < len) {
      byte = buffer[i];            // Get next byte.
      crc = crc ^ byte;
      for (j = 7; j >= 0; j--) {    // Do eight times.
         mask = -(crc & 1);
         crc = (crc >> 1) ^ (0xEDB88320 & mask);
      }
      i = i + 1;
   }
   return ~crc;
}

uint8_t getbyte(unsigned long a, uint8_t b) {
  return (uint8_t)((a>>(b*8)) & 0x000000ff);
}

//
// Initial setup of the arduino program
//

void setup() {

    // Start the console
    if (CONSOLE) {Serial.begin(9600);}

    // Setup the blinking LED
    pinMode(LEDPIN, OUTPUT);
    digitalWrite(LEDPIN, LOW);

    // Setup the RPi serial connection
    rPi.begin(SERIALBAUD);
    rPi.setTimeout(SERIALTIMEOUT);

    // Initialize the radio
    pinMode(RFM95_RST, OUTPUT);
    digitalWrite(RFM95_RST, HIGH);
    delay(10);
    digitalWrite(RFM95_RST, LOW);
    delay(10);
    digitalWrite(RFM95_RST, HIGH);
    delay(10);

    while (!radioinit()) {
        console("LoRa radio init failed");
        while (1);
    }
    // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
    if (!setFrequency(RF95_FREQ)) {
        console("setFrequency failed");
        while (1);
    }
    setTxPower(5, false);
    console("LoRa radio ready.");

    // Blink the LED three times
    blinkit(3);
}

//
// Set up the buffers for the RPi serial messages and the Radio messages...
//
// Radio messages have a header of HEADERBYTES bytes folloed by a messages of up to RADBUFSIZE bytes 
//

#define RPIBUFSIZE 4096
#define RADBUFSIZE 240
#define HEADERBYTES 4

uint8_t rpibuffer[RPIBUFSIZE];
uint8_t radbuffer[RADBUFSIZE+HEADERBYTES];

//
// Repeating loop
//

void loop() {

    // Blink LED as needed...
    blinker();

    // RPi Serial -> Radio messages
    while (1) {

        // Read message from RPi serial connection
        // If nothing available, break out
        unsigned int rpilen;
        rpilen = rPi.readBytes(rpibuffer, RPIBUFSIZE);
        if (rpilen == 0) {
            break;
        } else if (rpilen >= RPIBUFSIZE) {
            console("Serial: Packet too big (%d), dropping\n", rpilen);
            break;
        }

        // set up the radio message(s)...

        // compute a checksum for the entire message
        unsigned long rpicrc = crc32b(rpibuffer,rpilen);
        
        // header byte 1: message id, 0-254
        radbuffer[0] = random(255); 

        // header byte 2: least significant byte of crc32
        radbuffer[1] = getbyte(rpicrc,0); 

        // header byte 3: total number of blocks required
        radbuffer[2] = rpilen/RADBUFSIZE + 1*(rpilen%RADBUFSIZE != 0); // block count

        // send each block by radio
        bool failed = false;
        unsigned int blkstart;
        uint8_t blknum;
        for (blkstart=0, blknum=1; blkstart<rpilen; blkstart+=RADBUFSIZE, blknum+=1) {
            uint8_t blklen = min(RADBUFSIZE,rpilen-blkstart);
            // header byte 4: block number
            radbuffer[3] = blknum;
            memcpy(radbuffer+HEADERBYTES,rpibuffer+blkstart,blklen);
            if (!radiosend(radbuffer,(blklen+HEADERBYTES))) {
               failed = true;
               break;
            }
            // console("Radio: Block sent: msgig %d crc1 %d blkcnt %d blknum %d length %d\n",
            //         radbuffer[0],radbuffer[1],radbuffer[2],radbuffer[3],blklen+HEADERBYTES); 
            
        }
        if (!failed) {
          
           console("Serial ->> %4d %08lX ->> Radio\n",rpilen,rpicrc);

           blinkit(radbuffer[2]);
        
        }

        break;
        
    }

    // Radio -> RPi Serial 
    while (1) {
      
        unsigned int rpilen=0;

        // msgid 255 is not a valid msg id, use to indicate no radio message received. 
        uint8_t msgid = 255;
        uint8_t msgcrc1 = 0;
        uint8_t blkcnt = 0;
        uint8_t blknum = 0;
        
        while (1) {        

            // check for a radio message
            uint8_t radlen = RADBUFSIZE+HEADERBYTES;
            if (radiorecv(radbuffer, &radlen)) {
                    // console("Radio: Block received: msgig %d crc1 %d blkcnt %d blknum %d length %d\n",
                    //         radbuffer[0],radbuffer[1],radbuffer[2],radbuffer[3],radlen); 

                    if (radbuffer[3] > 1) {
                        // if this is not the first block, check that the first three bytes are consistent with the first block
                        if (radbuffer[0] != msgid || radbuffer[1] != msgcrc1 || radbuffer[2] != blkcnt) {
                            console("Radio: Bad block msgid (%d,%d), crc1 (%d,%d), or count (%d,%d) on following block.\n",
                                    radbuffer[0],msgid,radbuffer[1],msgcrc1,radbuffer[2],blkcnt);
                            // if not, break out and indicate that no message is available.
                            msgid = 255;
                            break;
                        }
                        // and the block number is one more than the previous one seen
                        if (radbuffer[3] != blknum+1) {
                            console("Radio: Bad block number (%d,%d) on following block.\n",radbuffer[3],blknum);
                            // if not, break out and indicate that no message is available.
                            msgid = 255;
                            break;
                        }
                    } else if (radbuffer[3] == 1) {
                      // record the values that do not change between blocks.
                      msgid = radbuffer[0];
                      msgcrc1 = radbuffer[1];
                      blkcnt = radbuffer[2];
                    } else {
                      console("Radio: Bad block number (%d).\n",radbuffer[3]);
                      msgid = 255;
                      break;
                    }
                    blknum = radbuffer[3];

                    // copy the payload of the block to the RPi buffer in the correct position. 
                    memcpy(rpibuffer+(blknum-1)*RADBUFSIZE,radbuffer+HEADERBYTES,radlen-HEADERBYTES);
                    
                    if (blknum == blkcnt) {
                        // if this is the last block, determine the length of the message, and break to send to RPi
                        rpilen = (blkcnt-1)*RADBUFSIZE + (radlen-HEADERBYTES);
                        break;
                    }
            } else {
                // if not message available and we do not already have one or more blocks, break out
                if (msgid == 255) {
                    break;
                } 
            }
        }

        // If we have a message to send...
        if (msgid != 255) {

            // check the message crc32 against the one-byte CRC from the message header. 
            unsigned long rpicrc = crc32b(rpibuffer, rpilen);
            if (msgcrc1 == getbyte(rpicrc,0)) {
          
                rPi.write(rpibuffer, rpilen);

                console("Serial <<- %4d %08lX <<- Radio\n",rpilen,rpicrc);
                
            } else {
              
                console("Radio: Bad packet crc, dropping.\n");
            
            }

        }
        
        break;

    }
}
