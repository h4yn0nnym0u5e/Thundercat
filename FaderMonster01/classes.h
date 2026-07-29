#if !defined(_CLASSES_H_)
#define _CLASSES_H_

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
    int32_t requested, executed, finished;

  private:
    Result (*fn)(void*);
    void* context;
  public:
    InterTaskRequest(Result (*_fn)(void*), void* _context)
    : status{inactive}, fn{_fn}, context{_context}
    {}
    InterTaskRequest() : InterTaskRequest(nullptr,nullptr) {}

    Result execute(void* ctxt) 
    { 
        status = running; 
        executed = micros();
        status = (*fn)(ctxt);
        finished = micros();
        return status;
    }
    Result execute(void) { return execute(context); }
    void setContext(void* ctxt) { context = ctxt; }
    void setInactive(void) { status = inactive; }
    bool isBusy(void) { return pending == status || running == status; }
    bool isFinished(void) { return done == status || failed == status; }
    bool isInactive(void) { return inactive == status || done == status || failed == status; }

    // instrumentation: find out how long various things took
    uint32_t responseTime(void)  { return executed - requested; } // delay due to handing off to another task
    uint32_t executionTime(void) { return finished - executed;  } // actual time taken to do it
    uint32_t overallTime(void)   { return finished - requested; } // 
};

/*
 * Wrapper for passing requests between tasks using FreeRTOS queues.
 * A task class is based on a RequestQueue, and other tasks send requests
 * using <taskInstance>.request()
 */
class RequestQueue
{
    public:
    QueueHandle_t queue;
  public:
    RequestQueue(int length) { queue = xQueueCreate(length, sizeof(InterTaskRequest*)); }

    // send a request
    // fails if request instance is already busy, or can't add it to the queue
    InterTaskRequest::Result request(InterTaskRequest& req, TickType_t timeout = 0) 
    { 
        InterTaskRequest* preq = &req;
        InterTaskRequest::Result result = InterTaskRequest::Result::failed;
        if (req.isInactive() && pdPASS == xQueueSend(queue, &preq, timeout))
        {
            req.status = result = InterTaskRequest::Result::pending;
            req.requested = micros();
        }
        return result; 
    }
    
    BaseType_t getRequest(InterTaskRequest** req, int timeout)
    {
        *req = (InterTaskRequest*) 0x1234;
        BaseType_t result = xQueueReceive(queue, req, timeout);
        return result;
    }

    InterTaskRequest::Result executeRequest(int timeout)
    {
        InterTaskRequest::Result result = InterTaskRequest::Result::inactive;
        InterTaskRequest* preq = nullptr;
        if (pdPASS == getRequest(&preq, timeout))
        {
            if (nullptr != preq)
            {
                InterTaskRequest& req = *preq;
                result = req.execute(); // execute() sets internal status
            }
        }

        return result;
    }
};


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

        // Inter-task request
        // The supplied function is run in the context of the called task, using
        // the supplied "context" information. By default it fails, but can
        // be overridden by tasks which support requests. In general the request
        // will be queued; the called function can use data from the context to
        // signal completion, or the caller can just poll.
        virtual InterTaskRequest::Result request(InterTaskRequest& req, TickType_t timeout=0) { return InterTaskRequest::Result::failed; };

        // Code that actually runs the task
        virtual void run(void) = 0;

        TaskHandle_t handle;
        void* params;
};

/*
class SuperTask : public FaderMonsterTask
{ 
  public:
    SuperTask(const char* _name, 
              configSTACK_DEPTH_TYPE _stackDepth = 512, 
              void* _params = nullptr,
              UBaseType_t _priority = 2)
    : FaderMonsterTask{_name, _stackDepth, _params, _priority}
    {}
    
    void run(void) override;
};
*/


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


class TouchTask : public FaderMonsterTask, public RequestQueue
{
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
              RequestQueue(_queueLength), touchChip{_atq}
        {}
    void run(void) override;
            
    InterTaskRequest::Result request(InterTaskRequest& req, TickType_t timeout=0) override
        { return RequestQueue::request(req, timeout); };
    InterTaskRequest::Result requestCalibration(InterTaskRequest* req = nullptr); 
    
    AT42QT2120& touchChip;
    bool supplyValid{false};
    bool checkChange{true}; // public: set by ISR
};


#endif // !defined(_CLASSES_H_)
