// Slightly modified version of the fire pattern from MessageTorch by Lukas Zeller:
// https://github.com/plan44/messagetorch

// The MIT License (MIT)

// Copyright (c) 2014 Lukas Zeller

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

// torch parameters

uint16_t cycle_waitFireYellow = 1; // 0..255

byte flame_minFireYellow = 70; // 0..255
byte flame_maxFireYellow = 250; // 0..255

byte random_spark_probabilityFireYellow = 1; // 0..100
byte spark_minFireYellow = 60; // 0..255
byte spark_maxFireYellow = 255; // 0..255

byte spark_tfrFireYellow = 40; // 0..256 how much energy is transferred up for a spark per cycle
uint16_t spark_capFireYellow = 200; // 0..255: spark cells: how much energy is retained from previous cycle

uint16_t up_radFireYellow = 50; // up radiation
uint16_t side_radFireYellow = 40; // sidewards radiation
uint16_t heat_capFireYellow = 0; // 0..255: passive cells: how much energy is retained from previous cycle

byte red_bgFireYellow = 0;
byte green_bgFireYellow = 0;
byte blue_bgFireYellow = 0;
byte red_biasFireYellow = 150;
byte green_biasFireYellow = 150;
byte blue_biasFireYellow = 0;
int red_energyFireYellow = 255;
int green_energyFireYellow = 255;
int blue_energyFireYellow = 0;

byte upside_downFireYellow = 0; // if set, flame (or rather: drop) animation is upside down. Text remains as-is

// torch mode
// ==========

byte currentEnergyFireYellow[numLeds]; // current energy level
byte nextEnergyFireYellow[numLeds]; // next energy level
byte energyModeFireYellow[numLeds]; // mode how energy is calculated for this point

enum {
  torch_passiveFireYellow = 0, // just environment, glow from nearby radiation
  torch_nopFireYellow = 1, // no processing
  torch_sparkFireYellow= 2, // slowly looses energy, moves up
  torch_sparkFireYellow_temp = 3, // a spark still getting energy from the level below
};

inline void reduceFireYellow(byte &aByte, byte aAmount, byte aMin = 0)
{
  int r = aByte-aAmount;
  if (r<aMin)
    aByte = aMin;
  else
    aByte = (byte)r;
}


inline void increaseFireYellow(byte &aByte, byte aAmount, byte aMax = 255)
{
  int r = aByte+aAmount;
  if (r>aMax)
    aByte = aMax;
  else
    aByte = (byte)r;
}

uint16_t randomFireYellow(uint16_t aMinOrMax, uint16_t aMax = 0)  // not really sure if this is needed at this stage
{
  if (aMax==0) {
    aMax = aMinOrMax;
    aMinOrMax = 0;
  }
  uint32_t r = aMinOrMax;
  aMax = aMax - aMinOrMax + 1;
  r += rand() % aMax;
  return r;
}

void resetEnergy6()
{
  for (int i=0; i<numLeds; i++) {
    currentEnergyFireYellow[i] = 0;
    nextEnergyFireYellow[i] = 0;
    energyModeFireYellow[i] = torch_passiveFireYellow;
  }
}

void calcnextEnergyFireYellow()
{
  int i = 0;
  for (int y=0; y<levels; y++) {
    for (int x=0; x<ledsPerLevel; x++) {
      byte e = currentEnergyFireYellow[i];
      byte m = energyModeFireYellow[i];
      switch (m) {
        case torch_sparkFireYellow: {
          // loose transfer up energy as long as the is any
          reduceFireYellow(e, spark_tfrFireYellow);
          // cell above is temp spark, sucking up energy from this cell until empty
          if (y<levels-1) {
            energyModeFireYellow[i+ledsPerLevel] = torch_sparkFireYellow_temp;
          }
          break;
        }
        case torch_sparkFireYellow_temp: {
          // just getting some energy from below
          byte e2 = currentEnergyFireYellow[i-ledsPerLevel];
          if (e2<spark_tfrFireYellow) {
            // cell below is exhausted, becomes passive
            energyModeFireYellow[i-ledsPerLevel] = torch_passive;
            // gobble up rest of energy
            increaseFireYellow(e, e2);
            // loose some overall energy
            e = ((int)e*spark_capFireYellow)>>8;
            // this cell becomes active spark
            energyModeFireYellow[i] = torch_sparkFireYellow;
          }
          else {
            increaseFireYellow(e, spark_tfrFireYellow);
          }
          break;
        }
        case torch_passive: {
          e = ((int)e*heat_capFireYellow)>>8;
          increaseFireYellow(e, ((((int)currentEnergyFireYellow[i-1]+(int)currentEnergyFireYellow[i+1])*side_radFireYellow)>>9) + (((int)currentEnergyFireYellow[i-ledsPerLevel]*up_radFireYellow)>>8));
        }
        default:
          break;
      }
      nextEnergyFireYellow[i++] = e;
    }
  }
}

const uint8_t energymapFireYellow[32] = {0, 64, 96, 112, 128, 144, 152, 160, 168, 176, 184, 184, 192, 200, 200, 208, 208, 216, 216, 224, 224, 224, 232, 232, 232, 240, 240, 240, 240, 248, 248, 248};

void calcNextColorsFireYellow()
{
  for (int i=0; i<numLeds; i++) {
    int ei; // index into energy calculation buffer
    if (upside_downFireYellow)
      ei = numLeds-i;
    else
      ei = i;
    uint16_t e = nextEnergyFireYellow[ei];
    currentEnergyFireYellow[ei] = e;
    if (e>250)
      leds[i] = CRGB(170, 170, e); // blueish extra-bright spark
    else {
      if (e>0) {
        // energy to brightness is non-linear
        byte eb = energymap[e>>3];
        byte r = red_biasFireYellow;
        byte g = green_biasFireYellow;
        byte b = blue_biasFireYellow;
        increaseFireYellow(r, (eb*red_energyFireYellow)>>8);
        increaseFireYellow(g, (eb*green_energyFireYellow)>>8);
        increaseFireYellow(b, (eb*blue_energyFireYellow)>>8);
        leds[i] = CRGB(r, g, b);
      }
      else {
        // background, no energy
        leds[i] = CRGB(red_bgFireYellow, green_bgFireYellow, blue_bgFireYellow);
      }
    }
  }
}

void injectRandomFireYellow()
{
  // random flame energy at bottom row
  for (int i=0; i<ledsPerLevel; i++) {
    currentEnergyFireYellow[i] = random8(flame_minFireYellow, flame_maxFireYellow);
    energyModeFireYellow[i] = torch_nopFireYellow;
  }
  // random sparks at second row
  for (int i=ledsPerLevel; i<2*ledsPerLevel; i++) {
    if (energyModeFireYellow[i]!=torch_sparkFireYellow && random8(100)<random_spark_probabilityFireYellow) {
      currentEnergyFireYellow[i] = random8(spark_minFireYellow, spark_maxFireYellow);
      energyModeFireYellow[i] = torch_sparkFireYellow;
    }
  }
}

uint16_t FireYellow() {
  injectRandomFireYellow();
  calcnextEnergyFireYellow();
  calcNextColorsFireYellow();
  return 1;
}
