#if !defined(_CLASSES_H_)
#define _CLASSES_H_

//           d8888 88888888888  d8888   .d8888b.   .d88888b. 88888888888 .d8888b.   d888    .d8888b.   .d8888b.  
//          d88888     888     d8P888  d88P  Y88b d88P" "Y88b    888    d88P  Y88b d8888   d88P  Y88b d88P  Y88b 
//         d88P888     888    d8P 888         888 888     888    888           888   888          888 888    888 
//        d88P 888     888   d8P  888       .d88P 888     888    888         .d88P   888        .d88P 888    888 
//       d88P  888     888  d88   888   .od888P"  888     888    888     .od888P"    888    .od888P"  888    888 
//      d88P   888     888  8888888888 d88P"      888 Y8b 888    888    d88P"        888   d88P"      888    888 
//     d8888888888     888        888  888"       Y88b.Y8b88P    888    888"         888   888"       Y88b  d88P 
//    d88P     888     888        888  888888888   "Y888888"     888    888888888  8888888 888888888   "Y8888P"  
//                                                       Y8b                                                     
//                                                                                                               
//
/*
 * Class to deal with AT42QT2120 touch chip
 */
class AT42QT2120_Wire
{
    TwoWire& theWire;
    int addr;
  public:
    AT42QT2120_Wire(TwoWire& _tw, int _addr) 
    : theWire{_tw}, addr{_addr}
    {}
    void begin(uint32_t frequency = 0);
    bool write(uint8_t* buf, int n);
    bool read(uint8_t* buf, int n);
};

class AT42QT2120_Wire_Async
{
    I2CMaster& theWire;
    int addr{0x55};

    bool finish(uint32_t timeout_millis = 50);
  public:
    AT42QT2120_Wire_Async(I2CMaster& _tw, int _addr) 
    : theWire{_tw}, addr{_addr}
    {}
    void begin(uint32_t frequency = 0);
    bool write(uint8_t* buf, int n);
    bool read(uint8_t* buf, int n);
};

template<class AT42QT2120_I2C>
class AT42QT2120
{
public:    
    AT42QT2120_I2C& theWire;
    uint8_t status[6];
    uint16_t oldK, newK, chg, mask;
  public:
    AT42QT2120(AT42QT2120_I2C& _wire, uint16_t _mask) 
        : theWire{_wire}, mask{_mask} 
        {}

    bool probe(int32_t freq)
    {
        uint8_t dummy{0x42};
        theWire.begin(freq);
        return theWire.write(&dummy,1);
    }

    void prepReadKeys(void)
    {
        uint8_t buf[6]{0};
        theWire.write(buf,1);
        theWire.read(buf,sizeof buf);
    }

    void readKeys(void)
    {
        uint8_t newKeys[2]{3};

        theWire.write(newKeys,1);
        theWire.read(newKeys, sizeof newKeys);

        oldK = (status[4]  << 8) | status[3];
        newK = (newKeys[1] << 8) | newKeys[0];
        chg = (newK ^ oldK) & mask; // ignore unimplemented bits

        status[3] = newKeys[0];
        status[4] = newKeys[1];
    }

    void calibrate(void)
    {
        uint8_t calibrate[]{6,1};
        theWire.write(calibrate,sizeof calibrate);
    }

    void setTouchRecalDelay(uint8_t theDelay)
    {
        uint8_t cmd[]{12,theDelay};
        theWire.write(cmd,sizeof cmd);
    }

    // get info on next changed key; returns false if no changes
    bool getChangedKey(int& keyNum, bool& state)
    {
        bool result = false;
        if (0 != chg)
        {
            uint32_t lsK = ((~chg) + 1) & chg; // find a changed key bit
            keyNum = 31 - __builtin_clz(lsK); // get key number
            state = (newK & lsK) != 0; // what it changed to
            chg &= ~lsK; // mark as dealt with
            result = true;
        }
        return result;
    }
};

class ButtonLED
{
    WS2812Serial& ledString;
    uint8_t* ledMemory;
    int num;
  public:
    ButtonLED(WS2812Serial& _string, uint8_t* mem, int n)
        : ledString{_string}, ledMemory{mem}, num{n}
        {}
          
    void setColour(uint32_t c)    { ledString.setPixel(num,c); }
    void show(void)               { ledString.show(); }
    void setBrightness(uint8_t n) { ledString.setBrightness(n); }
};

