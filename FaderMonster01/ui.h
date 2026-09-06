#if !defined(_UI_H_)
#define _UI_H_

#if 1

#include <TFT_eSPI.h>

#define MAX_TEXT_LEN 30
//==================================================================
//
//    888            d8b                                    
//    888            Y8P                                    
//    888                                                   
//    888888 888d888 888  .d88b.   .d88b.   .d88b.  888d888 
//    888    888P"   888 d88P"88b d88P"88b d8P  Y8b 888P"   
//    888    888     888 888  888 888  888 88888888 888     
//    Y88b.  888     888 Y88b 888 Y88b 888 Y8b.     888     
//     "Y888 888     888  "Y88888  "Y88888  "Y8888  888     
//                            888      888                  
//                       Y8b d88P Y8b d88P                  
//                        "Y88P"   "Y88P"                   
// 
// Define list of things that might 
// trigger a change to a display:
#define TRIGGER_LIST \
    TRIGGER_ITEM(int, nextPhase) \
    TRIGGER_ITEM(float, potValue) \
    TRIGGER_ITEM(float, faderValue) \
    TRIGGER_ITEM(GTPoint, touchPoint) \
    TRIGGER_ITEM(TouchStatus*, pTouchStatus) 

union UpdateTrigger
{
#define TRIGGER_ITEM(typ,nam) typ nam;
    TRIGGER_LIST
#undef TRIGGER_ITEM    
};

struct Trigger
{
    enum class eTriggerType {
#define TRIGGER_ITEM(typ,nam) nam,
    TRIGGER_LIST
#undef TRIGGER_ITEM    
    } type;
    UpdateTrigger trigger;
};

//==================================================================
//
//    888                                 
//    888                                 
//    888                                 
//    88888b.   8888b.  .d8888b   .d88b.  
//    888 "88b     "88b 88K      d8P  Y8b 
//    888  888 .d888888 "Y8888b. 88888888 
//    888 d88P 888  888      X88 Y8b.     
//    88888P"  "Y888888  88888P'  "Y8888  
//  
class UIclass
{
    static InterTaskRequest autoFail;
  protected:
    TFT_eSprite* pSprite;
    colours_t    colours;
    int          phase;
    Trigger      currentTrigger;
    elapsedMicros interval; // polling interval timer

    // standard drawing methods
    void drawArc(float s, float e, uint16_t fg, uint16_t bg);
    void drawTouchEllipse(int thickness = -1);
    void drawTouch(TouchStatus::eStatus estatus);

    // higher-level methods: can tell when item has changed
    bool setArc(float newPot, float& lastPot);
    bool setText(char* buf, char* lastString, size_t sizeofLastString);
    bool setFloat(float potPos, char* lastString, size_t sizeofLastString);
    bool setTouch(TouchStatus& touch, TouchStatus::eStatus& lastTouch);

    // display writing methods
    InterTaskRequest& writeToMainLCD(void);
    InterTaskRequest& writeToScribble(void);
    
  public:
    //! possible UI drawing states
    enum class State {done, //! UI update from currentTrigger is complete
                      next, //! no change to display, can do next step    
                      push, //! sprite changed, can push to display
                      busy  //! busy pushing to display
                     } state;
    virtual State begin(TFT_eSprite& sprite, colours_t c) 
        { pSprite = &sprite; colours = c;   return (state = State::done); }
    virtual State update(Trigger trigger) { return (state = State::done); }
    virtual InterTaskRequest& writeToDisplay(void) { return autoFail; }
    virtual uint32_t poll(void) { uint32_t result = interval; interval = 0; return result; }

    bool writeFinished(void);
    bool isDirty(void) { return pSprite->isDirty(); }

    static constexpr float POT_NOT_SET{-999.0f};
    static constexpr float sa{2*18.0f}, ea{360.0f - 2*18.0f}; // TFT_eSPI has zero at 6 o'clock
    static constexpr int BUF_SIZE{MAX_TEXT_LEN};

    InterTaskRequest updateReq;
};


//==================================================================
//
//                              d8b 888      888      888          
//                              Y8P 888      888      888          
//                                  888      888      888          
//    .d8888b   .d8888b 888d888 888 88888b.  88888b.  888  .d88b.  
//    88K      d88P"    888P"   888 888 "88b 888 "88b 888 d8P  Y8b 
//    "Y8888b. 888      888     888 888  888 888  888 888 88888888 
//         X88 Y88b.    888     888 888 d88P 888 d88P 888 Y8b.     
//     88888P'  "Y8888P 888     888 88888P"  88888P"  888  "Y8888  
// 
class ScribbleDummy : public UIclass {};
class ScribblePotArc : public UIclass 
{
    enum {idle, drawingArc, drawingNumber, drawingTouch, drawingBackground};
    char  lastText[MAX_TEXT_LEN]{0};
    float lastPot{POT_NOT_SET};
    TouchStatus::eStatus lastTouch{};

  public:
    virtual State begin(TFT_eSprite& sprite, colours_t c);
    virtual State update(Trigger trigger);
    virtual InterTaskRequest& writeToDisplay(void) { return writeToScribble(); }
};

//==================================================================
union ScribbleUI
{
    ScribbleDummy  dummy;
    ScribblePotArc potArc;
};

union ScribbleUIholder
{
    long long aligner;
    uint8_t space[sizeof(ScribbleUI)];
};


#endif // 0

#endif // !defined(_UI_H_)
