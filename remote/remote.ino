#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE 9
#define CSN 10

const byte address[7] = "123456";

RF24 radio(CE, CSN);

//package
struct {
  int speed = 0;
  double angle = 0;
  int rotation = 0;
} data;

unsigned long currentMillis;
unsigned long prevMillis;
unsigned long txIntervalMillis = 100;

void read();
void send();

void setup() {
  Serial.begin(9600);

  //joysticks pins
  DDRC = (0 << DDC0) | (0 << DDC1) | (0 << DDC2);

  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.setRetries(3, 5);
  radio.openWritingPipe(address);
  radio.stopListening();
}

void loop() {
  read();

  //send every fixed interval
  currentMillis = millis();
  if (currentMillis - prevMillis >= txIntervalMillis) {
    send();
    prevMillis = millis();
  }
}

void read() {
  double pos_x = analogRead(A0) - 512;
  double pos_y = analogRead(A1) - 512;
  double rot = analogRead(A2) - 512;

  //get polar coordinates and set limits
  double power = hypotf(pos_x, pos_y);
  if (power > 510)
    power = 510;
  if (power < -510)
    power = -510;
  if (-10 < power && 10 > power)
    power = 0;

  if (rot > 510)
    rot = 510;
  if (rot < -510)
    rot = -510;
  if (-100 < rot && 100 > rot)
    rot = 0;

  //write data
  data.speed = (int)power >> 1;
  data.angle = atan2(pos_x, pos_y);
  data.rotation = (int)rot >> 1;
}

void send() {
  bool rslt;
  rslt = radio.write(&data, sizeof(data));

  //for debug
  if (rslt)
    Serial.println("Sent!");
  else
    Serial.println("Failed!");
}