//                                             888                    
//                                             888                    
//                                             888                    
//    88888b.d88b.   .d88b.  88888b.  .d8888b  888888 .d88b.  888d888 
//    888 "888 "88b d88""88b 888 "88b 88K      888   d8P  Y8b 888P"   
//    888  888  888 888  888 888  888 "Y8888b. 888   88888888 888     
//    888  888  888 Y88..88P 888  888      X88 Y88b. Y8b.     888     
//    888  888  888  "Y88P"  888  888  88888P'  "Y888 "Y8888  888     
//
/*
 * Gather together all the components needed for
 * one of the Fader Monster's tasks
 */
extern void taskRoot(void* pfmt);
class FaderMonsterTask
{
        const char* name;
        configSTACK_DEPTH_TYPE stackDepth;
        UBaseType_t priority;
    public:
        FaderMonsterTask(const char* _name, 
                         configSTACK_DEPTH_TYPE _stackDepth = 512, 
                         void* _params = nullptr,
                         UBaseType_t _priority = 2)
        : name{_name}, stackDepth{_stackDepth}, priority{_priority}, params{_params}
        {}
        
        virtual BaseType_t create(const char* actualName) // creates task; may do other stuff
        { 
            return xTaskCreate(taskRoot, actualName, stackDepth, this, priority, &handle); 
        } 

        virtual BaseType_t create(void) { return create(name); }

        // Code that actually runs the task
        virtual void run(void) = 0;

        TaskHandle_t handle;
        void* params;
};

/*
class GenericTask : public FaderMonsterTask
{ 
  public:
    GenericTask(const char* _name, 
              configSTACK_DEPTH_TYPE _stackDepth, 
              void* _params,
              UBaseType_t _priority)
    : FaderMonsterTask{_name, _stackDepth, _params, _priority}
    {}
    
    void run(void) override;
};
*/

//    888b     d888 8888888 8888888b. 8888888 
//    8888b   d8888   888   888  "Y88b  888   
//    88888b.d88888   888   888    888  888   
//    888Y88888P888   888   888    888  888   
//    888 Y888P 888   888   888    888  888   
//    888  Y8P  888   888   888    888  888   
//    888   "   888   888   888  .d88P  888   
//    888       888 8888888 8888888P" 8888888 
// 
struct MIDImessage
{
    int type, cmd,value;
};

class MIDItask : public FaderMonsterTask
{
    //------------------------------------------------------------------------
    // stuff to deal with async requests from another task:
    //typedef InterTaskRequest::Result (MIDItask::* RequestExecutor)(void*);
    struct requestPayload
    {
        MIDImessage message;
    };
    RequestQueue<MIDItask, requestPayload> reqQueue;

    InterTaskRequest::Result doSendMIDI(MIDImessage& msg, uint32_t reqTime);
    //------------------------------------------------------------------------
    
  public:
    MIDItask(const char* _name, 
              configSTACK_DEPTH_TYPE _stackDepth, 
              void* _params,
              UBaseType_t _priority,
            
              int _reqQlen)
    : FaderMonsterTask{_name, _stackDepth, _params, _priority},
      reqQueue{_reqQlen}
    {}
    
    void run(void) override;
    InterTaskRequest::Result sendMIDI(InterTaskRequest& req, MIDImessage& msg)
    {
        InterTaskRequest::Result result = InterTaskRequest::Result::done;
        requestPayload payload{msg};
        RequestQueue<MIDItask, requestPayload>::queueEntry entry{&req, payload};

        result = reqQueue.request(entry, 0);

        return result;
    }
};
extern MIDItask midiTask;

//                                                
//    .d8888b  888  888 88888b.   .d88b.  888d888 
//    88K      888  888 888 "88b d8P  Y8b 888P"   
//    "Y8888b. 888  888 888  888 88888888 888     
//         X88 Y88b 888 888 d88P Y8b.     888     
//     88888P'  "Y88888 88888P"   "Y8888  888     
//                      888                       
//                      888                       
//                      888                       
//                                                
class SuperTask : public FaderMonsterTask
{ 
    //------------------------------------------------------------------------
    // stuff to deal with async requests from another task:
    typedef InterTaskRequest::Result (SuperTask::* RequestExecutor)(void*);
    struct requestPayload
    {
        RequestExecutor requestExecutor;
        void* context;
    };
    RequestQueue<SuperTask, requestPayload> reqQueue;

    //InterTaskRequest::Result doCalibrateTouch(void*);
    //------------------------------------------------------------------------

