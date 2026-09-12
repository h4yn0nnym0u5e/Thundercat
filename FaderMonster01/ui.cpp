#include "header.h"

//==========================================================================
//
//    888     888 8888888         888                            
//    888     888   888           888                            
//    888     888   888           888                            
//    888     888   888   .d8888b 888  8888b.  .d8888b  .d8888b  
//    888     888   888  d88P"    888     "88b 88K      88K      
//    888     888   888  888      888 .d888888 "Y8888b. "Y8888b. 
//    Y88b. .d88P   888  Y88b.    888 888  888      X88      X88 
//     "Y88888P"  8888888 "Y8888P 888 "Y888888  88888P'  88888P' 
// 

bool UIclass::isNewTouch(Trigger trigger)
{
    bool result = false;

    if (Trigger::eTriggerType::touchPoint == trigger.type
     && Trigger::eTriggerType::touchPoint == currentTrigger.type
     && (trigger.trigger.touchPoint.x !=  currentTrigger.trigger.touchPoint.x
      || trigger.trigger.touchPoint.y !=  currentTrigger.trigger.touchPoint.y
      || trigger.trigger.touchPoint.reserved !=  currentTrigger.trigger.touchPoint.reserved)
        )
        result = true;

    return result;
}

uint16_t* UIclass::makeCmap(uint16_t* cmap, uint16_t fg, uint16_t bg)
{
    for (int i=0;i<16;i++)
        cmap[i] = pSprite->alphaBlend(i*16,fg,bg);

    return cmap;
}


/**
 * Draw a button with text label.
 * pushImage seems to have issues with a Viewport,
 * so for now we have to use absolute screen co-ordinates.
 */
void UIclass::drawButton(int x, int y,
                const image_4bit_info& img, const uint16_t* cmap,
                const char* txt, int xoff, int yoff)
{
    if (nullptr != cmap) // check user hasn't forgotten!
        pSprite->pushImage(x,y, img.width,img.height, (uint8_t*) img.data, 0,false, (uint16_t*) cmap);
    if (nullptr != txt)
        pSprite->drawString(txt,x+xoff,y+yoff);
}                

void UIclass::drawArc(float s, float e, uint16_t fg, uint16_t bg)
{
  pSprite->drawArc(120, 120, 110, 80, s+sa, e+sa, fg, bg);
}

void UIclass::drawTouchEllipse(int thickness)
{
  int cx=120,cy=210,rx=24,ry=16;

  switch (thickness)
  {
    case -2:
        pSprite->fillEllipse(cx,cy,rx,ry,colours.bg);
        break;

    case -1:
        pSprite->fillEllipse(cx,cy,rx,ry,colours.fg);
        break;

    default:
    {
        pSprite->fillEllipse(cx,cy,rx,ry,colours.fg);
        pSprite->fillEllipse(cx,cy,rx-thickness,ry-thickness,colours.bg);
    }
        break;
  }
}

void UIclass::drawTouch(TouchStatus::eStatus estatus)
{
    switch (estatus)
    {
        default:
            drawTouchEllipse();
            break;

        case TouchStatus::eStatus::ON:
            drawTouchEllipse(3);
            break;

        case TouchStatus::eStatus::OFF:
        case TouchStatus::eStatus::JUST_OFF:
        case TouchStatus::eStatus::JUST_OFF_LONG:
            drawTouchEllipse(-2);
            break;
    }
}

// return true if change was worth drawing
bool UIclass::setArc(float newPot, float& lastPot)
{
    bool result = false;
    const float CHANGE_THRESHOLD = CHGTHR_DP;
    if (fabs(newPot - lastPot) > CHANGE_THRESHOLD)
    {
        if (POT_NOT_SET == lastPot)
        {
            // initial background and black arc
            pSprite->fillScreen(colours.bg);
            drawArc(0.0f, ea-sa, TFT_BLACK, colours.bg);
            lastPot = 0.0f;
        }

        if (newPot < lastPot)
            drawArc(newPot, lastPot, TFT_BLACK, colours.bg);
        else
            drawArc(lastPot, newPot, colours.fg, colours.bg);

        lastPot = newPot;
        
        int32_t x,y,w,h;
        result = pSprite->getDirtyArea(x,y,w,h);
    }

    return result;
}


