/*
Copyright (c) 2016, Embedded Adventures
All rights reserved.
Contact us at source [at] embeddedadventures.com
www.embeddedadventures.com
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
- Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.
- Neither the name of Embedded Adventures nor the names of its contributors
  may be used to endorse or promote products derived from this software
  without specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
THE POSSIBILITY OF SUCH DAMAGE.
*/
/*
 * IoT Sensor 1
 * Using: MOD1023, ESP12
 * I2C Protocol - GPIO5 = SDA, GPIO4 = SCK
 * Sends measurement data to datapond on stream 60971-60975, with power event 60981
 * Updates every 11 seconds
 * Embedded Adventures (embeddedadventures.com)
 */

#include <FiniteStateMachine.h>
#include <ESP8266WiFi.h>
#include <coap-packet.h>
#include <coap-protocol.h>
#include <coap-datapond.h>
#include <Wire.h> 
#include <iAQ-MOD1023.h>
#include <BME280_MOD-1022.h>

#define CONNECT_WIFI  1
#define LOGIN         2
#define CREATE_DATA   3

const char* ssid = "ssid";
const char* password = "password";

State Login = State(login);
State Connect = State(connectWifi);
State Datapond = State(datapondTransmission);

FSM IoTSensor = FSM(Connect);   //Init FSM, start in connect
CoapDatapond datapond("api.datapond.io", 1000, 5683); //IPAddress, localPort, serverPort


float measurement[5]; //Temperature, humidity, pressure, tvoc, prediction
int streams[6] = {60971, 60972, 60973, 60974, 60975, 60981}; 
int reboot; //-1, 0, 1 == waiting for response, no need to send reboot, need to send reboot
int loginState, nextState, secondCounter, nextStream;
static int loginAttempts = 0; //Max = 3
bool updateData = false;
long current, past;

 
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Welcome to the MOD1023 and CoAP Datapond demo program.");
  
  Wire.begin(); 
  delay(2);    

  datapond.begin("username", "password", 0x12);
  datapond.setLoginHandler(loggedIn);
  datapond.setHandlers(createDropletCallback, readDropletCallback, createStreamCallback, readStreamCallback);

  uns8 chipID = BME280.readChipId();
  Serial.print("BME280 Chip ID: ");
  Serial.println(chipID, HEX);

  performMeasurements();
  printMeasurement();
  reboot = 1;
  
  past = millis();
  secondCounter = 0;
}

void loop() {
  current = millis();
  if ((current - past) > 1000) {
    secondCounter++;
    past = current;
  }
  
  if (secondCounter == 11) {
    performMeasurements();
    updateData = true;
    secondCounter = 0;
  }
  else
    updateData = false;
    
  datapond.run();
  switch(nextState) {
    case CONNECT_WIFI: IoTSensor.transitionTo(Connect); break;
    case LOGIN: IoTSensor.transitionTo(Login); break;
    case CREATE_DATA: IoTSensor.transitionTo(Datapond); break;
  }
  IoTSensor.update();
}

void login() {
  //If we've tried to login 3x already, change to CONNECT_WIFI
  if (loginAttempts == 3) {
    nextState = CONNECT_WIFI;
    Serial.println("3 logins attempted and failed. Reconnect to WiFi");
    loginAttempts = 0;
  }
  //Login State = 0 means we need to send a login packet
  if (loginState == 0) {
    datapond.login();
    TEST1("Sending login");
    loginState = -1;
    loginAttempts++;
  }
  //Login successful. Next state = reboot stream
  else if (loginState == 1) {
    TEST1("login successful");
    loginAttempts = 0;
    nextState = CREATE_DATA;
  }
}

void connectWifi() {
  WiFi.begin(ssid, password);
  Serial.println("");
  // Wait for connection
  int timeout = 0;
  while ((WiFi.status() != WL_CONNECTED) && (timeout < 10)) {
    delay(500);
    Serial.print(".");
    timeout++;
  }
  //Timed out. Try again
  if (timeout == 10) {
    Serial.println("WiFi connection took too long...");
    nextState = CONNECT_WIFI;
  }
  //WiFi connected. Now go to log in
  else {
    Serial.print("\nConnected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    loginState = 0;
    nextState = LOGIN; //Go to login
  }
  Serial.println(WiFi.localIP()); 
}

void datapondTransmission() {
  //Reboot = 1 -> need to send reboot event droplet
  if (reboot == 1) {
    if (datapond.createDroplet(streams[5], "reboot event") == -1) {
      nextState = CREATE_DATA;
      reboot = 1;  
    }
    else {
      reboot = -1;
    }
  }
  //Reboot = 0 -> reboot droplet successful. Go to data
  else if (reboot == 0) {
    if (!updateData) 
      return;
    else {
      if (datapond.createDroplet(streams[nextStream], measurement[nextStream]) == 1) {
        TEST1("create Droplet added to txQueue");
        nextStream++;
        if (nextStream == 5)
          nextStream = 0;
      }
      nextState = CREATE_DATA;
    }
  }
}

void performMeasurements() {
  BME280.readCompensationParams();
  BME280.writeOversamplingPressure(os1x);  // 1x over sampling (ie, just one sample)
  BME280.writeOversamplingTemperature(os1x);
  BME280.writeOversamplingHumidity(os1x);
  BME280.writeMode(smForced);
  while (BME280.isMeasuring()) {  }
  BME280.readMeasurements();

  measurement[0] = BME280.getTemperature();
  measurement[1] = BME280.getHumidity();
  measurement[2] = BME280.getPressure();
  iaq.readRegisters();
  if (iaq.getStatusByte() == 0x00) {
    measurement[3] = iaq.getPrediction();
    measurement[4] = iaq.getTVOC();
  }
  else {  
    Serial.print("iAQ status - ");
    Serial.println(iaq.getStatusByte(), HEX);
    Serial.println("Ignoring the TVOC and CO2 prediction values");
  }
  printMeasurement();
}

void printMeasurement() {
  Serial.print("Temperature:\t");
  Serial.println(measurement[0]);
  Serial.print("Humidity:\t");
  Serial.println(measurement[1]);
  Serial.print("Pressure:\t");
  Serial.println(measurement[2]);
  Serial.print("TVOC:\t");
  Serial.println(iaq.getPrediction());
  Serial.print("CO2:\t");
  Serial.println(iaq.getTVOC());
}

/////////////////////////////////////
//        Callback Functions      ///
/////////////////////////////////////

void loggedIn(bool login) {
  if (!login) 
    loginState = 0;
  else
    loginState = 1;
}

void createDropletCallback(uns8 tkn, bool success) {
  if ((reboot == -1) && success) {
    reboot = 0;
    nextState = CREATE_DATA;
    nextStream = 0;
  }
  else if ((reboot == -1) && !success) {
    reboot = 1;
  }
  if (!success) {
    Serial.println(datapond.getPayload());
    Serial.println(datapond.getCookie());
  }
}

void readDropletCallback(uns8 tkn, bool success, String data) { }

void createStreamCallback(uns8 tkn, bool success) { }

void readStreamCallback(uns8 tkn, bool success, String data) {  }




