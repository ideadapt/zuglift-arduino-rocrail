#include <EEPROM.h>
#include <LiquidCrystal.h>
#include <NmraDcc.h>

#define P_STEP 13  // motor schritt
#define P_DIR 12   // motor richtung
#define P_EN 11    // motor aktiv / inaktiv
// 800steps/umdrehung
// speed=200umdrehung/minute
// 160000steps/minute
// 60s/160000steps=0.375ms für ein schritt
#define DUR_HALF_STEP 187  // 375/2 microseconds
//#define DUR_HALF_STEP 1000  // Test: 2ms pro Schritt

#define P_ZLU A2  // Echt A2. Test A4. Im Testmodus hat ZLu vorrang, ZLs und ZLu auf einem Button.
#define P_ZLO A3
#define P_ZLS A4

#define P_DCC_IN 2  // RocRail DCC für Ebenenwahl

#define P_LCD_EN 10
#define P_LCD_BTN A0
#define LCD_STOPP 1
#define LCD_STORE 2
#define LCD_UP 3
#define LCD_DOWN 4
#define LCD_PROG 5
#define LCD_UNKNOWN 6


// states
#define ST_EIN 10
#define ST_M 20
#define ST_MI 25
#define ST_E0 30
#define ST_EX 35
#define ST_STOPP 40
#define ST_PRG_ENTRY 50
#define ST_PRG 51
#define ST_PRG_M 60
#define ST_PRG_E 70
#define ST_PRG_EXIT 80


int RRPosIn = -1;
long stepC = 0;   // aktuelle schrittzahl
long stepCT = 0;  // ziel schrittzahl
short state = ST_EIN;


long exToStepC[] = {
  0,  // ebene 0
  // dummy values, will be set to values read from EEPROM
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,  // ebene 16
};

// https://maxpromer.github.io/LCD-Character-Creator
// https://docs.arduino.cc/learn/electronics/lcd-displays
byte arrUp[] = {
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B00100,
  B00100,
  B00100
};

byte arrDown[] = {
  B00100,
  B00100,
  B00100,
  B00100,
  B00100,
  B10101,
  B01110,
  B00100
};

byte ue[] = {
  B01010,
  B00000,
  B10001,
  B10001,
  B10001,
  B10001,
  B10001,
  B01110
};

byte ae[] = {
  B01010,
  B00000,
  B01110,
  B10001,
  B10001,
  B10011,
  B01101,
  B00000
};


short stPrgE_e = 1;
bool stPrgE_initDone = false;

short lcdBtn_lastState = LCD_UNKNOWN;

// Test
#define P_EX A5       // ebene gemäss potentiometer
#define P_BTN_EX A1
int btn_currentState;
int btn_lastState = HIGH;  // btn not pressed
unsigned long uptime;
unsigned long lastReglerDisplay;
int reglerEx;


// https://www.arduino.cc/reference/en/libraries/liquidcrystal/liquidcrystal
// https://wiki.dfrobot.com/LCD_KeyPad_Shield_For_Arduino_SKU__DFR0009
// LiquidCrystal(rs, enable, d4, d5, d6, d7)
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
NmraDcc Dcc;
void setup() {
  pinMode(P_STEP, OUTPUT);
  pinMode(P_DIR, OUTPUT);
  pinMode(P_EN, OUTPUT);
  pinMode(P_ZLU, INPUT_PULLUP);
  pinMode(P_ZLO, INPUT_PULLUP);
  pinMode(P_ZLS, INPUT_PULLUP);

  // Test
  Serial.begin(38400);
  while (!Serial)
    ;

  // fresh EEPROM is all 1
  bool freshEEPROM = EEPROM.read(0) == 0xFF;
  if (freshEEPROM) {
    logln("fressh EEPROM detected");
    for (int i = 1; i < 17; i++) {
      writeStepCEx(i, exToStepC[i]);
    }
    writeStepC(0);
  }

  long stepCEx = 0;
  for (int ex = 1; ex < 17; ex++) {
    stepCEx = readStepCEx(ex);
    exToStepC[ex] = stepCEx;

    Serial.print("E");
    Serial.print(ex);
    logln(" from EEPROM: ", stepCEx);
  }
  stepC = readStepC();
  logln("stepC from EEPROM: ", stepC);

  lcd.begin(16, 2);
  pinMode(P_LCD_EN, OUTPUT);
  lcd.createChar(0, arrDown);
  lcd.createChar(1, arrUp);
  lcd.createChar(3, ue);
  lcd.createChar(4, ae);

  Dcc.pin(P_DCC_IN, 0);
  Dcc.init(MAN_ID_DIY, 10, CV29_ACCESSORY_DECODER | CV29_OUTPUT_ADDRESS_MODE, 0);
}