//==========================================================================
// update sprite with text 
// assumes viewport has been set appropriately
// returns true if the text is different from the previous
bool UIclass::setText(char* buf, char* lastString, size_t sizeofLastString)
{
    bool result = strncmp(buf, lastString, sizeofLastString) != 0;
    if (result)
    {
        int w = pSprite->getViewportWidth(),
            h = pSprite->getViewportHeight();
        pSprite->fillRect(0,0,w,h,colours.bg); //fillSprite(bkgnds[i]);

        pSprite->setFreeFont(&FONT_DP);
        pSprite->setTextColor(colours.txt);
        // when drawing numbers with a leading space or minus sign,
        // ensure the first digit always appears at the same location
        int spaceOffset = buf[0] == ' '
                                 ?(pSprite->textWidth("-0") - pSprite->textWidth(" 0"))
                                 :0;
        pSprite->drawString(buf, spaceOffset, 0);

        strncpy(lastString, buf, sizeofLastString);
        lastString[sizeofLastString - 1] = 0; // force termination
    }

    return result;
}


// update sprite with a float value in the centre of the display
// returns true if the text is different from the previous
bool UIclass::setFloat(float potPos, char* lastString, size_t sizeofLastString)
{
  char buf[BUF_SIZE];
  if (potPos < 0.0f && potPos > -CHGTHR_DP / 100.0f) potPos = 0.0f; // don't show -0.000
  sprintf(buf,FMT_DP,potPos);

  return setText(buf, lastString, sizeofLastString);
}


bool UIclass::setTouch(TouchStatus* pTouch, TouchStatus::eStatus& lastTouch)
{
    TouchStatus::eStatus estatus = TouchStatus::eStatus::OFF;
    if (nullptr != pTouch) 
        estatus = pTouch->getExtendedStatus();
    bool changed = lastTouch != estatus;

    if (changed)
    {
        drawTouch(estatus);
        lastTouch = estatus;
    }

    return changed;
}


//==========================================================================
// Send request to screen task to update display
// Note the updateDirty() methods check for the sprite being dirty, so we
// don't have to do that - the request will just come back "done". 
//
// It's assumed our state on entry is "push"; if we can't because the request
// is active, we stay in that state; otherwise we adopt a new state
// depending on whether we can do the next phase or not.
//
//! \return reference to request object; its status tells you if this worked
// failed            : there's already a pending write - must call again
// pending / running : write pending or already in progress
// done              : nothing to do, or done REALLY fast!
InterTaskRequest& UIclass::writeToScribble(void)
{
    InterTaskRequest* result = &autoFail;
    if (updateReq.isInactive())
    {
        scribbleTask.updateDirty(updateReq, *pSprite, 0);
        result = &updateReq;

        if (result->isInactive())
            state = UIclass::State::next;
        else        
            state = UIclass::State::busy;
    }
        
    return *result;
}

InterTaskRequest& UIclass::writeToMainLCD(void)
{
    InterTaskRequest* result = &autoFail;
    if (updateReq.isInactive())
    {
        mainLCDtask.updateDirty(updateReq, *pSprite, 0);
        result = &updateReq;

        if (result->isInactive())
            state = UIclass::State::next;
        else        
            state = UIclass::State::busy;
    }

    return *result;
}

bool UIclass::writeFinished(void)
{
    bool result = false;
    if (updateReq.isInactive()) // finished?
    {
        result = true;                  // yes...
        state = UIclass::State::next;   // ...ready to do the next operation
    }

    return result;
}

//==========================================================================
InterTaskRequest UIclass::autoFail{InterTaskRequest::Result::failed};
//==========================================================================


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
UIclass::State ScribblePotArc::begin(TFT_eSprite& sprite, colours_t c)
{
    UIclass::begin(sprite, c);
    //pSprite->fillScreen(colours.bg); // setArc() will do this
    float lastPotRaw = lastPot*2.0f/(ea-sa) - 1.0f; // back to raw pot value
    currentTrigger.trigger.potValue = POT_NOT_SET == lastPot
                                    ?0.0f
                                    :lastPotRaw;
    lastPot = POT_NOT_SET;
    lastText[0] = 0;
    setArc();
    setFloat();

    currentTrigger.trigger.pTouchStatus = nullptr;
    setTouch();

    return (state = State::push); // need to update display
}

