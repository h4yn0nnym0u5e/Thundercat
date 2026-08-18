#if !defined(_CLASSES_H_)
#define _CLASSES_H_

//                                                        888    
//                                                        888    
//                                                        888    
//    888d888 .d88b.   .d88888 888  888  .d88b.  .d8888b  888888 
//    888P"  d8P  Y8b d88" 888 888  888 d8P  Y8b 88K      888    
//    888    88888888 888  888 888  888 88888888 "Y8888b. 888    
//    888    Y8b.     Y88b 888 Y88b 888 Y8b.          X88 Y88b.  
//    888     "Y8888   "Y88888  "Y88888  "Y8888   88888P'  "Y888 
//                         888                                   
//                         888                                   
//                         888                                   
//
/**
 * Request from one "client" task for a function to be run
 * by another "server" task. Typically the server will
 * provide exclusive access to some piece of hardware, 
 * e.g. a SPI or I²C bus
 */
class InterTaskRequest
{
  public:
    //! status / result of request
    enum Result {inactive,  //!< no request made
                 pending,   //!< request awaiting attention from server task
                 running,   //!< request being executed by server task
                 done,      //!< request completed successfully
                 failed     //!< request failed 
                } status;
    // performance measuring                 
    uint32_t requested, //!< timestamp when request was made (microseconds)
             executed,  //!< timestamp when request started execution (microseconds)
             finished;  //!< timestamp when request execution completed (microseconds)

    //! construct in inactive state
    InterTaskRequest(void)
    : status{inactive}
    {}

    //! force status to inactive
    void setInactive(void) { status = inactive; }

    //! \return true if request is busy
    bool isBusy(void) { return pending == status || running == status; }

    //! \return true if request has completed
    bool isFinished(void) { return done == status || failed == status; }

    //! \return true if request is not busy
    bool isInactive(void) { return inactive == status || done == status || failed == status; }

    // instrumentation: find out how long various things took
    //! \return time taken between request and execution start (microseconds)
    uint32_t responseTime(void)  { return executed - requested; } // delay due to handing off to another task
    
    //! \return time taken to execute request once started (microseconds)
    uint32_t executionTime(void) { return finished - executed;  } // actual time taken to do it
    
    //! \return time taken between request and execution completion (microseconds)
    uint32_t overallTime(void)   { return finished - requested; } // 
};

//                              .d88888b.                                      
//                             d88P" "Y88b                                     
//                             888     888                                     
//    888d888 .d88b.   .d88888 888     888 888  888  .d88b.  888  888  .d88b.  
//    888P"  d8P  Y8b d88" 888 888     888 888  888 d8P  Y8b 888  888 d8P  Y8b 
//    888    88888888 888  888 888 Y8b 888 888  888 88888888 888  888 88888888 
//    888    Y8b.     Y88b 888 Y88b.Y8b88P Y88b 888 Y8b.     Y88b 888 Y8b.     
//    888     "Y8888   "Y88888  "Y888888"   "Y88888  "Y8888   "Y88888  "Y8888  
//                         888        Y8b                                      
//                         888                                                 
//                         888                                                 
// 
/**
 * Wrapper for passing requests between tasks using FreeRTOS queues.
 * A task class contains a RequestQueue, and other tasks send requests
 * using methods of the class - the exact payload of the request is
 * opaque to the caller.
 */
template <class T, class P>
class RequestQueue
{
    QueueHandle_t queue;
  public:
    struct queueEntry 
    {
        InterTaskRequest* req;  //!< pointer to structure holding request status / instrumentation
        P payload; //!< request content
    };

    RequestQueue(int length) { queue = xQueueCreate(length, sizeof(queueEntry)); }

    //! Send a request.
    //!
    //! note that items are queued by copying their value, so the req 
    //! parameter can disappear once the request has been queued
    //!
    //! fails if request instance is already busy, or can't add it to the queue
    InterTaskRequest::Result request(queueEntry& req, TickType_t timeout = 0) 
    { 
        InterTaskRequest::Result result = InterTaskRequest::Result::pending;
        if (req.req->isInactive())
        {
            //char* callerName = pcTaskGetName(nullptr);
            //Serial.printf("[%u]: %s sent req at %08X\n", micros(), callerName, (uint32_t) req.req);
            req.req->status = result;
            req.req->requested = micros();
            if (pdPASS == xQueueSend(queue, &req, timeout))
                result = req.req->status; // may execute immediately!
            else
                result = InterTaskRequest::Result::failed;                
        }
        return result; 
    }
    
