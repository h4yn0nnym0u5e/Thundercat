#include <WS2812Serial.h>

class RingLEDs
{
    WS2812Serial& ledString;
    const int numRings;
    const int ledsPerRing;
    const int topOffset;
    const float step;    

    // scale colour intensity, 0-255
    int scaleIntensity(int colour, int intensity)
    {
        int r = colour >> 16, g = (colour >> 8) & 0xFF, b = colour & 0xFF;
        r = r * intensity / 255;
        g = g * intensity / 255;
        b = b * intensity / 255;
        return (r<<16) | (g<<8) | b;
    }

  public:
    RingLEDs(WS2812Serial& _string, int r, int n, int o)
        : ledString{_string}, numRings{r}, ledsPerRing{n}, topOffset{o},
          step{360.0f / ledsPerRing}
          {}

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

    // set a pixel in a ring to a colour
    // we count LEDs from the top=0, going clockwise
    void setPixel(int ring, int led, int colour, int intensity = -1)
    {
        if (intensity >= 0)
            colour = scaleIntensity(colour, intensity);

        if (led >= 0 && led < ledsPerRing) // just ignore if LED is outside ring
        {
            int ln = ring*ledsPerRing;
            led += topOffset;
            if (led >= ledsPerRing)
                led -= ledsPerRing;
            ln += led;
            ledString.setPixel(ln, colour);
        }
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
        
        frac = powf(256.0f, frac*2.0f) - 1.0f; // 1-256 => 0-255

        if (intensity >= 0)
            frac = frac*intensity;

        setPixel(ring, mainLED, colour, intensity);
        if (frac > 1.0f)
            setPixel(ring, secLED,  colour, frac);
        if (debug) Serial.printf("%d; %d@%.2f (%.2f)\n", mainLED, secLED, frac, frac2);
    }

    // draw clockwise arc from start to end angles
    void setArc(int ring, float start, float end, int colour, int intensity = -1)
    {
        const float step = 360.0f / ledsPerRing;
        float led = start;
        if (end < start) // goes through 0°
        {
            led = 0.0f;
            while (led < end)
            {
                setPixel(ring, led, colour, intensity);
                led += step;
            }
            led = start;
            end = 359.99f;
        }
        while (led < end)
        {
            setPixel(ring, led, colour, intensity);
            led += step;
        }
    }

    void show(void) { ledString.show(); }

    bool debug;
};