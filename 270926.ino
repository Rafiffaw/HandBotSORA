#define REMOTEXY_MODE__ESP32CORE_WIFI_POINT

#include <WiFi.h>
#include <RemoteXY.h>
#include <ESP32Servo.h> 

#define REMOTEXY_WIFI_SSID "ESP32_HandBot"
#define REMOTEXY_WIFI_PASSWORD "12345678"
#define REMOTEXY_SERVER_PORT 6377

#pragma pack(push, 1)  
uint8_t const PROGMEM RemoteXY_CONF_PROGMEM[] =   // 290 bytes V21 
  { 254,8,0,8,0,1,0,8,0,0,0,21,1,21,0,0,0,72,97,110,
  100,66,111,116,0,31,2,106,200,200,84,1,1,18,0,5,11,12,60,60,
  11,28,44,44,0,64,26,31,1,38,96,24,24,92,60,18,18,0,5,31,
  226,156,154,0,1,11,96,24,24,112,60,18,18,0,4,31,226,150,182,239,
  184,142,0,1,66,96,24,24,72,60,18,18,0,37,31,226,151,137,0,4,
  82,43,7,140,148,17,13,59,0,2,26,75,62,235,21,71,68,3,69,52,
  69,31,24,67,3,9,21,24,36,3,19,10,86,2,26,67,10,12,21,24,
  7,3,19,10,86,2,26,67,12,14,7,24,145,4,19,10,86,2,26,67,
  23,14,7,24,169,4,19,10,86,2,26,4,91,43,7,140,172,17,13,59,
  0,2,26,129,55,40,71,29,173,16,11,4,64,8,67,76,65,87,0,129,
  82,43,28,10,149,16,12,4,64,8,87,82,73,83,84,0,129,68,43,30,
  10,12,15,10,4,64,8,66,65,83,69,0,129,12,40,25,10,41,15,9,
  4,64,8,65,82,77,0,129,254,40,25,10,3,6,3,4,64,1,88,0,
  129,0,19,7,10,32,6,3,4,64,120,89,0,1,22,148,43,43,130,47,
  11,11,0,36,31,240,159,151,145,0 };
  
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
  uint8_t button_04; // =1 if button pressed, else =0, from 0 to 1

    // output variables
  int16_t value_01; // -32768 .. +32767
  int16_t value_02; // -32768 .. +32767
  int16_t value_03; // -32768 .. +32767
  int16_t value_04; // -32768 .. +32767

    // complex variables
  RemoteXYType_Terminal terminal_01; // call .print() or .println()

} RemoteXY;   
#pragma pack(pop)

// ----- Setup Pin Servo -----
const int PIN_CAPIT  = 33;
const int PIN_KANAN = 32;
const int PIN_KIRI   = 26;
const int PIN_GROUND = 25;


Servo servoCapit, servoSiku, servoGround, servoLengan;

// Variabel penyimpanan posisi
const int MAX_POSISI = 20;
int posisiTersimpan[MAX_POSISI][4];   
int jumlahTersimpan = 0;
bool recording = false;
bool savedPosition = false;

//Variabel playback
bool playing = false;
int indexPlayback = 0;
unsigned long waktuPindahTerakhir = 0;
const unsigned long JEDA_ANTAR_POSISI = 1000; 

//Variabel smoothing
float currentCapit = 90.0, currentSiku = 90.0, currentGround = 90.0, currentLengan = 90.0;
int targetCapit = 90, targetSiku = 90, targetGround = 90, targetLengan = 90;
const float SMOOTHING_FACTOR = 0.50; 
const float MAX_STEP = 1.2; 


float hitungPergerakanHalus(float current, float target) {
  float selisih = target - current;
  float langkah = selisih * SMOOTHING_FACTOR;
  
  if (langkah > MAX_STEP) langkah = MAX_STEP;
  if (langkah < -MAX_STEP) langkah = -MAX_STEP;
  
  return current + langkah;
}


void setup() 
{
  RemoteXY_Init (); 
  Serial.begin(115200);

  servoCapit.attach(PIN_CAPIT, 500, 2400);
  servoSiku.attach(PIN_KIRI, 500, 2400);
  servoGround.attach(PIN_GROUND, 500, 2400);
  servoLengan.attach(PIN_KANAN, 500, 2400);
}

