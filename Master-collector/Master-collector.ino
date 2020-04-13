/*
Author  : dhikihandika
Date    : 18/03/2020
*/

//==========================================================================================================================================//
//===================================================|   Initialization Program    |========================================================//                                         
//==========================================================================================================================================//
#include <RTClib.h>                                                 // Add library RTC
#include <Wire.h>                                                   // Add Library Wire RTC communication
#include <Ethernet.h>                                               // Add library ethernet
#include <SPI.h>                                                    // Add library protocol communication SPI
#include <ArduinoJson.h>                                            // Add library arduino json 
#include <PubSubClient.h>                                           // Add library PubSubClient MQTT

#define DEBUG

#define limitData 60000                             // limit countData
#define timer1 5000                                 // timer send command to sensor module 1
#define timer2 10000                                // timer send command to sensor module 2

#define COM1 32                                     // define LED communication slave1
#define COM2 33                                     // define LED communication slave2 


RTC_DS1307 RTC;                                     // Define type RTC as RTC_DS1307 (must be suitable with hardware RTC will be used)

/* configur etheret communication */
byte mac[]  = {0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };                // MAC Address by see sticker on Arduino Etherent Shield or self determine
IPAddress ip(192, 168, 0, 120);                                     // IP ethernet shield assigned, in one class over the server
IPAddress server(192, 168, 0, 108);                                 // IP LAN (Set ststic IP in PC/Server)
// IPAddress ip(192, 168, 50, 8);                                     // IP ethernet shield assigned, in one class over the server
// IPAddress server(192, 168, 50, 7);                                 // IP LAN (Set ststic IP in PC/Server)
int portServer = 1883;                                              // Determine portServer MQTT connection

EthernetClient ethClient;                                           // Instance of EthernetClient is a etcClient
PubSubClient client(ethClient);                                     // Instance of client ia a client

/* variable timer millis */
unsigned long currentMillis = 0;  
unsigned long currentMillis_errorData = 0;                   
unsigned long previousMillis = 0;

/* global variable to save date and time from RTC */
int year, month, day, hour, minute, second; 
String stringyear, stringmonth, stringday, stringhour, stringminute, stringsecond;

/* variable incoming data (current data) */
uint32_t data_S1 = 0;
uint32_t data_S2 = 0;

/* variable data publish */
uint32_t countData_S1 = 0;
uint32_t countData_S2 = 0;

/* variable status sensor */
int status_S1 = 0;
int status_S2 = 0;

/* varibale indexOf data */
int first = 0;
int last = 0;

/* varibale check status data */
int errorCheck_S1 = 0;
int errorCheck_S2 = 0;

String incomingData = "";                           // a String to hold incoming data
bool stringComplete = false;                        // whether the string is complete

/* varible check boolean prefix of data */
bool prefix_A = false;
bool prefix_B = false;

/* variable check boolean to identify subscribe */
bool timesubscribe = false;


//==========================================================================================================================================//
//=========================================================|   Procedure reconnect    |=====================================================//                                         
//==========================================================================================================================================//
void reconnect(){
  while(!client.connected()){
    #ifdef DEBUG
    Serial.print("Attemping MQTT connection...");
    #endif
    if(client.connect("ethernetClient")){
      Serial.println("connected");
      // Publish variable startup system
      const size_t restart = JSON_OBJECT_SIZE(2);
      DynamicJsonBuffer jsonBuffer(restart);
      JsonObject& root = jsonBuffer.createObject();
      
      root["id_controller"] = "CTR01";
      root["flagstart"] = 1;

      char buffermessage[300];
      root.printTo(buffermessage, sizeof(buffermessage));

      #ifdef DEBUG
      Serial.println("Sending message to MQTT topic...");
      Serial.println(buffermessage);
      #endif

      client.publish("PSI/countingbenang/datacollector/startcontroller", buffermessage);
      if (client.publish("PSI/countingbenang/datacollector/startcontroller", buffermessage) == true){
        #ifdef DEBUG
        Serial.println("Succes sending message");
        Serial.println("--------------------------------------------");
        Serial.println("");
        #endif
      } else {
      #ifdef DEBUG
      Serial.println("ERROR PUBLISHING");
      Serial.println("--------------------------------------------");
      Serial.println("");
      #endif
      }
    }
    else{
      #ifdef DEBUG
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("try again 5 second");
      #endif
      delay(2000);
    }
  }
}


