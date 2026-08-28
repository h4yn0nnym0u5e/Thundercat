
// we assume IOCON.BANK = 0
// Unfortunately IOCON moves its own address when the 
// bank bit is changed! This constitutes Poor Design...
#define REG_IODIRA 0x00
#define REG_IODIRB 0x01
#define REG_GPPUA  0x0C
#define REG_GPPUB  0x0D
#define REG_GPIOA  0x12 // can use for read or write
#define REG_GPIOB  0x13

#define htonl(x) ( ((x)<<24 & 0xFF000000UL) | \
                   ((x)<< 8 & 0x00FF0000UL) | \
                   ((x)>> 8 & 0x0000FF00UL) | \
                   ((x)>>24 & 0x000000FFUL) )

extern void initDPEX(void);
extern void writeU5(uint8_t reg, uint8_t val);
extern void scribbleReset(void);
extern void ADCsReset(void);

class DPex
{
    uint8_t addr;   //!< address (0-7)
    uint16_t gpio;  //!< current value
    uint16_t dirty; //!< dirty bits for output later
    uint32_t asyncTx;     //!< async transmit buffer
    uint32_t asyncResult; //!< place to put async read result
    void setCS(bool b) { digitalWriteFast(DPEX_CS, b); }
  public:    
    DPex(uint8_t _addr) 
    : addr{_addr}
    {}
    static SPISettings spiSettings;
    uint32_t makeTransactionWord(uint8_t reg = REG_GPIOA, bool write = false, uint16_t value = 0)
    {
        uint32_t result = ((0b01000001ul | (addr<<1)) << 24) | ( reg<<16 ); // read
        if (write)
        {
            result |= value;
            result &= ~(1<<24); // change to write
        }

        return result;
    }

    void begin(uint16_t iodir, uint16_t gppu)
    {
        write16(REG_GPIOA, iodir);
        write16(REG_GPPUA, gppu);
        gpio = read16(REG_GPIOA);
    }

    void assertCS(void) {setCS(0); }
    void negateCS(void) {setCS(1); }
    static void beginTransaction(void) { DPEX_SPI.beginTransaction(spiSettings); }
    static void endTransaction(void)   { DPEX_SPI.endTransaction(); }


    //! write a 16-bit word to two consecutive registers
    void write16(uint8_t reg, uint16_t val, uint16_t mask = 0xFFFF);
    //! read a 16-bit word from two consecutive registers
    uint16_t read16(uint8_t reg);

    // Async support. Asserts /CS, but transaction wrappers 
    // need to be added by the application, and event response
    // needs to negate /CS
    //! write a 16-bit word to two consecutive registers asynchronously
    void write16async(uint8_t reg, uint16_t val, EventResponderRef event_responder, uint16_t mask = 0xFFFF);
    //! read a 16-bit word from two consecutive registers asynchronously
    void read16async(uint8_t reg, EventResponderRef event_responder);
    uint16_t convertAsyncRead(void) { return (htonl(asyncResult)) & 0xFFFF; }

    bool getBit(uint8_t n)
    {
        gpio = read16(REG_GPIOA);
        return (gpio & (1<<n)) != 0;
    }

    //! get stored GPIO value.
    //! may be stale if there are dirty bits pending write
    uint16_t getGPIO(void) { return gpio; }

    //! set stored GPIO value, e.g. after async read
    void setGPIO(uint16_t val) { gpio = val; dirty = 0; }

    //! \return true if stored gpio has dirty bits
    bool isDirty(void) { return dirty != 0; }

    //! get stored GPIO bit.
    //! may be stale if there are dirty bits pending write
    bool getGPIObit(uint8_t n) { return (gpio & (1<<n)) != 0; }

    //! set a bit in the stored GPIO value
    void setGPIObit(uint8_t n, bool state)
    {
        uint16_t mask = 1<<n;
        if (state)
            gpio |= mask;
        else            
            gpio &= ~mask;
        dirty |= mask;            
    }

    //! set GPIO bit immediately in hardware.
    //! keeps state and marks it as clean
    void setBit(uint8_t n, bool state)
    {
        setGPIObit(n, state);
        write16(REG_GPIOA, gpio, 1<<n);
        dirty &= ~(1<<n);
    }


    //! write all dirty bits to port expander
    //! may do nothing if there's no dirty bits
    void setFromGPIO(void)
    {
        if (0 != dirty)
        {
            write16(REG_GPIOA, gpio, dirty);
            dirty = 0;
        }
    }

    //! poll GPIO; do pending writes, read current value
    void poll(void)
    {
        setFromGPIO();
        getBit(0);
    }
};

extern DPex U3, U5;

// "pin" parameter comes from expanders.h...
#define _DO_SET_A(fn,val, u,b,o) u.fn(b+8,val)
#define _DO_SET_B(fn,val, u,b,o) u.fn(b,val)
#define _DO_SET(fn,val, u,p,b,o) _DO_SET_##p(fn,val, u,b,o)
#define DO_SET(fn,val,pin) _DO_SET(fn,val,pin)

#define _DO_GET_A(fn, u,b,o) u.fn(b+8)
#define _DO_GET_B(fn, u,b,o) u.fn(b)
#define _DO_GET(fn, u,p,b,o) _DO_GET_##p(fn, u,b,o)
#define DO_GET(fn,pin) _DO_GET(fn,pin)

#define SET_BIT(pin,val) _DO_SET(setGPIObit,val,pin)
#define SET_BIT_NOW(pin,val) _DO_SET(setBit,val,pin)
#define GET_BIT(pin) _DO_GET(getGPIObit, pin)
#define GET_BUTTON(pin) !_DO_GET(getGPIObit, pin)
