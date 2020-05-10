#include <Encoder.h>

// Continous Controller
const uint8_t NOTE_OFF = 0x80;
const uint8_t NOTE_ON = 0x90;
const uint8_t KEY_PRESSURE = 0xA0;
const uint8_t CC = 0xB0;
const uint8_t PROGRAM_CHANGE = 0xC0;
const uint8_t CHANNEL_PRESSURE = 0xD0;
const uint8_t PITCH_BEND = 0xE0;

// Pin Definitions
// Rows are connected to
const int row1 = 4;
const int row2 = 5;
const int row3 = 6;
const int row4 = 7;

// The 74HC595 uses a serial communication 
// link which has three pins
const int clock = 10;
const int latch = 9;
const int data = 8;


uint8_t keyToMidiMap[32];

boolean keyPressed[32];

int noteVelocity = 127;


// use prepared bit vectors instead of shifting bit left everytime
int bits[] = { B00000001, B00000010, B00000100, B00001000, B00010000, B00100000, B01000000, B10000000 };

enum relativeCCmode {
  TWOS_COMPLEMENT,
  BINARY_OFFSET,
  SIGN_MAGNITUDE
};

const uint8_t channel = 1;        // MIDI channel 1
const uint8_t controller = 0x10;  // General Purpose Controller 1

const Encoder encoder (11, 12);  // A rotary encoder connected to pins 2 and 3

const relativeCCmode negativeRepresentation = SIGN_MAGNITUDE;  // Select the way negative numbers are represented


// 74HC595 shift to next column
void scanColumn(int value) {
  digitalWrite(latch, LOW); //Pulls the chips latch low
  shiftOut(data, clock, MSBFIRST, value); //Shifts out the 8 bits to the shift register
  digitalWrite(latch, HIGH); //Pulls the latch high displaying the data
}

void setup() {
  
  // Map scan matrix buttons/keys to actual Midi note number. Lowest num 41 corresponds to F MIDI note.
  keyToMidiMap[0] = 60;
  keyToMidiMap[1] = 53;
  keyToMidiMap[2] = 54;
  keyToMidiMap[3] = 55;
  keyToMidiMap[4] = 56;
  keyToMidiMap[5] = 57;
  keyToMidiMap[6] = 58;
  keyToMidiMap[7] = 59;

  keyToMidiMap[8] = 52;
  keyToMidiMap[1 + 8] = 45;
  keyToMidiMap[2 + 8] = 46;
  keyToMidiMap[3 + 8] = 47;
  keyToMidiMap[4 + 8] = 48;
  keyToMidiMap[5 + 8] = 49;
  keyToMidiMap[6 + 8] = 50;
  keyToMidiMap[7 + 8] = 51;

  keyToMidiMap[16] = 44;
  keyToMidiMap[1 + 16] = 37;
  keyToMidiMap[2 + 16] = 38;
  keyToMidiMap[3 + 16] = 39;
  keyToMidiMap[4 + 16] = 40;
  keyToMidiMap[5 + 16] = 41;
  keyToMidiMap[6 + 16] = 42;
  keyToMidiMap[7 + 16] = 43;

  keyToMidiMap[24] = 36;
  keyToMidiMap[1 + 24] = 29;
  keyToMidiMap[2 + 24] = 30;
  keyToMidiMap[3 + 24] = 31;
  keyToMidiMap[4 + 24] = 32;
  keyToMidiMap[5 + 24] = 33;
  keyToMidiMap[6 + 24] = 34;
  keyToMidiMap[7 + 24] = 35;

  // setup pins output/input mode
  pinMode(data, OUTPUT);
  pinMode(clock, OUTPUT);
  pinMode(latch, OUTPUT);

  pinMode(row1, INPUT);
  pinMode(row2, INPUT);
  pinMode(row3, INPUT);
  pinMode(row4, INPUT);

    Serial.begin(9600);

  delay(1000);

}