// //==========================================================================================================================================//
// //============================================|   Procedure publish data to app server  |===================================================//                                         
// //==========================================================================================================================================//
/* publish data sensor 1 */
void publishData_S1(){
  #ifdef DEBUG
  Serial.print("Publish data S1= ");
  Serial.println(data_S1);                                                              // line debugging
  #endif

  RTCprint();                                                                           // Call procedure sync time RTC

/* ArduinoJson create jsonDoc 
Must be know its have a different function 
if you use library ArduinoJson ver 5.x.x or 6.x.x
-- in this program using library ArduinoJson ver 5.x.x
*/
const size_t BUFFER_SIZE = JSON_OBJECT_SIZE(7);                                         // define number of key-value pairs in the object pointed by the JsonObject.

 DynamicJsonBuffer jsonBuffer(BUFFER_SIZE);                                             // memory management jsonBuffer which is allocated on the heap and grows automatically (dynamic memory)
  JsonObject& JSONencoder = jsonBuffer.createObject();                                  // createObject function jsonBuffer

  /* Encode object in jsonBuffer */
  JSONencoder["id_controller"] = "CTR01";                                               // key/object its = id_controller
  JSONencoder["id_machine"] = "id_machine1";                                            // key/object its = id_machine
  JSONencoder["clock"] = stringyear +"-"+stringmonth+"-"+stringday+" "+stringhour+":"+stringminute+":"+stringsecond;
  JSONencoder["count"] = countData_S1;                                                  // key/object its = count
  JSONencoder["status"] = status_S1;                                                    // key/object its = status
  JSONencoder["temp_data"] = data_S1;                                                   // key/object its = temp_data
  JSONencoder["flagsensor"] = 1;                                                        // key/object its = limit

  /* when use dummy data publish */
  // JSONencoder["id_controller"] = "CTR01";                                            // key/object its = id_controller
  // JSONencoder["id_machine"] = "MAC01_01";                                            // key/object its = id_machine
  // JSONencoder["date"] = "2020-02-02";
  // JSONencoder["clock"] = "02:02";
  // JSONencoder["time"] = "2020-02-02 02:02";
  // JSONencoder["count"] = 12;                                                         // key/object its = count
  // JSONencoder["length"] = 602.88;                                                    // key/object its = length for debbuging
  // JSONencoder["status"] = 0;                                                         // key/object its = status
  // JSONencoder["flagsensor"] = 1;                                                     // key/object its = limit

  char JSONmessageBuffer[500];                                                          // array of char JSONmessageBuffer is 500
  JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));                    // “minified” a JSON document

  #ifdef DEBUG
  Serial.println("Sending message to MQTT topic...");                                   // line debugging
  Serial.println(JSONmessageBuffer);                                                    // line debugging
  #endif

  client.publish("PSI/countingbenang/datacollector/reportdata", JSONmessageBuffer);     // publish payload to broker <=> client.publish(topic, payload);

  /* error correction */
  if(client.publish("PSI/countingbenang/datacollector/reportdata", JSONmessageBuffer) == true){
    #ifdef DEBUG
    Serial.println("SUCCESS PUBLISHING PAYLOAD");
    #endif
  } else {
    #ifdef DEBUG
    Serial.println("ERROR PUBLISHING");
    #endif
  }
}


