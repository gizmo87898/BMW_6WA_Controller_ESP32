
#include <CAN.h>
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
/*
 * bmw 6wa cluster controller by gizmo
 */
#define lo8(x) ((int)(x)&0xff)
#define hi8(x) ((int)(x)>>8)

//CAN loop coutners
unsigned long sinceLast100msLoop = 0;
unsigned long sinceLast200msLoop = 0;
unsigned long sinceLast1000msLoop = 0;
unsigned long sinceLast5sLoop = 0;

//testing shit
int randomId = 0x1ee;
int randomData[] = {0, 0, 0, 0, 0, 0, 0, 0};
int count = 0;
int testcount = 0;
int gearcounter = 0x0d;

//cluster variables
int rpm = 1000; // 0-7500 rpm
float speed = 0; // m/s
int temp = 120; // C
char gear = 0;
int fuel = 50; // 0-100
bool parking_lights = false;
bool high_beam = false;
bool auto_start_stop = true;
bool tpms = false;
int fuel_frame[5] = {0x20, 0x10, 0x20, 0x10, 0xFF};
int abscounter = 0xb3;


void setup() {
  // Begin serial at 115200bps
  Serial.begin(115200);
  // Set CAN RX/TX pins to 2 and 15
  CAN.setPins(4, 2);
  // start the CAN bus at 125 kbps
  if (!CAN.begin(500E3)) {
    Serial.println("Starting CAN failed!");
    while (1);
  }
  Serial.println("Initialized");
}


void loop() {
  unsigned long currentLoop = millis();
  String data;
  if (Serial.available()) {
    speed = String(Serial.readStringUntil(';')).toFloat();
    rpm = String(Serial.readStringUntil(';')).toInt();
    temp = String(Serial.readStringUntil(';')).toInt();
    fuel = String(Serial.readStringUntil(';')).toInt();
  }
  int packetSize = CAN.parsePacket();
  if (packetSize) {
    // if a packet is present
    while (CAN.available()) {
      data.concat(String(CAN.read(), HEX));
    }
    //Serial.print("id: 0x");
    //Serial.print(CAN.packetId(), HEX);
    //Serial.print(" DLC: ");
    //Serial.print(packetSize);
    //Serial.print("  data: ");
    //Serial.println(data);

  }
  if (currentLoop - sinceLast100msLoop > 100) {
    sinceLast100msLoop = currentLoop;
    int randomData1 = random(255);
    int randomData2 = random(255);
    int randomData3 = random(255);
    int randomData4 = random(255);
    int randomData5 = random(255);
    int randomData6 = random(255);
    int randomData7 = random(255);
    int randomData8 = random(255);
    randomData[0] = randomData1;
    randomData[1] = randomData2;
    randomData[2] = randomData3;
    randomData[3] = randomData4;
    randomData[4] = randomData5;
    randomData[5] = randomData6;
    randomData[6] = randomData7;
    randomData[7] = randomData8;
    sendf3(); // RPM
    sendd7(); // airbag
    sendc0(); // jbbe
    send1a1(); // speed
    send12f(); // Ignition
    send1d2(); // gear
    //send36e(); // abs data
    send21a(); // lighting
    send291(); // mil,units
    //send2a7(); // power steering
    send30b(); // auto start/stop
    send3a7(); // Drive Mode
    send34f(); // handbrtake
    send349(); // fuel
    send3f9(); // Temp
    send581(); // seatbelt
    //sendb6e(); //abs coutner 1
    //sendb68(); // tpms
    if (count == 0xFF) {
      count = 0;
    }
    else {
      count++;
    }

  }
  if (currentLoop - sinceLast1000msLoop > 500) {
    sinceLast1000msLoop = currentLoop;
    
    //fuel++;
    //speed++;
    //Serial.println(speed);
    //if(speed>260){
    //  speed = 0;
    //}
    testcount++;
  }
  if (currentLoop - sinceLast200msLoop > 2) {
    sinceLast200msLoop = currentLoop;
    //rpm++;
    //if (rpm >= 7500) {
    //  rpm = 0;
    //}
    
  }
  if (currentLoop - sinceLast5sLoop > 2000) {
    sinceLast5sLoop = currentLoop;
    //randomId -= 1;
    //cnt+=1;
    //sendRandom(); // Testing

  }
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
  CAN.write(0);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.endPacket();
}


void send36e() { // abs data, doesnt really work
  CAN.beginPacket(0x36e);
  CAN.write(random(255));
  CAN.write(count);
  CAN.write(random(255));
  CAN.write(random(255));
  CAN.write(0);
  CAN.write(0);
  CAN.write(0);
  CAN.write(0);
  CAN.endPacket();
}


void send1a1() { // speed
  int speedval = speed*230;
  CAN.beginPacket(0x1a1);
  CAN.write(count);
  CAN.write(count);
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
  CAN.write(gearcounter);
  CAN.write(0xF0);
  CAN.write(0xFF);
  CAN.endPacket();
  if (gearcounter == 0xFD) {
    gearcounter = 0x0d;
  }
  else {
    gearcounter += 0x10;
  }
}


void send21a() { // Lighting
  CAN.beginPacket(0x21a);
  if (parking_lights) {
    CAN.write(0x05);
    CAN.write(0x12);
    CAN.write(0xF7);
  }
  else {
    CAN.write(0x00);
    CAN.write(0x00);
    CAN.write(0xF7);
  }
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
  CAN.write(random(255));
  CAN.write(56); 
  CAN.write(163);
  Serial.println(testcount);
  CAN.write(0);
  CAN.write(0);
  CAN.write(0);
  CAN.write(0);
  CAN.write(0);
  CAN.endPacket();
}


void send30b() { // auto startstop

  CAN.beginPacket(0x30b);
  CAN.write(0);
  CAN.write(0);
  CAN.write(0);
  if (auto_start_stop) {
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


void send3a7() { // drive mode
  CAN.beginPacket(0x3a7);
  CAN.write(0);
  CAN.write(0);
  CAN.write(0);
  CAN.write(0);
  CAN.write(4);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.endPacket();
}


void send3f9() { // temp
  CAN.beginPacket(0x3f9);
  CAN.write(0x02);
  CAN.write(temp + 48);
  CAN.write(count);
  CAN.write(temp+48);
  CAN.write(temp+48);
  
  CAN.write(int((0.983607*temp) + 51.3169)); //oil temp

  /*
      60c = 110
      121c = 170
      182f = 230
  */
  CAN.write(temp + 48);
  CAN.write(count);
  CAN.endPacket();
}


void send581() { // seatbelt
  CAN.beginPacket(0x581);
  CAN.write(0x40);
  CAN.write(0x4d);
  CAN.write(random(255));
  CAN.write(0x28);
  CAN.write(0xff);
  CAN.write(0xff);
  CAN.write(0xff);
  CAN.write(0xff);
  CAN.endPacket();
}
void sendb6e() { // ABS Counter 1
  CAN.beginPacket(0xb6e);
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
  CAN.write(randomData[0]);
  CAN.write(randomData[1]);
  //CAN.write(randomData[2]);
  //CAN.write(randomData[3]);
  //CAN.write(randomData[4]);
  //CAN.write(randomData[5]);
  //CAN.write(randomData[6]);
  //CAN.write(randomData[7]);

  CAN.endPacket();
  Serial.print("Sending 0x");
  Serial.print(randomId, HEX);
  Serial.print("  ");
  for (int i = 0; i < 8; i++) {
    Serial.print(randomData[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}
