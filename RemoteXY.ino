#define REMOTEXY_MODE__ESP32CORE_WIFI_POINT

#include <WiFi.h>
#include <ESP32Servo.h>
#include <SoftwareSerial.h>
#include <RemoteXY.h>

#define REMOTEXY_WIFI_SSID "ESP32_HandBot"
#define REMOTEXY_WIFI_PASSWORD "12345678"
#define REMOTEXY_SERVER_PORT 6377

#define REMOTEXY_SERIAL_RX 2
#define REMOTEXY_SERIAL_TX 3
#define REMOTEXY_SERIAL_SPEED 9600

 
#pragma pack(push, 1)  
uint8_t const PROGMEM RemoteXY_CONF_PROGMEM[] =   // 46 bytes V19 
  { 255,5,0,0,0,39,0,19,0,0,0,0,31,1,106,200,1,1,3,0,
  5,13,9,60,60,0,2,26,31,4,80,69,9,61,0,2,26,5,8,122,
  60,60,0,2,26,31 };
struct {

  int8_t joystick_01_x; // from -100 to 100
  int8_t joystick_01_y; // from -100 to 100
  int8_t slider_01; // from 0 to 100
  int8_t joystick_02_x; // from -100 to 100
  int8_t joystick_02_y; // from -100 to 100

  uint8_t connect_flag;

} RemoteXY;   
#pragma pack(pop)

Servo servo1;
Servo servo2;
Servo servo3;

void setup() 
{
  RemoteXY_Init (); 
  Serial.begin(115200);
  servo1.attach(33);
  servo2.attach(25);
  servo3.attach(26);  
  
}

void loop() 
{ 
  RemoteXYEngine.handler ();   

  int angle1 = map(RemoteXY.joystick_01_x, 0, 100, 0, 180);
  servo1.write(angle1);
  Serial.println(angle1);

  int angle2 = map(RemoteXY.joystick_01_y, 0, 100, 0, 180);
  servo2.write(angle2);
  Serial.println(angle2);

  int angle3 = map(RemoteXY.joystick_02_y, 0, 100, 0, 180);
  servo3.write(angle3);
  Serial.println(angle3);

  // servo1.write(slider_01);
  // Serial.println(slider_01);
  // RemoteXYEngine.delay(500);

}