/* publish data sensor 2 */
void publishData_S2(){
  #ifdef DEBUG
  Serial.print("Publish data S2= ");
  Serial.println(data_S2);                                                              // line debugging
  #endif

  RTCprint();                                                                           // Call procedure sync time RTC

/* ArduinoJson create jsonDoc 
Must be know its have a different function 
if you use library ArduinoJson ver 5.x.x or 6.x.x
-- in this program using library ArduinoJson ver 5.x.x
*/
const size_t BUFFER_SIZE = JSON_OBJECT_SIZE(7);                                         // define number of key-value pairs in the object pointed by the JsonObject.

 DynamicJsonBuffer jsonBuffer(BUFFER_SIZE);                                             // memory management jsonBuffer which is allocated on the heap and grows automatically (dynamic memory)
  JsonObject& JSONencoder = jsonBuffer.createObject();                                  // createObject function jsonBuffer

  /* Encode object in jsonBuffer */
  JSONencoder["id_controller"] = "CTR01";                                               // key/object its = id_controller
  JSONencoder["id_machine"] = "id_machine1";                                              // key/object its = id_machine
  JSONencoder["clock"] = stringyear +"-"+stringmonth+"-"+stringday+" "+stringhour+":"+stringminute+":"+stringsecond;
  JSONencoder["count"] = countData_S2;                                                       // key/object its = count
  JSONencoder["status"] = status_S2;                                                    // key/object its = status
  JSONencoder["temp_data"] = data_S2;                                                       // key/object its = temp_data
  JSONencoder["flagsensor"] = 1;                                                        // key/object its = limit

  char JSONmessageBuffer[500];                                                          // array of char JSONmessageBuffer is 500
  JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));                    // “minified” a JSON document

  #ifdef DEBUG
  Serial.println("Sending message to MQTT topic...");                                   // line debugging
  Serial.println(JSONmessageBuffer);                                                    // line debugging
  #endif

  client.publish("PSI/countingbenang/datacollector/reportdata", JSONmessageBuffer);     // publish payload to broker <=> client.publish(topic, payload);

  /* error correction */
  if(client.publish("PSI/countingbenang/datacollector/reportdata", JSONmessageBuffer) == true){
    #ifdef DEBUG
    Serial.println("SUCCESS PUBLISHING PAYLOAD");
    #endif
  } else {
    #ifdef DEBUG
    Serial.println("ERROR PUBLISHING");
    #endif
  }
}


//==========================================================================================================================================//
//==========================================================|   Procedure callback    |=====================================================//                                         
//==========================================================================================================================================//
char data[80];
StaticJsonBuffer<200> jsonBuffer;
void callback(char* topic, byte* payload, unsigned int length) {
  #ifdef DEBUG
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  #endif

  char inData[120];
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    inData[(i - 0)] = (const char*)payload[i];
  }

  #ifdef DEBUG
  Serial.println();
  Serial.println("-----------------------");
  #endif
  // Parse object JSON from subscribe data timestamp
  JsonObject& root = jsonBuffer.parseObject(inData); 
  String time = root["current_time"];
  int statusTime = root["flagtime"];

  #ifdef DEBUG
  Serial.println(time);
  Serial.println(statusTime);
  #endif

  if(statusTime == 1){
    timesubscribe = true;
    status_S1 = 0;
    status_S2 = 0;
  }
  
  // Parse timestamp value
  year = time.substring(1,5).toInt();
  month = time.substring(6,8).toInt();
  day = time.substring(9,11).toInt();
  hour = time.substring(12,14).toInt();
  minute = time.substring(15,17).toInt();
}


//==========================================================================================================================================//
//=========================================================|   Send Command    |============================================================//                                         
//==========================================================================================================================================//
void sendCommand(){
    currentMillis = millis();
    if(currentMillis - previousMillis == timer1){
        #ifdef DEBUG
        Serial.println("");
        Serial.println("---------------------------------------------------------------");  
        Serial.println("Send command to sensor module 1 (TX3)");
        Serial.println("");
        #endif // DEBUG
        Serial3.print("S_1\n");
    } else {
        if(currentMillis - previousMillis == timer2){
           previousMillis = currentMillis;  
            #ifdef DEBUG
            Serial.println("");
            Serial.println("---------------------------------------------------------------");  
            Serial.println("Send command to sensor module 2 (TX3)");
            Serial.println("");
            #endif // DEBUG
            Serial3.print("S_2\n");
        } 
    }
}

