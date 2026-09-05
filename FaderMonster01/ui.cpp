#if 1
#include "header.h"

//==========================================================================
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


bool UIclass::setTouch(TouchStatus& touch, TouchStatus::eStatus& lastTouch)
{
    TouchStatus::eStatus estatus = touch.getExtendedStatus();
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


UIclass::State ScribblePotArc::begin(TFT_eSprite& sprite, colours_t c)
{
    UIclass::begin(sprite, c);
    pSprite->fillScreen(colours.bg);

    return State::push; // need to update display
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
            if (!setArc((currentTrigger.trigger.potValue + 1.0f) * (ea - sa) / 2.0f, lastPot))
                result = State::next;
            break;

        case Trigger::eTriggerType::pTouchStatus: // draw button value change
            currentTrigger = trigger; // keep the trigger and value(s)
            phase = drawingTouch;
            if (!setTouch(*currentTrigger.trigger.pTouchStatus, lastTouch))
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
                    // here is where we choose the position:
                    pSprite->setViewport(55+10*(SCRIBBLE_DP - 3),  100, 
                                         140-15*(SCRIBBLE_DP - 3), 45);

                    if (!setFloat(currentTrigger.trigger.potValue, lastText, sizeof lastText))
                        result = State::done;
                    pSprite->resetViewport();                        
                    break;
            }
            break;
    }

    state = result; // hang on to result for later

    return result;
}

#endif // 0