void notifyDccAccTurnoutOutput(uint16_t Addr, uint8_t Direction, uint8_t OutputPower) {
  // Addr = (RR_Addr - 1) * 4 + Port
  // => Ebene 1 = (13 - 1) * 4 + 1 = 48
  RRPosIn = Addr - 48 -36;
  logln("Gewünschte Ebene: ", RRPosIn);
}

void loop() {
  uptime = millis();
  Dcc.process();

  /* Test lcd button pins
  int vol = analogRead(P_LCD_BTN);
  if(vol < 1010){
    Serial.print(vol);
    Serial.print(", ");
  }
  return;
  */

  switch (state) {
    case ST_EIN:
      {
        digitalWrite(P_LCD_EN, LOW);
        lcd.clear();
        motorOff();

        if (ZLo()) {
          state = ST_E0;
          logln("ST_EIN -> ST_E0");
          break;
        }

        stepC = readStepC();
        if (
          stepC == exToStepC[1] || stepC == exToStepC[2] || stepC == exToStepC[3] || stepC == exToStepC[4] || stepC == exToStepC[5] || stepC == exToStepC[6] || stepC == exToStepC[7] || stepC == exToStepC[8] || stepC == exToStepC[9] || stepC == exToStepC[10] || stepC == exToStepC[11] || stepC == exToStepC[12] || stepC == exToStepC[13] || stepC == exToStepC[14] || stepC == exToStepC[15] || stepC == exToStepC[16]) {
          state = ST_EX;
          logln("ST_EIN -> ST_EX");
        } else if (readEx() == 0) {
          state = ST_MI;
          logln("ST_EIN -> ST_MI");
        } else {
          // stored step count not in ebenen config. only init helps.
        }
      }
      break;
    case ST_EX:
      {
        motorOff();
        writeStepC(stepC);
        int ex = readEx();  // 0-16
        if (ex > 0) {
          stepCT = exToStepC[ex];
          logln("Gehe zu Schritt ", stepCT);
          logln(" der Ebene ", ex);
          state = ST_M;
          logln("ST_EX -> ST_M");
        } else if (ex == 0) {
          state = ST_MI;
          logln("ST_EX -> ST_MI");
        }
      }
      break;
    case ST_MI:
      {
        // ZLu() should never happen, since direction towards E0 / ZLo is opposite of ZLu
        if (ZLu() || ZLs()) {
          state = ST_STOPP;
          logln("ST_MI -> ST_STOPP");
          break;
        }

        if (ZLo()) {
          state = ST_E0;
          logln("ST_MI -> ST_E0");
          break;
        }

        motorUp();
        motorStep();
        stepC -= 1;
      }
      break;
    case ST_E0:
      {
        motorOff();
        stepC = 0;
        writeStepC(stepC);
        int ex = readEx();  // 0-16
        if (ex > 0) {
          stepCT = exToStepC[ex];
          logln("Gehe zu Schritt ", stepCT);
          logln(" der Ebene ", ex);
          state = ST_M;
          logln("ST_E0 -> ST_M");
        } else {
          short btn = readLcdButton();
          if (btn == LCD_PROG) {
            state = ST_PRG_ENTRY;
            logln("ST_E0 -> ST_PRG_ENTRY");
          }
        }
      }
      break;
    case ST_M:
      {
        if (ZLu() || ZLs()) {
          state = ST_STOPP;
          logln("ST_M -> ST_STOPP");
          break;
        }

        if (ZLo()) {
          state = ST_E0;
          logln("ST_M -> ST_E0");
          break;
        }

        if (stepC < stepCT) {
          motorDown();
          motorStep();
          stepC += 1;
        } else if (stepC > stepCT) {
          motorUp();
          motorStep();
          stepC -= 1;
        } else {
          // ziel erreicht
          state = ST_EX;
          logln("ST_M -> ST_EX");
        }
      }
      break;
    case ST_STOPP:
      {
        motorOff();

        int ex = readEx();  // 0-16
        if (ex > 0) {
          stepCT = exToStepC[ex];
          logln("Gehe zu Schritt ", stepCT);
          logln(" der Ebene ", ex);
          state = ST_M;
          logln("ST_STOPP -> ST_M");
        } else if (ex == 0) {
          state = ST_MI;
          logln("ST_STOPP -> ST_MI");
        }
      }
      break;
    case ST_PRG_ENTRY:
      {
        motorOff();
        digitalWrite(P_LCD_EN, HIGH);
        stPrgE_initDone = false;
        lcd.display();
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Bereit f");
        lcd.write(byte(3));
        lcd.print("r Init");
        lcd.setCursor(0, 1);
        lcd.print("ZL ");
        lcd.write(byte(1));
        lcd.print(" / ");
        lcd.write(byte(0));
        lcd.print(" bewegen");
        state = ST_PRG;
        logln("ST_PRG_ENTRY -> ST_PRG");
      }
      break;
    case ST_PRG:
      {
        short btn = readLcdButton();
        if (btn == LCD_PROG) {
          state = ST_PRG_EXIT;
          logln("ST_PRG -> ST_PRG_EXIT");
        } else {
          if (btn == LCD_DOWN || btn == LCD_UP) {
            state = ST_PRG_M;
            logln("ST_PRG -> ST_PRG_M");
          }
        }
      }
      break;
    case ST_PRG_M:
      {
       // TODO  
       // if (ZLu() || ZLs() || ZLo()) {
       //   state = ST_PRG_ENTRY;
       //   logln("ST_PRG_M -> ST_PRG_ENTRY");
       //   break;
       // }

        lcdBtn_lastState = LCD_UNKNOWN;  // reset state memory, to keep moving the motor as long as button in pressed
        short btn = readLcdButton();
        if (btn == LCD_DOWN) {
          motorDown();
          motorStep();
          stepC += 1;
        } else if (btn == LCD_UP && stepC > 0) {  // can only move up if at least once moved down (PRG mode starts at step 0, which is E0 aka ZLo)
          motorUp();
          motorStep();
          stepC -= 1;
        } else if (btn == LCD_STOPP) {
          state = ST_PRG_E;
          logln("ST_PRG_M -> ST_PRG_E");
        }
      }
      break;
    case ST_PRG_E:
      {
        motorOff();
        if (stPrgE_initDone == false) {
          lcd.clear();  // clear only once to avoid flickering
          lcd.setCursor(0, 0);
          lcd.print("Ebene w");
          lcd.write(byte(4));
          lcd.print("hlen: ");
          printEx(stPrgE_e);
          stPrgE_initDone = true;
        }

        short btn = readLcdButton();
        if (btn == LCD_PROG) {
          state = ST_PRG_EXIT;
          logln("ST_PRG_E -> ST_PRG_EXIT");
        } else {
          if (btn == LCD_DOWN && stPrgE_e > 1) {
            stPrgE_e -= 1;
            printEx(stPrgE_e);
          }
          if (btn == LCD_UP && stPrgE_e < 16) {
            stPrgE_e += 1;
            printEx(stPrgE_e);
          }

          if (btn == LCD_STORE) {
            writeStepCEx(stPrgE_e, stepC);
            exToStepC[stPrgE_e] = stepC;
            logln(stPrgE_e, stepC);
            lcd.clear();
            lcd.print("E");
            lcd.print(stPrgE_e);
            lcd.print(" = ");
            lcd.print(stepC);
            lcd.setCursor(0, 1);
            lcd.print("Schritte gespeic");
            delay(5000);
            state = ST_PRG_ENTRY;
            logln("ST_PRG_E -> ST_PRG_ENTRY");
          }
        }
      }
      break;
    case ST_PRG_EXIT:
      {
        motorOff();
        lcd.clear();
        lcd.print("Kalibrierung");
        lcd.setCursor(0, 1);
        lcd.print("         beendet");
        delay(5000);
        lcd.flush();
        lcd.clear();
        lcd.noDisplay();
        digitalWrite(P_LCD_EN, LOW);
        state = ST_MI;
        logln("ST_PRG_EXIT -> ST_MI");
      }
      break;
  }
}

