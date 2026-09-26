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
uint8_t const PROGMEM RemoteXY_CONF_PROGMEM[] =   // 183 bytes V21 
  { 254,7,0,8,0,1,0,8,0,0,0,170,0,21,0,0,0,72,97,110,
  100,66,111,116,0,31,2,106,200,200,84,1,1,11,0,5,11,12,60,60,
  12,28,44,44,0,64,26,31,1,38,96,24,24,92,60,18,18,0,5,31,
  226,156,154,0,1,11,96,24,24,112,60,18,18,0,4,31,226,150,182,239,
  184,142,0,1,66,96,24,24,72,60,18,18,0,37,31,226,151,137,0,4,
  82,43,7,140,150,17,13,59,0,2,26,75,62,235,21,71,68,3,69,52,
  69,31,24,67,3,9,21,24,10,3,19,10,86,2,26,67,10,12,21,24,
  35,3,19,10,86,2,26,67,12,14,7,24,145,4,19,10,86,2,26,67,
  23,14,7,24,169,4,19,10,86,2,26,4,91,43,7,140,172,17,13,59,
  0,2,26 };
  
// this structure defines all the variables and events of your control interface 
struct {

    // input variables
  int8_t joystick_01_x; // from -100 to 100
  int8_t joystick_01_y; // from -100 to 100
  uint8_t button_01; // =1 if button pressed, else =0, from 0 to 1
  uint8_t button_02; // =1 if button pressed, else =0, from 0 to 1
  uint8_t button_03; // =1 if button pressed, else =0, from 0 to 1
  int8_t slider_01; // from 0 to 100
  int8_t slider_02; // from 0 to 100

    // output variables
  int16_t value_01; // -32768 .. +32767
  int16_t value_02; // -32768 .. +32767
  int16_t value_03; // -32768 .. +32767
  int16_t value_04; // -32768 .. +32767

    // complex variables
  RemoteXYType_Terminal terminal_01; // call .print() or .println()

} RemoteXY;   
#pragma pack(pop)



// const int MAX_POSISI = 20;              // kapasitas maksimal posisi yang disimpan
// int posisiTersimpan[MAX_POSISI][3];     // [index][0]=servo1, [1]=servo2, [2]=servo3
// int jumlahTersimpan = 0;                // penghitung berapa posisi sudah disimpan
bool recording = false;

Servo servoCapit;
Servo servoSiku;
Servo servoGround;
Servo servoLengan;

void setup() 
{
  RemoteXY_Init (); 
  Serial.begin(115200);
  servoCapit.attach(33);
  servoSiku.attach(25);
  servoGround.attach(26);  
  servoLengan.attach(22);
  
}





void loop() 
{ 
  RemoteXYEngine.handler ();  
  
  if (RemoteXY.button_03 == 1) {
    recording = true;
    RemoteXY.button_03 = 0;
    RemoteXY.terminal_01.println("Recording . . .");
  }

  if ((RemoteXY.button_02 == 1) && (recording == true)) {
    RemoteXY.button_02 = 0;
    RemoteXY.terminal_01.println("Playing . . .");
    recording = false;
  }

  if ((RemoteXY.button_01 == 1) && (recording == true)) {
    RemoteXY.button_01 = 0;
    RemoteXY.terminal_01.println("Add position to . . .");
  }


  int angleCapit = map(RemoteXY.slider_02, 0, 100, 35, 180);
  RemoteXY.value_04 = angleCapit;
  servoCapit.write(angleCapit);

  Serial.print("remoteCapit : ");
  Serial.println(RemoteXY.slider_02);
  Serial.print("angleCapit : ");
  Serial.println(angleCapit);


  int angleSiku = map(RemoteXY.slider_01, 0, 100, 10, 170);
  RemoteXY.value_03 = angleSiku;
  servoSiku.write(angleSiku);

  Serial.print("remoteSiku : ");
  Serial.println(RemoteXY.slider_01);
  Serial.print("angleSiku : ");
  Serial.println(angleSiku);


  int angleGround = map(RemoteXY.joystick_01_y, 0, 100, 10, 170);
  RemoteXY.value_02 = angleGround;
  servoGround.write(angleGround);

  Serial.print("remoteGround : ");
  Serial.println(RemoteXY.joystick_01_y);
  Serial.print("angleGround : ");
  Serial.println(angleGround);


  int angleLengan = map(RemoteXY.joystick_01_x, -100, 100, 0, 180);
  RemoteXY.value_01 = angleLengan;
  servoLengan.write(angleLengan);

  Serial.print("remoteLengan : ");
  Serial.println(RemoteXY.joystick_01_x);
  Serial.print("angleLengan : ");
  Serial.println(angleLengan);


}

// void simpanPosisi(int p1, int p2, int p3) {
//   if (jumlahTersimpan < MAX_POSISI) {
//     posisiTersimpan[jumlahTersimpan][0] = p1;
//     posisiTersimpan[jumlahTersimpan][1] = p2;
//     posisiTersimpan[jumlahTersimpan][2] = p3;
//     jumlahTersimpan++;

//     Serial.print("Posisi ke-");
//     Serial.print(jumlahTersimpan);
//     Serial.print(" disimpan: ");
//     Serial.print(p1); Serial.print(", ");
//     Serial.print(p2); Serial.print(", ");
//     Serial.println(p3);
//   } else {
//     RemoteXY.terminal_01.println("Penyimpanan penuh!");
//   }
// }

// void gerakkanKePosisi(int index) {
//   if (index >= 0 && index < jumlahTersimpan) {
//     servoCapit.write(posisiTersimpan[index][0]);
//     servoSiku.write(posisiTersimpan[index][1]);
//     servoGround.write(posisiTersimpan[index][2]);
//   }
// }