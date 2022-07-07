#include <ArduinoBLE.h>
#include <Servo.h>

/*---BLE parameter---*/
const char* padService = "180F";
const char* TBtnValChar = "5442";
const char* RBtnValChar = "5242";
const char* LBtnValChar = "4c42";
const char* RadValChar = "5241";
const char* DegValChar = "4445";

/*---BLE parameter---*/
float oldTBtn = 0; float oldRBtn = 0; float oldLBtn = 0;
float oldRad = 0; float oldDeg = 0;

Servo servo1;  // 建立SERVO物件
Servo servo2;
Servo servo3;
Servo servo4;
Servo servo5;

int servoDeg1 = 90; // 馬達初始角度
int servoDeg2 = 90;
int servoDeg3 = 90;
int servoDeg4 = 45;
int servoDeg5 = 135;

const int FILTER_N = 10; //馬達濾波參數
int Filter1_Value; 
int Filter2_Value;
int Filter3_Value;
int Filter4_Value;
int Filter5_Value;


void setup() {
  Serial.begin(9600);

  int i;
  for (i = 0; i < FILTER_N; i++) {
    Filter1_Value = Filter1();
    Filter2_Value = Filter2();
    Filter3_Value = Filter3();
    Filter4_Value = Filter4();
    Filter5_Value = Filter5();
    delay(500);
  }

  servo1.attach(6);  // 設定要將伺服馬達接到哪一個PIN腳
  servo1.write(Filter1_Value);
  delay(500);
  servo2.attach(5);
  servo2.write(Filter2_Value);
  delay(500);
  servo3.attach(4);
  servo3.write(Filter3_Value);
  delay(500);
  servo4.attach(3);
  servo4.write(Filter4_Value);
  delay(500);
  servo5.attach(2);
  servo5.write(Filter5_Value);
  delay(500);

  BLE_init();
}

void loop() {
  connectToPeripheral();
}

void connectToPeripheral() {
  BLEDevice peripheral;

  Serial.println("- Discovering peripheral device...");

  do
  {
    BLE.scanForUuid(padService);
    peripheral = BLE.available();
  } while (!peripheral);

  if (peripheral) {
    Serial.println("* Peripheral device found!");
    Serial.print("* Device MAC address: ");
    Serial.println(peripheral.address());
    Serial.print("* Device name: ");
    Serial.println(peripheral.localName());
    Serial.print("* Advertised service UUID: ");
    Serial.println(peripheral.advertisedServiceUuid());
    Serial.println(" ");
    BLE.stopScan();
    controlPeripheral(peripheral);
  }
}

void controlPeripheral(BLEDevice peripheral) {
  Serial.println("- Connecting to peripheral device...");

  if (peripheral.connect()) {
    Serial.println("* Connected to peripheral device!");
    Serial.println(" ");
  } else {
    Serial.println("* Connection to peripheral device failed!");
    Serial.println(" ");
    return;
  }

  Serial.println("- Discovering peripheral device attributes...");
  if (peripheral.discoverAttributes()) {
    Serial.println("* Peripheral device attributes discovered!");
    Serial.println(" ");
  } else {
    Serial.println("* Peripheral device attributes discovery failed!");
    Serial.println(" ");
    peripheral.disconnect();
    return;
  }

  BLECharacteristic TBtnValChar = peripheral.characteristic("5442");
  BLECharacteristic RBtnValChar = peripheral.characteristic("5242");
  BLECharacteristic LBtnValChar = peripheral.characteristic("4c42");
  BLECharacteristic RadValChar = peripheral.characteristic("5241");
  BLECharacteristic DegValChar = peripheral.characteristic("4445");

  if (!TBtnValChar || !RBtnValChar || !LBtnValChar || !RadValChar || !DegValChar) {
    Serial.println("* Peripheral device does not have gesture_type characteristic!");
    peripheral.disconnect();
    return;
  } else if (!TBtnValChar.canSubscribe() || !RBtnValChar.canSubscribe() ||
             !LBtnValChar.canSubscribe() || !RadValChar.canSubscribe() || !DegValChar.canSubscribe()) {
    Serial.println("simple key characteristic is not subscribable!");
    peripheral.disconnect();
    return;
  } else if (!TBtnValChar.subscribe() || !RBtnValChar.subscribe() ||
             !LBtnValChar.subscribe() || !RadValChar.subscribe() || !DegValChar.subscribe()) {
    Serial.println("subscription failed!");
    peripheral.disconnect();
    return;
  }

  while (peripheral.connected()) {
    /*--readData--*/
    oldTBtn = getPadVal(TBtnValChar, oldTBtn, 1);
    oldRBtn = getPadVal(RBtnValChar, oldRBtn, 1);
    oldLBtn = getPadVal(LBtnValChar, oldLBtn, 1);
    oldRad = getPadVal(RadValChar, oldRad, 1);
    oldDeg = getPadVal(DegValChar, oldDeg, 1);
    /*--readData--*/

    Serial.print("Already get all data!");
    Serial.println("");

    if (oldRad == 0 || oldDeg == 0 ) { //----- servo1、2 控制
      servoDeg1 = 90;
      servoDeg2 = 90;
    }
    else if (oldRad > 0 && oldDeg >= 0 && oldDeg <= 180) {
      servoDeg1 = map(oldDeg, 0, 180, 45, 135);
      servoDeg2 = round(90 - oldRad * 30);
    }
    else if (oldDeg < 0 && oldDeg >= -90) {
      servoDeg1 = 45;
      servoDeg2 = round(90 - oldRad * 30);
    }
    else if (oldDeg < -90) {
      servoDeg1 = 135;
      servoDeg2 = round(90 - oldRad * 30);
    }
    Filter1_Value = Filter1();
    servo1.write(Filter1_Value);
    Filter2_Value = Filter2();
    servo2.write(Filter2_Value);

    if (oldTBtn == 1) { //----- servo3 控制
      if (servoDeg3 > 30) {
        servoDeg3 = servoDeg3 - 12;
      }
    }
    else if (oldTBtn == 0) {
      if (servoDeg3 < 90) {
        servoDeg3 = servoDeg3 + 12;
      }
    }
    Filter3_Value = Filter3();
    servo3.write(Filter3_Value);

    if (oldRBtn == 1) { //----- servo4 控制
      if (servoDeg4 < 135) {
        servoDeg4 = servoDeg4 + 18;
      }
    }
    else if (oldRBtn == 0) {
      if (servoDeg4 > 45) {
        servoDeg4 = servoDeg4 - 18;
      }
    }
    Filter4_Value = Filter4();
    servo4.write(Filter4_Value);

    if (oldLBtn == 1) { //----- servo5 控制
      if (servoDeg5 > 45) {
        servoDeg5 = servoDeg5 - 18;
      }
    }
    else if (oldLBtn == 0) {
      if (servoDeg5 < 135) {
        servoDeg5 = servoDeg5 + 18;
      }
    }
    Filter5_Value = Filter5();
    servo5.write(Filter5_Value);

  }
  Serial.println("- Peripheral device disconnected!");
}

