/*Writer  : Redieta FZ
Developed : March, 16th 2020
*/

//=======================================================Counting Machine========================================================
#include <avr/wdt.h>              // Add library WDT
#include <SoftwareSerial.h>       // initiation of software serial 
SoftwareSerial SerialOne (11,12); // software serial pinout for RX, TX

String inputString = "";          // string for hold upcoming data
bool stringComplete = false;      // initiation of string complete requirement
bool sensor_read = false;         // status initiation of activation sensor reading (true/false)

uint32_t count = 0;               // counting start from 0
uint32_t limit = 60000;           // setup limit
int sensor = 2;                   // pinout initiation of sensor E18
int SensorNow;                    // initiation of current sensor reading

// pinout initiation of dipswitch and index
int dipswitch[] = {6,7,8,9};int i;                          

// initiation pin communication & counting LED
int LED_communication = 4;const byte LED_counting = 5;


//===============================================Sensor, LED, and Counting Reading====================================================
void active(){
  if(SensorNow == LOW){                         // if current sensor NOW
    count = count+1;                            // counting fomula
    digitalWrite(LED_counting, LOW);            // counting LED is on (although the logic is LOW) 
    sensor_read = true;                         // activate sensor reading
  }

  if(SensorNow == HIGH){                        // if current sensor HIGH
    digitalWrite(LED_counting, HIGH);           // counting LED is off
  }
}


//======================================================Slave-Master Serial Communication==============================================
void execution_1(){                              
  if(stringComplete){                         // if string complete true
    SerialOne.println(inputString);           // slave serial monitor printed the string that was entered
    digitalWrite(LED_communication, HIGH);    // communication LED is ON

    if(inputString == "S_1\n"){               // if the input string entered is sensor 1
      Serial.print("A");                      // master serial monitor printed letter A
      Serial.print(count);                    // master serial monitor printed result of counting
      Serial.print("\n");                     // master serial monitor printed line changes
      digitalWrite(LED_communication, LOW);   // communication LED is off
    }
   inputString = "";                          // flexible input string initiation
   stringComplete = false;                    // string complete finished
  }
}

void execution_2(){                                 
  if(stringComplete){                         // if string complete true
    SerialOne.println(inputString);           // slave serial monitor printed the string that was entered
    digitalWrite(LED_communication, HIGH);    // communication LED is ON

    if(inputString == "S_2\n"){               // if the input string entered is sensor 2
      Serial.print("B");                      // master serial monitor printed letter B
      Serial.print(count);                    // master serial monitor printed result of counting
      Serial.print("\n");                     // master serial monitor printed line changes
      digitalWrite(LED_communication, LOW);   // communication LED is off
    }
   inputString = "";                          // flexible input string initiation
   stringComplete = false;                    // string complete finished  
  }
}

//===========================================================VOID SETUP==========================================================
void setup() {
  Serial.begin(9600);                         // baudrate declaration of master serial monitor
  SerialOne.begin(9600);                      // baudrate declaration of slave serial monitor
  inputString.reserve(200);                   // declaration of datas that is served
  
  pinMode(sensor, INPUT_PULLUP);              // pin mode declaration of sensor 1 which is INPUT PULLUP
  pinMode(LED_counting, OUTPUT);              // pin mode declaration of counting LED which OUTPUT
  pinMode(LED_communication, OUTPUT);         // pin mode declaration of communication LED which is OUTPUT

  pinMode(11, INPUT_PULLUP);                  // pin mode declaration of dipswitch pinout 11 whish is INPUT PULLUP
  pinMode(10, INPUT_PULLUP);                  // pin mode declaration of dipswitch pinout 10 whish is INPUT PULLUP
  pinMode(9, INPUT_PULLUP);                   // pin mode declaration of dipswitch pinout 9 whish is INPUT PULLUP
  pinMode(8, INPUT_PULLUP);                   // pin mode declaration of dipswitch pinout 8 whish is INPUT PULLUP

  attachInterrupt(digitalPinToInterrupt(sensor), active, CHANGE); // interrupt sensor for turning off LED

  /* actived WDT */
  wdt_enable(WDTO_2S);
}

//=============================================================VOID LOOP=========================================================
void loop() {
  SensorNow = digitalRead(sensor);            // current sensor reads value of sensor 1
  switch(address()){
    case B0000:                               // biner dipswitch 0
      sensor_read = false;                    // sensor reading deactivated
      count = 0;                              // counting default 0 (in serial monitor printed 1)
      digitalWrite(LED_counting, LOW);        // counting LED is off
    break;             // break
    case B0001:                               // biner dipswitch 1
      execution_1();                          // into void execution_1()
    break;             // break
    case B0010:                               // biner dipswitch 2
      execution_2();                          // into void execution_2()
    break;             // break
    }
  
    if(sensor_read == true){                  // if sensor already activated
        SerialOne.println(count);             // slave serial monitor printed counting result
        sensor_read = false;                  // sensor deactivated
    }

    if(count >= limit){                       // if counting reached the limit
        count=0;                              // reset from 1
    }

    // use to reset program 
    wdt_reset();
}

//=====================================================Function Addressing Dipswitch===============================================
byte address(){
  int j=0;                                    // initiation of variable i and j = 0
  for(i=0; i<=3; i++){                        // for 0<i<=3
    j = (j << 1) | digitalRead(dipswitch[i]); // variable j shifting to the left ke kiri about 1 bit
  }  
  return j;                                   // back to j
}

//==========================================Serial Communication of Reading Master Message=========================================
void serialEvent() {
  while (Serial.available()) {                // take the new byte
    char inChar = (char)Serial.read();        // add the new byte to the input string
    inputString += inChar;                    // if there is a new character in a new line, set it. so, main loop can process something
    if (inChar == '\n') {                     // if inChar reads \n
      stringComplete = true;                  // string complete started
    }
  }
}
 