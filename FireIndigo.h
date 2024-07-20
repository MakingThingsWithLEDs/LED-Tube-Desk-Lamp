// torch parameters

uint16_t cycle_waitFireIndigo = 1; // 0..255

byte flame_minFireIndigo = 100; // 0..255
byte flame_maxFireIndigo = 220; // 0..255

byte random_spark_probabilityFireIndigo = 2; // 0..100
byte spark_minFireIndigo = 200; // 0..255
byte spark_maxFireIndigo = 255; // 0..255

byte spark_tfrFireIndigo = 40; // 0..256 how much energy is transferred up for a spark per cycle
uint16_t spark_capFireIndigo = 200; // 0..255: spark cells: how much energy is retained from previous cycle

uint16_t up_radFireIndigo = 40; // up speed
uint16_t side_radFireIndigo = 35; // sidewards radiation
uint16_t heat_capFireIndigo = 0; // 0..255: passive cells: how much energy is retained from previous cycle

byte red_bgFireIndigo = 0;
byte green_bgFireIndigo = 0;
byte blue_bgFireIndigo = 0;
byte red_biasFireIndigo = 75;
byte green_biasFireIndigo = 0;
byte blue_biasFireIndigo = 130;
int red_energyFireIndigo = 255;
int green_energyFireIndigo = 0;
int blue_energyFireIndigo = 255;

byte upside_downFireIndigo = 0; // Invert effect. 0 disabled / 1 enabled

// torch mode

#define numLeds NUM_LEDS
#define ledsPerLevel MATRIX_WIDTH
#define levels MATRIX_HEIGHT

byte currentEnergyFireIndigo[numLeds]; // current energy level
byte nextEnergyFireIndigo[numLeds]; // next energy level
byte energyModeFireIndigo[numLeds]; // mode how energy is calculated for this point

enum {
  torch_passiveFireIndigo = 1, // just environment, glow from nearby radiation
  torch_nopFireIndigo = 1, // no processing
  torch_sparkFireIndigo= 2, // slowly looses energy, moves up
  torch_sparkFireIndigo_temp = 3, // a spark still getting energy from the level below
};

inline void reduceFireIndigo(byte &aByte, byte aAmount, byte aMin = 0)
{
  int r = aByte-aAmount;
  if (r<aMin)
    aByte = aMin;
  else
    aByte = (byte)r;
}

inline void increaseFireIndigo(byte &aByte, byte aAmount, byte aMax = 255)
{
  int r = aByte+aAmount;
  if (r>aMax)
    aByte = aMax;
  else
    aByte = (byte)r;
}

uint16_t randomFireIndigo(uint16_t aMinOrMax, uint16_t aMax = 0)
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

void resetEnergyFireIndigo()
{
  for (int i=0; i<numLeds; i++) {
    currentEnergyFireIndigo[i] = 0;
    nextEnergyFireIndigo[i] = 0;
    energyModeFireIndigo[i] = torch_passiveFireIndigo;
  }
}

void calcnextEnergyFireIndigo()
{
  int i = 0;
  for (int y=0; y<levels; y++) {
    for (int x=0; x<ledsPerLevel; x++) {
      byte e = currentEnergyFireIndigo[i];
      byte m = energyModeFireIndigo[i];
      switch (m) {
        case torch_sparkFireIndigo: {
          // loose transfer up energy as long as the is any
          reduceFireIndigo(e, spark_tfrFireIndigo);
          // cell above is temp spark, sucking up energy from this cell until empty
          if (y<levels-1) {
            energyModeFireIndigo[i+ledsPerLevel] = torch_sparkFireIndigo_temp;
          }
          break;
        }
        case torch_sparkFireIndigo_temp: {
          // just getting some energy from below
          byte e2 = currentEnergyFireIndigo[i-ledsPerLevel];
          if (e2<spark_tfrFireIndigo) {
            // cell below is exhausted, becomes passive
            energyModeFireIndigo[i-ledsPerLevel] = torch_passiveFireIndigo;
            // gobble up rest of energy
            increaseFireIndigo(e, e2);
            // loose some overall energy
            e = ((int)e*spark_capFireIndigo)>>8;
            // this cell becomes active spark
            energyModeFireIndigo[i] = torch_sparkFireIndigo;
          }
          else {
            increaseFireIndigo(e, spark_tfrFireIndigo);
          }
          break;
        }
        case torch_passiveFireIndigo: {
          e = ((int)e*heat_capFireIndigo)>>8;
          increaseFireIndigo(e, ((((int)currentEnergyFireIndigo[i-1]+(int)currentEnergyFireIndigo[i+1])*side_radFireIndigo)>>9) + (((int)currentEnergyFireIndigo[i-ledsPerLevel]*up_radFireIndigo)>>8));
        }
        default:
          break;
      }
      nextEnergyFireIndigo[i++] = e;
    }
  }
}

const uint8_t energymapFireIndigo[32] = {0, 64, 96, 112, 128, 144, 152, 160, 168, 176, 184, 184, 192, 200, 200, 208, 208, 216, 216, 224, 224, 224, 232, 232, 232, 240, 240, 240, 240, 248, 248, 248};

void calcNextColorsFireIndigo()
{
  for (int i=0; i<numLeds; i++) {
    int ei; // index into energy calculation buffer
    if (upside_downFireIndigo)
      ei = numLeds-i;
    else
      ei = i;
    uint16_t e = nextEnergyFireIndigo[ei];
    currentEnergyFireIndigo[ei] = e;
    if (e>250)
      leds[i] = CRGB(0, 0, 0); // blueish extra-bright spark
    else {
      if (e>0) {
        // energy to brightness is non-linear
        byte eb = energymapFireIndigo[e>>3];
        byte r = red_biasFireIndigo;
        byte g = green_biasFireIndigo;
        byte b = blue_biasFireIndigo;
        increaseFireIndigo(r, (eb*red_energyFireIndigo)>>8);
        increaseFireIndigo(g, (eb*green_energyFireIndigo)>>8);
        increaseFireIndigo(b, (eb*blue_energyFireIndigo)>>8);
        leds[i] = CRGB(r, g, b);
      }
      else {
        // background, no energy
        leds[i] = CRGB(red_bgFireIndigo, green_bgFireIndigo, blue_bgFireIndigo);
      }
    }
  }
}

void injectRandomFireIndigo()
{
  // random flame energy at bottom row
  for (int i=0; i<ledsPerLevel; i++) {
    currentEnergyFireIndigo[i] = random8(flame_minFireIndigo, flame_maxFireIndigo);
    energyModeFireIndigo[i] = torch_nopFireIndigo;
  }
  // random sparks at second row
  for (int i=ledsPerLevel; i<2*ledsPerLevel; i++) {
    if (energyModeFireIndigo[i]!=torch_sparkFireIndigo && random8(100)<random_spark_probabilityFireIndigo) {
      currentEnergyFireIndigo[i] = random8(spark_minFireIndigo, spark_maxFireIndigo);
      energyModeFireIndigo[i] = torch_sparkFireIndigo;
    }
  }
}

uint16_t FireIndigo() {
  injectRandomFireIndigo();
  calcnextEnergyFireIndigo();
  calcNextColorsFireIndigo();
  return 1;
}