int readEx() {
  return RRPosIn;
  // Test
  // unsigned long passed = uptime - lastReglerDisplay;
  // if (passed > 2000) {
  //   int regler = analogRead(P_EX);
  //   reglerEx = map(regler, 0, 1023, 0, 16);
  //   Serial.print(reglerEx);
  //   Serial.print(" ");
  //   lastReglerDisplay = millis();
  // }

  // btn_currentState = analogReadDigital(P_BTN_EX);
  // int ex = -1;
  // if (btn_currentState == LOW && btn_lastState == HIGH) {
  //   ex = reglerEx;
  //   Serial.println();
  //   btn_lastState = btn_currentState;
  // } else if (btn_currentState == HIGH && btn_lastState == LOW) {
  //   btn_lastState = btn_currentState;
  // }
  // return ex;
}

int ZLu() {
  // can only hit AnschlUnten if moving downwards
  if (digitalRead(P_DIR) == HIGH) {
    return false;
  }
  btn_currentState = analogReadDigital(P_ZLU);
  if (btn_currentState == LOW && btn_lastState == HIGH) {
    return true;
    //btn_lastState = btn_currentState; // when commented, button fires repeatedly, as long as pressed.
  } else if (btn_currentState == HIGH && btn_lastState == LOW) {
    btn_lastState = btn_currentState;
  }
  return false;
}

