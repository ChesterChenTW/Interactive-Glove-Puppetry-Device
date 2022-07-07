#include <ArduinoBLE.h>
#include <math.h>

int TopButton_P = 4;  //Pull-up resistor for TopButton
int TopButton_G = 5; //Ground for TopButton
int RightButton_P = 6;
int RightButton_G = 7;
int LeftButton_P = 9;
int LeftButton_G = 11;
int JoyStick_V = 19; //Vin for joystick
int JoyStick_G = 18; //Gnd for joystick
int JoyStick_X = 3; //Vx for joystick
int JoyStick_Y = 2; //Vy for joystick

/*---BLE parameter---*/
/*-- BLE33 BatteryMonitor Example --*/
// Bluetooth® Low Energy Battery Service
BLEService padService("180F");

// Bluetooth® Low Energy Battery Level Characteristic
BLEFloatCharacteristic TBtnValChar("5442",  //TBtn  // standard 16-bit characteristic UUID
                                   BLERead | BLENotify); // remote clients will be able to get notifications if this characteristic changes
BLEFloatCharacteristic RBtnValChar("5242", BLERead | BLENotify);  //RBtn
BLEFloatCharacteristic LBtnValChar("4c42", BLERead | BLENotify);  //LBtn
BLEFloatCharacteristic  RadValChar("5241", BLERead | BLENotify);  //RAD
BLEFloatCharacteristic  DegValChar("4445", BLERead | BLENotify);  //DEG

float oldTBtn = 0; float oldRBtn = 0; float oldLBtn = 0;
float oldRad = 0; float oldDeg = 0;

long previousMillis = 0;  // last time the battery level was checked, in ms
const long intervalMillis = 50;
/*---BLE parameter---*/

void setup() {
  Serial.begin(9600);
  pinMode(TopButton_P, INPUT_PULLUP);
  pinMode(RightButton_P, INPUT_PULLUP);
  pinMode(LeftButton_P, INPUT_PULLUP);

  pinMode(TopButton_G, OUTPUT);
  pinMode(RightButton_G, OUTPUT);
  pinMode(LeftButton_G, OUTPUT);
  pinMode(JoyStick_V, OUTPUT);
  pinMode(JoyStick_G, OUTPUT);

  digitalWrite(TopButton_G, LOW);
  digitalWrite(RightButton_G, LOW);
  digitalWrite(LeftButton_G, LOW);
  digitalWrite(JoyStick_V, HIGH);
  digitalWrite(JoyStick_G, LOW);

  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);

  digitalWrite(LEDR, LOW);
  digitalWrite(LEDG, LOW);
  digitalWrite(LEDB, LOW);

  BLE_init(); //initialize BLE as peripheral
}

void loop() {
  connectedByCentral();
}

void BLE_init() {
  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");

    while (1);
  }

  BLE.setLocalName("PadController");
  BLE.setAdvertisedService(padService); // add the service UUID
  padService.addCharacteristic(TBtnValChar); // add the characteristic
  padService.addCharacteristic(RBtnValChar);
  padService.addCharacteristic(LBtnValChar);
  padService.addCharacteristic(RadValChar);
  padService.addCharacteristic(DegValChar);
  BLE.addService(padService); // Add the battery service
  float initInt = 0;
  TBtnValChar.writeValue(initInt); // set initial value for this characteristic
  RBtnValChar.writeValue(initInt);
  LBtnValChar.writeValue(initInt);
  RadValChar.writeValue(initInt);
  DegValChar.writeValue(initInt);

  // start advertising
  BLE.advertise();

  Serial.println("Bluetooth® device active, waiting for connections...");
}

void connectedByCentral() {
  // wait for a Bluetooth® Low Energy central
  BLEDevice central = BLE.central();

  // if a central is connected to the peripheral:
  if (central) {
    /*---connect to central---*/
    Serial.print("Connected to central: ");
    Serial.println(central.address());
    /*-- turn on the LED to indicate the connection: --*/
    digitalWrite(LED_BUILTIN, HIGH);

    /*---central connecting & streaming data---*/
    // while the central is connected:
    // check the val every intervalMillis
    while (central.connected()) {
      long currentMillis = millis();
      // if intervalMillis have passed, check the val:
      if (currentMillis - previousMillis >= intervalMillis) {
        previousMillis = currentMillis;
        updatePadVal(); // read new val & send to central
      }
    }

    /*---central disconnect---*/
    digitalWrite(LED_BUILTIN, LOW);
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
}

int counter = 2;
void updatePadVal() {
  if (counter % 2 == 0 ) {
    oldTBtn = updateBtnState(TopButton_P); //偵測TBtn是否按
    TBtnValChar.writeValue(oldTBtn);
    oldRBtn = updateBtnState(RightButton_P); //偵測RBtn是否按下
    RBtnValChar.writeValue(oldRBtn);
    oldLBtn = updateBtnState(LeftButton_P); //偵測LBtn是否按下
    LBtnValChar.writeValue(oldLBtn);
    //TBtnValChar.writeValue((oldTBtn * 1) + (oldRBtn * 2) + (oldLBtn * 4));
  } else if (counter % 3 == 0 ) {
    updateJoystickVal();
  }

  counter = (counter % 3 == 0) ? 1 : counter ;
  counter++;
}

float updateBtnState(int iBtn) {
  float btnData = 0;
  btnData = (digitalRead(iBtn) == LOW) ? 1.0 : 0.0;
  return btnData;
}

void updateJoystickVal() {
  long JS_VX = analogRead(JoyStick_X) - 512;
  long JS_VY = 512 - analogRead(JoyStick_Y);
  float RAD = sqrt(JS_VX * JS_VX + JS_VY * JS_VY) / 500; //轉換成極座標且半徑等於1
  int DEG = atan2 (JS_VY, JS_VX) * 57.296; //轉換成極座標角度

  if (RAD >= 0.25 && RAD <= 1) {
    RadValChar.writeValue(RAD);
    DegValChar.writeValue(DEG);
    oldRad = RAD; oldDeg = DEG;
  }
  else if (RAD > 1) {
    RadValChar.writeValue(1.0);
    DegValChar.writeValue(DEG);
  }
  else {
    RadValChar.writeValue(0.0);
    DegValChar.writeValue(0.0);
  }
}
