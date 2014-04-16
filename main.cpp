// SFML_TVout ... port of TVout library to SFML / Jonne Valola

#include <SFML/Graphics.hpp>
#include <iostream>
#include <math.h>
#include "wrapper.h"
#include "TVout.h"
#include "fontALL.h"
#include "schematic.h"
#include "TVOlogo.h"
#include <string.h>
#include <vector>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */



using namespace std;
using namespace sf;

void mixbuffers(float,float);

typedef unsigned char byte;
typedef unsigned char prog_char;
typedef bool boolean;
#define PROGMEM

int random(int a, int b) {
    int r; //Arduino random(a,b) EXCLUDES b !!!
    if (b < 1) return 0;
    if (a < 0) return 0;
    r = rand() % (b - a) + a ;
    return  r;
}

byte pgm_read_byte(unsigned char* pointer) {
    return *pointer;
}

byte pgm_read_word(unsigned int* pointer) {
    return *pointer;
}

// START OF ARDUINO CODE

#include <Controllers.h>
#define FMAX    1200
#define FMIN    10
#define WAVES   7
#define NUM_WAVES 7
#define NUM_KNOBS 10
#define SAMPLES 16001*2

#define set_bit(v,b) v |= _BV(b)
#define clear_bit(v,b) v &= ~_BV(b)
#define pulse_bit(v,b) do { set_bit(v,b); clear_bit(v,b); } while(0)
#define pulse_bit_low(v,b) do { clear_bit(v,b); set_bit(v,b); } while(0)

#define PASTE(x,y) x ## y
#define PORT(x) PASTE(PORT,x)
#define PIN(x) PASTE(PIN,x)
#define DDR(x) PASTE(DDR,x)

#define AUDIO B
#define AUDIO_PIN 0

#define LEDS B
#define LFO_LED_PIN 2
#define ENV_LED_PIN 3
#define PWR_LED_PIN 4



const char *potnames[] = {"OSC_1",
                            " VOL ",
                            " FRQ ",
                            "PULSE",
                            " GAP ",
                            "OSC_2",
                            " AMP ",
                            " FRQ ",
                            "PULSE",
                            " GAP "};
#define POTRAD 6

// parameters
uint16_t pitch = 200; // set pitch
uint16_t lfofreq = 0; // lfo period
uint16_t lfodepth = 0; // lfo wave amplitude
uint8_t envfreq = 0; // envelope period
uint8_t pulse = 0; // osc1 envelope period
uint8_t gap = 0; // osc1 envelope period
uint8_t envwidth = 255; // envelope duty cycle
volatile int8_t wavenum = 7; // oscillator waveform
uint8_t lfowavenum = 7; // lfo waveform
uint8_t vol = 128; // main osc volume

// state
uint16_t outpitch = 0; // OCR1A value
uint8_t lfotimer = 0; // lfo value, counts from 0 to lfofreq
uint16_t lfoval = 0; // amount to add to output pitch
uint16_t lfodelta = 0;
uint16_t envtimer = 0; // envelope value, counts from 0 to envfreq
volatile uint8_t envval = 1; // high or low; ANDed with output wave
volatile uint16_t waveform;
volatile uint8_t noteon = 1; // if note is on

// my stuff
float px1 = viewWidth/4, py1 = viewHeight/4,pa=0;
float px2 = viewWidth/2, py2 = viewHeight/4;
int currentPot =0;