    void loopFn(void);
    InterTaskRequest touchCalibrationRequest;

    // display UI
    MainUIholder _ui;
    UIclass& ui;
    bool processUI(void);

  public:
    SuperTask(const char* _name, 
              configSTACK_DEPTH_TYPE _stackDepth = 512, 
              void* _params = nullptr,
              UBaseType_t _priority = 2,
            
              int _queueLength = 1)
    : FaderMonsterTask{_name, _stackDepth, _params, _priority},
      reqQueue{_queueLength},
      ui{*((UIclass*) _ui.space)}
    {}
    
    void run(void) override;
};

//    888                              888      
//    888                              888      
//    888                              888      
//    888888 .d88b.  888  888  .d8888b 88888b.  
//    888   d88""88b 888  888 d88P"    888 "88b 
//    888   888  888 888  888 888      888  888 
//    Y88b. Y88..88P Y88b 888 Y88b.    888  888 
//     "Y888 "Y88P"   "Y88888  "Y8888P 888  888 
//
typedef AT42QT2120<AT42QT2120_Wire_Async> touchChipDriver;
class TouchTask : public FaderMonsterTask
{
    //------------------------------------------------------------------------
    // stuff to deal with async requests from another task:
    typedef InterTaskRequest::Result (TouchTask::* RequestExecutor)(void*);
    struct requestPayload
    {
        RequestExecutor requestExecutor;
        void* context;
    };
    RequestQueue<TouchTask, requestPayload> reqQueue;

    InterTaskRequest::Result doCalibrateTouch(void*);
    //------------------------------------------------------------------------

    void initTouch(void);
    void updateTouch(void);
    void pollTouch(void);
    void updateKeyStatuses(touchChipDriver& touch);

    void startGT911(TaskHandle_t owner);
    uint8_t updateGT911(void);
    void processGT911(int n);


  public:
    static constexpr uint32_t touchFlag = 1;
    static constexpr uint32_t GT911Flag = 2;

    TouchTask(const char* _name, 
              configSTACK_DEPTH_TYPE _stackDepth, 
              void* _params,
              UBaseType_t _priority,

              int _queueLength,
              touchChipDriver& _atq
            )
            : FaderMonsterTask{_name, _stackDepth, _params, _priority},
              reqQueue{_queueLength}, touchChip{_atq}
        {}
    void run(void) override;
            
    //------------------------------------------------------------------------
    // stuff to allow another task to make async requests:
    InterTaskRequest::Result requestCalibration(InterTaskRequest& req)
    {
        requestPayload payload{&TouchTask::doCalibrateTouch, nullptr};
        RequestQueue<TouchTask, requestPayload>::queueEntry entry{&req, payload};
        return reqQueue.request(entry);
    }
    //------------------------------------------------------------------------
    
    touchChipDriver& touchChip;

    // CTP touch screen stuff
    GTPoint lastTouch;
    uint32_t lastTouchTime;
    bool touchReady{false}; // weird reset sequence is completed

    bool supplyValid{false};
    bool checkChange{true}; // public: set by ISR
    static TouchStatus keyStatuses[NUM_POTS];
    UBaseType_t messagesWaiting(void) { return reqQueue.messagesWaiting(); }
};

//            d8b                   888      8888888888 8888888b.           
//            Y8P                   888      888        888  "Y88b          
//                                  888      888        888    888          
//    888d888 888 88888b.   .d88b.  888      8888888    888    888 .d8888b  
//    888P"   888 888 "88b d88P"88b 888      888        888    888 88K      
//    888     888 888  888 888  888 888      888        888    888 "Y8888b. 
//    888     888 888  888 Y88b 888 888      888        888  .d88P      X88 
//    888     888 888  888  "Y88888 88888888 8888888888 8888888P"   88888P' 
//                              888                                         
//                         Y8b d88P                                         
//                          "Y88P"                                          
//  
class RingLEDsTask : public FaderMonsterTask
{ 
  public:
    RingLEDsTask(const char* _name, 
              configSTACK_DEPTH_TYPE _stackDepth, 
              void* _params,
              UBaseType_t _priority)
    : FaderMonsterTask{_name, _stackDepth, _params, _priority}
    {}
    
    void run(void) override;
};

