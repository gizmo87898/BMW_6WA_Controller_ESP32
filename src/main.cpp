
#include <CAN.h>
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
/*
 * esp32 & sn65hvd230 cluster controller by gizmo87898 on github
 */
#define lo8(x) ((int)(x)&0xff)
#define hi8(x) ((int)(x)>>8)

//CAN loop coutners
unsigned long sinceLast100msLoop = 0;
unsigned long sinceLast200msLoop = 0;
unsigned long sinceLast1000msLoop = 0;
unsigned long sinceLast5sLoop = 0;

//testing shit
int randomId = 0x320;
int count = 0; // for ignition, abs, etc
int testcount = 0;


//cluster variables
int rpm = 1000; // 0-7500 rpm
float speed = 0; // m/s
int coolant_temp = 120; // C
int oil_temp = 120;
int drive_mode = 0;  // 1 = traction, 2 = comfort+, 4 = sport, 5 = sport+, 6 = dsc off, 7 = eco pro
int fuel = 50; // 0-100
int gear = 0;
// 0 = N
// 1 = 1
// ...
// 6 = 6
// 7 = P
// 8 = R
// 9 = D
char lights[16] = {0,0,0,0,0,0,0,0,0,0,0}; 
// Lights Bitfield:
// 0 ABS
// 1 LeftDir
// 2 RightDir
// 3 TPMS
// 4 Brake
// 5 TC
// 6 TC_Off
// 7 FogLights
// 8 AutoStartStop
// 9 ParkingLights
// 10 Headlights
// 11 HighBeam
// 12 CEL
// 13 Auto Highbeam
//time and date
int hour = 12;
int minute = 34;
int seconds = 56;
int day = 23;
int month = 1;
int year = 2027;

void setup() {
  // Begin serial at 115200bps
  Serial.begin(115200);
  // Set CAN RX/TX pins to 2 and 15
  CAN.setPins(25,26);
  while (!Serial);
  // start the CAN bus at 125 kbps
  if (!CAN.begin(500E3)) {
    Serial.println("Starting CAN failed!");
    while (1);
  }
  CAN.loopback();
  Serial.println("Initialized in Loopback");
}




void sendc0() { // JBBE alive counter
  CAN.beginPacket(0xc0);
  CAN.write(count | 0xF0);
  CAN.write(0xFF);
  CAN.endPacket();
}


void sendd7() { // Airbag Light
  CAN.beginPacket(0xd7);
  CAN.write(random(255));
  CAN.write(random(255));
  CAN.endPacket();
}


void sendf3() { // RPM
  CAN.beginPacket(0xf3);
  CAN.write(0);
  CAN.write(int(rpm * 1.557) & 0xff); // RPM Lowbyte
  CAN.write(int(rpm * 1.557) >> 8); // RPM Highbyte
  CAN.write(0);
  CAN.write(random(255));
  CAN.write(random(255));
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.endPacket();

}



void send12f() { // Ignition Status
  CAN.beginPacket(0x12f);
  CAN.write(0xfb);
  CAN.write(count);
  CAN.write(0x8a);
  CAN.write(0x1c);
  CAN.write(count);
  CAN.write(0x05);
  CAN.write(0x30);
  CAN.write(0x00==86);
  CAN.endPacket();
  count++;
  if (count == 255) {
    count = 0;
  }
}


void send1f6() { // directionals
  CAN.beginPacket(0x1f6);
  CAN.write(0xb1);
  CAN.write(0xf1);
  CAN.endPacket();
}




void send1a1() { // speed
  int speedval = speed*230;
  CAN.beginPacket(0x1a1);
  CAN.write(random(255));
  CAN.write(random(255));
  CAN.write(lo8(speedval));
  CAN.write(hi8(speedval));
  CAN.write(0xAA);
  CAN.endPacket();
}


