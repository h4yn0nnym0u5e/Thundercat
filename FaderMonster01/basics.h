#if !defined(_BASICS_H_)
#define _BASICS_H_

// Essentials needed for various classes
//=================================================
// Hacky information functions
struct LibInfo 
{
  const char *lib,*path,*version,*sha,*branch;
  int modified;
};

struct SketchInfo 
{
  const char *path,*sha,*branch;
  int modified;
};

//=================================================
//#if !defined(constrain)
template<typename T> 
T constrain(T v, T l, T u)
{
    T result = v;
    if (v<l) result = l;
    if (v>u) result = u;
    return result;
}
//#endif // !defined(constrain)

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
                 failed,    //!< request failed 
                 qfull      //!< queue was full
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
    bool isFinished(void) { return done == status || failed == status || qfull == status; }

    //! \return true if request has failed
    bool isFailed(void) { return failed == status || qfull == status; }

    //! \return true if request is not busy
    bool isInactive(void) { return inactive == status || done == status || failed == status || qfull == status; }

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
        InterTaskRequest::Result result = InterTaskRequest::Result::failed;
        if (req.req->isInactive())
        {
            //char* callerName = pcTaskGetName(nullptr);
            //Serial.printf("[%u]: %s sent req at %08X\n", micros(), callerName, (uint32_t) req.req);
            req.req->status = InterTaskRequest::Result::pending;
            req.req->requested = micros();
            if (pdPASS == xQueueSend(queue, &req, timeout))
                result = req.req->status; // may execute immediately!
            else
                result = InterTaskRequest::Result::qfull;                
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

    char xChar{0}; // debug only - set to character to monitor when execution occurs
    //! De-queue a request and execute it.
    //! Polled in task's loop; does nothing if no requests are pending
    InterTaskRequest::Result executeRequest(T& instance, int timeout)
    {
        InterTaskRequest::Result result = InterTaskRequest::Result::inactive; // did nothing
        queueEntry entry;
        if (pdPASS == getRequest(&entry, timeout))
        {
            entry.req->executed = micros();
            if (xChar)
            {
                Serial.print(xChar);
            }
            entry.req->status = InterTaskRequest::Result::running; 
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

//     .d8888b.                                 888    888    d8P                    888      
//    d88P  Y88b                                888    888   d8P                     888      
//    Y88b.                                     888    888  d8P                      888      
//     "Y888b.   88888b.d88b.   8888b.  888d888 888888 888d88K     88888b.   .d88b.  88888b.  
//        "Y88b. 888 "888 "88b     "88b 888P"   888    8888888b    888 "88b d88""88b 888 "88b 
//          "888 888  888  888 .d888888 888     888    888  Y88b   888  888 888  888 888  888 
//    Y88b  d88P 888  888  888 888  888 888     Y88b.  888   Y88b  888  888 Y88..88P 888 d88P 
//     "Y8888P"  888  888  888 "Y888888 888      "Y888 888    Y88b 888  888  "Y88P"  88888P"  
// 
struct SmartKnobReport
{
    uint32_t ms;
    int32_t position;
    float   sub_position;
    bool    isInteger;
    int32_t min, max; // min and max values
};

struct SmartKnobConfigStackEntry
{
    PB_SmartKnobConfig config;
};

//! record of MIDI request sent for each controller
struct MIDIreq
{
    InterTaskRequest req;   //!< request status
    int lastValue;          //!< last-sent value (float * 10000)
    elapsedMicros lastTime; //!< time since last sent
    float pendingValue;     //!< value if we couldn't send it
    bool  isPending{false};
};

//     .d888                  888             
//    d88P"                   888             
//    888                     888             
//    888888 .d88b.  88888b.  888888 .d8888b  
//    888   d88""88b 888 "88b 888    88K      
//    888   888  888 888  888 888    "Y8888b. 
//    888   Y88..88P 888  888 Y88b.       X88 
//    888    "Y88P"  888  888  "Y888  88888P' 
//
class FontSetter
{
  public:    
    static void setFreeFont(TFT_eSprite& spr, const GFXfont *f = NULL)
    {
        spr.unloadFont(); // this masks the free font, so unload it
        spr.setFreeFont(f);
    }

    static void loadFont(TFT_eSprite& spr, const uint8_t array[])
    {
        spr.loadFont(array); // masks free font; unload should restore it
    }

    static void unloadFont(TFT_eSprite& spr)
    {
        spr.unloadFont(); // this masks the free font, so unload it
    }

    static constexpr const uint8_t* SLIDER_LABEL{NotoSansRegular14};
    static constexpr const uint8_t* RADIO_LABEL{NotoSansRegular16};
    static constexpr const uint8_t* TEXT_ENTRY{NotoSansRegular20};
    static constexpr const uint8_t* TITLE_BAR{NotoSansRegular24};
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
#include <string.h>
extern const char* MIDIcontrolNames[];
/*
 * Configuration for a strip
 * This is outside the strip, as we want a convenient method to
 * load it from the filesystem
 */
 
//! pattern for one LED ring 
class pattern_t
{
    int values[LEDS_PER_RING];
  public:
    pattern_t(int c) : values{c} {}
    pattern_t(int c, int c2) : values{c,c2} {}
    pattern_t() = default; // {}
    int& operator[](int n) { return values[n]; }
    int* getPointer(void) { return &values[0]; }
};

//! what we need to know about a setting item to read or write it
typedef struct 
        {
            int offset; //!< offset of member in class
            int count;  //!< number of items in an array member
            int size;   //!< size of entry in array member
            bool (*setter)(void* dst, const char* src); //!< function to convert string into structure value
            bool (*getter)(char* dst, void* src); //!< retrieve value in string format
        } 
        offsetResult;

//! Class for saving and loading settings
class CfgBaseOffset
{
  public:
    //! Find offset of class member given its name as a string
    virtual offsetResult toOffset(const char* str, int& consume) = 0;
    //! Get names of class and its members
    virtual const char* getName(int n) = 0;
    //! Get number of members in class
    virtual int getMemberCount(void) = 0;
};

//! List of MIDI control type macros
#define MIDI_CONTROLS \
    MIDI_CONTROL(undefined,undefined,0) \
    MIDI_CONTROL(CC,CC,1) \
    MIDI_CONTROL(RPN,RPN,2) \
    MIDI_CONTROL(NRPN,NRPN,3) \
    MIDI_CONTROL(BEND,Bend,4) \
    MIDI_CONTROL(PC,Prog,5) \
    MIDI_CONTROL(AT,AfTch,6) \
    MIDI_CONTROL(NOTE,Note,7)
    
//! Possible MIDI control types
enum class MIDIcontrolType : int 
{
#define MIDI_CONTROL(enm,txt,num)  enm = num,
    MIDI_CONTROLS
#undef MIDI_CONTROL    
/*
    undefined = 0,
    CC = 1,     //!< control change
    RPN = 2,    //!< registered parameter number
    NRPN = 3,   //!< non-registered parameter number
    BEND = 4,   //!< pitch bend
    PC = 5,     //!< program change
    AT = 6,     //!< aftertouch
    NOTE = 7    //!< note on / off (button)
*/    
};

#define TO_OFFSET(mbr) \
        { int mbrlen = strlen(#mbr); \
        if (0 == strncmp(str, #mbr, mbrlen) && ('.' == str[mbrlen] || 0 == str[mbrlen])) \
            { result.offset = (char*) &mbr - (char*) this; consumed += mbrlen+1; str += mbrlen; \
              if (0 != *str) { offsetResult extra = mbr.toOffset(str+1, consume); \
                               if (extra.offset >= 0) extra.offset += result.offset; \
                               result = extra; } \
              break; }}

#define TO_OFFSET_LEAF(mbr, typ) \
        { int mbrlen = strlen(#mbr); \
        if (0 == strncmp(str, #mbr, mbrlen) && ('.' == str[mbrlen]  || 0 == str[mbrlen])) \
            { result.offset = (char*) &mbr - (char*) this; consumed += mbrlen+1; str += mbrlen;\
              result.getter = get##typ; result.setter = set##typ;    \
                break; }}

#define TO_OFFSET_ARRAY(mbr) \
        { int mbrlen = strlen(#mbr); \
        if (0 == strncmp(str, #mbr, mbrlen) && ('.' == str[mbrlen] || 0 == str[mbrlen])) \
        { \
            result.offset = (char*) &mbr - (char*) this; consumed += mbrlen+1; str += mbrlen; \
            result.size = sizeof mbr[0]; result.count = sizeof mbr / result.size; \
            if (0 == *str) break; /* isn't .n. : still useful */ \
            int index, n; n = sscanf(str+1,"%d%n",&index,&mbrlen); \
            if (n<1) { result.offset = -1; break; } else { result.offset += index*(sizeof mbr[0]); str += mbrlen+1; } \
            if (0 != *str) { offsetResult extra = mbr[0].toOffset(str+1, consume); \
                             if (extra.offset >= 0) extra.offset += result.offset; \
                             result = extra; } \
            break; }}

// from configMaker.py:
#include "settings.h"

typedef TFTcolours colours_t;

//================================================================
//
//    d8b                                                   
//    Y8P                                                   
//                                                          
//    888 88888b.d88b.   8888b.   .d88b.   .d88b.  .d8888b  
//    888 888 "888 "88b     "88b d88P"88b d8P  Y8b 88K      
//    888 888  888  888 .d888888 888  888 88888888 "Y8888b. 
//    888 888  888  888 888  888 Y88b 888 Y8b.          X88 
//    888 888  888  888 "Y888888  "Y88888  "Y8888   88888P' 
//                                    888                   
//                               Y8b d88P                   
//                                "Y88P"                    
//
typedef unsigned char image_4bit_data;
typedef struct 
{
    int width, height;
    const image_4bit_data* data;
} image_4bit_info;

#define IMAGES_AS_HEADER
#include "images.cpp"
#undef IMAGES_AS_HEADER


//================================================================
//
//    888                     888    d8b  .d888 d8b                  
//    888                     888    Y8P d88P"  Y8P                  
//    888                     888        888                         
//    888888 .d88b.  888  888 888888 888 888888 888  .d88b.  888d888 
//    888   d8P  Y8b `Y8bd8P' 888    888 888    888 d8P  Y8b 888P"   
//    888   88888888   X88K   888    888 888    888 88888888 888     
//    Y88b. Y8b.     .d8""8b. Y88b.  888 888    888 Y8b.     888     
//     "Y888 "Y8888  888  888  "Y888 888 888    888  "Y8888  888     
//
class Textifier
{
  public:
    static constexpr size_t MAX_STRING{30+1};
  private:    
    char buf[MAX_STRING];
    char* target; // assumed at least big enough for buffer!
    size_t offset{0};
    size_t max_string;
    bool ok{true};

  public:    
    Textifier(char* _target, size_t _max = MAX_STRING) 
        : target{_target},
          max_string{_max < MAX_STRING?_max:MAX_STRING}
        {}

    /**
     * Overwrite target with possibly new string.
     * \return true if new string is well-formed and different
     */
    bool toTarget(void)
    {
        bool result = false;
        if (ok && 0 != strcmp(target, buf))
        {
            strcpy(target,buf);
            result = true;
        }
        return result;
    }

    Textifier& setPadding(TFT_eSprite* pSprite, int extra = 3)
    {
        int16_t oldWidth = pSprite->textWidth(target);
        int16_t newWidth = pSprite->textWidth(buf);
        if (newWidth < oldWidth)
        {
            pSprite->setTextPadding(oldWidth+extra); // occasional dropped pixels - hack!
        }
        return *this;
    }

    char* get(void) { return target; }
    char* getBuffer(void) { return buf; }

    Textifier& z(void) 
    {
        offset = 0;
        *buf = 0;
        return *this;
    }

    Textifier& s(const char* _s) 
    { 
        size_t l = strlen(_s);
        if (l <= max_string-offset)
        {
            strcpy(buf+offset, _s);
            offset += l;
        }
        else
            ok = false;

        return *this;
    }

    Textifier& del(size_t n = 1) 
    {
        if (n > offset)
            offset = 0;
        else
            offset -= n;            
        buf[offset] = 0;
        return *this;
    }


    Textifier& cc(MIDIcontrolType cType, int ccNum) 
    { 
        s(MIDIcontrolNames[(int) cType]);
        switch (cType)
        {
            default: break;

            case MIDIcontrolType::CC:
            case MIDIcontrolType::RPN:
            case MIDIcontrolType::NRPN:
                c(' ').u(ccNum);
                break;

            case MIDIcontrolType::NOTE:
                c(' ').n(ccNum);
                break;
        }
        return *this;
    }


    Textifier& i(int _i) 
    { 
        char _s[20];
        sprintf(_s,"% d", _i);
        return s(_s);
    }

    Textifier& u(int _i) 
    { 
        char _s[20];
        sprintf(_s,"%d", _i);
        return s(_s);
    }

    /**
     * Create a string from a MIDI note number
     */
    Textifier& n(int _n) 
    { 
        char _s[20];
        int note = _n % 12; // 0 = C etc
        int octave = (_n / 12) - 1;
        const char names[]{"C C#D D#E F F#G G#A A# B "};
        sprintf(_s,"%.2s", names+note*2);
        sprintf(_s+(' ' == _s[1]?1:2),"%d",octave);
        return s(_s);
    }

    Textifier& c(char _c) 
    { 
        char _s[2]{_c,0};
        return s(_s);
    }

    Textifier& f(float _f, const char* _fmt = "%.3f") 
    { 
        char _s[20];
        sprintf(_s,_fmt, _f);
        return s(_s);
    }

    Textifier& x4(int _i) 
    { 
        char _s[20];
        sprintf(_s,"%04X", _i & 0xFFFF);
        return s(_s);
    }  
};

#endif // !defined(_BASICS_H_)