void loop() 
{ 
  RemoteXYEngine.handler ();  

  if (!playing) {
    targetCapit  = map(RemoteXY.slider_02, 0, 100, 35, 180);
    targetSiku   = map(RemoteXY.slider_01, 0, 100, 60, 170);
    targetGround = map(RemoteXY.joystick_01_x, -100, 100, 170, 10);
    targetLengan = map(RemoteXY.joystick_01_y, -100, 100, 0, 100);
  } else {
    if (millis() - waktuPindahTerakhir >= JEDA_ANTAR_POSISI) {
      indexPlayback++;
      if (indexPlayback >= jumlahTersimpan) {
        playing = false;
        RemoteXY.terminal_01.println("Playback selesai.");
      } else {
        targetCapit  = posisiTersimpan[indexPlayback][0];
        targetSiku   = posisiTersimpan[indexPlayback][1];
        targetGround = posisiTersimpan[indexPlayback][2];
        targetLengan = posisiTersimpan[indexPlayback][3];
        waktuPindahTerakhir = millis();
      }
    }
  }

  if ((RemoteXY.button_03 == 1) && (!recording)) { 
    RemoteXY.button_03 = 0;
    recording = true;
    jumlahTersimpan = 0; 
    savedPosition  = false;
    RemoteXY.terminal_01.println("Recording Started...");
  } else if ((RemoteXY.button_03 == 1) && (recording)) {
    RemoteXY.button_03 = 0;
    RemoteXY.terminal_01.println("Already Recording...");
  }

  if ((RemoteXY.button_01 == 1) && (recording)) { 
    RemoteXY.button_01 = 0;
    savedPosition = true;
    simpanPosisi(targetCapit, targetSiku, targetGround, targetLengan);
    RemoteXY.terminal_01.print("Added: G:" + String(targetGround) + " L:" + String(targetLengan) + " S:" + String(targetSiku)); 
  }

  if ((RemoteXY.button_02 == 1) && (recording) && (savedPosition)) {  
    RemoteXY.button_02 = 0;
    RemoteXY.terminal_01.println("Playing sequence...");
    recording = false;
    savedPosition = false;
    mulaiPlayback();
  } else if ((RemoteXY.button_02 == 1) && (recording) && (!savedPosition)) {
    RemoteXY.button_02 = 0;
    RemoteXY.terminal_01.println("No Saved Position...");
  }

  if (RemoteXY.button_04 == 1) {  
    RemoteXY.button_04 = 0;
    RemoteXY.terminal_01.println("Remove All Saved Position");
  }

  currentCapit  = hitungPergerakanHalus(currentCapit, targetCapit);
  currentSiku   = hitungPergerakanHalus(currentSiku, targetSiku);
  currentGround = hitungPergerakanHalus(currentGround, targetGround);
  currentLengan = hitungPergerakanHalus(currentLengan, targetLengan);

  servoCapit.write(currentCapit);
  servoSiku.write(currentSiku);
  servoGround.write(currentGround);
  servoLengan.write(currentLengan);

  RemoteXY.value_04 = currentCapit;
  RemoteXY.value_03 = currentSiku;
  RemoteXY.value_02 = currentGround;
  RemoteXY.value_01 = currentLengan;

  // Delay kecil untuk responsivitas maksimal
  delay(5); 
}

void simpanPosisi(int p1, int p2, int p3, int p4) {
  if (jumlahTersimpan < MAX_POSISI) {
    posisiTersimpan[jumlahTersimpan][0] = p1;
    posisiTersimpan[jumlahTersimpan][1] = p2;
    posisiTersimpan[jumlahTersimpan][2] = p3;
    posisiTersimpan[jumlahTersimpan][3] = p4;
    jumlahTersimpan++;

    RemoteXY.terminal_01.print("Posisi ke-");
    char buf[5];
    sprintf(buf, "%d", jumlahTersimpan);
    RemoteXY.terminal_01.println(buf);
  } else {
    RemoteXY.terminal_01.println("Penyimpanan penuh!");
  }
}

void mulaiPlayback() {
  if (jumlahTersimpan == 0) {
    RemoteXY.terminal_01.println("Belum ada posisi tersimpan!");
    return;
  }
  playing = true;
  indexPlayback = 0;
  
  targetCapit  = posisiTersimpan[0][0];
  targetSiku   = posisiTersimpan[0][1];
  targetGround = posisiTersimpan[0][2];
  targetLengan = posisiTersimpan[0][3];
  
  waktuPindahTerakhir = millis();
}