// update the sprite with new pixels
// \return true if sprite has changed
UIclass::State ScribblePotArc::update(Trigger trigger)
{
    State result = State::push; 

    switch (trigger.type)
    {
        default: // we don't react to that trigger type
            result = State::done;
            break;

        //----------------------------------------------------------------------
        // Valid starting triggers
        //----------------------------------------------------------------------
        case Trigger::eTriggerType::potValue: // draw pot value change
            currentTrigger = trigger; // keep the trigger and value(s)
            phase = drawingArc;
            if (!setArc())
                result = State::next;
            break;

        case Trigger::eTriggerType::pTouchStatus: // draw button value change
            currentTrigger = trigger; // keep the trigger and value(s)
            phase = drawingTouch;
            if (!setTouch())
                result = State::done;
            break;
        

        //----------------------------------------------------------------------
        case Trigger::eTriggerType::nextPhase:
            switch (phase)
            {
                case drawingNumber: // nothing to do after this
                default:    // nothing pending
                    result = State::done;
                    break;

                case drawingArc:
                    phase = drawingNumber;
                    if (!setFloat())
                        result = State::done;
                    break;
            }
            break;
    }

    state = result; // hang on to result for later

    return result;
}

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
//==================================================================
//
//    888                     888    8888888b.                   888             
//    888                     888    888   Y88b                  888             
//    888                     888    888    888                  888             
//    888888 .d88b.  .d8888b  888888 888   d88P .d88b.   .d8888b 888888 .d8888b  
//    888   d8P  Y8b 88K      888    8888888P" d8P  Y8b d88P"    888    88K      
//    888   88888888 "Y8888b. 888    888 T88b  88888888 888      888    "Y8888b. 
//    Y88b. Y8b.          X88 Y88b.  888  T88b Y8b.     Y88b.    Y88b.       X88 
//     "Y888 "Y8888   88888P'  "Y888 888   T88b "Y8888   "Y8888P  "Y888  88888P' 
//
UIclass::State MainTestRects::begin(TFT_eSprite& sprite, colours_t c)
{
    UIclass::begin(sprite, c);
    pSprite->fillScreen(colours.bg);
    pSprite->setTextFont(1);

    return State::push; // need to update display
}

