#define REMOTEXY_MODE__ESP32CORE_WIFI_POINT

#include <WiFi.h>
#include <RemoteXY.h>
#include <ESP32Servo.h> 

#define REMOTEXY_WIFI_SSID "ESP32_HandBot"
#define REMOTEXY_WIFI_PASSWORD "12345678"
#define REMOTEXY_SERVER_PORT 6377

#pragma pack(push, 1)  
uint8_t const PROGMEM RemoteXY_CONF_PROGMEM[] =   // 312 bytes V21 
  { 254,10,0,8,0,1,0,8,0,0,0,43,1,21,0,0,0,72,97,110,
  100,66,111,116,0,31,2,106,200,200,84,1,1,20,0,5,11,12,60,60,
  11,28,44,44,0,64,26,31,1,38,96,24,24,92,45,18,18,0,5,31,
  226,156,154,0,1,11,96,24,24,112,45,18,18,0,4,31,226,150,182,239,
  184,142,0,1,66,96,24,24,72,45,18,18,0,37,31,226,151,137,0,4,
  82,43,7,140,148,17,13,59,0,2,26,75,62,235,21,71,68,3,69,37,
  69,31,24,67,3,9,21,24,36,3,19,10,86,2,26,67,10,12,21,24,
  7,3,19,10,86,2,26,67,12,14,7,24,145,4,19,10,86,2,26,67,
  23,14,7,24,169,4,19,10,86,2,26,4,91,43,7,140,172,17,13,59,
  0,2,26,129,55,40,71,29,173,16,11,4,64,8,67,76,65,87,0,129,
  82,43,28,10,149,16,12,4,64,8,87,82,73,83,84,0,129,68,43,30,
  10,12,15,10,4,64,8,66,65,83,69,0,129,12,40,25,10,41,15,9,
  4,64,8,65,82,77,0,129,254,40,25,10,3,6,3,4,64,1,88,0,
  129,0,19,7,10,32,6,3,4,64,120,89,0,1,28,129,57,57,114,66,
  14,14,3,29,31,0,1,57,153,7,33,94,66,14,14,3,27,31,0,1,
  41,153,7,33,74,66,14,14,3,29,31,0 };

struct {
  int8_t joystick_01_x; // from -100 to 100
  int8_t joystick_01_y; // from -100 to 100
  uint8_t button_01; // ADD
  uint8_t button_02; // PLAY
  uint8_t button_03; // RECORD
  int8_t slider_01; // from 0 to 100
  int8_t slider_02; // from 0 to 100
  uint8_t button_05; // MENU KANAN
  uint8_t button_06; // MENU TENGAH
  uint8_t button_07; // MENU KIRI

  int16_t value_01; // -32768 .. +32767
  int16_t value_02; // -32768 .. +32767
  int16_t value_03; // -32768 .. +32767
  int16_t value_04; // -32768 .. +32767

  RemoteXYType_Terminal terminal_01; // call .print() or .println()
} RemoteXY;   
#pragma pack(pop)

//Setup Pin Servo
const int PIN_CAPIT  = 33;
const int PIN_KANAN = 32;
const int PIN_KIRI   = 26;
const int PIN_GROUND = 25;

Servo servoCapit, servoSiku, servoGround, servoLengan;

// Var penyimpanan memori
const int MAX_POSISI = 20;
int slotPosisi[3][MAX_POSISI][4];  // [Slot 0-2][Max Posisi][4 Servo]
int slotJumlah[3] = {0, 0, 0};     

int tempPosisi[MAX_POSISI][4];     // Penyimpanan sementara (sebelum di-save)
int tempJumlah = 0;
bool recording = false;

// Var Playback
bool playing = false;
int activePlaySlot = -1;           
int indexPlayback = 0;

int playbackStep = 0;            // 0:Ground, 1:Lengan, 2:Siku, 3:Capit, 4:Jeda posisi
bool isWaitingDelay = false;
unsigned long waitTimer = 0;

const unsigned long DELAY_ANTAR_SENDI = 300;
const unsigned long JEDA_ANTAR_POSISI = 1000; 

