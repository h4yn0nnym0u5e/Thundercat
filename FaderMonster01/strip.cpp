#include "header.h"

int StripTask::globalBright{39};

void StripTask::setDot(float value, uint32_t colour)
{
  if (0 == bright)
  {
    ring.clear();
  }
  else
  {
    const int firstLED = 12;

    if (value < -1.0f) value = -1.0f;
    if (value > +1.0f) value = +1.0f;

    value = value*(17*18 / 2 - 0.02f); // angle: ±152.8
    if (value < 0.0f)
      value += 360.0f; // 207.2 minimum

    /*
    if (echoOnce)
    {
      ring.debug = true;
      echoOnce = false;
    }
    */

    ring.setArc(18.0f*firstLED - 8.99f, 8*18+8.99f, TFT_BLACK);
    ring.setArc(18.0f*firstLED - 8.99f, value, colour, bright);
    ring.ensurePixelVisible(firstLED,colour);
    if (ring.debug)
      Serial.println();
    ring.debug = false;

    //ring.setPixel(10, keyStatuses[ring.ring]?xWHITE:TFT_BLACK,bright);
  }
}

InterTaskRequest::Result StripTask::doPotChange(void* pNothing)
{
    setDotCurrent();
    scribbleState = ScribbleState::start;

    return InterTaskRequest::Result::done;
}    



//-------------------------------------------------------------------------
// Functions to deal with scribble display
void StripTask::drawArc(TFT_TYPE& tft, float s, float e, uint16_t fg, uint16_t bg)
{
  tft.drawArc(120, 120, 110, 80, s+sa, e+sa, fg, bg);
}

void StripTask::drawTouch(TFT_TYPE& tft, uint16_t colour)
{
  tft.fillEllipse(120,210,24,16,colour);
}

// return true if change was worth drawing
bool StripTask::setArc(TFT_eSprite& tft, float newPot, colours_t& colours)
{
    bool result = false;
    const float CHANGE_THRESHOLD = CHGTHR_DP;
    if (fabs(newPot - lastPot) > CHANGE_THRESHOLD)
    {
        //Serial.printf("Pot %d: ", i);
        if (POT_NOT_SET == lastPot)
        {
            // initial background and black arc
            tft.fillScreen(colours.bg);
            drawArc(tft, 0.0f, ea-sa, TFT_BLACK, colours.bg);
            lastPot = 0.0f;
        }

        if (newPot < lastPot)
            drawArc(tft, newPot, lastPot, TFT_BLACK, colours.bg);
        else
            drawArc(tft, lastPot, newPot, colours.fg, colours.bg);

        lastPot = newPot;
        
        int32_t x,y,w,h;
        result = tft.getDirtyArea(x,y,w,h);
    }

    return result;
}


// update sprite with text in the centre of the display
// returns true if the text is different from the previous
bool StripTask::setText(TFT_eSprite& sprite, char* buf, colours_t& colours)
{
    bool result = strncmp(buf, lastString, sizeof lastString) != 0;

    if (result)
    {
        int x = 55+10*(SCRIBBLE_DP - 3),  y = 100, 
            w = 140-15*(SCRIBBLE_DP - 3), h =  45;
    
        sprite.fillRect(x,y,w,h,colours.bg); //fillSprite(bkgnds[i]);

        sprite.setFreeFont(&FONT_DP);
        sprite.setTextColor(colours.txt);
        sprite.drawString(buf, x + (buf[0] == ' '?spaceOffset:0), y);

        strncpy(lastString, buf, sizeof lastString);
        lastString[sizeof lastString - 1] = 0; // force termination
    }

    return result;
}


// update sprite with a float value in the centre of the display
// returns true if the text is different from the previous
bool StripTask::setFloat(TFT_eSprite& sprite, float potPos, colours_t& colours)
{
  char buf[BUF_SIZE];
  if (potPos < 0.0f && potPos > -CHGTHR_DP / 100.0f) potPos = 0.0f; // don't show -0.000
  sprintf(buf,FMT_DP,potPos);

  return setText(sprite, buf, colours);
}


