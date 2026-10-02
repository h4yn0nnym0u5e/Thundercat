#if !defined(_UI_H_)
#define _UI_H_

#if 1

#include <TFT_eSPI.h>

#define MAX_TEXT_LEN 30
#define TFT_DARKERGREY 0x39E7
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
        notrigger,
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
    friend class UIbutton;
    friend class UIgraphicButton;

    static InterTaskRequest autoFail;
  protected:
    TFT_eSprite* pSprite;
    colours_t    colours;
    int          phase{0};
    Trigger      currentTrigger{Trigger::eTriggerType::notrigger,0};
    elapsedMicros interval; // polling interval timer
    static constexpr const char* bks{""};
#define BKS (char*) bks    

    // standard drawing methods
    void drawArc(float s, float e, uint16_t fg, uint16_t bg);
    void drawTouchEllipse(int thickness = -1);
    void drawTouch(TouchStatus::eStatus estatus);

    // higher-level methods: can tell when item has changed
    bool setArc(float newPot, float& lastPot);
    bool setText(char* buf, char* lastString, size_t sizeofLastString, bool setFont = true);
    bool setFloat(float potPos, char* lastString, size_t sizeofLastString);
    bool setTouch(TouchStatus* pTouch, TouchStatus::eStatus& lastTouch);

    void drawButton(int x, int y,
                    const image_4bit_info& img, const uint16_t* cmap,
                    const char* txt, int xoff, int yoff);
    bool isNewTouch(Trigger trigger);
    void drawHeader(const char* txt);

    // utilities
    /**
     * Checks timeout, and resets it if it has been exceeded
     * \return true if timeout parameter was exceeded
     */
    bool intervalElapsed(uint32_t usecs) //!< timeout in microseconds
    {
        bool result = interval >= usecs;
        if (result)
        {
            interval -= usecs;
            if (interval > usecs) interval = 0;
        }

        return result;
    }

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
    void debugPrint(void);
    static uint16_t* makeCmap(uint16_t* cmap, uint16_t fg, uint16_t bg);

    static constexpr float POT_NOT_SET{-999.0f};
    static constexpr float sa{2*18.0f}, ea{360.0f - 2*18.0f}; // TFT_eSPI has zero at 6 o'clock
    static constexpr int BUF_SIZE{MAX_TEXT_LEN};

    InterTaskRequest updateReq;
};

//==================================================================
//    888               888    888                     
//    888               888    888                     
//    888               888    888                     
//    88888b.  888  888 888888 888888 .d88b.  88888b.  
//    888 "88b 888  888 888    888   d88""88b 888 "88b 
//    888  888 888  888 888    888   888  888 888  888 
//    888 d88P Y88b 888 Y88b.  Y88b. Y88..88P 888  888 
//    88888P"   "Y88888  "Y888  "Y888 "Y88P"  888  888 
//  
class UIbutton
{
  protected:
    enum buttonState_e {notDrawn, 
          drawnNormal, drawnHit, drawnLatched,
           setNormal,   setHit,   setLatched} 
            state{notDrawn};
  public:
    TFTcolours colours; // copied from reference at construction time
    int x,y,w,h;
    char* label; 
    const GFXfont* font{nullptr};
    const uint8_t* smoothFont{nullptr};
    int labXoff, labYoff;
  protected: 
    bool drawLabel(TFT_eSprite* pSprite, int datum, int x, int y, uint16_t txt, int bg = -1)
    {
        bool result = false;

        if (nullptr != label)
        {
            setFont(pSprite);
            pSprite->setTextDatum(datum);
            if (bg < 0) 
                pSprite->setTextColor(txt);
            else                
                pSprite->setTextColor(txt, bg);
            pSprite->drawString(label,x,y);

            result = true;
        }
        return result;
    }

    bool lifted{true};  //!< touch was lifted rather than being slid out
    /**
     * Set lifted and state values depending on whether touch is inside button.
     * \result true if touch is in, OR if just exited and re-draw is needed
     */
    bool setLifted(bool in,     //!< touch is in button's active area
                   GTPoint& pt) //!< touch point - used to see if touch just ended
    {
        lifted = in && 255 == pt.reserved; // "in" the button, but actually touch has ended

        // work out if a re-draw is indicated
        if (in)
        {
            if (drawnHit != state && drawnLatched != state)
                state = setHit;
        }

        if (!in || lifted) // out, sideways or upwards!
        {
            if (drawnNormal != state    // isn't already drawn in "out" state...
             && drawnLatched != state)  // ...and isn't supposed to stay latched
            {
                state = setNormal;
                in = true; // say we're "in", as re-draw is needed
            }
        } 

        return in;
    }

