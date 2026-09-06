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

//-------------------------------------------------------------------------
//         888 d8b                   888                   
//         888 Y8P                   888                   
//         888                       888                   
//     .d88888 888 .d8888b  88888b.  888  8888b.  888  888 
//    d88" 888 888 88K      888 "88b 888     "88b 888  888 
//    888  888 888 "Y8888b. 888  888 888 .d888888 888  888 
//    Y88b 888 888      X88 888 d88P 888 888  888 Y88b 888 
//     "Y88888 888  88888P' 88888P"  888 "Y888888  "Y88888 
//                          888                        888 
//                          888                   Y8b d88P 
//                          888                    "Y88P"  
//
// Functions to deal with scribble display
// Now dealt with in the UI module, according to whichever UI
// class has been loaded. Makes it "easy" to switch the display
// when a touch sensor triggers...
InterTaskRequest::Result StripTask::doPotChange(void* pNothing)
{
    setDotCurrent();

    Trigger trigger{.type    = Trigger::eTriggerType::potValue, 
                    .trigger = { .potValue = pot.getCurrent() }};
    ui.update(trigger);

    return InterTaskRequest::Result::done;
}    


InterTaskRequest::Result StripTask::doTouchChange(void* pNothing)
{
    ring.setPixel(10, potTouch?xWHITE:xBLACK,bright);
    
    Trigger trigger{.type    = Trigger::eTriggerType::pTouchStatus, 
                    .trigger = { .pTouchStatus = &potTouch }};
    ui.update(trigger);

    return InterTaskRequest::Result::done;
}    


//-------------------------------------------------------------------------
//    888                      888      
//    888                      888      
//    888                      888      
//    888888  8888b.  .d8888b  888  888 
//    888        "88b 88K      888 .88P 
//    888    .d888888 "Y8888b. 888888K  
//    Y88b.  888  888      X88 888 "88b 
//     "Y888 "Y888888  88888P' 888  888 
//
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
  
  // set the initial UI presentation on the display
  new(_ui.space) ScribblePotArc; // placement new
  ui.begin(scribble, cfg.scribble.colours);
  
  while (1)
  {
    bool wait = true;
    // we're responsible solely for the UI - real-time MIDI etc.
    // is dealt with separately by a high-priority task
    [[maybe_unused]] uint32_t pollInterval = ui.poll(); // allow UI to do internally-timed stuff
    switch (ui.state)
    {
        case UIclass::State::done: // ready for a new trigger
            // poll for queued requests
            if (InterTaskRequest::Result::inactive == reqQueue.executeRequest(*this, 10))
                wait = false; // already waited 10 ticks
            break;

        case UIclass::State::next: // can do next phase, if any
            ui.update({Trigger::eTriggerType::nextPhase});
            wait = false; // have probably changed state - loop quickly
            break;

        case UIclass::State::push:
            if (ui.writeToDisplay().isInactive()) // will change state for us, or not
                wait = false; // active - wait for display task to finish
            break;

        case UIclass::State::busy:
            if (ui.writeFinished())
                wait = false;
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
                    rings,               // task-specific stuff
                    potsTask.getPot(i), TouchTask::keyStatuses[i],      
                    scribble,
                    faderMonsterSettings.stripsConfig[i]}; 

        // now create the FreeRTOS task to run it
        sprintf(buffer, "Strip%d", i+1);  // give it...
        tasks[i]->create(buffer);       // ...a unique name

        // Tell the pots task that we want to 
        // know about changes on a specific pot
        potsTask.setOwner(tasks[i], i);

        // use pattern on some rings
        if (i>5)
            tasks[i]->useRingPattern = true;
    }
}