void drawPot(int x,int y,float potvalue,int minval, int maxval,int potnum) {
    char name[20];
    char numchars=1;
    int dispvalue = (int) potvalue;
    strcpy(name, potnames[potnum]);
    potvalue = (float)potvalue - minval;
    potvalue = (float)potvalue/maxval;
    potvalue = (float)potvalue*2*PI;
    TV.draw_circle(x,y,POTRAD+2,0,0); // unselect
    if (currentPot == potnum) TV.draw_circle(x,y,POTRAD+2,1,0); // selected
    TV.draw_circle(x,y,POTRAD,1,0); // knob itsel
    TV.draw_line(x,y,x+cos(potvalue-PI/2)*POTRAD,y+sin(potvalue-PI/2)*POTRAD,1);
    if (dispvalue>9) numchars=2;
    if (dispvalue>99) numchars=3;
    TV.print(x-6/2*4-1,y-POTRAD*2-5,"       ");
    if (potnum == 5 || potnum == 0) {
    int tempwavenum = wavenum;
    if (potnum == 5) tempwavenum=lfowavenum;
    switch (tempwavenum) {
        case 0:
        TV.print(x-6/2*4-1,y-POTRAD*2-5,"square");
        break;
        case 1:
        TV.print(x-6/2*4-1,y-POTRAD*2-5,"triang");
        break;
        case 2:
        TV.print(x-6/2*4-1,y-POTRAD*2-5,"sawtot");
        break;
        case 3:
        TV.print(x-6/2*4-1,y-POTRAD*2-5,"invsaw");
        break;
        case 4:
        TV.print(x-6/2*4-1,y-POTRAD*2-5,"1/4 sq");
        break;
        case 5:
        TV.print(x-6/2*4-1,y-POTRAD*2-5,"3/4 sq");
        break;
        case 6:
        TV.print(x-6/2*4-1,y-POTRAD*2-5,"noise");
        break;
        case 7:
        TV.print(x-6/2*4-1,y-POTRAD*2-5," off ");
        break;

    }
    } else {
    TV.print(x-numchars/2*4-1,y-POTRAD*2-5,dispvalue);
    }

    TV.print(x-(sizeof(potnames[potnum])+1)/2*4-3,y+POTRAD+5,name);
}


void increaseCurrentPot(){
    switch (currentPot) {
    case 5:
    if (lfowavenum < NUM_WAVES) { lfowavenum++; } else {lfowavenum=0;}
        break;
    case 8:
    if (envwidth < 255) { envwidth++; } else {envwidth=0;}
        break;
    case 6:
    if (lfodepth < 255-8) { lfodepth += 8 ; } else {lfodepth = 255;}
        break;
    case 7:
    if (lfofreq < 200-5) { lfofreq += 5 ; } else {lfofreq = 200;}
        break;
    case 2:
    if (pitch < FMAX-10) { pitch+= 10 ; } else {pitch = FMAX;}
        break;
    case 1:
    if (vol < 255-5) { vol+= 5 ; } else {vol = 255;}
        break;
    case 9:
    if (envfreq < 255-10) { envfreq += 10 ; } else { envfreq = 255;}
        break;
    case 0:
    if (wavenum < NUM_WAVES) { wavenum++; } else { wavenum=0; }
        break;
    case 3:
    if (pulse < 16) { pulse ++; } else {pulse = 16;}
        break;
    case 4:
    if (gap < 16) { gap ++; } else {gap = 16;}
        break;
    default:
        break;
    }
}

void decreaseCurrentPot(){
    switch (currentPot) {
    case 5:
    if (lfowavenum > 0) { lfowavenum--; } else {lfowavenum=NUM_WAVES;}
        break;
    case 8:
    if (envwidth > 0) { envwidth--; } else {envwidth=255;}
        break;
    case 6:
    if (lfodepth > 8) { lfodepth -= 8 ; } else {lfodepth = 0;}
        break;
    case 7:
    if (lfofreq > 5) { lfofreq -= 5 ; } else {lfofreq = 0;}
        break;
    case 2:
    if (pitch > FMIN) { pitch-= 10 ; } else {pitch = FMIN;}
        break;
    case 1:
    if (vol > 5) { vol-= 5 ; } else {vol = 0;}
        break;
    case 9:
    if (envfreq > 10) { envfreq -= 10 ; } else { envfreq = 0;}
        break;
    case 0:
    if (wavenum > 0) { wavenum--; } else { wavenum=NUM_WAVES; }
        break;
    case 3:
    if (pulse > 0) { pulse--; } else {pulse=0;}
        break;
    case 4:
    if (gap > 0) { gap--; } else {gap=0;}
        break;
    default:
        break;
    }
}


void updatePots() {

    // row 1
    drawPot(7+POTRAD+0*viewWidth/5,viewHeight/3,wavenum,0,8,0);
    drawPot(7+POTRAD+1*viewWidth/5,viewHeight/3,vol,0,0xff,1);
    drawPot(7+POTRAD+2*viewWidth/5,viewHeight/3,pitch,FMIN,FMAX,2);
    drawPot(7+POTRAD+3*viewWidth/5,viewHeight/3,pulse,0,16,3);
    drawPot(7+POTRAD+4*viewWidth/5,viewHeight/3,gap,0,16,4);

    // row 2
    drawPot(7+POTRAD+0*viewWidth/5,2*viewHeight/3+10,lfowavenum,0,8,5);
    drawPot(7+POTRAD+1*viewWidth/5,2*viewHeight/3+10,lfodepth,0,255,6);
    drawPot(7+POTRAD+2*viewWidth/5,2*viewHeight/3+10,lfofreq,0,100,7);
    drawPot(7+POTRAD+3*viewWidth/5,2*viewHeight/3+10,envwidth,0,255,8);
    drawPot(7+POTRAD+4*viewWidth/5,2*viewHeight/3+10,envfreq,0,255,9);

}

