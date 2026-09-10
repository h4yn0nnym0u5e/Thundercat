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
    bool setTouch(TouchStatus* pTouch, TouchStatus::eStatus& lastTouch);

    uint16_t* makeCmap(uint16_t* cmap, uint16_t fg, uint16_t bg);
    void drawButton(int x, int y,
                    const image_4bit_info& img, const uint16_t* cmap,
                    const char* txt, int xoff, int yoff);
    bool isNewTouch(Trigger trigger);

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
        { 
            pSprite = &sprite; 
            colours = c;   

            pSprite->setTextDatum(TL_DATUM);
            pSprite->resetViewport();

            return (state = State::done); 
        }
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
class ScribblePotArc : public UIclass 
{
    enum {idle, drawingArc, drawingNumber, drawingTouch, drawingBackground};
    char  lastText[MAX_TEXT_LEN]{0};
    float lastPot{POT_NOT_SET};
    TouchStatus::eStatus lastTouch{TouchStatus::eStatus::OFF};

    bool setArc(void)
        { return UIclass::setArc((currentTrigger.trigger.potValue + 1.0f) * (ea - sa) / 2.0f, lastPot); }
    bool setFloat(void)
        {
            bool retVal;
            // here is where we choose the position:
            pSprite->setViewport(55+10*(SCRIBBLE_DP - 3),  100, 
                                    140-15*(SCRIBBLE_DP - 3), 45);

            retVal = UIclass::setFloat(currentTrigger.trigger.potValue, lastText, sizeof lastText);
            pSprite->resetViewport();                        
            return retVal;
        }
    bool setTouch(void)
        { return UIclass::setTouch(currentTrigger.trigger.pTouchStatus, lastTouch); }

  public:
    virtual State begin(TFT_eSprite& sprite, colours_t c);
    virtual State update(Trigger trigger);
    virtual InterTaskRequest& writeToDisplay(void) { return writeToScribble(); }
};

//==================================================================
class ScribbleDummy : public UIclass {};
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

//==================================================================
//
//    888b     d888          d8b          888      .d8888b.  8888888b.  
//    8888b   d8888          Y8P          888     d88P  Y88b 888  "Y88b 
//    88888b.d88888                       888     888    888 888    888 
//    888Y88888P888  8888b.  888 88888b.  888     888        888    888 
//    888 Y888P 888     "88b 888 888 "88b 888     888        888    888 
//    888  Y8P  888 .d888888 888 888  888 888     888    888 888    888 
//    888   "   888 888  888 888 888  888 888     Y88b  d88P 888  .d88P 
//    888       888 "Y888888 888 888  888 88888888 "Y8888P"  8888888P"  
// 
class MainTestRects : public UIclass 
{
    enum {idle, drawingRect};
    void randomRect(void);

  public:
    virtual State begin(TFT_eSprite& sprite, colours_t c);
    virtual State update(Trigger trigger);
    virtual InterTaskRequest& writeToDisplay(void) { return writeToMainLCD(); }
    virtual uint32_t poll(void);
};
//------------------------------------------------------------------
class MainColourPicker : public UIclass 
{
    enum {idle, doUnMarkHue, doMarkHue,  
                doGradients,
                doUnMarkText, doMarkText,
                doUnMarkBg, doMarkBg,  doDrawColours, doDrawExample,
            doDrawPoint};
    //GTPoint lastTouch;

    // working variables
    const int mr{13}; // hue marker radius
    const int hueX{120}, hueY{120}; // centre of hue circle
    float textLevel{0.66f}, bgLevel{0.66f};
    int dx, dy, mx, my;
    float touchRadius, touchAngle, oldAngle, newLevel;
    bool gradientOnly; // no hue change, just draw a gradient and example

    // the actual result!
    uint16_t hue, textColour, bgColour;
    TFTcolours colours; // for export

    uint16_t angleToHue(int a);
    void hueCircle(int x, int y, int r, int ir, uint16_t bgcolour);
    void gradients(int x, int x2, int y, int w, int h, uint16_t c);
    int rad2TFT(float rad);
    void unMarkHue(
             int cx, int cy,  // selection ring centre...
             int cr,          // ...and radius:outer...
             int cri,         // ...and inner
             int mr,          // marker radius
             float a,         // conventional angle, ±pi
             int& mx,   // return screen position of marker
             int& my);

    uint16_t markHue(
             int cx, int cy,  // selection ring centre...
             int cr,          // ...and radius:outer...
             int cri,         // ...and inner
             int mr,          // marker radius
             float a,         // conventional angle, ±pi
             int& mx,   // return screen position of marker
             int& my);
    bool isOldAngle(float a); 
    void drawFatRect(int x, int y, int w, int h, int t, int colour);
    uint16_t getBlend(float l, uint16_t top, uint16_t hue);
    uint16_t markGradient(
                      int x, int y, int w, int h, // gradient rectangle
                      uint16_t hue, uint16_t top, // colours
                      int d,                 // depth of marker
                      float l, float& oldL); 
    bool isSameLevel(float level, float oldLevel, uint16_t hue, uint16_t top);                      
    void drawSettingsExample(void);
    void showColours(void);

  public:
    virtual State begin(TFT_eSprite& sprite, colours_t c);
    virtual State update(Trigger trigger);
    virtual InterTaskRequest& writeToDisplay(void) { return writeToMainLCD(); }
};
//------------------------------------------------------------------
class MainQwerty : public UIclass 
{
    enum {idle, drawingRect};
    int kbd{0}, kbdTop;
    char currentKey;
    uint16_t* tempCmap{nullptr};
    static constexpr int kWidth{28}, kHeight{28}, kPadding{1},
                         kXoff{3}, 
                         xo{5}, yo{30}, xh{32}, yh{36};

    void drawRow(const char* keys, int row, int off);
    void drawKeyboard(int& n);
    char whichKey(int x, int y);
    const image_4bit_info* getKeyCap(char c);
    void blankTheChar(void) { pSprite->fillRect(xo,yo,xh,yh, colours.bg); }


  public:
    virtual State begin(TFT_eSprite& sprite, colours_t c);
    virtual State update(Trigger trigger);
    virtual InterTaskRequest& writeToDisplay(void) { return writeToMainLCD(); }
    //virtual uint32_t poll(void);
};
//==================================================================
class MainDummy : public UIclass {};
union MainUI
{
    MainDummy  dummy;
    MainTestRects testRects;
    MainColourPicker colourPicker;
    MainQwerty qwerty;
};

union MainUIholder
{
    long long aligner;
    uint8_t space[sizeof(MainUI)];
};



#endif // 0

#endif // !defined(_UI_H_)