  public:
    UIbutton(TFTcolours& c,
             int _x = 0, int _y = 0, int _w = 4, int _h = 4,
             char* _label = (char*) "!", const GFXfont *_f = &FONT_BUTTON, 
             int _labXoff = 0, int _labYoff = -3)
      : colours{c},
        x{_x}, y{_y}, w{_w}, h{_h},
        label{_label}, 
        font{_f}, labXoff{_labXoff}, labYoff{_labYoff}
        { }
    virtual bool draw(TFT_eSprite* pSprite);
    virtual bool isIn(GTPoint&);
    virtual bool isLifted(void) { bool result = lifted; lifted = false; return result; }    
    
    virtual void setColours(TFTcolours c) {colours = c;}
    virtual void setFont(const GFXfont* f) { font = f; }
    virtual void setFont(const uint8_t* f) { smoothFont = f; }
    virtual void setFont(TFT_eSprite* pSprite)
    {
        if (nullptr != smoothFont)
            FontSetter::loadFont(*pSprite, smoothFont);
        else
            FontSetter::setFreeFont(*pSprite, font);

    }

    virtual bool needsDrawing(void) 
    { 
        return setNormal == state || setHit == state || setLatched == state || notDrawn == state; 
    }

    /**
     * Force current drawing state towards normal, if it isn't already
     */ 
    void forceNormal(void) { if (drawnNormal != state) state = setNormal; } 

    /**
     * Change button from normal to latched state.
     * Does nothing if the state is not (about to be) hit.
     * \return true if state was changed
     */
    bool latch(void) 
    { 
        bool result = false; 
        switch (state)
        {
            case setHit:
                state = setLatched;
                result = true;
                break;
                
            case drawnHit:
                state = drawnLatched;
                result = true;
                break;

            default:
                break;                                
        }
        return result;
    }
};

class UIgraphicButton : public UIbutton
{
  protected:
    const image_4bit_info* pGraphic; // pointer: might want to change it
  public:    
    UIgraphicButton(TFTcolours& c,
             int _x, int _y, 
             char* _label, const image_4bit_info& _graphic,
             const GFXfont *_f = &FONT_BUTTON, 
             int _labXoff = 0, int _labYoff = 0)
        : UIbutton{c,_x,_y, _graphic.width, _graphic.height, _label,_f,_labXoff,_labYoff},
          pGraphic{&_graphic}
        {}
    virtual bool draw(TFT_eSprite* pSprite);
    virtual bool isIn(GTPoint& p) { return UIbutton::isIn(p); }
    virtual bool isIn(GTPoint& p, int threshold);
};


class UIvGradButton : public UIbutton
{
  public:    
    UIvGradButton(TFTcolours& c,
             int _x = 0, int _y = 0, int _w = 4, int _h = 4,
             char* _label = (char*) "#",
             const GFXfont *_f = &FONT_BUTTON, 
             int _labXoff = 0, int _labYoff = 0)
        : UIbutton{c,_x,_y,_w,_h, _label,_f,_labXoff,_labYoff}
        {}
    virtual bool draw(TFT_eSprite* pSprite);
};

class UIradioButton : public UIgraphicButton
{
public:    
    uint16_t& selectedColour;
    uint32_t& groupFlags;
    int buttonNum;
  public:
    UIradioButton(TFTcolours& c,
             uint16_t& _selectedColour,
             uint32_t& _groupFlags,
             int _buttonNum,
             int _x = 0, int _y = 0, int _w = 4, int _h = 4,
             char* _label = (char*) "#", 
             const image_4bit_info& _graphic = radio_button_20_info, 
             const GFXfont *_f = &FONT_BUTTON)
        : UIgraphicButton(c, _x,_y, _label, _graphic, _f, _graphic.width+3),
          selectedColour{_selectedColour}, groupFlags{_groupFlags}, buttonNum{_buttonNum}
        { 
            w = _w; h = _h; 
            if (0 == groupFlags) 
            { 
                groupFlags = 1<<buttonNum; 
                state = setLatched;
            }
        }
    virtual bool draw(TFT_eSprite* pSprite);
    bool isActive(void) { return 0 != (groupFlags & (1<<buttonNum));}
    void setActive(void) { state = setLatched; groupFlags = 1<<buttonNum; }
    static bool processTouch(UIradioButton* buttons, int count, GTPoint& touch);
    static UIclass::State processDraw(TFT_eSprite* pSprite, UIradioButton* buttons, int count);
    static bool setActive(UIradioButton* buttons, int count, int which);
    int getActive(void) { return 31 - __builtin_clz(groupFlags); }
};