//==========================================================================================================================================//
//======================================================|   Procedure to showDaota    |=====================================================//                                         
//==========================================================================================================================================//
void showData(){
  /* variable diff data */
  int diffData_S1 = 0;
  int diffData_S2 = 0;

  /* variable last incoming data */
  uint32_t lastData_S1 = 0;
  uint32_t lastData_S2 = 0;

  /* Show data for sensor 1 */
  if(prefix_A){
    if(stringComplete){
      #ifdef DEBUG
      Serial.println("Prefix_A --OK--");
      Serial.print("incomming data= ");Serial.print(incomingData);
      #endif

      digitalWrite(COM1, HIGH);
      status_S1 = 0;
      /* remove header and footer */
      first = incomingData.indexOf('A');                                         // determine indexOf 'A'
      last = incomingData.lastIndexOf('/n');                                     // determine lastInndexOf '\n

      /* Parse incoming data to particular variable */ 
      String datasensor1 = incomingData.substring(first, last);                  // substring 
      datasensor1.remove(0,1);                                                   // remove header incomming data
      datasensor1.remove(datasensor1.length()-1, datasensor1.length() - 0);      // remove fotter incomming data (/n)
      data_S1 = datasensor1.toInt();                                             // covert string to integer datasensor1 and save to 'data_S1'

      stringComplete = false;
      prefix_A = false;
      incomingData = "";

      //Processing Data
      lastData_S1 = data_S1 - 1;
      diffData_S1 = data_S1 - lastData_S1;
      if(diffData_S1<0){
        countData_S1 = diffData_S1 + limitData; 
      } else {
        countData_S1 = diffData_S1;
      }

      publishData_S1();

      #ifdef DEBUG
      Serial.print("data S1= ");Serial.print(data_S1); 
      Serial.print(" | status S1= ");Serial.println(status_S1); 
      Serial.println("------------------------------||-------------------------------\n");                                              
      #endif //DEBUG
      digitalWrite(COM1, LOW);
      } 
  } else {
    /* Show data for sensor 2 */
    if(prefix_B){
      if(stringComplete){
      #ifdef DEBUG
      Serial.println("Prefix_B --OK--");
      Serial.print("incomming data= ");Serial.print(incomingData);
      #endif

      digitalWrite(COM2, HIGH);
      status_S2 = 0;
      first = incomingData.indexOf('B');                                         // determine indexOf 'A'
      last = incomingData.lastIndexOf('/n');                                     // determine lastInndexOf '\n
      /* When true value is 0 and false is "-1" */

      /* Parse incoming data to particular variable */ 
      String datasensor2 = incomingData.substring(first, last);                  // substring 
      datasensor2.remove(0,1);                                                   // remove header incomming data
      datasensor2.remove(datasensor2.length()-1, datasensor2.length() - 0);      // remove fotter incomming data (/n)
      data_S2 = datasensor2.toInt();                                             // covert string to integer datasensor1 and save to 'data_S1'

      stringComplete = false;
      prefix_B = false;
      incomingData = "";

      // Processing Data
      lastData_S2 = data_S2 - 1;
      diffData_S2 = data_S2 - lastData_S2;
      if(diffData_S2<0){
        countData_S2 = diffData_S2 + limitData; 
      } else {
        countData_S2 = diffData_S2;
      }

      publishData_S2();

      #ifdef DEBUG
      Serial.print("data_S2= ");Serial.print(data_S2); 
      Serial.print(" | status S2= ");Serial.println(status_S2); 
      Serial.println("------------------------------||-------------------------------\n");                                                  
      #endif //DEBUG
      digitalWrite(COM2, LOW);
      } 
    } 
  }
}


