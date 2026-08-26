
// we assume IOCON.BANK = 0
// Unfortunately IOCON moves its own address when the 
// bank bit is changed! This constitutes Poor Design...
#define REG_IODIRA 0x00
#define REG_IODIRB 0x01
#define REG_GPPUA  0x0C
#define REG_GPPUB  0x0D
#define REG_GPIOA  0x12 // can use for read or write
#define REG_GPIOB  0x13

extern void initDPEX(void);
extern void writeU5(uint8_t reg, uint8_t val);
extern void writeU5_16(uint8_t reg, uint16_t val, uint16_t mask = 0xFFFF);
extern uint16_t readU5_16(uint8_t reg);
extern void scribbleReset(void);