// SENSOR SUHU : LM35 //
// Inisiasi Pin
const int tempPin = A1;
// Inisiasi Variabel
float temp = 0;

// PULSE SENSOR //
// Memanggil Library
#define USE_ARDUINO_INTERRUPTS true
#include <PulseSensorPlayground.h>
// Inisiasi Pin
const int PulsePin = 0;
// Inisiasi Variabel
int Threshold = 550;
int bpm = 0;
PulseSensorPlayground pulseSensor;

// ACCEL SENSOR : MPU6050 //
// Inisiasi Library
#include "I2Cdev.h"
#include "MPU6050.h"
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif
// Inisiasi Variabel
MPU6050 accelgyro;
int16_t ax, ay, az;
int16_t gx, gy, gz;
// Inisiasi Variabel Penghitung Langkah
int rangeAvg = 10;
int arrStep[10];
int arrTurn[10];
int iArr = 0;
long int sumarrStep;
long int sumarrTurn;
double avgarrStep;
double avgarrTurn;
bool first = true;

int thresholdUpStep;
int thresholdDownStep;
int thresholdUpTurn;
int thresholdDownTurn;

bool flagStep = false;
int countStep = 0;
bool flagLeft = false;
int countLeft = 0;
bool flagRight = false;
int countRight = 0;

// SD CARD //
// Memanggil Library
#include <SPI.h>
#include <SD.h>
// Inisiasi Pin
const int chipSelect = 10;

// Inisiasi Paket Data
String dataString = "";

// Inisiasi Waktu
int period = 1000;
unsigned long time_now = millis();


void setup() {
  // put your setup code here, to run once:

  //Inisiasi Serial Monitor
  Serial.begin(9600);

  // Inisiasi Pulse Sensor
  pulseSensor.analogInput(PulsePin);
  pulseSensor.setThreshold(Threshold);
  if (pulseSensor.begin()) {
    Serial.println("Yey pulse sensor objek di buat! :)");
  }

  // Inisiasi Accel Sensor
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
  Wire.begin();
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
  Fastwire::setup(400, true);
#endif
  accelgyro.initialize();
  Serial.println(accelgyro.testConnection() ? "Koneksi MPU6050 berhasil :)" : "MPU6050 connection failed");

  // Inisiasi SD Card
  if (!SD.begin(chipSelect)) {
    Serial.println("Memory cardnya tidak ada :'(");
    // don't do anything more:
    while (1);
  }
  // Menghapus file SD Card
  SD.remove("datalog.txt");
}

void loop() {
  // put your main code here, to run repeatedly:

  // Membaca Sensor Suhu
  temp = analogRead(tempPin) ;
  temp = temp * 500 / 1024;

  // Membaca Pulse Sensoer
  bpm = pulseSensor.getBeatsPerMinute();

  // Membaca Accel Sensor
  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // Menghitung Langkah
  if (iArr < rangeAvg - 1) {
    arrStep[iArr] = ax;
    arrTurn[iArr] = gx;
    iArr++;
  } else {
    arrStep[iArr] = ax;
    arrTurn[iArr] = gx;
    sumarrStep = 0;
    sumarrTurn = 0;
    for (int i = 0; i < rangeAvg; i++) {
      sumarrStep = arrStep[i] + sumarrStep;
      sumarrTurn = arrTurn[i] + sumarrTurn;
    }
    avgarrStep = sumarrStep / rangeAvg;
    avgarrTurn = sumarrTurn / rangeAvg;

    if (first) {
      thresholdUpStep = avgarrStep + 500;
      thresholdDownStep = avgarrStep - 1000;
      thresholdUpTurn = avgarrTurn + 11000;
      thresholdDownTurn = avgarrTurn - 11000;
    }
    first = false;

    if ((avgarrStep > thresholdUpStep) && (!flagStep)) {
      flagStep = true;
    }

    if ((avgarrStep < thresholdDownStep) && (flagStep)) {
      flagStep = false;
      countStep++;
    }

    if ((avgarrTurn > thresholdUpTurn) && (!flagRight)) {
      flagRight = true;
    }

    if ((avgarrTurn < thresholdUpTurn / 2) && (flagRight)) {
      flagRight = false;
      countRight++;
    }

    if ((avgarrTurn < thresholdDownTurn) && (!flagLeft)) {
      flagLeft = true;
    }

    if ((avgarrTurn > thresholdDownTurn / 2) && (flagLeft)) {
      flagLeft = false;
      countLeft++;
    }
    for (int i = 0; i < rangeAvg - 1; i++) {
      arrStep[i] = arrStep[i + 1];
      arrTurn[i] = arrTurn[i + 1];
    }
  }

  // Memasukan Paket Data ke String
  dataString = "";
  dataString += String(temp);
  dataString += ", ";
  dataString += String(bpm);
  dataString += ", ";
  dataString += String(ax);
  dataString += ",";
  dataString += String(ay);
  dataString += ",";
  dataString += String(az);
  dataString += ",";
  dataString += String(gx);
  dataString += ",";
  dataString += String(gy);
  dataString += ",";
  dataString += String(gz);
  dataString += ",";
  dataString += String(avgarrStep);
  dataString += ",";
  dataString += String(countStep);
  dataString += ",";
  dataString += String(avgarrTurn);
  dataString += ",";
  dataString += String(countLeft);
  dataString += ",";
  dataString += String(countRight);
  dataString += ",";

  // Menuliskan data di SD Card
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
  } else {
    Serial.println("error opening datalog.txt");
  }

  // Melakukan Serial Print Setiap Satu Detik

  while (millis() > time_now + period) {
    Serial.print("{");
    Serial.print(temp);
    Serial.print(",");
    Serial.print(bpm);
    Serial.print(",");
    Serial.print(countStep);
    Serial.print("}");
    Serial.println();

    time_now = millis();
  }

  // Melakukan Delay
  delay(20);
}