void loop() {

  for (int col = 0; col < 8; col++) {
    
    // shift scan matrix to following column
    scanColumn(bits[col]);

    // check if any keys were pressed - rows will have HIGH output in this case corresponding
    int groupValue1 = digitalRead(row1);
    int groupValue2 = digitalRead(row2);
    int groupValue3 = digitalRead(row3);
    int groupValue4 = digitalRead(row4);

    // process if any combination of keys pressed
    if (groupValue1 != 0 || groupValue2 != 0 || groupValue3 != 0
        || groupValue4 != 0) {

      if (groupValue1 != 0 && !keyPressed[col]) {
        keyPressed[col] = true;
        noteOn(0x91, keyToMidiMap[col], noteVelocity,col," row1 ");
      }

      if (groupValue2 != 0 && !keyPressed[col + 8]) {
        keyPressed[col + 8] = true;
        noteOn(0x91, keyToMidiMap[col + 8], noteVelocity,col," row2 ");
      }

      if (groupValue3 != 0 && !keyPressed[col + 16]) {
        keyPressed[col + 16] = true;
        noteOn(0x91, keyToMidiMap[col + 16], noteVelocity,col," row3 ");
                                

      }

      if (groupValue4 != 0 && !keyPressed[col + 24]) {
        keyPressed[col + 24] = true;
        noteOn(0x91, keyToMidiMap[col + 24], noteVelocity,col," row4 ");
      }

    }

    //  process if any combination of keys released
    if (groupValue1 == 0 && keyPressed[col]) {
      keyPressed[col] = false;
      noteOn(0x91, keyToMidiMap[col], 0,col," row1 ");
    }

    if (groupValue2 == 0 && keyPressed[col + 8]) {
      keyPressed[col + 8] = false;
      noteOn(0x91, keyToMidiMap[col + 8], 0,col," row2 ");
    }

    if (groupValue3 == 0 && keyPressed[col + 16]) {
      keyPressed[col + 16] = false;
      noteOn(0x91, keyToMidiMap[col + 16], 0,col," row3 ");
    }

    if (groupValue4 == 0 && keyPressed[col + 24]) {
      keyPressed[col + 24] = false;
      noteOn(0x91, keyToMidiMap[col + 24], 0,col," row4 ");
    }

  }

  // Encoder
static int32_t previousPosition = 0; // A static variable for saving the previous encoder position
int32_t position = encoder.read(); // Read the current encoder position
int32_t difference = position - previousPosition; // Calculate the relative movement
difference /= 4; // One tick for every 4 pulses
difference = constrain(difference, -15, 15); // Make sure that only 15 ticks are sent at once
if (difference != 0) 
   { // If the encoder was moved
         uint8_t CC_value = mapRelativeCC(difference); // Change the representation of negative numbers
         sendMIDI(CC, channel, controller, CC_value); // Send the relative position change over MIDI
         previousPosition += difference * 4; // Add the pulses sent over MIDI to the previous position
         Serial.print(CC_value);
}

      // End loop
}



uint8_t twosComplementTo7bitSignedMagnitude(int8_t value) {  // Convert an 8-bit two's complement integer to 7-bit sign-magnitude format
  uint8_t mask = value >> 7;
  uint8_t abs = (value + mask) ^ mask;
  uint8_t sign = mask & 0b01000000;
  return (abs & 0b00111111) | sign;
}

uint8_t mapRelativeCC(int8_t value) {  // Convert an 8-bit two's complement integer to a 7-bit value to send over MIDI
  switch (negativeRepresentation) {
    case TWOS_COMPLEMENT:
      return value;  // Remember that the sendMIDI function does the bit masking, so you don't have to worry about bit 7 being 1.
    case BINARY_OFFSET:
      return value + 64;
    case SIGN_MAGNITUDE:
      return twosComplementTo7bitSignedMagnitude(value);
  }
}


void noteOn(int cmd, int midiNote, int midiVelocity, int col, String row) {
        char buf1 [64]; // must be large enough for the whole string
        char buf2 [64]; 

    Serial.write(cmd); // send note on, or note off
    Serial.write(midiNote);    // send pitch data
    Serial.write(midiVelocity);  // send velocity data
       // sprintf (buf1, "The col is: %d  and the row is ", col);
       
       
       // Serial.print(col);
       // Serial.print (row);
       // Serial.print("\r\n");
       // if(col == 1 && row == "row3" )
      //  {
        
          //  midiNote = 29;
          //  Serial.write(midiNote);
      //  }
                              

        
}

void sendMIDI(uint8_t messageType, uint8_t channel, uint8_t data1, uint8_t data2) {
channel--; // Decrement the channel, because MIDI channel 1 corresponds to binary channel 0
uint8_t statusByte = messageType | channel; // Combine the messageType (high nibble)
// with the channel (low nibble)
// Both the message type and the channel should be 4 bits wide
Serial.write(statusByte);
Serial.write(data1);
Serial.write(data2);
}


void sendMIDI(uint8_t statusByte, uint8_t dataByte1, uint8_t dataByte2) {
Serial.write(statusByte);
Serial.write(dataByte1);
Serial.write(dataByte2);
}
