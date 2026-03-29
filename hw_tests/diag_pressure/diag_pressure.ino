/*!
 * @file diag_pressure.ino
 * Diagnostic: dump raw pressure/temp registers, try single-shot + periodic
 */

#include <Wire.h>

#define WF100DPZ_ADDR 0x6D
#define REG_CMD 0x30
#define REG_STATUS 0x02
#define REG_DATA_MSB 0x06  // pressure 3 bytes
#define REG_TEMP_MSB 0x09  // temp 2 bytes
#define REG_PART_ID 0x28

uint8_t readReg(uint8_t reg) {
  Wire.beginTransmission(WF100DPZ_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(WF100DPZ_ADDR, (uint8_t)1);
  return Wire.available() ? Wire.read() : 0xFF;
}

void readRegs(uint8_t reg, uint8_t *buf, uint8_t len) {
  Wire.beginTransmission(WF100DPZ_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(WF100DPZ_ADDR, len);
  for (uint8_t i = 0; i < len; i++) {
    buf[i] = Wire.available() ? Wire.read() : 0xFF;
  }
}

void writeReg(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(WF100DPZ_ADDR);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

void dumpState(const char *label) {
  uint8_t pdata[3], tdata[2];
  uint8_t status = readReg(REG_STATUS);
  uint8_t cmd = readReg(REG_CMD);
  readRegs(REG_DATA_MSB, pdata, 3);
  readRegs(REG_TEMP_MSB, tdata, 2);

  Serial.print(F("--- "));
  Serial.print(label);
  Serial.println(F(" ---"));
  Serial.print(F("CMD=0x"));
  Serial.print(cmd, HEX);
  Serial.print(F("  STATUS=0x"));
  Serial.println(status, HEX);
  Serial.print(F("PRES raw: 0x"));
  Serial.print(pdata[0], HEX);
  Serial.print(F(" 0x"));
  Serial.print(pdata[1], HEX);
  Serial.print(F(" 0x"));
  Serial.println(pdata[2], HEX);
  Serial.print(F("TEMP raw: 0x"));
  Serial.print(tdata[0], HEX);
  Serial.print(F(" 0x"));
  Serial.println(tdata[1], HEX);

  // decode
  uint32_t uraw = ((uint32_t)pdata[0] << 16) | ((uint32_t)pdata[1] << 8) | pdata[2];
  int32_t sraw = (uraw >= 0x800000) ? (int32_t)uraw - 0x1000000 : (int32_t)uraw;
  float pressure = (float)sraw / 8388608.0 * 250.0 + 25.0;
  float temp = -1.2 + (float)(int8_t)tdata[0] + (float)tdata[1] * 0.004;
  float temp_no_offset = 0.0 + (float)(int8_t)tdata[0] + (float)tdata[1] * 0.004;
  Serial.print(F("P="));
  Serial.print(pressure, 3);
  Serial.print(F(" kPa  T(-1.2)="));
  Serial.print(temp, 3);
  Serial.print(F(" C  T(0.0)="));
  Serial.print(temp_no_offset, 3);
  Serial.println(F(" C"));
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Wire.begin();
  Serial.println(F("=== WF100DPZ PRESSURE DIAGNOSTIC ==="));

  uint8_t partId = readReg(REG_PART_ID);
  Serial.print(F("Part ID: 0x"));
  Serial.println(partId, HEX);

  // Dump all registers 0x00-0x3F
  Serial.println(F("\n--- FULL REGISTER DUMP ---"));
  for (uint8_t r = 0; r <= 0x3F; r++) {
    uint8_t val = readReg(r);
    if (r % 16 == 0) {
      Serial.print(F("0x"));
      if (r < 0x10) Serial.print('0');
      Serial.print(r, HEX);
      Serial.print(F(": "));
    }
    if (val < 0x10) Serial.print('0');
    Serial.print(val, HEX);
    Serial.print(' ');
    if (r % 16 == 15) Serial.println();
  }
  Serial.println();

  // 1) Power-on state
  dumpState("POWER ON (no cmd)");

  // 2) Single-shot: Mode=0x01 (single), Sco=1 (bit3)
  Serial.println(F(">>> Writing CMD=0x09 (single-shot, Sco=1)"));
  writeReg(REG_CMD, 0x09);
  delay(10);
  dumpState("SINGLE-SHOT 10ms");

  // wait longer
  delay(100);
  dumpState("SINGLE-SHOT 110ms");

  // 3) Try mode=0x02 (continuous) with Sco=1
  Serial.println(F(">>> Writing CMD=0x0A (continuous, Sco=1)"));
  writeReg(REG_CMD, 0x0A);
  delay(200);
  dumpState("CONTINUOUS 200ms");

  // 4) Periodic sleep mode (what simpletest uses)
  // sleep_125ms=0x01 in upper nibble, Sco=1, mode=0x03
  Serial.println(F(">>> Writing CMD=0x1B (sleep periodic 125ms, Sco=1)"));
  writeReg(REG_CMD, 0x1B);
  delay(200);
  dumpState("PERIODIC 200ms");

  // 5) Read pressure 10 times in periodic mode
  Serial.println(F("--- 10 PERIODIC READS (250ms apart) ---"));
  for (int i = 0; i < 10; i++) {
    uint8_t pdata[3];
    readRegs(REG_DATA_MSB, pdata, 3);
    uint8_t status = readReg(REG_STATUS);
    Serial.print(i);
    Serial.print(F(": P=0x"));
    Serial.print(pdata[0], HEX);
    Serial.print(pdata[1], HEX);
    Serial.print(pdata[2], HEX);
    Serial.print(F("  S=0x"));
    Serial.println(status, HEX);
    delay(250);
  }
}

void loop() {}