uint32_t MainTestRects::poll(void)
{
    uint32_t result = interval;
    if (interval >= 100'000) // every 100ms
    {
        state = State::next; // do next (and only) step
        phase = drawingRect;
        interval -= 100'000; // try to stay in sync
    }
    return result;
}

UIclass::State MainTestRects::update(Trigger trigger)
{
    State result = State::push; 

    switch (trigger.type)
    {
        default: // we don't react to that trigger type
            // Serial.print("meh ");
            result = State::done;
            break;

        //----------------------------------------------------------------------
        case Trigger::eTriggerType::nextPhase:
            switch (phase)
            {
                default:    // nothing pending
                    result = State::done;
                    break;

                case drawingRect:
                    phase = idle;
                    randomRect();
                    break;
            }
            break;
    }

    state = result; // hang on to result for later

    return result;
}

void MainTestRects::randomRect(void)
{
  int x,y, w, h;
  w = random(140); h = random(80);
  uint16_t colour = random(65536);

  do
  {
    x = random(pSprite->width());
    y = random(pSprite->height());
  } while (x+w > pSprite->width() || y+h > pSprite->height() - 25);

  pSprite->setViewport(x,y,w,h);
  pSprite->fillScreen(colour);

  char buf[50];
  sprintf(buf,"%dx%d @ %d,%d", w,h,x,y);
  
  pSprite->setTextColor(~colour);
  pSprite->setTextWrap(true);
  pSprite->drawString(buf,1,1);
  pSprite->resetViewport();
}

//==================================================================
//
//                     888                           8888888b.  d8b          888                       
//                     888                           888   Y88b Y8P          888                       
//                     888                           888    888              888                       
//     .d8888b .d88b.  888  .d88b.  888  888 888d888 888   d88P 888  .d8888b 888  888  .d88b.  888d888 
//    d88P"   d88""88b 888 d88""88b 888  888 888P"   8888888P"  888 d88P"    888 .88P d8P  Y8b 888P"   
//    888     888  888 888 888  888 888  888 888     888        888 888      888888K  88888888 888     
//    Y88b.   Y88..88P 888 Y88..88P Y88b 888 888     888        888 Y88b.    888 "88b Y8b.     888     
//     "Y8888P "Y88P"  888  "Y88P"   "Y88888 888     888        888  "Y8888P 888  888  "Y8888  888     
//
// 
// Return hue based on angle: 0=red, 60=yellow etc
uint16_t MainColourPicker::angleToHue(int a)
{
  TFT_eSprite& tft = *pSprite;

  uint16_t result = TFT_BLACK;
  if (a < 0) a += 720; // deal with reasonable negative angles
  a %= 360;
  int s = a/60;
  a = a - s*60;
  a = 255-a*255/60;

  switch (s)
  {
    case 0: result = tft.alphaBlend(a,TFT_RED,    TFT_YELLOW ); break;
    case 1: result = tft.alphaBlend(a,TFT_YELLOW, TFT_GREEN  ); break;
    case 2: result = tft.alphaBlend(a,TFT_GREEN,  TFT_CYAN   ); break;
    case 3: result = tft.alphaBlend(a,TFT_CYAN,   TFT_BLUE   ); break;
    case 4: result = tft.alphaBlend(a,TFT_BLUE,   TFT_MAGENTA); break;
    case 5: result = tft.alphaBlend(a,TFT_MAGENTA,TFT_RED    ); break;
    default:
      break;
  }
  return result;
}

// N.B. TFT_eSPI angles for arcs have 0 at the 6 o'clock position
void MainColourPicker::hueCircle(int x, int y, int r, int ir, uint16_t bgcolour)
{
  TFT_eSprite& tft = *pSprite;

  for (int ii=0;ii<60;ii++)
  {
    int i = ii;
    int a = i + 181;

    tft.drawArc(x,y,r,ir, (i    )%360,(i+  1)%360, angleToHue(a    ), bgcolour); // red - yellow
    tft.drawArc(x,y,r,ir, (i+ 60)%360,(i+ 61)%360, angleToHue(a+ 60), bgcolour); // yellow - green
    tft.drawArc(x,y,r,ir, (i+120)%360,(i+121)%360, angleToHue(a+120), bgcolour); // green - cyan
    tft.drawArc(x,y,r,ir, (i+180)%360,(i+181)%360, angleToHue(a+180), bgcolour); // cyan - blue
    tft.drawArc(x,y,r,ir, (i+240)%360,(i+241)%360, angleToHue(a+240), bgcolour); // blue - magenta
    tft.drawArc(x,y,r,ir, (i+300)%360,(i+301)%360, angleToHue(a+300), bgcolour); // magenta - red
  }
}

void MainColourPicker::gradients(int x, int x2, int y, int w, int h, uint16_t c)
{
  TFT_eSprite& tft = *pSprite;

  tft.fillRectVGradient( x,y,w,h, TFT_WHITE, c);
  tft.fillRectVGradient(x2,y,w,h, TFT_BLACK, c);
}

// convert angle in radians to TFT_eSPI angle
int MainColourPicker::rad2TFT(float rad)
{
  return (int)(-90 + 360 - rad*180.0f/PI) % 360;
}

/** 
 * Mark the selected hue.
 * Hue angle zero is at 12 o'clock, whereas TFT_eSPI zero is 6 o'clock,
 * and conventional at 3 o'clock and runs anticlockwise. Sigh.
 */
void MainColourPicker::unMarkHue(
             int cx, int cy,  // selection ring centre...
             int cr,          // ...and radius:outer...
             int cri,         // ...and inner
             int mr,          // marker radius
             float a,         // conventional angle, ±pi
             int& mx,   // return screen position of marker
             int& my)
{
  TFT_eSprite& tft = *pSprite;

  int crm = (cr+cri)/2;
  mx = crm * cosf(oldAngle) + cx;
  my = cy - crm * sinf(oldAngle);

  //tft.drawCircle(mx,my,mr,TFT_BLACK);
  tft.drawArc(mx,my,mr,mr-2, 0,360, TFT_BLACK,TFT_BLACK);
  for (int i=-10;i<11;i++)
  {
    int tftAngle = rad2TFT(oldAngle)+i;
    int hueAngle = tftAngle + 180;
    tftAngle = (tftAngle + 360) % 360;
    tft.drawArc(cx,cy,cr,cri, tftAngle, tftAngle+1, angleToHue( hueAngle), TFT_BLACK);
  }
}

uint16_t MainColourPicker::markHue(
             int cx, int cy,  // selection ring centre...
             int cr,          // ...and radius:outer...
             int cri,         // ...and inner
             int mr,          // marker radius
             float a,         // conventional angle, ±pi
             int& mx,   // return screen position of marker
             int& my)
{
  TFT_eSprite& tft = *pSprite;

  int crm = (cr+cri)/2;
  oldAngle = a;
  mx = crm * cosf(oldAngle) + cx, my = cy - crm * sinf(oldAngle);
  //tft.drawCircle(mx,my,mr,TFT_LIGHTGREY);
  int tftAngle = rad2TFT(oldAngle);
  int hueAngle = tftAngle + 180;
  uint16_t hue = angleToHue( hueAngle);
  tft.drawArc(mx,my,mr,mr-2, 0,360, TFT_LIGHTGREY,hue);

  return hue;
}

bool MainColourPicker::isOldAngle(float a)
{
  return fabs(a - oldAngle) < PI/180; // angles differ by less than one degree
}

void MainColourPicker::drawFatRect(int x, int y, int w, int h, int t, int colour)
{
  TFT_eSprite& tft = *pSprite;

  if (2*t >= h || 2*t >= w) // huge thickness means...
    tft.fillRect(x,y,w,h,colour);
  else
  {
    tft.fillRect(x,    y,    w,t,    colour);
    tft.fillRect(x,    y+h-t,w,t,    colour);
    tft.fillRect(x,    y+t,  t,h-2*t,colour);
    tft.fillRect(x+w-t,y+t,  t,h-2*t,colour);
  }    
}


uint16_t MainColourPicker::getBlend(float l, uint16_t top, uint16_t hue)
{
  return pSprite->alphaBlend(l*255+0.5f, top, hue);
}


uint16_t MainColourPicker::markGradient(
                      int x, int y, int w, int h, // gradient rectangle
                      uint16_t hue, uint16_t top, // colours
                      int d,                 // depth of marker
                      float l, float& oldL)   // fractional level (from bottom)
{
  TFT_eSprite& tft = *pSprite; 

  int hh = y+(1-oldL)*h;

  if (l != oldL) // might be just changing hue
    drawFatRect(x-2, hh-d/2, w+4, d+1, 2, TFT_BLACK);

  // draw part of the gradient using a viewport
  // needs a bug fix in TFT_eSPI
  tft.setViewport(x-2, hh-d/2, w+4, d+1, false);
  tft.fillRectVGradient(x,y,w,h,top,hue);
  tft.resetViewport();

  oldL = l;
  hh = y+(1-oldL)*h;
  drawFatRect(x-2, hh-d/2, w+4, d+1, 2, top==TFT_WHITE?TFT_DARKGREY:TFT_LIGHTGREY);

  return getBlend(l, top, hue);
}

bool MainColourPicker::isSameLevel(float level, float oldLevel, uint16_t hue, uint16_t top)
{
  /*
  Serial.printf("%.3f -> %.3f; %04X -> %04X\n", oldLevel, level,
          tft.alphaBlend(oldLevel*255+0.5f, top, hue),
          tft.alphaBlend(   level*255+0.5f, top, hue) 
  );
  */
  return getBlend(level,    top, hue)
      == getBlend(oldLevel, top, hue);
}

// render what settings look like inside hue circle
// Hack hack - it's a 115x60 area
void MainColourPicker::drawSettingsExample(void)
{
  TFT_eSprite& tft = *pSprite; 

  int yp = 100;
  // Main background:
  //tft.fillRect(65,yp,115,60, bgColour);
  // ... but save time by not overwriting with hue:
  tft.fillRect(65,yp,      115,60-10-15, bgColour);
  tft.fillRect(65,yp+60-10,115,   10   , bgColour);
  tft.fillRect(65,    yp+60-25, 10,   15   , bgColour);
  tft.fillRect(65+105,yp+60-25, 10,   15   , bgColour);
  //tft.setCursor(70,95);
  tft.setTextColor(textColour);
  tft.drawString("Text", 75, yp+5, 4);
  tft.fillRect(75,yp+60-10-15,95,15, hue);
}


void MainColourPicker::showColours(void)
{
  TFT_eSprite& tft = *pSprite; 

  char buffer[20];
  tft.setTextColor(TFT_WHITE);
  tft.setTextFont(1);

  sprintf(buffer," Hue: %04X", hue & 0xFFFF);
  tft.fillRect(90,60,70,10,TFT_BLACK);
  tft.drawString(buffer, 90,60);

  sprintf(buffer,"Text: %04X", textColour & 0xFFFF);
  tft.fillRect(90,70,70,10,TFT_BLACK);
  tft.drawString(buffer, 90,70);

  sprintf(buffer,"Bgnd: %04X", bgColour & 0xFFFF);
  tft.fillRect(90,80,70,10,TFT_BLACK);
  tft.drawString(buffer, 90,80);
}

UIclass::State MainColourPicker::begin(TFT_eSprite& sprite, colours_t c)
{
    UIclass::begin(sprite, c);

    pSprite->fillScreen(TFT_BLACK);
    hueCircle(hueX,hueY, 100,80, TFT_BLACK);            // hue circle
    hue = markHue(hueX,hueY, 100,80, 13, PI/2, mx, my); // mark to get initial hue
    gradients(240,280, 20,20,200, hue);                 // text and background gradients

    // gradient markers
    textColour = markGradient(240,20,20,200, hue,TFT_WHITE, 9, textLevel, textLevel);
    bgColour   = markGradient(280,20,20,200, hue,TFT_BLACK, 9, bgLevel, bgLevel);

    drawSettingsExample();  // text + foreground + background
    showColours();          // hex values for the current colours

    //*
    // button image
    uint16_t colour = TFT_LIGHTGREY;
    uint16_t cmap[16];
    makeCmap(cmap,colour,TFT_BLACK);

    pSprite->setFreeFont(&FreeSans9pt7b);
    pSprite->setTextColor(pSprite->alphaBlend(64, TFT_WHITE, TFT_BLACK), cmap[15]);

    drawButton(0,0, tl_button_info, cmap, "TFT", 10,15);
    drawButton(0,239-bl_button_info.height, bl_button_info, cmap, "Ring", 10,60);

    //*/
    return (state = State::push); // need to update display
}

UIclass::State MainColourPicker::update(Trigger trigger)
{
    State result = State::push; 

    switch (trigger.type)
    {
        default: // we don't react to that trigger type
            result = State::done;
            break;
        //----------------------------------------------------------------------
        case Trigger::eTriggerType::touchPoint:
        {
            currentTrigger = trigger; // keep the trigger and value(s)
            // assume the touch point is not interesting
            phase = idle;
            result = State::done;
            gradientOnly = true; // assume user touched text or background gradient
            GTPoint lastTouch = trigger.trigger.touchPoint;

            // change a strip's colour scheme?
            if (lastTouch.x < 50 && lastTouch.y < 50 && lastTouch.reserved == 255) // hacky hack!
            {
                colours = {hue,bgColour,textColour}; 
                for (int i=0;i<NUM_POTS;i++)
                {
                    TouchStatus& stripTouch = StripTask::getStripPotTouch(i);
                    if (TouchStatus::eStatus::LONG == stripTouch.getExtendedStatus())
                    {
                        faderMonsterSettings.stripsConfig.colours[i].scribble = colours;
                        StripTask::getStripTask(i).tftColourChanged();
                    }
                }
                break; // nothing else needed
            }

            // change a ring LED's colour?
            if (lastTouch.x < 50 && lastTouch.y > (240 - 50) && lastTouch.reserved == 255) // hacky hack!
            {
                uint32_t LEDcolour = pSprite->color16to24(bgColour); // allow brightness control
                for (int i=0;i<NUM_POTS;i++)
                {
                    TouchStatus& stripTouch = StripTask::getStripPotTouch(i);
                    if (TouchStatus::eStatus::LONG == stripTouch.getExtendedStatus())
                    {
                        faderMonsterSettings.stripsConfig.colours[i].ringLEDs.colour = LEDcolour;
                        StripTask::getStripTask(i).tftColourChanged();
                    }
                }
                break; // nothing else needed
            }

            // Are we in the colour circle - if so select hue
            dx = lastTouch.x - hueX; dy = lastTouch.y - hueY;
            touchRadius = sqrtf(dx*dx+dy*dy);
            touchAngle = atan2(hueY - lastTouch.y, lastTouch.x - hueX);
            if (touchRadius < 105.0f && touchRadius > 70.0f && !isOldAngle(touchAngle))
            {
                gradientOnly = false; // change hue and both gradients
                phase = doUnMarkHue;
                result = State::next;
            }
            else
            {
                // see if we're in the text or background sliders
                if (lastTouch.x>=230 && lastTouch.y>=18 && lastTouch.y<=222)
                {
                    newLevel = (220 - lastTouch.y)/200.0f;
                    newLevel = constrain<float>(newLevel,0.0f,1.0f);

                    if (lastTouch.x<270)
                    {
                        if (!isSameLevel(newLevel,textLevel, hue,TFT_WHITE))
                        {
                            phase = doMarkText;
                            result = State::next; // gonna draw something!
                        }
                    }
                    else            
                    {
                        if (!isSameLevel(newLevel,bgLevel, hue,TFT_BLACK))
                        {
                            phase = doMarkBg;
                            result = State::next; // gonna draw something!
                        }
                    }
                }
            }
        }
            break;
        
        //----------------------------------------------------------------------
        case Trigger::eTriggerType::nextPhase:
            switch (phase)
            {
                default:    // nothing pending
                    result = State::done;
                    break;

                case doUnMarkHue:
                    unMarkHue(hueX,hueY, 100,80, mr, touchAngle, mx, my);
                    phase = doMarkHue;
                    break;

                case doMarkHue:
                    hue = markHue(hueX,hueY, 100,80, mr, touchAngle, mx, my);
                    phase = doGradients;
                    break;

                case doGradients:
                    gradients(240,280, 20,20,200, hue);
                    phase = doMarkText;
                    break;

                case doMarkText:
                    if (!gradientOnly) newLevel = textLevel;
                    textColour = markGradient(240,20,20,200, hue,TFT_WHITE, 9, newLevel, textLevel);
                    phase = gradientOnly?doDrawColours:doMarkBg;
                    break;

                case doMarkBg:
                    if (!gradientOnly) newLevel = bgLevel;
                    bgColour = markGradient(280,20,20,200, hue,TFT_BLACK, 9, newLevel, bgLevel);
                    phase = doDrawColours;
                    break;
                           
                case doDrawColours:
                    showColours();
                    phase = doDrawExample;
                    break;
                    
                case doDrawExample:
                    drawSettingsExample();
                    phase = idle;
                    break;

                case doDrawPoint:
                {
                    GTPoint lastTouch = currentTrigger.trigger.touchPoint;
                    pSprite->fillRect(lastTouch.x, lastTouch.y, 2,2, TFT_RED);
                    phase = idle;
                }
                    break;

            }
            break;
    }

    state = result; // hang on to result for later

    return result;
}

//==================================================================
//
//                                            888             
//                                            888             
//                                            888             
//     .d88888 888  888  888  .d88b.  888d888 888888 888  888 
//    d88" 888 888  888  888 d8P  Y8b 888P"   888    888  888 
//    888  888 888  888  888 88888888 888     888    888  888 
//    Y88b 888 Y88b 888 d88P Y8b.     888     Y88b.  Y88b 888 
//     "Y88888  "Y8888888P"   "Y8888  888      "Y888  "Y88888 
//         888                                            888 
//         888                                       Y8b d88P 
//         888                                        "Y88P"  
// 
static constexpr char kbds[][4][15]
{
    {"qwertyuiop","asdfghjkl;","zxcvbnm,./",    "\x01\x14 \x12\x13"},
    {"QWERTYUIOP","ASDFGHJKL:","ZXCVBNM<>?",    "\x02\x14 \x12\x13"},
    {"!\x22#$%^&*()","1234567890","_+-=[]{}'@", "\x03\x14 \x12\x13"},
};

const image_4bit_info* MainQwerty::getKeyCap(char c)
{
    const image_4bit_info* img = &keycap28_info; // assume letter keys  
    switch (c)
    {
        default:
            break;

        case ' ':
            img = &keycap90_info;
            break;

        case '\x01':
            img = &keycap54_Aa_info;
            break;

        case '\x02':
            img = &keycap54_A1_info;
            break;

        case '\x03':
            img = &keycap54_1a_info;
            break;

        case '\x11':
            img = &keycap54_info;
            break;

        case '\x12':
            img = &keycap54_larr_info;
            break;

        case '\x13':
            img = &keycap54_tick_info;
            break;

        case '\x14':
            img = &keycap54_cross_info;
            break;
    } 
    return img;
}

void MainQwerty::drawRow(const char* keys, int row, int off)
{
    const image_4bit_info* img = &keycap28_info; // assume letter keys
    size_t cols = strlen(keys);
    char buf[2]{0};
    int x = off+kXoff, y = kbdTop + (kHeight + kPadding)*row;
    for (size_t i=0;i<cols;i++)
    {
        *buf = keys[i];
        img = getKeyCap(keys[i]);

        if (buf[0] <= ' ')
            pSprite->fillRect(x+2,y+2, img->width - 4, img->height - 4, colours.txt);
        drawButton(x,y,*img, tempCmap, buf[0]>' '?buf:nullptr, kWidth/2,3);
        x += img->width + kPadding;
    }
}

void MainQwerty::drawKeyboard(int& n)
{
    uint16_t cmap[16];
    pSprite->setFreeFont(&FreeSansBold9pt7b);

    tempCmap = makeCmap(cmap, TFT_BLACK, colours.bg);
    pSprite->setTextColor(colours.fg, cmap[15], false); // no background fill

    if (n >= COUNT_OF(kbds)) n = 0;

    pSprite->fillRect(0,kbdTop,pSprite->width(),4*(kHeight+kPadding),colours.bg);
    drawRow(&kbds[n][0][0],0,0);
    drawRow(&kbds[n][1][0],1,(kWidth + kPadding) / 4);
    drawRow(&kbds[n][2][0],2,(kWidth + kPadding) / 2);
    drawRow(&kbds[n][3][0],3,0);

    tempCmap = nullptr;
}

char MainQwerty::whichKey(int x, int y)
{
    char result = 0; // Not A Key
    x -= kXoff;

    //Serial.printf("x: %d, y: %d; ");
    do
    {
        if (y < kbdTop)
            break;

        y = (y - kbdTop)/(kHeight + kPadding); // figure out row number
        //Serial.printf("row: %d; ", y);
        if (y > 3) // beyond bottom row 
            break;

        switch (y)
        {
            default:
                break;
            case 1:
                x -= (kWidth + kPadding) / 4;
                break;
            case 2:
                x -= (kWidth + kPadding) / 2;
                break;
        }

        if (3 != y) // simple keys
        {
            x /= kWidth + kPadding; // column number
            //Serial.printf("col: %d; ", x);
            if ((size_t) x >= strlen(kbds[kbd][y])) // off the right
                break;
            result = kbds[kbd][y][x];
            //Serial.printf("char: %c", result);
        }
        else // row 4
        {
            const char* rowChars = kbds[kbd][y];
            size_t nKeys = strlen(rowChars);
            for (size_t i=0;i < nKeys && 0 == result;i++)
            {
                const image_4bit_info* img = getKeyCap(rowChars[i]);
                if (x < img->width)
                    result = rowChars[i];
                x -= img->width + kPadding;
            }
            //Serial.printf("Key: %02X\n", result);
        }
    } while (0);
    //Serial.println();

    return result;
}

UIclass::State MainQwerty::begin(TFT_eSprite& sprite, colours_t c)
{
    State result = State::push;
    UIclass::begin(sprite, c); // do standard setup
    kbdTop = pSprite->height() - (kHeight + kPadding)*4 - 10;

    // clear screen
    pSprite->fillScreen(colours.bg);

    // draw heading 
    pSprite->fillRect(0,0,pSprite->width(), yo, TFT_BLACK);
    pSprite->setFreeFont(&FreeSans12pt7b);
    pSprite->setTextColor(colours.fg, TFT_BLACK);
    pSprite->drawString(mainLCDtask.headerText,2,2);
    
    // preset text choices
    pSprite->setFreeFont(&FreeSansBold9pt7b);
    pSprite->setTextColor(colours.fg, colours.bg, false); // no background fill
    pSprite->setTextDatum(TC_DATUM);

    // draw the initial keyboard
    drawKeyboard(kbd);

    return (state = result); // save and return state
}

UIclass::State MainQwerty::update(Trigger trigger)
{
    State result = State::done;

    switch (trigger.type)
    {
        default: // we don't react to that trigger type
            result = State::done;
            break;
        //----------------------------------------------------------------------
        case Trigger::eTriggerType::touchPoint:
        {
            GTPoint& newPt = trigger.trigger.touchPoint;
            if (isNewTouch(trigger))
            {
                currentTrigger = trigger; // keep the trigger and value(s)
                char theKey = whichKey(newPt.x, newPt.y);
                switch (theKey)
                {
                    case '\x13': // tick
                        theKey = '\n';
                    default:
                        if (theKey != currentKey)
                        {
                            char buf[2]{0};
                            currentKey = theKey;
                            if (theKey > ' ') // can't draw a newline!
                            {
                                pSprite->setFreeFont(&FreeSansBold18pt7b);
                                pSprite->setTextColor(colours.fg, colours.bg, true); // no background fill
                                pSprite->setTextDatum(TC_DATUM);
                                *buf = theKey;
                                blankTheChar();
                                pSprite->drawString(buf,xo+15,yo+2);
                            }
                            else if (' ' == theKey)
                            {
                                const int indent = 4;
                                blankTheChar();
                                pSprite->setViewport(xo,yo,xh,yh);
                                pSprite->fillRect(indent,  yh*2/3,xh-indent*2,  4,colours.fg);
                                pSprite->fillRect(indent+2,yh*2/3,xh-indent*2-4,2,colours.bg);
                                pSprite->resetViewport();
                            }
                        }
                        if (255 == newPt.reserved)
                        {
                            if (theKey >= ' ' || '\n' == theKey)
                                Serial.print(theKey); // do something better here!
                            blankTheChar();
                            currentKey = 0; // allow for double letters!
                        }
                        break;

                    case '\x12': // back-arrow
                        break;

                    case '\x14': // cross
                        break;

                    case '\x01':
                    case '\x02':
                    case '\x03':
                        if (255 == newPt.reserved)
                        {
                            kbd++;
                            drawKeyboard(kbd);
                        }
                        blankTheChar();
                        break;
                }

                result = State::push;
            }
        }
            break;
    }
    return (state = result); // save and return state
}