//                              d8b 888      888      888          
//                              Y8P 888      888      888          
//                                  888      888      888          
//    .d8888b   .d8888b 888d888 888 88888b.  88888b.  888  .d88b.  
//    88K      d88P"    888P"   888 888 "88b 888 "88b 888 d8P  Y8b 
//    "Y8888b. 888      888     888 888  888 888  888 888 88888888 
//         X88 Y88b.    888     888 888 d88P 888 d88P 888 Y8b.     
//     88888P'  "Y8888P 888     888 88888P"  88888P"  888  "Y8888  
// 
class ScribbleTask : public FaderMonsterTask
{ 
    //------------------------------------------------------------------------
    // stuff to deal with async requests from another task:
    typedef InterTaskRequest::Result (ScribbleTask::* RequestExecutor)(void*);
    struct requestPayload
    {
        RequestExecutor requestExecutor;
        void* context;
    };
    RequestQueue<ScribbleTask, requestPayload> reqQueue;

    InterTaskRequest::Result doUpdateDirty(void* pScribble);
    //------------------------------------------------------------------------

    static TFT_TYPE tft1, tft2, tft3, tft4, 
                    tft5, tft6, tft7, tft8;
    static TFT_TYPE* scribbles[NUM_POTS];
    static InterTaskRequest updateDirtyReq;
    static bool initComplete;
    uint16_t* DMAbuffer;

    void setDMAcompletionISR(void (*isr)(TFT_eSPI& which))
    {
        for (int i=0; i<NUM_POTS; i++)
            scribbles[i]->dmaAttachCompletionISR(isr);
    }

    void initDisplayPins(void);
    bool doAphase(int i, int& phase);
    void phasedInit(void);
    void fillUnique(TFT_TYPE& tft, int i);

  public:
    ScribbleTask(const char* _name, 
              configSTACK_DEPTH_TYPE _stackDepth, 
              void* _params,
              UBaseType_t _priority,
            
              int _queueLength)
    : FaderMonsterTask{_name, _stackDepth, _params, _priority},
      reqQueue{_queueLength},
      DMAbuffer{nullptr}
    {}
   
    static TFT_TYPE& getTFT(uint8_t n) { return *scribbles[n % NUM_POTS]; }
    
    void run(void) override;
    void setDMAbuffer(uint16_t* buf) { DMAbuffer = buf; }
    static bool tftInitComplete(void) { return initComplete; }

    //------------------------------------------------------------------------
    // stuff to allow another task to make async requests:
    InterTaskRequest& updateDirty(InterTaskRequest& req, TFT_eSprite& scribble, TickType_t timeout = 0);
};

//    888b     d888          d8b          888      .d8888b.  8888888b.  
//    8888b   d8888          Y8P          888     d88P  Y88b 888  "Y88b 
//    88888b.d88888                       888     888    888 888    888 
//    888Y88888P888  8888b.  888 88888b.  888     888        888    888 
//    888 Y888P 888     "88b 888 888 "88b 888     888        888    888 
//    888  Y8P  888 .d888888 888 888  888 888     888    888 888    888 
//    888   "   888 888  888 888 888  888 888     Y88b  d88P 888  .d88P 
//    888       888 "Y888888 888 888  888 88888888 "Y8888P"  8888888P"  
//  
class MainLCDtask : public FaderMonsterTask
{ 
    //------------------------------------------------------------------------
    // stuff to deal with async requests from another task:
    typedef InterTaskRequest::Result (MainLCDtask::* RequestExecutor)(void*);
    struct requestPayload
    {
        RequestExecutor requestExecutor;
        void* context;
    };
    RequestQueue<MainLCDtask, requestPayload> reqQueue;

    InterTaskRequest::Result doUpdateDirty(void* pScribble);
    //------------------------------------------------------------------------

    TFT_TYPE& tft;
    TFT_eSprite& sprite;
    //TFT_TYPE* ptft;
    InterTaskRequest updateDirtyReq;
    bool initComplete;
    uint16_t* DMAbuffer;

    void setDMAcompletionISR(void (*isr)(TFT_eSPI& which))
    {
        tft.dmaAttachCompletionISR(isr);
    }

    void initDisplayPins(void);
    bool doAphase(int& phase);
    void phasedInit(void);
    bool TFTdmaWait(int pixels);

  public:
    MainLCDtask(const char* _name, 
              configSTACK_DEPTH_TYPE _stackDepth, 
              void* _params,
              UBaseType_t _priority,
            
              TFT_TYPE& _tft,
              TFT_eSprite& _spr,
              int _queueLength)
    : FaderMonsterTask{_name, _stackDepth, _params, _priority},
      reqQueue{_queueLength},
      tft{_tft}, sprite{_spr},
      DMAbuffer{nullptr}
    {}
   