/**
 * Set the colours for an array of buttons.
 * Has to be a template, since the various derived classes are different sizes.
 */
template<class B>
void setColours(B* buttons, int count, TFTcolours c)
{
    for (int i=0;i<count;i++) buttons[i].setColours(c);
}

/**
 * Make a button array into a grid or horizontal / vertical line.
 * The first button has the correct size and top left position,
 * subsequent buttons have the same size and fill top to bottom,
 * left to right.
 */
template<class B>
void makeGrid(B* buttons, int count, int columns, int spcX, int spcY = 0)
{
    int x = buttons[0].x, y = buttons[0].y,
        w = buttons[0].w, h = buttons[0].h;
    int rows = count / columns;
    int n = 0; // copies first button's size and position to itself, but easier!

    while (count > 0)
    {
        int yy = y;
        for (int i=0;i<rows && count > 0;i++)
        {
            buttons[n].x = x;
            buttons[n].y = yy;
            buttons[n].w = w;
            buttons[n].h = h;

            yy += spcY;
            n++;
            count--;
        }
        x += spcX;
    }
}


/**
 * Draw an array of buttons
 */
template<class B>
void drawAll(TFT_eSprite* pSprite, B* buttons, int count)
{
    for (int i=0;i<count;i++) buttons[i].draw(pSprite);
}

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
                pollButtons,
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

    static uint16_t angleToHue(int a);
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
    TFTcolours coloursButtons{TFT_DARKERGREY, TFT_BLACK, TFT_LIGHTGREY};
    UIgraphicButton buttonRing{coloursButtons, 0,0, (char*) "Ring", tl_button_info, &FreeSans9pt7b, -15,-25};
    UIgraphicButton buttonTFT{coloursButtons, 0,239-bl_button_info.height, (char*)"TFT", bl_button_info, &FreeSans9pt7b, -15,20};

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
};

//------------------------------------------------------------------
class MainExprTune : public UIclass 
{
    enum {idle,
          drawCurrent, drawScaled, 
          drawMin, drawMax, 
          drawBar, drawGain,
          pollButtons} phase{idle};
    static constexpr int barX{10}, barY{50}, barW{300}, barH{20}, 
                     textW{60}, textH{25}, textLen{10};
    UIclass::State drawBarTo(float pos);
    UIclass::State _setFloat(float f, char* buf, char* stash);
    void clearBar(void)
    {
        pSprite->fillRect(barX, barY, barW, barH, colours.bg);
        max = min = last;
    }

    UIbutton set    {colours,   20,    180, 80,40, (char*) "Set"};
    UIbutton clear  {colours, 160- 40, 180, 80,40, (char*) "Clear"};
    UIbutton autocal{colours, 320-100, 180, 80,40, (char*) "Cal"};

    float min{0.0f}, max{0.0f}, last{0.0f};
    char minText[textLen]{0}, maxText[textLen]{0}, 
         curText[textLen]{0}, scaledText[textLen]{0};

  public:
    virtual State begin(TFT_eSprite& sprite, colours_t c);
    virtual State update(Trigger trigger);
    virtual InterTaskRequest& writeToDisplay(void) { return writeToMainLCD(); }
    virtual uint32_t poll(void);
};

//------------------------------------------------------------------
class MainSceneLoad : public UIclass 
{
    enum {idle, pollButtons} phase{idle};
    static constexpr int barXspc{150}, barYspc{40},
                     numScenes{10}, nameLength{14};

    char sceneNames[numScenes][nameLength+1];
    colours_t buttonColours;
    UIvGradButton sceneButtons[numScenes] 
    {
           {buttonColours,  20,35, 130, 24, }, // top left button
           {buttonColours}, 
           {buttonColours}, {buttonColours}, {buttonColours}, {buttonColours}, 
           {buttonColours}, {buttonColours}, {buttonColours}, {buttonColours}
        }; 

  public:
    virtual State begin(TFT_eSprite& sprite, colours_t c);
    void setButtonColours(colours_t bc) { buttonColours = bc; }
    virtual State update(Trigger trigger);
    virtual InterTaskRequest& writeToDisplay(void) { return writeToMainLCD(); }
};

