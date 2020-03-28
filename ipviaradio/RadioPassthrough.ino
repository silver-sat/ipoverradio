
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
// State variables for sending...
//

bool rpiread=true;
bool radiotx=false;

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
    console("LoRa radio ready.\n");

    // initial state
    rpiread=true;
    radiotx=false;

    // Blink the LED three times
    blinkit(3);

}

//
// Set up the buffers for the RPi serial messages and the Radio messages...
//

#define RPIBUFSIZE 4096
#define RADBUFSIZE 255
#define RADMAXMSG1 240
#define RADMAXMSG2 220
#define RADMAXMSG3 190

uint8_t rpibuffer[RPIBUFSIZE];
unsigned int rpibuflen = 0;
unsigned int rpibufpos = 0;

uint8_t radbuffer[RADBUFSIZE];
uint8_t radbuflen;

#define FEND 0xC0

//
// Repeating loop
//

void loop() {

    // Blink LED as needed...
    blinker();

    if (rpiread) {
        rpibuflen = rPi.readBytes(rpibuffer, RPIBUFSIZE);
        if (rpibuflen > 0) {
            console(" Serial read: %d bytes\n",rpibuflen);
            rpiread = false;
            radiotx = true;
            rpibufpos = 0;
        }
    }

    if (radiotx) {
        unsigned int i,j,l;
        i = rpibufpos;
        for (j=(i+1); j<rpibuflen; j++) {
            if (rpibuffer[j] == FEND) {
                break;
            }
        }
        j = min((j+1),rpibuflen);
        l = j-i;
        
        // Frame goes from i ... (j-1), length of frame is (j-i)
        if ((l <= 3*RADMAXMSG3) && (rpibuffer[i] == FEND) && ((rpibuffer[i+1]&0x0F) == 0) && (rpibuffer[j-1] == FEND)) {
            console("      Packet: 0x%02X 0x%02X ... 0x%02X (%u bytes)\n",rpibuffer[i],(rpibuffer[i+1]&0x0F),rpibuffer[j-1],l);
            if (l <= RADMAXMSG1) {
                console("  Radio send: %u - %u (%d bytes)\n",i,j-1,l);
                radiosend(rpibuffer+i,l);
                blinkit();
            } else if (l <= 2*RADMAXMSG2) {
                console("  Radio send: %u - %u (%d bytes)\n",i,i+RADMAXMSG2-1,RADMAXMSG2);
                radiosend(rpibuffer+i,RADMAXMSG2);
                blinkit();
                console("  Radio send: %u - %u (%d bytes)\n",i+RADMAXMSG2,j-1,l-RADMAXMSG2);
                radiosend(rpibuffer+i+RADMAXMSG2,l-RADMAXMSG2);
                blinkit();
            } else {
                console("  Radio send: %u - %u (%d bytes)\n",i,i+RADMAXMSG3-1,RADMAXMSG3);
                radiosend(rpibuffer+i,RADMAXMSG3);
                blinkit();
                console("  Radio send: %u - %u (%d bytes)\n",i+RADMAXMSG3,i+2*RADMAXMSG3-1,RADMAXMSG3);
                radiosend(rpibuffer+i+RADMAXMSG3,RADMAXMSG3);
                blinkit();
                console("  Radio send: %u - %u (%d bytes)\n",i+2*RADMAXMSG3,j-1,l-2*RADMAXMSG3);
                radiosend(rpibuffer+i+2*RADMAXMSG3,l-2*RADMAXMSG3);
                blinkit();
            }
        } else {
            if ((j-i) == 4) {
              console(" Ctl Packet: 0x%02X 0x%02X 0x%02X 0x%02X\n",rpibuffer[i],(rpibuffer[i+1]&0x0F),rpibuffer[i+2],rpibuffer[j-1]);
            } else {
              console(" Bad Packet: 0x%02X 0x%02X ... 0x%02X\n",rpibuffer[i],(rpibuffer[i+1]&0x0F),rpibuffer[j-1]);
            }
        }

        rpibufpos = j;
        if (rpibufpos == rpibuflen) {
            rpibufpos = 0;
            rpiread = true;
            radiotx = false;
        } 
        
    }

    while (true) {
        radbuflen = RADBUFSIZE;
        if (radiorecv(radbuffer, &radbuflen)) {
            console("  Radio recv: %d bytes\n",radbuflen);
            rPi.write(radbuffer, radbuflen);
            console("Serial write: %d bytes\n",radbuflen);
        } else {
            break;
        }
    }

}