// Variabel smoothing
float currentCapit = 90.0, currentSiku = 90.0, currentGround = 90.0, currentLengan = 90.0;
int targetCapit = 90, targetSiku = 90, targetGround = 90, targetLengan = 90;
const float SMOOTHING_FACTOR = 0.90; 
const float MAX_STEP = 1.9;

// Deklarasi fungsi
void handleMenuButton(int slotIndex, const char* menuName);
void mulaiPlayback(int slotIndex);

float hitungPergerakanHalus(float current, float target) {
  float selisih = target - current;
  float langkah = selisih * SMOOTHING_FACTOR;
  if (langkah > MAX_STEP) langkah = MAX_STEP;
  if (langkah < -MAX_STEP) langkah = -MAX_STEP;
  return current + langkah;
}

void setup() {
  RemoteXY_Init (); 
  Serial.begin(115200);

  servoCapit.attach(PIN_CAPIT, 500, 2400);
  servoSiku.attach(PIN_KIRI, 500, 2400);
  servoGround.attach(PIN_GROUND, 500, 2400);
  servoLengan.attach(PIN_KANAN, 500, 2400);
}

void loop() { 
  RemoteXYEngine.handler ();  

  //TARGET SERVO
  if (!playing) {
    targetCapit  = map(RemoteXY.slider_02, 0, 100, 35, 180);
    targetSiku   = map(RemoteXY.slider_01, 0, 100, 60, 170);
    targetGround = map(RemoteXY.joystick_01_x, -100, 100, 170, 10);
    targetLengan = map(RemoteXY.joystick_01_y, -100, 100, 0, 100);
  } else {
    //PLAYBACK SEQUENTIAL (NON-BLOCKING)
    int tCapit, tSiku, tGround, tLengan;
    if (activePlaySlot == 3) { // Slot preview
      tCapit  = tempPosisi[indexPlayback][0];
      tSiku   = tempPosisi[indexPlayback][1];
      tGround = tempPosisi[indexPlayback][2];
      tLengan = tempPosisi[indexPlayback][3];
    } else { // Slot menu
      tCapit  = slotPosisi[activePlaySlot][indexPlayback][0];
      tSiku   = slotPosisi[activePlaySlot][indexPlayback][1];
      tGround = slotPosisi[activePlaySlot][indexPlayback][2];
      tLengan = slotPosisi[activePlaySlot][indexPlayback][3];
    }

    if (isWaitingDelay) {
      if (millis() - waitTimer >= DELAY_ANTAR_SENDI) {
        isWaitingDelay = false;
        playbackStep++;
        waitTimer = millis();
      }
    } else {
      bool arrived = false;
      
      if (playbackStep == 0) { // 1: GROUND
        targetGround = tGround;
        if (abs(currentGround - targetGround) <= 1.0) arrived = true; //cek
      } 
      else if (playbackStep == 1) { // 2: LENGAN
        targetLengan = tLengan;
        if (abs(currentLengan - targetLengan) <= 1.0) arrived = true;
      } 
      else if (playbackStep == 2) { // 3: SIKU
        targetSiku = tSiku;
        if (abs(currentSiku - targetSiku) <= 1.0) arrived = true;
      } 
      else if (playbackStep == 3) { // 4: CAPIT
        targetCapit = tCapit;
        if (abs(currentCapit - targetCapit) <= 1.0) arrived = true;
      } 
      else if (playbackStep == 4) { // Delay next step
        if (millis() - waitTimer >= JEDA_ANTAR_POSISI) {
          indexPlayback++;
          int maxIndex = (activePlaySlot == 3) ? tempJumlah : slotJumlah[activePlaySlot];
          
          if (indexPlayback >= maxIndex) {
            playing = false;
            RemoteXY.terminal_01.println("Playback selesai.");
          } else {
            playbackStep = 0; // Ulangi ke ground next step
          }
        }
      }

      // Jika sampai target, mulai timer jeda
      if (arrived && playbackStep < 4) {
        isWaitingDelay = true;
        waitTimer = millis();
      }
    }
  }

  //TOMBOL RECORD
  if (RemoteXY.button_03 == 1) {    
    RemoteXY.button_03 = 0;
    if (!recording) {
      recording = true;
      tempJumlah = 0; 
      playing = false;
      RemoteXY.terminal_01.println("Recording... Tekan ADD");
    } else {
      RemoteXY.terminal_01.println("Sedang Recording!");
    }
  }

  //TOMBOL ADD
  if (RemoteXY.button_01 == 1) {     
    RemoteXY.button_01 = 0;
    if (recording) {
      if (tempJumlah < MAX_POSISI) {
        tempPosisi[tempJumlah][0] = targetCapit;
        tempPosisi[tempJumlah][1] = targetSiku;
        tempPosisi[tempJumlah][2] = targetGround;
        tempPosisi[tempJumlah][3] = targetLengan;
        tempJumlah++;
        
        char buf[50];
        sprintf(buf, "Add Pos-%d", tempJumlah);
        RemoteXY.terminal_01.println(buf); 
      } else {
        RemoteXY.terminal_01.println("Memori Penuh!");
      }
    }
  }

  //TOMBOL PLAY (PREVIEW)
  if (RemoteXY.button_02 == 1) {    
    RemoteXY.button_02 = 0;
    if (recording && tempJumlah > 0) {
      RemoteXY.terminal_01.println("Preview rekaman...");
      mulaiPlayback(3); // 3 = slot temporary
    } else if (!recording) {
      RemoteXY.terminal_01.println("Pilih menu untuk Play.");
    }
  }

  //MENU KANAN (Slot 2)
  if (RemoteXY.button_05 == 1){  
    RemoteXY.button_05 = 0;
    handleMenuButton(2, "Kanan");
  }

  //MENU TENGAH (Slot 1)
  if (RemoteXY.button_06 == 1){  
    RemoteXY.button_06 = 0;
    handleMenuButton(1, "Tengah");
  }

  //MENU KIRI (Slot 0)
  if (RemoteXY.button_07 == 1){  
    RemoteXY.button_07 = 0;
    handleMenuButton(0, "Kiri");
  }
  
  //PERGERAKAN SERVO
  currentGround = hitungPergerakanHalus(currentGround, targetGround);
  currentLengan = hitungPergerakanHalus(currentLengan, targetLengan);
  currentSiku   = hitungPergerakanHalus(currentSiku, targetSiku);
  currentCapit  = hitungPergerakanHalus(currentCapit, targetCapit);

  servoCapit.write(currentCapit);
  servoSiku.write(currentSiku);
  servoGround.write(currentGround);
  servoLengan.write(currentLengan);

  RemoteXY.value_04 = currentCapit;
  RemoteXY.value_03 = currentSiku;
  RemoteXY.value_02 = currentGround;
  RemoteXY.value_01 = currentLengan;

  delay(5); 
}