int ZLs() {
  btn_currentState = analogReadDigital(P_ZLS);
  if (btn_currentState == LOW && btn_lastState == HIGH) {
    return true;
    //btn_lastState = btn_currentState; // when commented, button fires repeatedly, as long as pressed.
  } else if (btn_currentState == HIGH && btn_lastState == LOW) {
    btn_lastState = btn_currentState;
  }
  return false;
}

int ZLo() {
  // can only hit AnschlOben if moving upwards
  // if(down() && running()) else (up() || stopped())
  if (digitalRead(P_DIR) == LOW && digitalRead(P_EN) == LOW) {
    return false;
  }
  btn_currentState = analogReadDigital(P_ZLO);
  if (btn_currentState == LOW && btn_lastState == HIGH) {
    return true;
    //btn_lastState = btn_currentState; // when commented, button fires repeatedly, as long as pressed.
  } else if (btn_currentState == HIGH && btn_lastState == LOW) {
    btn_lastState = btn_currentState;
  }
  return false;
}

long lastR = 0;
int analogReadDigital(int pin) {
  unsigned long passed = uptime - lastR;
  if (passed > 500) {
    Serial.print(pin); Serial.print(": ");
    Serial.print(digitalRead(pin)); Serial.print("; ");
    lastR = millis();
  }
  // LOW if pressed (pin is an analog pin with INPUT_PULLUP)
  return digitalRead(pin);
}

inline void motorUp() {
  digitalWrite(P_DIR, HIGH);
  digitalWrite(P_EN, LOW);
}

inline void motorDown() {
  digitalWrite(P_DIR, LOW);
  digitalWrite(P_EN, LOW);
}

inline void motorOff() {
  digitalWrite(P_EN, HIGH);
}

inline void motorStep() {
  digitalWrite(P_STEP, HIGH);
  delayMicroseconds(DUR_HALF_STEP);
  digitalWrite(P_STEP, LOW);
  delayMicroseconds(DUR_HALF_STEP);
}

// only useful for ex >= 1
long readStepCEx(int ex) {
  return readEEPROM(ex * 4);
}

// only useful for ex >= 1
void writeStepCEx(int ex, long stepC) {
  writeEEPROM(ex * 4, stepC);
}

long readStepC() {
  return readEEPROM(0);
}

void writeStepC(long stepC) {
  writeEEPROM(0, stepC);
}

// only write if stored value is different to provided value
void writeEEPROM(int address, long value) {
  long storedValue = -333l;
  EEPROM.get(address, storedValue);
  if (storedValue != value) {
    EEPROM.put(address, value);
    Serial.print("put at ");
    Serial.print(address);
    Serial.print(": ");
    Serial.println(value);
    Serial.print("  was before: ");
    Serial.println(storedValue);
  }
}

long readEEPROM(int address) {
  long storedValue = 0l;
  EEPROM.get(address, storedValue);
  return storedValue;
}

// return pressed button on first read. repeated reads during same button press return LCD_UNKNOWN.
// to support repeated reads manually set lcdBtn_lastState = LCD_UNKNOWN before function call.
short readLcdButton() {
  int state = LCD_UNKNOWN;
  int vol = analogRead(P_LCD_BTN);
  if (vol > 1000) state = LCD_UNKNOWN;
  // testmessung: select: 718, left: 476, up: 130, down: 304, right: 0
  else if (vol < 50) state = LCD_STORE;   // right
  else if (vol < 180) state = LCD_UP;     // up
  else if (vol < 360) state = LCD_DOWN;   // down
  else if (vol < 550) state = LCD_STOPP;  // left
  else if (vol < 850) state = LCD_PROG;   // select
  if (lcdBtn_lastState != state) {
    lcdBtn_lastState = state;
    return state;
  } else {
    return LCD_UNKNOWN;
  }
}

inline void printEx(short ex) {
  lcd.setCursor(0, 1);
  lcd.print(ex);
  lcd.print(" ");  // clear a potential 0 left over when decreasing 10 to 9
}

inline void logln(char a[]) {
  logln(a, -333);
}

inline void logln(char a[], long b) {
  if (sizeof(b) > 0 && b != -333) {
    Serial.print(a);
    Serial.println(b);
  } else {
    Serial.println(a);
  }
}