    //! receive a request
    BaseType_t getRequest(queueEntry* req, int timeout)
    {
        BaseType_t result = xQueueReceive(queue, req, timeout);
        if (pdPASS == result)
        {
            //char* callerName = pcTaskGetName(nullptr);
            //Serial.printf("[%u]: %s received req at %08X ... ", micros(), callerName, (uint32_t) req->req);
        }
        return result;
    }

    //! retrieve a request's payload.
    // assumes it's all done with
    BaseType_t getPayload(P& payload, int timeout, uint32_t* pRequested = nullptr)
    {
        queueEntry entry;

        BaseType_t result = xQueueReceive(queue, &entry, timeout);
        if (pdPASS == result)
        {
            payload = entry.payload;
            entry.req->executed = micros();
            entry.req->finished = micros();
            entry.req->status = InterTaskRequest::Result::done; 
            if (nullptr != pRequested)
                *pRequested = entry.req->requested;
        }
        return result;
    }


    //! De-queue a request and execute it.
    //! Polled in task's loop; does nothing if no requests are pending
    InterTaskRequest::Result executeRequest(T& instance, int timeout)
    {
        InterTaskRequest::Result result = InterTaskRequest::Result::inactive; // did nothing
        queueEntry entry;
        if (pdPASS == getRequest(&entry, timeout))
        {
            entry.req->executed = micros();
            //Serial.println("execute");
            result = (instance.*entry.payload.requestExecutor)(entry.payload.context);
            entry.req->finished = micros();
            entry.req->status = result; 
        }

        return result;
    }

    //! \return count of messages waiting in the request queue
    UBaseType_t messagesWaiting(void) { return uxQueueMessagesWaiting(queue); }
};

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
class AT42QT2120
{
public:    
    TwoWire& theWire;
    int touch_addr; // stops warning about ambiguous call
    uint8_t status[6];
    uint16_t oldK, newK, chg, mask;
  public:
    AT42QT2120(TwoWire& _wire, uint8_t _addr, uint16_t _mask) 
        : theWire{_wire}, touch_addr{_addr}, mask{_mask} 
        {}
    bool probe(void);
    void prepReadKeys(void);        
    void readKeys(void);
    void calibrate(void);
    void setTouchRecalDelay(uint8_t theDelay);

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
    void loopFn(void);
    InterTaskRequest touchCalibrationRequest;

  public:
    SuperTask(const char* _name, 
              configSTACK_DEPTH_TYPE _stackDepth = 512, 
              void* _params = nullptr,
              UBaseType_t _priority = 2)
    : FaderMonsterTask{_name, _stackDepth, _params, _priority}
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

    void updateTouch(void);
    void pollTouch(void);
    void updateKeyStatuses(AT42QT2120& touch);

  public:
    TouchTask(const char* _name, 
              configSTACK_DEPTH_TYPE _stackDepth, 
              void* _params,
              UBaseType_t _priority,

              int _queueLength,
              AT42QT2120& _atq
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
    
    AT42QT2120& touchChip;
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
 */
struct colours_t {
    uint16_t fg,bg,txt;
};

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
    float lastPot;
    TouchStatus::eStatus lastTouch; // previous extended status
    int   spaceOffset;
    char  lastString[BUF_SIZE]{0};
    enum ScribbleState {done, start, arc, text, touch} scribbleState;

    InterTaskRequest updateReq; // used to monitor progress of display update

    // ring LEDs
    void setDot(float value, uint32_t colour);
    void setDotCurrent(void) { setDot(pot.getCurrent(), useRingPattern?RingLEDs<NUM_POTS>::USE_PATTERN:cfg.ringLEDs.colour); }

    // sprite (TFT)
    void drawArc(TFT_TYPE& tft, float s, float e, uint16_t fg, uint16_t bg);
    void drawTouch(TFT_TYPE& tft, colours_t& colours, int thickness = -1);
    void drawTouch(TFT_TYPE& tft, TouchStatus::eStatus estatus, colours_t& colours);
    bool setArc(TFT_eSprite& tft, float newPot, colours_t& colours);
    bool setText(TFT_eSprite& sprite, char* buf, colours_t& colours);
    bool setFloat(TFT_eSprite& sprite, float value, colours_t& colours);
    bool setTouch(TFT_eSprite& sprite, TouchStatus& touch, colours_t& colours);

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
      lastPot{POT_NOT_SET}, lastTouch{false}, spaceOffset{0},
      scribbleState{done},
      num{_num},
      useRingPattern{false}
    {}
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
};
#endif // !defined(_CLASSES_H_)