void handleMenuButton(int slotIndex, const char* menuName) {
  if (recording) {
    if (tempJumlah > 0) {
      for (int i = 0; i < tempJumlah; i++) {
        slotPosisi[slotIndex][i][0] = tempPosisi[i][0];
        slotPosisi[slotIndex][i][1] = tempPosisi[i][1];
        slotPosisi[slotIndex][i][2] = tempPosisi[i][2];
        slotPosisi[slotIndex][i][3] = tempPosisi[i][3];
      }
      slotJumlah[slotIndex] = tempJumlah;
      recording = false;

      char buf[40];
      sprintf(buf, "%d Pergerakan Disimpan ke %s", tempJumlah, menuName);
      RemoteXY.terminal_01.println(buf);
    } else {
      RemoteXY.terminal_01.println("Kosong! Dibatalkan.");
      recording = false;
    }
  } else {
    if (slotJumlah[slotIndex] > 0) {
      char buf[40];
      sprintf(buf, "Menjalankan Menu %s", menuName);
      RemoteXY.terminal_01.println(buf);
      mulaiPlayback(slotIndex);
    } else {
      RemoteXY.terminal_01.println("Menu Kosong!");
    }
  }
}

void mulaiPlayback(int slotIndex) {
  activePlaySlot = slotIndex;
  playing = true;
  indexPlayback = 0;
  
  playbackStep = 0;        
  isWaitingDelay = false;  
}