#if !defined(_BASICS_H_)
#define _BASICS_H_

// Essentials needed for various classes

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
    InterTaskRequest(Result r) : status{r} {}
    InterTaskRequest(void) : InterTaskRequest{inactive} {}

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


struct colours_t {
    uint16_t fg,bg,txt;
};

#endif // !defined(_BASICS_H_)
