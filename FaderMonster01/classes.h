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
/*
 * Request from one "client" task for a function to be run 
 * by another "server" task. Typically the server will
 * provide exclusive access to some piece of hardware, 
 * e.g. a SPI or I²C bus
 */
class InterTaskRequest
{
  public:
    enum Result {inactive, pending, running, done, failed} status;
    int32_t requested, executed, finished; // performance measuring

    InterTaskRequest(void)
    : status{inactive}
    {}

    void setInactive(void) { status = inactive; }
    bool isBusy(void) { return pending == status || running == status; }
    bool isFinished(void) { return done == status || failed == status; }
    bool isInactive(void) { return inactive == status || done == status || failed == status; }

    // instrumentation: find out how long various things took
    uint32_t responseTime(void)  { return executed - requested; } // delay due to handing off to another task
    uint32_t executionTime(void) { return finished - executed;  } // actual time taken to do it
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
/*
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
        InterTaskRequest* req;
        P payload;
    };

    RequestQueue(int length) { queue = xQueueCreate(length, sizeof(queueEntry)); }

    // send a request
    //
    // note that items are queued by copying their value, so the req 
    // parameter can disappear once the request has been queued
    //
    // fails if request instance is already busy, or can't add it to the queue
    InterTaskRequest::Result request(queueEntry& req, TickType_t timeout = 0) 
    { 
        InterTaskRequest::Result result = InterTaskRequest::Result::failed;
        if (req.req->isInactive() && pdPASS == xQueueSend(queue, &req, timeout))
        {
            Serial.printf("sent req at %08X\n", (uint32_t) req.req);
            req.req->status = result = InterTaskRequest::Result::pending;
            req.req->requested = micros();
        }
        return result; 
    }
    
    // receive a request
    BaseType_t getRequest(queueEntry* req, int timeout)
    {
        BaseType_t result = xQueueReceive(queue, req, timeout);
        if (pdPASS == result)
            Serial.printf("received req at %08X\n", (uint32_t) req->req);
        return result;
    }

    // polled in task's loop
    InterTaskRequest::Result executeRequest(T& instance, int timeout)
    {
        InterTaskRequest::Result result = InterTaskRequest::Result::inactive; // did nothing
        queueEntry entry;
        if (pdPASS == getRequest(&entry, timeout))
        {
            entry.req->executed = micros();
            Serial.println("execute");
            result = (instance.*entry.payload.requestExecutor)(entry.payload.context);
            entry.req->finished = micros();
            entry.req->status = result; 
        }

        return result;
    }

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
    //------------------------------------------------------------------------
    static StripTask* tasks[NUM_POTS];

    StripConfig& cfg;

    LEDring<NUM_POTS> ring;   // our LED ring
    ContinuousPot& pot;       // ...continuous pot...
    TFT_eSprite& scribble;        // ...scribble strip display
    // Fader& fader;            // ...fader
    // Button& button;          // ...button (control and LED)

    static constexpr float POT_NOT_SET = -999.0f;
    static constexpr float sa = 2*18.0f, ea = 360.0f - 2*18.0f; // TFT_eSPI has zero at 6 o'clock
    float lastPot;
    bool  lastTouch;
    int   spaceOffset;
    enum ScribbleState {done, start, arc, text, touch} scribbleState;

    InterTaskRequest updateReq; // used to monitor progress of display update

    // ring LEDs
    void setDot(float value, uint32_t colour);

    // sprite (TFT)
    void drawArc(TFT_TYPE& tft, float s, float e, uint16_t fg, uint16_t bg);
    void drawTouch(TFT_TYPE& tft, uint16_t colour);
    bool setArc(TFT_TYPE& tft, float newPot, colours_t& colours);
    void setText(TFT_eSprite& sprite, char* buf, colours_t& colours);
    void setFloat(TFT_eSprite& sprite, float value, colours_t& colours);
    bool setTouch(TFT_eSprite& sprite, bool touch, colours_t& colours);

  public:
    StripTask(const char* _name, 
              configSTACK_DEPTH_TYPE _stackDepth, 
              void* _params,
              UBaseType_t _priority,
            
              int _num, int _reqQlen,
              RingLEDs<NUM_POTS>& _rings,
              ContinuousPot& _pot,
              TFT_eSPI& _tft,
              StripConfig& _cfg
            )
    : FaderMonsterTask{_name, _stackDepth, _params, _priority},
      reqQueue{_reqQlen}, cfg{_cfg},
      ring{LEDring{_rings,_num}}, pot{_pot}, 
      scribble{*new TFT_eSprite{&_tft}}, 
      lastPot{POT_NOT_SET}, lastTouch{false}, spaceOffset{0},
      num{_num},
      bright{39}, useRingPattern{false}
    {}
    static void CreateTasks(void);
    
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

    int num; // which strip this is (0-7)
    int bright;
    bool useRingPattern;
    InterTaskRequest displayReq, potReq;
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
// pots, faders and port expanders are all on the
// same SPI bus, so need to be dealt with together
class PotsTask : public FaderMonsterTask
{ 
    static ContinuousPot allPots[NUM_POTS];
    static StripTask* stripTasks[NUM_POTS];

    void updateADCs(void);
    float raw2volts(uint16_t raw) { return (float) raw / 65535.0f * 5.0f; }

  public:
    PotsTask(const char* _name, 
              configSTACK_DEPTH_TYPE _stackDepth, 
              void* _params,
              UBaseType_t _priority)
    : FaderMonsterTask{_name, _stackDepth, _params, _priority}
    {}
    
    void run(void) override;
    ContinuousPot& getPot(int n) { return allPots[n]; }
    void setOwner(StripTask* pTask, int n) { stripTasks[n] = pTask; }
    void notifyOwner(int n) { stripTasks[n]->potChanged(); }
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
