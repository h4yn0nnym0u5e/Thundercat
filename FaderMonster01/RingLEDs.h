#include <WS2812Serial.h>

template<int NUM_RINGS>
class RingLEDs
{
    WS2812Serial& ledString;
    uint8_t* ledMemory;
    int* patterns[NUM_RINGS];

    const int numRings;
    const int ledsPerRing;
    const int topOffset;
    const float step;
    
    void fixRingNum(int& ring)
    {
        ring = numRings - 1 - ring;
    }

    // scale colour intensity, 0-255
    int scaleIntensity(int colour, int intensity)
    {
        int r = colour >> 16, g = (colour >> 8) & 0xFF, b = colour & 0xFF;
        r = r * intensity / 255;
        g = g * intensity / 255;
        b = b * intensity / 255;
        return (r<<16) | (g<<8) | b;
    }

    // see if we're using a colour pattern, and
    // pick relevant colour from it if so
    void fixColour(int ring, int led, int& colour)
    {
        if (colour < 0 && nullptr != patterns[ring])
            colour = patterns[ring][led];
    }

    // Return which LED corresponds to the given angle.
    // Assuming 20-LED rings, angle must be in the range -9° to 377° 
    // so we get an output of 0-20 (20 later gets mapped back to 0)
    int whichLED(float angle)
    {
        return floor((angle + step*0.5f) / step);
    }

    // Draw clockwise arc from start to end angles.
    // Must NOT transition through 0°; end must be >= start.
    // We allow end to be 360°
    void setPartArc(int ring, float start, float end, int colour, 
                    float& startFrac, float& endFrac,
                    int intensity = -1)
    {
        // find all involved LEDs:
        int startI = whichLED(start);   // first LED in arc
        int endI   = whichLED(end);     // last one; >= first one

        if (debug) Serial.printf("%.1f - %.1f; %d, %d; %.3f; ", start, end, startI, endI, endFrac);

        // light from startI to endI exclusive
        for (int led = startI + 1; led < endI; led++)
            setPixel(ring, led, colour, intensity);

        // just the ends
        float extra = (startI == endI)?0.0f:0.5f; // extra to allow back to fully-lit LEDs
        startFrac = startI - start / step + extra;
        if (debug) Serial.printf(" : s @ %.2f ", startFrac);
        setPixel(ring, startI, colour, intensity, startFrac);
        endFrac += end / step - endI + extra;
        if (startI == endI) 
        {
            endFrac += startFrac;
            startFrac = endFrac;
        }
        if (debug) Serial.printf(" : e @ %.2f ", endFrac);
        setPixel(ring,   endI, colour, intensity,   endFrac);
    }

  public:
    RingLEDs(WS2812Serial& _string, uint8_t* mem, int n, int o)
        : ledString{_string}, ledMemory{mem}, patterns{{nullptr}},
          numRings{NUM_RINGS}, ledsPerRing{n}, topOffset{o},
          step{360.0f / ledsPerRing}
          {}

    static constexpr int USE_PATTERN{-1};
    
    void begin(void) { ledString.begin(); }

    void clear(int ring)
    {
        for (int i=0;i<ledsPerRing;i++)
            setPixel(ring,i,0);
    }

    void clear(void)
    {
        for (int r=0;r<numRings;r++)
            clear(r);
    }

    void getRGB(int ring, int led, int& r, int& g, int& b)
    {
        uint8_t* pColour = ledMemory;
        fixRingNum(ring);
        led += topOffset;
        if (led >= ledsPerRing)
            led -= ledsPerRing;
        pColour += (ring * ledsPerRing + led)*3; // or 4...
 
        b = pColour[0]; g = pColour[1]; r = pColour[2];
    }

    void setPattern(int ring, int* colours)
    {
        fixRingNum(ring);
        patterns[ring] = colours;
    }

    // set a pixel in a ring to a colour
    // we count LEDs from the top=0, going clockwise
    void setPixel(int ring, int led, int colour, int intensity = -1)
    {
        // start counting from the left; the hardware
        // actually starts from the right, for hysterical raisins
        fixRingNum(ring);
        int dbgLED = led;

        // adjust colour if using pattern
        fixColour(ring, led, colour);

        if (intensity >= 0)
            colour = scaleIntensity(colour, intensity);

        // just ignore if LED is outside ring, but we allow
        // wrapping the one past last back to 0
        if (led >= 0 && led <= ledsPerRing) 
        {
            int ln = ring*ledsPerRing;
            led += topOffset;
            if (led >= ledsPerRing)
                led -= ledsPerRing;
            ln += led;
            ledString.setPixel(ln, colour);
        }
        if (debug) Serial.printf("%d,%d: %06X (%d)", ring, dbgLED, colour, intensity);
    }

