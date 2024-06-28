// parameters

uint16_t cycle_waitFireAqua = 1; // 0..255

byte flame_minFireAqua = 100; // 0..255
byte flame_maxFireAqua = 240; // 0..255

byte random_spark_probabilityFireAqua = 2; // 0..100
byte spark_minFireAqua = 100; // 0..255
byte spark_maxFireAqua = 225; // 0..255

byte spark_tfrFireAqua = 40; // 0..256 how much energy is transferred up for a spark per cycle
uint16_t spark_capFireAqua = 200; // 0..255: spark cells: how much energy is retained from previous cycle

uint16_t up_radFireAqua = 40; // up radiation
uint16_t side_radFireAqua = 30; // sidewards radiation
uint16_t heat_capFireAqua = 5; // 0..255: passive cells: how much energy is retained from previous cycle

// BACKGROUND COLOURS
byte red_bgFireAqua      = 0;
byte green_bgFireAqua    = 5;
byte blue_bgFireAqua     = 5;
// FLAME COLOUR
byte red_biasFireAqua    = 0;
byte green_biasFireAqua  = 10;
byte blue_biasFireAqua   = 10;
int red_energyFireAqua   = 0;
int green_energyFireAqua = 255; // 145;
int blue_energyFireAqua  = 255;
// FLAME DIRECTION
byte upside_downFireAqua = 0; // if set, flame (or rather: drop) animation is upside down. Text remains as-is

// torch mode
// ==========
#define numLeds NUM_LEDS
#define ledsPerLevel MATRIX_WIDTH
#define levels MATRIX_HEIGHT

byte currentEnergyFireAqua[numLeds]; // current energy level
byte nextEnergyFireAqua[numLeds]; // next energy level
byte energyModeFireAqua[numLeds]; // mode how energy is calculated for this point

enum {
  torch_passiveFireAqua = 1, // just environment, glow from nearby radiation
  torch_nopFireAqua = 1, // no processing
  torch_sparkFireAqua= 2, // slowly looses energy, moves up
  torch_sparkFireAqua_temp = 3, // a spark still getting energy from the level below
};

inline void reduceFireAqua(byte &aByte, byte aAmount, byte aMin = 0)
{
  int r = aByte-aAmount;
  if (r<aMin)
    aByte = aMin;
  else
    aByte = (byte)r;
}


inline void increaseFireAqua(byte &aByte, byte aAmount, byte aMax = 255)
{
  int r = aByte+aAmount;
  if (r>aMax)
    aByte = aMax;
  else
    aByte = (byte)r;
}

uint16_t randomFireAqua(uint16_t aMinOrMax, uint16_t aMax = 0)  // not really sure if this is needed at this stage
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

void resetEnergyFireAqua()
{
  for (int i=0; i<numLeds; i++) {
    currentEnergyFireAqua[i] = 0;
    nextEnergyFireAqua[i] = 0;
    energyModeFireAqua[i] = torch_passiveFireAqua;
  }
}

void calcnextEnergyFireAqua()
{
  int i = 0;
  for (int y=0; y<levels; y++) {
    for (int x=0; x<ledsPerLevel; x++) {
      byte e = currentEnergyFireAqua[i];
      byte m = energyModeFireAqua[i];
      switch (m) {
        case torch_sparkFireAqua: {
          // loose transfer up energy as long as the is any
          reduceFireAqua(e, spark_tfrFireAqua);
          // cell above is temp spark, sucking up energy from this cell until empty
          if (y<levels-1) {
            energyModeFireAqua[i+ledsPerLevel] = torch_sparkFireAqua_temp;
          }
          break;
        }
        case torch_sparkFireAqua_temp: {
          // just getting some energy from below
          byte e2 = currentEnergyFireAqua[i-ledsPerLevel];
          if (e2<spark_tfrFireAqua) {
            // cell below is exhausted, becomes passive
            energyModeFireAqua[i-ledsPerLevel] = torch_passiveFireAqua;
            // gobble up rest of energy
            increaseFireAqua(e, e2);
            // loose some overall energy
            e = ((int)e*spark_capFireAqua)>>8;
            // this cell becomes active spark
            energyModeFireAqua[i] = torch_sparkFireAqua;
          }
          else {
            increaseFireAqua(e, spark_tfrFireAqua);
          }
          break;
        }
        case torch_passiveFireAqua: {
          e = ((int)e*heat_capFireAqua)>>8;
          increaseFireAqua(e, ((((int)currentEnergyFireAqua[i+1]+(int)currentEnergyFireAqua[i+1])*side_radFireAqua)>>9) + (((int)currentEnergyFireAqua[i-ledsPerLevel]*up_radFireAqua)>>8));
        }
        default:
          break;
      }
      nextEnergyFireAqua[i++] = e;
    }
  }
}

const uint8_t energymapFireAqua[32] = {0, 64, 96, 112, 128, 144, 152, 160, 168, 176, 184, 184, 192, 200, 200, 208, 208, 216, 216, 224, 224, 224, 232, 232, 232, 240, 240, 240, 240, 248, 248, 248};

void calcNextColorsFireAqua()
{
  for (int i=0; i<numLeds; i++) {
    int ei; // index into energy calculation buffer
    if (upside_downFireAqua)
      ei = numLeds-i;
    else
      ei = i;
    uint16_t e = nextEnergyFireAqua[ei];
    currentEnergyFireAqua[ei] = e;
    if (e>250)
      leds[i] = CRGB(177, 177, e); // blueish extra-bright spark
    else {
      if (e>0) {
        // energy to brightness is non-linear
        byte eb = energymapFireAqua[e>>3];
        byte r = red_biasFireAqua;
        byte g = green_biasFireAqua;
        byte b = blue_biasFireAqua;
        increaseFireAqua(r, (eb*red_energyFireAqua)>>8);
        increaseFireAqua(g, (eb*green_energyFireAqua)>>8);
        increaseFireAqua(b, (eb*blue_energyFireAqua)>>8);
        leds[i] = CRGB(r, g, b);
      }
      else {
        // background, no energy
        leds[i] = CRGB(red_bgFireAqua, green_bgFireAqua, blue_bgFireAqua);
      }
    }
  }
}

void injectRandomFireAqua()
{
  // // random flame energy at bottom row
  // for (int i=0; i<ledsPerLevel; i++) {
  //   currentEnergyFireAqua[i] = randomFireAqua(flame_minFireAqua, flame_maxFireAqua);
  //   energyModeFireAqua[i] = torch_nopFireAqua;
  // }
  // random sparks at second row
     for (int i=0; i<ledsPerLevel -1; i++) {
    if (energyModeFireAqua[i]!=torch_sparkFireAqua && randomFireAqua(100)<random_spark_probabilityFireAqua) {
      currentEnergyFireAqua[i] = randomFireAqua(spark_minFireAqua, spark_maxFireAqua);
      energyModeFireAqua[i] = torch_sparkFireAqua;
    }
  }
}

uint16_t FireAqua() {
  injectRandomFireAqua();
  calcnextEnergyFireAqua();
  calcNextColorsFireAqua();
  return 1;
}