    TFT_TYPE& getTFT(void) { return tft; }
    
    void run(void) override;
    void setDMAbuffer(uint16_t* buf) { DMAbuffer = buf; }
    bool tftInitComplete(void) { return initComplete; }
    TFT_eSprite& getSprite(void) { return sprite; }

    bool pauseOutput{false}; // temporary hack...
    bool zapScreen{false}; // as is this
    //------------------------------------------------------------------------
    // stuff to allow another task to make async requests:
    InterTaskRequest& updateDirty(InterTaskRequest& req, TFT_eSprite& scribble, TickType_t timeout = 0);
};

//             888            d8b          
//             888            Y8P          
//             888                         
//    .d8888b  888888 888d888 888 88888b.  
//    88K      888    888P"   888 888 "88b 
//    "Y8888b. 888    888     888 888  888 
//         X88 Y88b.  888     888 888 d88P 
//     88888P'  "Y888 888     888 88888P"  
//                                888      
//                                888      
//                                888      
//                                         
//
/*
 * Configuration for a strip
 * This is outside the strip, as we want a convenient method to
 * load it from the filesystem
 * /
struct colours_t {
    uint16_t fg,bg,txt;
};
*/

class StripConfig
{
  public:
    // ring LEDs
    struct {
        int colour; // colour
        int pattern[LEDS_PER_RING]; // pattern
    } ringLEDs;

    // scribble display
    struct {
        colours_t colours;
    } scribble;
};

// strip task
class StripTask : public FaderMonsterTask
{ 
    //------------------------------------------------------------------------
    // stuff to deal with async requests from another task:
    typedef InterTaskRequest::Result (StripTask::* RequestExecutor)(void*);
    struct requestPayload
    {
        RequestExecutor requestExecutor;
        void* context;
    };
    RequestQueue<StripTask, requestPayload> reqQueue;

    InterTaskRequest::Result doPotChange(void* pNothing);
    InterTaskRequest::Result doTouchChange(void* pNothing);
    //------------------------------------------------------------------------
    static StripTask* tasks[NUM_POTS];

    StripConfig& cfg;

    LEDring<NUM_POTS> ring;   // our LED ring
    ContinuousPot& pot;       // ...continuous pot...
    TouchStatus& potTouch;    // ...and its touch sensor...
    TFT_eSprite& scribble;    // ...scribble strip display...
    // Fader& fader;            // ...fader
    // Button& button;          // ...button (control and LED)

    static constexpr float POT_NOT_SET{-999.0f};
    static constexpr float sa{2*18.0f}, ea{360.0f - 2*18.0f}; // TFT_eSPI has zero at 6 o'clock
    static constexpr int BUF_SIZE{30};

    InterTaskRequest updateReq; // used to monitor progress of display update

    // ring LEDs
    void setDot(float value, uint32_t colour);
    void setDotCurrent(void) { setDot(pot.getCurrent(), useRingPattern?RingLEDs<NUM_POTS>::USE_PATTERN:cfg.ringLEDs.colour); }

    // display UI
    ScribbleUIholder _ui;
    UIclass& ui;
  public:
    StripTask(const char* _name, 
              configSTACK_DEPTH_TYPE _stackDepth, 
              void* _params,
              UBaseType_t _priority,
            
              int _num, int _reqQlen,
              RingLEDs<NUM_POTS>& _rings,
              ContinuousPot& _pot, TouchStatus& _potTouch,
              TFT_eSprite& _scribble,
              StripConfig& _cfg
            )
    : FaderMonsterTask{_name, _stackDepth, _params, _priority},
      reqQueue{_reqQlen}, cfg{_cfg},
      ring{LEDring{_rings,_num}}, pot{_pot}, potTouch{_potTouch},
      scribble{_scribble}, 
      ui{*((UIclass*) _ui.space)},
      num{_num}, bright{0},
      useRingPattern{false}
    { }
    static void CreateTasks(void);
    static StripTask& getStripTask(int n) { return *tasks[n]; }
    
    void run(void) override;

    //------------------------------------------------------------------------
    // stuff we use to make async requests to another task :
    InterTaskRequest::Result updateDisplay(ScribbleTask& scribbleTask)
    {
        return scribbleTask.updateDirty(displayReq, scribble, 0).status;
    }