bool StripTask::setTouch(TFT_eSprite& sprite, bool touch, colours_t& colours)
{
    bool changed = lastTouch != touch;

    if (changed)
    {
        drawTouch(sprite, touch?colours.fg:colours.bg);
        lastTouch = touch;
    }

    return changed;
}


//-------------------------------------------------------------------------

void StripTask::run(void)
{
  ring.clear();
  ring.setPattern(cfg.ringLEDs.pattern); // null pointer is OK here

  // for now, make background and text colours out of foreground:
  cfg.scribble.colours.bg  = scribble.alphaBlend( 70, cfg.scribble.colours.fg, TFT_BLACK);
  cfg.scribble.colours.txt = scribble.alphaBlend( 80, cfg.scribble.colours.fg, TFT_WHITE);
                                            /* / 255 */
  
  // wait for display init to be finished
  while (!ScribbleTask::tftInitComplete())
    vTaskDelay(5);

  // create sprite buffer in PSRAM
  int w, h;
  scribble.getTFTarea(w,h);
  scribble.createInPSRAM(true);
  taskENTER_CRITICAL();
  scribble.createSprite(w,h);
  taskEXIT_CRITICAL();
                
  // basic settings
  scribble.setSpriteSwapBytes(false);
  scribble.setFreeFont(&FONT_DP);
  scribble.setTextColor(cfg.scribble.colours.txt, cfg.scribble.colours.bg, true);
  spaceOffset = scribble.textWidth("-0") - scribble.textWidth(" 0");

  while (1)
  {
    bool wait = true;
    // we're responsible solely for the UI - real-time MIDI etc.
    // is dealt with separately by a high-priority task
    switch (scribbleState)
    {
        case ScribbleState::done:
            // poll for queued requests
            reqQueue.executeRequest(*this, 10);
            wait = false; // already waited 10 ticks
            break;

        case ScribbleState::start:
            if (updateReq.isInactive())
            {
                if (setArc(scribble, (pot.getCurrent() + 1.0f) * (ea - sa) / 2.0f, cfg.scribble.colours))
                    scribbleTask.updateDirty(updateReq, scribble, 0);
                scribbleState = ScribbleState::arc;
            }
            break;

        case ScribbleState::arc:
            if (updateReq.isInactive())
            {
                if (setFloat(scribble, pot.getCurrent(), cfg.scribble.colours))
                    scribbleTask.updateDirty(updateReq, scribble, 0);
                scribbleState = ScribbleState::done;
            }
            break;
    }

    // see if brightness needs changing
    if (bright != globalBright)
    {
        bright = globalBright;
        setDotCurrent();
    }

    // Serial.printf("%u: ring task; bits: %02X\n", xTaskGetTickCount(), bits);
    uint8_t mask = bits & (1<<(NUM_POTS - 1 - num));
    ring.setPixel(9,mask?cfg.ringLEDs.colour:TFT_BLACK,bright);

    if (wait) // poll until scribble finishes display write
        vTaskDelay(2);        
  }    
}


// We want multiple copies of this, so actually 
// generate them in setup(), by calling this (just once!):
StripTask* StripTask::tasks[NUM_POTS];
void StripTask::CreateTasks(void)
{
    char buffer[5+10+1]; // max configMAX_TASK_NAME_LEN, really

    for (int i=0;i<COUNT_OF(StripTask::tasks);i++)
    {
        TFT_eSPI& tft = ScribbleTask::getTFT(i); 
        TFT_eSprite& scribble = *(new TFT_eSprite{&tft});

        // create an instance of the StripTask class
        tasks[i] = new StripTask{"<strip>", 512, nullptr, 2,   // base task stuff
                    i, // strip number
                    5, // request queue length - just a guess
                    rings, potsTask.getPot(i),       // task-specific stuff
                    scribble,
                    faderMonsterSettings.stripsConfig[i]}; 

        // now create the FreeRTOS task to run it
        sprintf(buffer, "Strip%d", i+1);  // give it...
        tasks[i]->create(buffer);       // ...a unique name

        // Tell the pots task that we want to 
        // know about changes on a specific pot
        potsTask.setOwner(tasks[i], i);
    }
}
