// torch parameters

uint16_t cycle_waitFireBarbiePink = 1; // 0..255

byte flame_minFireBarbiePink = 100; // 0..255
byte flame_maxFireBarbiePink = 220; // 0..255

byte random_spark_probabilityFireBarbiePink = 2; // 0..100
byte spark_minFireBarbiePink = 200; // 0..255
byte spark_maxFireBarbiePink = 255; // 0..255

byte spark_tfrFireBarbiePink = 40; // 0..256 how much energy is transferred up for a spark per cycle
uint16_t spark_capFireBarbiePink = 200; // 0..255: spark cells: how much energy is retained from previous cycle

uint16_t up_radFireBarbiePink = 40; // up speed
uint16_t side_radFireBarbiePink = 35; // sidewards radiation
uint16_t heat_capFireBarbiePink = 0; // 0..255: passive cells: how much energy is retained from previous cycle

byte red_bgFireBarbiePink = 0;
byte green_bgFireBarbiePink = 0;
byte blue_bgFireBarbiePink = 0;
byte red_biasFireBarbiePink = 218;
byte green_biasFireBarbiePink = 24;
byte blue_biasFireBarbiePink = 132;
int red_energyFireBarbiePink = 255;
int green_energyFireBarbiePink = 0;
int blue_energyFireBarbiePink = 255;

byte upside_downFireBarbiePink = 0; // Invert effect. 0 disabled / 1 enabled

// torch mode

#define numLeds NUM_LEDS
#define ledsPerLevel MATRIX_WIDTH
#define levels MATRIX_HEIGHT

byte currentEnergyFireBarbiePink[numLeds]; // current energy level
byte nextEnergyFireBarbiePink[numLeds]; // next energy level
byte energyModeFireBarbiePink[numLeds]; // mode how energy is calculated for this point

enum {
  torch_passiveFireBarbiePink = 1, // just environment, glow from nearby radiation
  torch_nopFireBarbiePink = 1, // no processing
  torch_sparkFireBarbiePink= 2, // slowly looses energy, moves up
  torch_sparkFireBarbiePink_temp = 3, // a spark still getting energy from the level below
};

inline void reduceFireBarbiePink(byte &aByte, byte aAmount, byte aMin = 0)
{
  int r = aByte-aAmount;
  if (r<aMin)
    aByte = aMin;
  else
    aByte = (byte)r;
}

inline void increaseFireBarbiePink(byte &aByte, byte aAmount, byte aMax = 255)
{
  int r = aByte+aAmount;
  if (r>aMax)
    aByte = aMax;
  else
    aByte = (byte)r;
}

uint16_t randomFireBarbiePink(uint16_t aMinOrMax, uint16_t aMax = 0)
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

void resetEnergyFireBarbiePink()
{
  for (int i=0; i<numLeds; i++) {
    currentEnergyFireBarbiePink[i] = 0;
    nextEnergyFireBarbiePink[i] = 0;
    energyModeFireBarbiePink[i] = torch_passiveFireBarbiePink;
  }
}

void calcnextEnergyFireBarbiePink()
{
  int i = 0;
  for (int y=0; y<levels; y++) {
    for (int x=0; x<ledsPerLevel; x++) {
      byte e = currentEnergyFireBarbiePink[i];
      byte m = energyModeFireBarbiePink[i];
      switch (m) {
        case torch_sparkFireBarbiePink: {
          // loose transfer up energy as long as the is any
          reduceFireBarbiePink(e, spark_tfrFireBarbiePink);
          // cell above is temp spark, sucking up energy from this cell until empty
          if (y<levels-1) {
            energyModeFireBarbiePink[i+ledsPerLevel] = torch_sparkFireBarbiePink_temp;
          }
          break;
        }
        case torch_sparkFireBarbiePink_temp: {
          // just getting some energy from below
          byte e2 = currentEnergyFireBarbiePink[i-ledsPerLevel];
          if (e2<spark_tfrFireBarbiePink) {
            // cell below is exhausted, becomes passive
            energyModeFireBarbiePink[i-ledsPerLevel] = torch_passiveFireBarbiePink;
            // gobble up rest of energy
            increaseFireBarbiePink(e, e2);
            // loose some overall energy
            e = ((int)e*spark_capFireBarbiePink)>>8;
            // this cell becomes active spark
            energyModeFireBarbiePink[i] = torch_sparkFireBarbiePink;
          }
          else {
            increaseFireBarbiePink(e, spark_tfrFireBarbiePink);
          }
          break;
        }
        case torch_passiveFireBarbiePink: {
          e = ((int)e*heat_capFireBarbiePink)>>8;
          increaseFireBarbiePink(e, ((((int)currentEnergyFireBarbiePink[i-1]+(int)currentEnergyFireBarbiePink[i+1])*side_radFireBarbiePink)>>9) + (((int)currentEnergyFireBarbiePink[i-ledsPerLevel]*up_radFireBarbiePink)>>8));
        }
        default:
          break;
      }
      nextEnergyFireBarbiePink[i++] = e;
    }
  }
}

const uint8_t energymapFireBarbiePink[32] = {0, 64, 96, 112, 128, 144, 152, 160, 168, 176, 184, 184, 192, 200, 200, 208, 208, 216, 216, 224, 224, 224, 232, 232, 232, 240, 240, 240, 240, 248, 248, 248};

void calcNextColorsFireBarbiePink()
{
  for (int i=0; i<numLeds; i++) {
    int ei; // index into energy calculation buffer
    if (upside_downFireBarbiePink)
      ei = numLeds-i;
    else
      ei = i;
    uint16_t e = nextEnergyFireBarbiePink[ei];
    currentEnergyFireBarbiePink[ei] = e;
    if (e>250)
      leds[i] = CRGB(0, 0, 0); // blueish extra-bright spark
    else {
      if (e>0) {
        // energy to brightness is non-linear
        byte eb = energymapFireBarbiePink[e>>3];
        byte r = red_biasFireBarbiePink;
        byte g = green_biasFireBarbiePink;
        byte b = blue_biasFireBarbiePink;
        increaseFireBarbiePink(r, (eb*red_energyFireBarbiePink)>>8);
        increaseFireBarbiePink(g, (eb*green_energyFireBarbiePink)>>8);
        increaseFireBarbiePink(b, (eb*blue_energyFireBarbiePink)>>8);
        leds[i] = CRGB(r, g, b);
      }
      else {
        // background, no energy
        leds[i] = CRGB(red_bgFireBarbiePink, green_bgFireBarbiePink, blue_bgFireBarbiePink);
      }
    }
  }
}

void injectRandomFireBarbiePink()
{
  // random flame energy at bottom row
  for (int i=0; i<ledsPerLevel; i++) {
    currentEnergyFireBarbiePink[i] = random8(flame_minFireBarbiePink, flame_maxFireBarbiePink);
    energyModeFireBarbiePink[i] = torch_nopFireBarbiePink;
  }
  // random sparks at second row
  for (int i=ledsPerLevel; i<2*ledsPerLevel; i++) {
    if (energyModeFireBarbiePink[i]!=torch_sparkFireBarbiePink && random8(100)<random_spark_probabilityFireBarbiePink) {
      currentEnergyFireBarbiePink[i] = random8(spark_minFireBarbiePink, spark_maxFireBarbiePink);
      energyModeFireBarbiePink[i] = torch_sparkFireBarbiePink;
    }
  }
}

uint16_t FireBarbiePink() {
  injectRandomFireBarbiePink();
  calcnextEnergyFireBarbiePink();
  calcNextColorsFireBarbiePink();
  return 1;
}