    //------------------------------------------------------------------------
    // stuff to allow another task to make async requests:
    // called from potsTask to say our pot value has changed
    void potChanged(void)
    {
        requestPayload payload{&StripTask::doPotChange, nullptr};
        RequestQueue<StripTask, requestPayload>::queueEntry entry{&potReq, payload};

        reqQueue.request(entry, 0);
    }

    void touchChanged(void)
    {
        requestPayload payload{&StripTask::doTouchChange, nullptr};
        RequestQueue<StripTask, requestPayload>::queueEntry entry{&touchReq, payload};

        reqQueue.request(entry, 0);
    }

    int num; // which strip this is (0-7)
    static int globalBright;
    int bright;
    bool useRingPattern;
    InterTaskRequest displayReq,        // outgoing
                     potReq, touchReq;  // incoming
};

//                      888             
//                      888             
//                      888             
//    88888b.   .d88b.  888888 .d8888b  
//    888 "88b d88""88b 888    88K      
//    888  888 888  888 888    "Y8888b. 
//    888 d88P Y88..88P Y88b.       X88 
//    88888P"   "Y88P"   "Y888  88888P' 
//    888                               
//    888                               
//    888                               
// 

/**
    Pots task.
    pots, faders and port expanders are all on the
    same SPI bus, so need to be dealt with together
 */
class PotsTask : public FaderMonsterTask
{ 
    //! record of MIDI request sent for each pot
    static struct MIDIreq
    {
        InterTaskRequest req; //!< request status
        int lastValue;        //!< last-sent value
    } midiReqs[NUM_POTS];

    static ContinuousPot allPots[NUM_POTS]; //!< status for all pots
    static StripTask* stripTasks[NUM_POTS]; //!< pointers to strip tasks that "own" each pot
    
    //! Update ADC readings
    void updateADCs(void);

    //! Convert raw ADC reading to volts
    // \return voltage level
    float raw2volts(uint16_t raw) //!< raw ADC reading
        { return (float) raw / 65535.0f * 5.0f; }

  public:
    PotsTask(const char* _name, //!< task name
              configSTACK_DEPTH_TYPE _stackDepth, //!< task stack size (in 32-bit words)
              void* _params, //!< parameters
              UBaseType_t _priority) //!< priority
    : FaderMonsterTask{_name, _stackDepth, _params, _priority}
    {}
    
    void run(void) override;    //!< FreeRTOS task

    //! \return reference to a specific pot instance
    ContinuousPot& getPot(int n) //!< instance index
        { return allPots[n]; }

    //! Inform pots task of the strip task that "owns" a pot instance        
    void setOwner(StripTask* pTask, //!< pointer to a strip task instance
                  int n) //!< pot instance index
                  { stripTasks[n] = pTask; }

    //! Notify strip task that the pot it owns has changed value.
    //! Also sends a MIDI message if the value is different from the previous one.
    void notifyOwner(int n) //!< pot instance index
    { 
        static elapsedMillis lastMIDI = 0;
        if (nullptr != stripTasks[n]) 
            stripTasks[n]->potChanged(); 

        // this will actually use scene settings...
        int newValue = (int) roundf(allPots[n].getCurrent() * 10000.0f);
        if (midiReqs[n].lastValue != newValue && lastMIDI >= 10)
        {
            lastMIDI = 0; // try not to overwhelm Serial (for now)
            midiReqs[n].lastValue = newValue;
            MIDImessage msg{42, (n+1)*111, newValue};
            midiTask.sendMIDI(midiReqs[n].req, msg);
        }
    }
};


//                      888    888    d8b                            
//                      888    888    Y8P                            
//                      888    888                                   
//    .d8888b   .d88b.  888888 888888 888 88888b.   .d88b.  .d8888b  
//    88K      d8P  Y8b 888    888    888 888 "88b d88P"88b 88K      
//    "Y8888b. 88888888 888    888    888 888  888 888  888 "Y8888b. 
//         X88 Y8b.     Y88b.  Y88b.  888 888  888 Y88b 888      X88 
//     88888P'  "Y8888   "Y888  "Y888 888 888  888  "Y88888  88888P' 
//                                                      888          
//                                                 Y8b d88P          
//                                                  "Y88P"           
//
class FaderMonsterSettings
{
  public:
    StripConfig stripsConfig[NUM_POTS];
    colours_t mainColours;
};
#endif // !defined(_CLASSES_H_)