//==========================================================================================================================================//
//==================================================|     Procedure error data        |=====================================================//                                         
//==========================================================================================================================================//
void errorData(){
  if((millis() - currentMillis_errorData)>=5000){
    currentMillis_errorData = millis();
    errorCheck_S1++;
    errorCheck_S2++;
  }
  if(errorCheck_S1 == 3){
    status_S1 = 1;
    errorCheck_S1 = 0;
    Serial.println("=========================");
    Serial.println("        ERROR !!!        ");
    Serial.print("status S1= ");Serial.println(status_S1); 
    publishData_S1();
    Serial.println("=========================");
    Serial.println(" ");
  }
  if(errorCheck_S2 == 3){
    status_S2 = 1;
    errorCheck_S2 = 0;
    Serial.println("=========================");
    Serial.println("        ERROR !!!        ");
    Serial.print("status S2= ");Serial.println(status_S2); 
    publishData_S2();
    Serial.println("=========================");
    Serial.println(" ");
  }
}

//==========================================================================================================================================//
//================================================|    Procedure lcd Get RealTimeClock   |==================================================//                                         
//==========================================================================================================================================//
void RTCprint(){
  DateTime now = RTC.now();

  #ifdef DEBUG
  Serial.print("date : ");
  Serial.print(now.day(), DEC);
  Serial.print("/");
  Serial.print(now.month(), DEC);
  Serial.print("/");
  Serial.println(now.year(), DEC);
  Serial.print("clock : ");
  Serial.print(now.hour(), DEC);
  Serial.print(":");
  Serial.print(now.minute(), DEC);
  Serial.print(":");
  Serial.print(now.second(), DEC);
  Serial.println(" WIB");
  #endif

  year = now.year(), DEC;
  month = now.month(), DEC;
  day = now.day(), DEC;
  hour = now.hour(), DEC;
  minute = now.minute(), DEC;
  second = now.second(), DEC;
  
  stringyear= String(year);
  stringmonth= String(month);
  stringday= String(day);
  stringhour= String(hour);
  stringminute= String(minute);
  stringsecond= String(second);
}

//==========================================================================================================================================//
//=============================================|   Procedure Sync data and time RTC with Server   |=========================================//                                         
//==========================================================================================================================================//
void syncDataTimeRTC(){
  if(timesubscribe == true){
    RTC.adjust(DateTime(year, month, day, hour, minute));
    timesubscribe = false;
  }
}


//==========================================================================================================================================//
//=========================================================|   Setup Program    |===========================================================//                                         
//==========================================================================================================================================//
void setup(){
    /* Configuration baud rate serial */
    Serial.begin(9600);
    Serial3.begin(9600);

    /* Mode pin definition */
    pinMode(COM1, OUTPUT);
    pinMode(COM2, OUTPUT);
    
    /* Callibration RTC module with NTP Server */
    Wire.begin();
    RTC.begin();
    RTC.adjust(DateTime(__DATE__, __TIME__));       //Adjust data and time from PC every startup
    // RTC.adjust(DateTime(2019, 8, 21, timeClient.getHours(), timeClient.getMinutes(), timeClient.getSeconds()));

    /* Setup Ethernet connection */
    Ethernet.begin(mac, ip);
    /* Setup Broker (server) MQTT Connection */
    client.setServer(server, portServer);
    client.setCallback(callback);
    reconnect();
    
    client.subscribe("PSI/countingbenang/server/infotimestamp");    //topic get data timestamp from server
    client.subscribe("PSI/countingbenang/tablet/setreset");    //topic get reset from server
    /* reserve 200 bytes for the incomingData*/
    incomingData.reserve(200);
}


//==========================================================================================================================================//
//===========================================================|   Main Loop    |=============================================================//                                         
//==========================================================================================================================================//
void loop(){
    syncDataTimeRTC();
    sendCommand();
    showData();
    errorData();
    client.loop();   // Use to loop callback function
}



//==========================================================================================================================================//
//==========================================================|   Serial ISR    |=============================================================//                                         
//==========================================================================================================================================//
/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
void serialEvent3(){
  while (Serial3.available()){
    // get the new byte:
    char inChar = (char)Serial3.read();
    // add it to the incomingData
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    incomingData += inChar;
    if(inChar == 'A'){
      prefix_A = true;
      errorCheck_S1 = 0;
    } else {
      if(inChar == 'B'){
      prefix_B = true;
      errorCheck_S2 = 0;
      } else {
        if(inChar == '\n'){ 
          stringComplete = true;
        }
      }
    }
  }
}
