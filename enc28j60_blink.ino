/*
  Blink
  Blinking the LEDs on ENC28J60 Ethernet Controller
*/

#define ADDR_MASK   0x1F
#define BANK_MASK   0x60
#define SPRD_MASK   0x80

#define ECON1       0x1F
#define ECON1_BSEL1 0x02
#define ECON1_BSEL0 0x01
#define PHLCON      0x14

#define MIREGADR    (0x14|0x40|SPRD_MASK)
#define MIWR        (0x16|0x40|SPRD_MASK)
#define MISTAT      (0x0A|0x60|SPRD_MASK)
#define MISTAT_BUSY 0x01

/* SPI operation codes */
#define READ_CTRL_REG    0x00
#define READ_BUF_MEM    0x3A
#define WRITE_CTRL_REG    0x40
#define WRITE_BUF_MEM   0x7A
#define BIT_FIELD_SET   0x80
#define BIT_FIELD_CLR   0xA0
#define SOFT_RESET      0xFF

// this is pin 10 on Uno and pin 53 on Mega256
#define ENC28J60_CONTROL_CS     	SS

#define enableChip()   digitalWrite(ENC28J60_CONTROL_CS, LOW)
#define disableChip()   digitalWrite(ENC28J60_CONTROL_CS, HIGH)

/* LEDA on LEDB off */
#define ENC28J60_BLINK_MODE1  0x3896
/* LEDA off LEDB on */
#define ENC28J60_BLINK_MODE2  0x3986

#include <SPI.h>

void setup() { 
  SPI.begin();
}

void loop() {  
  writePhy(PHLCON, ENC28J60_BLINK_MODE1); // LEDA on, LEDB off
  delay(500);                        
  writePhy(PHLCON, ENC28J60_BLINK_MODE2); // LEDA off, LEDB on
  delay(500);         
}

void setBank(uint8_t address)
{
  static uint8_t bank = 0;

  if ((address & BANK_MASK) != bank) {
    writeOp(BIT_FIELD_CLR, ECON1, ECON1_BSEL1|ECON1_BSEL0);
    bank = address & BANK_MASK;
    writeOp(BIT_FIELD_SET, ECON1, bank>>5);
  }  
}

static uint8_t readRegByte(uint8_t address)
{
  setBank(address);
  return readOp(READ_CTRL_REG, address);
}

static uint8_t readOp(uint8_t op, uint8_t address)
{
  enableChip();
  SPI.transfer(op | (address & ADDR_MASK));  

  // SPI is full-duplex (simultaneous send and receive)
  // do dummy read if needed (for MAC and MII registers)
  if (address & 0x80)
    SPI.transfer(0x00);

  uint8_t result = SPI.transfer(0x00);
  disableChip();
  return result;
}

static void writeOp(uint8_t op, uint8_t address, uint8_t data)
{
  enableChip();
  SPI.transfer(op | (address & ADDR_MASK));
  SPI.transfer(data);
  disableChip();
}

static void writeRegByte(uint8_t address, uint8_t data)
{
  setBank(address);
  writeOp(WRITE_CTRL_REG, address, data);
}

static void writeReg(uint8_t address, uint16_t data)
{
  writeRegByte(address, data);
  writeRegByte(address + 1, data >> 8);
}

static void writePhy(uint8_t address, uint16_t data)
{
  writeRegByte(MIREGADR, address);
  writeReg(MIWR, data);
  while (readRegByte(MISTAT) & MISTAT_BUSY)
    ;
}