//------------------------------------------------------------------
class MainMIDIsettings : public UIclass 
{
    enum {idle,
          pollCtls, pollStrips, pollSelect} phase{idle};
    static constexpr int stripXspc{25}, ccYspc{24},
                     ctlsPerStrip{3}, ctlX{230},
                     numCtlTypes{7}; // should pick this up somehow...

    // strip select
#define FMS_SC(n) faderMonsterSettings.stripsConfig.colours[n].scribble, \
                  faderMonsterSettings.stripsConfig.colours[n].scribble.fg
    uint32_t stripFlags{0};
    UIradioButton stripSelectButtons[NUM_POTS]  
    {
        {FMS_SC(0), stripFlags, 0, 5, 35, 22, 22, nullptr}, // top left button
        {FMS_SC(1), stripFlags, 1, 0,0,0,0, nullptr}, 
        {FMS_SC(2), stripFlags, 2, 0,0,0,0, nullptr}, 
        {FMS_SC(3), stripFlags, 3, 0,0,0,0, nullptr}, 
        {FMS_SC(4), stripFlags, 4, 0,0,0,0, nullptr}, 
        {FMS_SC(5), stripFlags, 5, 0,0,0,0, nullptr}, 
        {FMS_SC(6), stripFlags, 6, 0,0,0,0, nullptr},
        {FMS_SC(7), stripFlags, 7, 0,0,0,0, nullptr}
    }; 
#undef FMS_SC

    // MIDI control type selection
    colours_t buttonColours;
    uint16_t  rimColour;
    uint32_t groupFlags{0};
    UIradioButton ctlTypeButtons[numCtlTypes]  
    {
        {buttonColours, rimColour, groupFlags, 0, 15, 70, 80, 22, (char*) "CC"}, // top left button
        {buttonColours, rimColour, groupFlags, 1, 0,0,0,0, (char*) "RPN"}, 
        {buttonColours, rimColour, groupFlags, 2, 0,0,0,0, (char*) "NRPN"}, 
        {buttonColours, rimColour, groupFlags, 3, 0,0,0,0, (char*) "Bend"}, 
        {buttonColours, rimColour, groupFlags, 4, 0,0,0,0, (char*) "Prog"}, 
        {buttonColours, rimColour, groupFlags, 5, 0,0,0,0, (char*) "AfTch"}, 
        {buttonColours, rimColour, groupFlags, 6, 0,0,0,0, (char*) "Note"}
    }; 

    uint32_t midiCtlFlags{0};
    UIradioButton ctlSelectButtons[ctlsPerStrip]  
    {
        {buttonColours, rimColour, midiCtlFlags, 0, ctlX + 00, 35, 22, 22, nullptr, },
        {buttonColours, rimColour, midiCtlFlags, 1, ctlX + 25, 35, 22, 22, nullptr, fader_button_info},
        {buttonColours, rimColour, midiCtlFlags, 2, ctlX + 50, 35, 22, 22, nullptr, button_button_info},
    };


    MIDIcontrolSetting* pControl;

  public:
    virtual State begin(TFT_eSprite& sprite, colours_t c);
    void setButtonColours(colours_t bc) { buttonColours = bc; }
    void setRimColour(uint16_t rc) { rimColour = rc; }
    virtual State update(Trigger trigger);
    virtual InterTaskRequest& writeToDisplay(void) { return writeToMainLCD(); }
    void selectStrip(int n, int which = 0) 
    { 
        StripControls& ctls = faderMonsterSettings.stripsConfig.controls[n];

        UIradioButton::setActive(stripSelectButtons, NUM_POTS, n);
        UIradioButton::setActive(ctlSelectButtons, ctlsPerStrip, which);
        switch (which)
        {
            case 0: pControl = &ctls.pot; break;
            case 1: pControl = &ctls.fader; break;
            case 2: pControl = &ctls.button; break;
        }
        UIradioButton::setActive(ctlTypeButtons, numCtlTypes, 
                                 (int) pControl->controlType - 1);
    }
};

//==================================================================
class MainDummy : public UIclass {};
union MainUI
{
    MainDummy  dummy;
    MainTestRects testRects;
    MainColourPicker colourPicker;
    MainQwerty qwerty;
    MainExprTune expr;
    MainSceneLoad scene;
    MainMIDIsettings midiSet;
};
#define MAIN_UI_COUNT 6

union MainUIholder
{
    long long aligner;
    uint8_t space[sizeof(MainUI)];
};


#undef BKS

#endif // 0

#endif // !defined(_UI_H_)