    void ensurePixelVisible(int ring, int led, int colour)
    {
        int r,g,b;
        getRGB(ring,led, r,g,b); // get stored value
        if (0 == r && 0 == g && 0 == b) // led is black: set to minimum visible intensity
        {
            // adjust colour if using pattern
            fixColour(ring, led, colour);
            
            r = colour >> 16; g = (colour >> 8) & 0xFF; b = colour & 0xFF;
            int max = r;
            max = g>max?g:max;
            max = b>max?b:max;

            if (max > 0) // make sure we're not being asked to make black visible!
            {
                int intensity = 255/max;
                while (0 == (max*intensity/255))
                    intensity++;

                setPixel(ring, led, colour, intensity);
                if (debug) Serial.printf("ensure %d: %d ", led, intensity);
            }
        }
    }

    // set an LED to a fraction of the declared intensity
    // used to "pad" the ends of an arc or show intermediate values,
    // so we allow the caller to over-or under-flow and we'll manage it
    void setPixel(int ring, int led, int colour, int intensity, float frac)
    {
        // allow modulus operation here
        led = led % ledsPerRing;
        if (led < 0)
            led += ledsPerRing;

        // at full intensity, starts becoming visible at 1/8th
        // of the LED interval - too late?
        const float expAmount = 0.5f;
        int frac2 = floor(frac*255.0f); // linear
        frac = powf(256.0f, frac) - 1.0f; // 1-256 => 0-255, exponential

        frac = expAmount * frac + (1.0 - expAmount) * frac2;
        if (frac > 250.0f) // ensure we hit the maximum
            frac = 255.0f;

        if (intensity >= 0)
            frac = frac*intensity/255;

        setPixel(ring, led,  colour, frac);
    }

    void setPixel(int ring, float angle, int colour, int intensity = -1)
    {
        int mainLED = (int)(angle / step); // truncates down - for now
        float frac = (angle - mainLED*step)/step;
        float frac2 = frac;
        int secLED = mainLED+1;
        if (frac > 0.5f)
        {
            mainLED = secLED;
            secLED = mainLED - 1;
            frac = 1.0f - frac;
        }
        if (mainLED >= ledsPerRing) mainLED = 0;
        if (secLED  >= ledsPerRing) secLED  = 0;

        setPixel(ring, mainLED, colour, intensity);
        setPixel(ring, secLED,  colour, frac*2.0f);

        if (debug) Serial.printf("%d; %d@%.2f (%.2f)\n", mainLED, secLED, frac, frac2);
    }

    void setArc(int ring, float start, float end, int colour, int intensity = -1)
    {
        float startFrac = 0.0f, endFrac = 0.0f;
        if (end > start)
        {
            setPartArc(ring, start, end, colour, startFrac, endFrac, intensity);
        }
        else 
        {
            float startFrac = 0.0f, endFrac = 0.0f;
            // draw in two parts which don't cross 0°
            setPartArc(ring, 0.0f, end, colour, startFrac, endFrac, intensity);
            if (debug) Serial.println();
            endFrac = startFrac; // add this to the join
            setPartArc(ring, start, 360.0f, colour, startFrac, endFrac, intensity);
        }
    }

    void show(void) { ledString.show(); }

    bool debug;
};

template <int NUM_RINGS>
class LEDring 
{
        RingLEDs<NUM_RINGS>& rings;
    public:
        LEDring(RingLEDs<NUM_RINGS>& r, int n)
        : rings{r}, debug{r.debug}, ring{n}
        {}

        void setPattern(int* colours) { rings.setPattern(ring, colours); }
        void show(void)  { rings.show(); }
        void clear(void) { rings.clear(ring); }
        void setPixel(int led, int colour, int intensity = -1)
            { rings.setPixel(ring, led, colour, intensity); }
        void setArc(float start, float end, int colour, int intensity = -1)
            { rings.setArc(ring, start, end, colour, intensity); }
        void ensurePixelVisible(int led, int colour)
            { rings.ensurePixelVisible(ring, led, colour); }

        bool& debug;
        int ring;        
};