int getInput() {

      if (Controller.firePressed()) {
        if (mysound.getStatus() == 0) {
            noteon = 1;
            } else {noteon=0;}
            return 1;
      }
      if (Controller.upPressed()) {
        increaseCurrentPot();
        return 1;
      }

      if (Controller.downPressed()) {
        decreaseCurrentPot();
        return 1;
      }

      if (Controller.leftPressed()) {
        currentPot--;
        if (currentPot<0) currentPot = NUM_KNOBS;
        return 1;
      }

      if (Controller.rightPressed()) {
        currentPot++;
        if (currentPot>NUM_KNOBS) currentPot = 0;
        return 1;
      }
      return 0;
}



void mixbuffers(float pitch1, float pitch2){
    double iLength;
    Uint16 osc_add1, osc_add2, osc_ticks, env_ticks, pulse_ticks, gap_ticks,
    osc_ticks2,osc_value1=0,osc_value2 = 0;
    Int16 dResult, noise, pulsecount=0, gapcount=0;
    sf::Sound testsnd;
    sf::SoundBuffer testbuf;
    iLength = SAMPLES;// 2 sec at 8khz samples
    Int16 sbuf[SAMPLES]; // osc 1 results
    Int16 sbuf2[SAMPLES]; // osc 2 results
    osc_ticks = 8000 / pitch1; // how many ticks per cycle
    if (pitch2) osc_ticks2 = 8000 / pitch2; // lfo
    osc_add1 = 256*256/osc_ticks; // incrementer of 16-bit osc_value
    if (pitch2) osc_add2 = 256*256/osc_ticks2; // incrementer of 16-bit osc_value
    if (envfreq) env_ticks = 8000 / envfreq ; // on off envelope
    if (pulse) pulse_ticks = 8000 / pulse ; // pulses per sec
    if (gap) gap_ticks = 8000 / gap ; // pulses per sec

    pulsecount = pulse_ticks; gapcount = pulsecount + gap_ticks;
    std::vector<sf::Int16> FinalSamplesVector;
    FinalSamplesVector.reserve(iLength);

    noise =  rand() % 65536 - 32767; envtimer=0;
    // CALCULATE OSCILLATOR 1
    for(uint16_t i = 0; i < iLength; i++) // the length of the entire sample
    {

            if (wavenum == 7) { // off
                dResult = 0;
            }


            if (wavenum == 0) { // square
                if (osc_value1 < 256*256 / 2) {
                //first half of cycle
                dResult = 32767;
                } else {
                //2nd half of cycle
                dResult = -32767;
                }
            }

            if (wavenum == 1) { // triangle
                if (osc_value1 < 256*256 / 2) {
                //first half of cycle
                dResult = osc_value1 -32767;
                } else {
                //2nd half of cycle
                dResult = 32767-osc_value1;
                }
            }

            if (wavenum==2) { // sawtooth
                dResult = osc_value1;
            }

            if (wavenum==3) { // inverted sawtooth
                dResult = 32767 - osc_value1;
            }

            if (wavenum == 4) { // 1/4 square
                if (osc_value1 < 256*256 / 4) {
                //first half of cycle
                dResult = 32767;
                } else {
                //2nd half of cycle
                dResult = - 32767;
                }
            }

            if (wavenum == 5) { // 3/4 square
                if (osc_value1 < 256*256 / 4) {
                //first half of cycle
                dResult = 32767;
                } else {
                //2nd half of cycle
                dResult = - 32767;
                }
            }

            if (wavenum == 6) { // noise
                if (osc_value1 < 256*256 / 2) {
                //first half of cycle
                dResult = noise;
                } else {
                //2nd half of cycle
                dResult = -noise;
                }
            }
            if (osc_value1 % osc_ticks == 0) {
             noise =  rand() % 65536 - 32767;
            }
            osc_value1 += osc_add1;
            if (pulse) {
                if (i > pulsecount && i < gapcount)
                    {
                    sbuf[i] = dResult;
                    } else {
                    sbuf[i] = 0;
                    }
                if (i > gapcount) {pulsecount = i + pulse_ticks-gap_ticks; gapcount = pulsecount + gap_ticks;}
            } else { sbuf[i] = dResult; }
    }

    noise =  rand() % 65536 - 32767; envtimer=0;
    // CALCULATE OSCILLATOR 2
    for(uint16_t i = 0; i < iLength; i++) // the length of the entire sample
    {
            if (lfowavenum == 7) { // off
                dResult = 0;
            }


            if (lfowavenum == 0) { // square
                if (osc_value2 < 256*256 / 2) {
                //first half of cycle
                dResult = 32767;
                } else {
                //2nd half of cycle
                dResult = -32767;
                }
            }

            if (lfowavenum == 1) { // triangle
                if (osc_value2 < 256*256 / 2) {
                //first half of cycle
                dResult = osc_value2 -32767;
                } else {
                //2nd half of cycle
                dResult = 32767-osc_value2;
                }
            }

            if (lfowavenum==2) { // sawtooth
                dResult = osc_value2;
            }

            if (lfowavenum==3) { // inverted sawtooth
                dResult = 32767 - osc_value2;
            }

            if (lfowavenum == 4) { // 1/4 square
                if (osc_value2 < 256*256 / 4) {
                //first half of cycle
                dResult = 32767;
                } else {
                //2nd half of cycle
                dResult = - 32767;
                }
            }

            if (lfowavenum == 5) { // 3/4 square
                if (osc_value2 < 256*256 / 4) {
                //first half of cycle
                dResult = 32767;
                } else {
                //2nd half of cycle
                dResult = - 32767;
                }
            }

            if (lfowavenum == 6) { // noise
                if (osc_value2 < 256*256 / 2) {
                //first half of cycle
                dResult = noise;
                } else {
                //2nd half of cycle
                dResult = -noise;
                }
            }


            if (osc_value2 % osc_ticks == 0) {
             noise =  rand() % 65536 - 32767;
            }

            osc_value2 += osc_add2;
            envtimer++;
            if (envwidth < 255 && envtimer) {
                if (envtimer < env_ticks / 2) {
                    if (pitch2) {
                        sbuf2[i] = dResult;}
                        else {
                        sbuf2[i] = 0;}
                }
            } else { sbuf2[i] = dResult;}
            if (envtimer>env_ticks) envtimer = 0;
    }

    // Add TOGETHER OSC1 AND OSC2
    for(int i = 0; i < iLength; i++)
    {
    Int16 a,b;
    a = (sbuf[i]*vol)/255; b =(sbuf2[i]*lfodepth)/255;
    dResult =
    // If both samples are negative, mixed signal must have an amplitude between the lesser of A and B, and the minimum permissible negative amplitude
    a < 0 && b < 0 ? ((int)a + (int)b) - (((int)a * (int)b)/INT16_MIN) :
    // If both samples are positive, mixed signal must have an amplitude between the greater of A and B, and the maximum permissible positive amplitude
    ( a > 0 && b > 0  ? ((int)a + (int)b) - (((int)a * (int)b)/INT16_MAX)
    // If samples are on opposite sides of the 0-crossing, mixed signal should reflect that samples cancel each other out somewhat
            : a + b);

    FinalSamplesVector.push_back(static_cast<sf::Int16>(dResult));
    }

    int foo = testbuf.loadFromSamples(&FinalSamplesVector[0], FinalSamplesVector.size(), 1, 8000);
    int bar = testbuf.saveToFile("output.wav");
    testsnd.setBuffer(testbuf);
    testsnd.setLoop(true);
    testsnd.play();
    while (!getInput()) {
    //TV.delay(100);
    }
    updatePots();
    TV.delay(100);
}

void setup() {
  srand (time(NULL));
  TV.begin(PAL,viewWidth,viewHeight);
  TV.select_font(font4x6);
  updatePots();
  TV.delay(1);
  refresh();
}

void loop() {

  mixbuffers(pitch,lfofreq);
  updatePots();
  refresh();
}

// END OF ARDUINO CODE

int main()
{
    std::cout << "SFML TVout emulator started" << std::endl;
	TVsetup();
    setup();

while (window.isOpen())
    {
	loop();
    }
	return 0;
}