void send1d2() { // Gear, doesnt work
  CAN.beginPacket(0x1d2);
  CAN.write(0x78);
  CAN.write(0x0c);
  CAN.write(0x8b);
  CAN.write(count);
  CAN.write(0xF0);
  CAN.write(0xFF);
  CAN.endPacket();
}


void send21a() { // Lighting
  int lights_buf = 0;
  
  if (lights[9]) {
    lights_buf += 4; // parking lights
  }
  
  if(lights[11]) {
    lights_buf += 2; // highbeam
  }

  if(lights[7]) {
    lights_buf += 32; // foglights
  }

  CAN.beginPacket(0x21a); 
  CAN.write(lights_buf); // ?,rear_foglight,foglight,?,?,parkinglights,highbeam,?
  CAN.write(0x00);
  CAN.write(0xF7);
  CAN.endPacket();
}


void send291() { // MIL, Units
  CAN.beginPacket(0x291);
  CAN.write(3); //language 2 = english 4 = spanish
  CAN.write(0x04); // 0x12 = celcius 0x04 = farenheight
  CAN.write(0x18); //0x04 = mpg 0-50, 0x01 = l/km, 0x08 = mpg 0-60, 0x00 = km, 0x10 = mi
  CAN.write(0);
  CAN.write(0);
  CAN.write(0);
  CAN.write(0);
  CAN.write(0x04);
  CAN.endPacket();
}


void send2a7() { // power steering
  CAN.beginPacket(0x2a7);
  CAN.write(6);
  CAN.write(54);
  CAN.write(0);
  CAN.write(0);
  CAN.write(25);
  CAN.endPacket();
  //Serial.println(testcount);
}

void send2c4() { // mpg?

  CAN.beginPacket(0x2c4);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.write(0); 
  CAN.write(count); //mpg level, 0 = all the way left
  CAN.write(0);
  CAN.write(0);
  
  CAN.endPacket();
}


void send30b() { // auto startstop

  CAN.beginPacket(0x30b);
  CAN.write(0);
  CAN.write(0);
  CAN.write(0);
  if (lights[8]) {
    CAN.write(4);
  }
  else {
    CAN.write(2);
  }
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.endPacket();
}


void send349() { // Fuel
  uint16_t level = min(1+(fuel*1*8), 9000);



  CAN.beginPacket(0x349);
  CAN.write(lo8(level));
  CAN.write(hi8(level));
  CAN.write(lo8(level));
  CAN.write(hi8(level));
  CAN.write(lo8(level));
  CAN.write(hi8(level));
  CAN.endPacket();
}


void send34f() { // Hnadbrake
  CAN.beginPacket(0x34f);
  CAN.write(0xfd);
  CAN.write(0xff);
  CAN.endPacket();
}


void send36a() { // auto highbeam
  CAN.beginPacket(0x36a);
  CAN.write(255);
  CAN.write(255);
  CAN.write(255);
  CAN.write(255);
  CAN.write(0); 
  CAN.write(0);
  CAN.write(0);
  CAN.write(0);
  CAN.endPacket();
}


void send36e() { // abs data, doesnt really work
  CAN.beginPacket(0x36e);
  CAN.write(0);
  CAN.write(testcount);
  CAN.write(0);
  CAN.write(0);
  CAN.write(0);
  CAN.write(0);
  CAN.write(0);
  CAN.write(0);
  CAN.endPacket();
}


void send36f() { // park light
  CAN.beginPacket(0x36f);
  CAN.write(255);
  CAN.write(255);
  CAN.write(255);
  CAN.write(255);
  CAN.write(0); 
  CAN.write(0);
  CAN.write(0);
  CAN.write(0);
  CAN.endPacket();
}

void send39e() { // date and time
  CAN.beginPacket(0x39e);
  CAN.write(hour);
  CAN.write(minute);
  CAN.write(seconds);
  CAN.write(day);
  CAN.write((month << 4) | 0x0f);
  CAN.write(year);
  CAN.write(year>>8);
  CAN.write(0xF2);
  CAN.endPacket();
}


