#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE 48
#define CSN 49

const byte address[7] = "123456";

RF24 radio(CE, CSN);

//package
struct {
  int speed = 0;
  double angle = 0;
  int rotation = 0;
} data;

//used for debug if necessary
bool newData = false;

void read();
void write();

void setup() {
  Serial.begin(9600);

  //direction pins
  DDRA |= 0b11111111;
  //pwm pins
  DDRH |= (1 << DDH3) | (1 << DDH4) | (1 << DDH5) | (1 << DDH6);

  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.openReadingPipe(1, address);
  radio.startListening();
}

void loop() {
  read();
  write();
}

void read() {
  if (radio.available()) {
    radio.read(&data, sizeof(data));
    newData = true;
  }
}

void write() {
  double sinb = sin(data.angle - PI / 4);
  double cosb = cos(data.angle - PI / 4);
  double maxb = max(abs(sinb), abs(cosb));

  double leftFront, rightFront, leftRear, rightRear;

  //calculate speed
  leftFront = data.speed * cosb / maxb + data.rotation;
  rightFront = data.speed * sinb / maxb - data.rotation;
  leftRear = data.speed * sinb / maxb + data.rotation;
  rightRear = data.speed * cosb / maxb - data.rotation;

  //tone it down if translating and roatating
  if ((data.speed + abs(data.rotation)) > 255) {
    leftFront = 255 * (leftFront / (data.speed + data.rotation));
    rightFront = 255 * (rightFront / (data.speed + data.rotation)) ;
    leftRear = 255 * (leftRear / (data.speed + data.rotation));
    rightRear = 255 * (rightRear / (data.speed + data.rotation));
  }

  //set direction
  digitalWrite(22, leftFront >= 0);
  digitalWrite(23, leftFront < 0);
  digitalWrite(24, rightFront >= 0);
  digitalWrite(25, rightFront < 0);
  digitalWrite(26, leftRear >= 0);
  digitalWrite(27, leftRear < 0);
  digitalWrite(28, rightRear >= 0);
  digitalWrite(29, rightRear < 0);

  //set speed
  analogWrite(6, abs((int)leftFront));
  analogWrite(7, abs((int)rightFront));
  analogWrite(8, abs((int)leftRear));
  analogWrite(9, abs((int)rightRear));

  Serial.println(newData);
  newData = false;
}