void BLE_init() {
  if (!BLE.begin()) {
    Serial.println("* Starting Bluetooth® Low Energy module failed!");
    while (1);
  }

  BLE.setLocalName("Nano 33 BLE (Central)");
  BLE.advertise();
  Serial.println("Arduino Nano 33 BLE Sense (Central Device)");
  Serial.println(" ");
}

float getPadVal(BLECharacteristic c1, float oldF, int mod) {
  float f1 = oldF;
  Serial.print(f1);
  Serial.print(',');

  c1.read();
  if (c1.valueUpdated())
    f1 = getData(c1.value(), c1.valueLength(), mod);

  return f1;
}

union dat {
    unsigned char asdf[4];
    float zxcv;
    int qwer;
  };

float getData(const unsigned char data[], int length, int mod) {
  dat dat;
  for (int i = 0; i < length; i++) {
    dat.asdf[i] = data[i];
  }
  return mod == 1 ? dat.zxcv : dat.qwer;
}

int filter1_buf[FILTER_N + 1];
int Filter1() {
  int i;
  int filter1_sum = 0;
  filter1_buf[FILTER_N] = servoDeg1;
  for (i = 0; i < FILTER_N; i++) {
    filter1_buf[i] = filter1_buf[i + 1]; // 所有資料左移，低位仍掉
    filter1_sum += filter1_buf[i];
  }
  return (int)(filter1_sum / FILTER_N);
}

int filter2_buf[FILTER_N + 1];
int Filter2() {
  int i;
  int filter2_sum = 0;
  filter2_buf[FILTER_N] = servoDeg2;
  for (i = 0; i < FILTER_N; i++) {
    filter2_buf[i] = filter2_buf[i + 1]; // 所有資料左移，低位仍掉
    filter2_sum += filter2_buf[i];
  }
  return (int)(filter2_sum / FILTER_N);
}

int filter3_buf[FILTER_N + 1];
int Filter3() {
  int i;
  int filter3_sum = 0;
  filter3_buf[FILTER_N] = servoDeg3;
  for (i = 0; i < FILTER_N; i++) {
    filter3_buf[i] = filter3_buf[i + 1]; // 所有資料左移，低位仍掉
    filter3_sum += filter3_buf[i];
  }
  return (int)(filter3_sum / FILTER_N);
}

int filter4_buf[FILTER_N + 1];
int Filter4() {
  int i;
  int filter4_sum = 0;
  filter4_buf[FILTER_N] = servoDeg4;
  for (i = 0; i < FILTER_N; i++) {
    filter4_buf[i] = filter4_buf[i + 1]; // 所有資料左移，低位仍掉
    filter4_sum += filter4_buf[i];
  }
  return (int)(filter4_sum / FILTER_N);
}

int filter5_buf[FILTER_N + 1];
int Filter5() {
  int i;
  int filter5_sum = 0;
  filter5_buf[FILTER_N] = servoDeg5;
  for (i = 0; i < FILTER_N; i++) {
    filter5_buf[i] = filter5_buf[i + 1]; // 所有資料左移，低位仍掉
    filter5_sum += filter5_buf[i];
  }
  return (int)(filter5_sum / FILTER_N);
}