void send3a7() { // drive mode
  CAN.beginPacket(0x3a7);
  CAN.write(0);
  CAN.write(count);
  CAN.write(0);
  CAN.write(random(255));
  CAN.write(drive_mode);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.endPacket();
}


void send3f9() { // temp
  CAN.beginPacket(0x3f9);
  CAN.write(0x02);
  CAN.write(coolant_temp + 48);
  CAN.write(count);
  CAN.write(coolant_temp+48);
  CAN.write(coolant_temp+48);
  
  CAN.write(int((0.983607*oil_temp) + 51.3169)); //oil temp

  /*
      60c = 110
      121c = 170
      182f = 230
  */
  CAN.write(oil_temp + 48);
  CAN.write(count);
  CAN.endPacket();
}


void send581() { // seatbelt
  CAN.beginPacket(0x581);
  CAN.write(0x40);
  CAN.write(0x4d);
  CAN.write(0);
  CAN.write(0x28);
  CAN.write(0xff);
  CAN.write(0xff);
  CAN.write(0xff);
  CAN.write(0xff);
  CAN.endPacket();
}

void sendb6e() { // ABS Counter 1
  CAN.beginPacket(0xb6e);
  CAN.write(testcount);
  CAN.write(testcount);
  CAN.write(testcount);
  CAN.write(testcount);
  CAN.write(testcount);
  CAN.write(testcount);
  CAN.write(testcount);
  CAN.write(testcount);
  CAN.endPacket();
  Serial.println(testcount);
}

void sendb68() { // TPMS
  CAN.beginPacket(0xb68);
  CAN.write(0);
  CAN.write(count);
  CAN.write(0);
  CAN.write(0);
  CAN.write(0);
  CAN.write(0);
  CAN.write(0);
  CAN.write(0);
  CAN.write(0);
  CAN.endPacket();
}



void sendRandom() {
  CAN.beginPacket(randomId);
  CAN.write(random(255));
  CAN.write(random(255));
  CAN.write(random(255));
  CAN.write(random(255));
  CAN.write(random(255));
  CAN.write(random(255));
  CAN.write(random(255));
  CAN.write(random(255));
  CAN.endPacket();
}


void loop() {


  unsigned long currentLoop = millis();
  String data;

  // read data from client
  if (Serial.available()) {
    speed = String(Serial.readStringUntil(';')).toFloat();
    rpm = String(Serial.readStringUntil(';')).toInt();
    oil_temp = String(Serial.readStringUntil(';')).toInt();
    coolant_temp = String(Serial.readStringUntil(';')).toInt();

    fuel = String(Serial.readStringUntil(';')).toInt();
    //lights = String(Serial.readStringUntil(';')).toInt(); fix this :D
  }

  //decode bitfield into variables

  int packetSize = CAN.parsePacket();
  if (packetSize) {
    // if a packet is present
    while (CAN.available()) {

      int buf = CAN.read();
      if(String(buf, HEX).length() == 1) {
        buf = buf | 0xF0;
      }
      data.concat(String(buf, HEX) + " ");
    }
    Serial.print(data);
    Serial.print(" | ");
    Serial.print(packetSize);
    Serial.print(" | ");
    Serial.println(CAN.packetId(), HEX);

  }

  
  if (currentLoop - sinceLast100msLoop > 100) {
    sinceLast100msLoop = currentLoop;
    sendf3(); // RPM
    send12f(); // Igniton Status
  }

  
  if (currentLoop - sinceLast200msLoop > 200) {
    sinceLast200msLoop = currentLoop;
  }

  
  if (currentLoop - sinceLast1000msLoop > 1000) {
    sinceLast1000msLoop = currentLoop;
    sendRandom(); 

  }

  
  if (currentLoop - sinceLast5sLoop > 5000) {
    sinceLast5sLoop = currentLoop;
    randomId += 1;
  }

  
}
