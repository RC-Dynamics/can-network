#include <mbed.h>


// ######## INIT - Debug ########
#define PIN_RX D8
#define Toggle D7
#define HardSync D6
#define SoftSync D5
#define State1 D3
#define State2 D4

DigitalOut Tog(Toggle);
DigitalOut Har(HardSync);
DigitalOut Sof(SoftSync);
DigitalOut St1(State1);
DigitalOut St2(State2);
// ######## END - Debug ########

#define F_OSC 16000000
#define TIME_QUANTA_S 0.01
// #define TIME_QUANTA_S (2 / F_OSC)
// #define BIT_RATE 500000

#define SJW 1
#define SYNC_SEG 1
#define PROP_SEG 1
#define PHASE_SEG1 7
#define PHASE_SEG2 7

int sample_pt = 0;
int hard_sync = 0;
int soft_sync = 0;
int idle = 0;
int wrt_pt = 1;

InterruptIn RX(PIN_RX);
Serial pc(USBTX, USBRX, 115200);
Ticker tq_clock;

enum  states {
  SYNC_ST = 0,
  PHASE1_ST,
  PHASE2_ST
} states;

void edgeDetector(){
  if (idle)
  {
    soft_sync = 0;
    hard_sync = 1;
  }
  else
  {
    hard_sync = 0;
    soft_sync = 1;
  }
  pc.printf("Soft: %d, Hard: %d\n", soft_sync, hard_sync);
}

void bitTimingSM(){
  static int state = PHASE1_ST;
  static int count = 0;
  // Debug
  Tog != Tog;

  pc.printf("State: %d, Count: %d, Wrt: %d, Sample: %d\n", state, count, wrt_pt, sample_pt);
  // 
  switch(state){
    case SYNC_ST:
      // Debug
      St1 = 0;
      St2 = 0;
      //
      count++;
      wrt_pt = 1;
      if(count == 1){
        state = PHASE1_ST;
        count = 0;
      }
    break;
    case PHASE1_ST:
      // Debug
      St1 = 1;
      St2 = 0;
      //
      wrt_pt = 0;
      sample_pt = 0;
      if(hard_sync){
        count = 0;
      } else if (soft_sync){
        count -= fmin(SJW, count);
        soft_sync = 0;
      } 
      if(count < PROP_SEG + PHASE_SEG1){
        count++;
      } 
      if(count == PROP_SEG + PHASE_SEG1){
        count = 0;
        sample_pt = 1;
        state = PHASE2_ST;
      }
      break;
    case PHASE2_ST:
      // Debug
      St1 = 0;
      St2 = 1;
      //
      wrt_pt = 0;
      sample_pt = 0;
      if(hard_sync){
        hard_sync = 0;
        count = 0;
        state = PHASE1_ST;
        wrt_pt = 1;
        break;
      } else if (soft_sync && (SJW > PHASE_SEG2 - count)){
        count = 0;
        soft_sync = 0;
        state = PHASE1_ST;
        wrt_pt = 1;
        break;
      } else if (soft_sync && PHASE_SEG2 - count == SJW){
        count = 0;
        soft_sync = 0;
        state = SYNC_ST;
        break;
      } else if(soft_sync && PHASE_SEG2 - count > SJW){
        count += SJW;
        soft_sync = 0;
      } 
      if(count < PHASE_SEG2){
        count++;
      }
      if(count == PHASE_SEG2){
        count = 0;
        state = SYNC_ST;
      }
      break;
  }
}



int main() {
  Tog = 1;
  RX.fall(&edgeDetector);
  tq_clock.attach(bitTimingSM, TIME_QUANTA_S);
  

  while(1) {
    Har = hard_sync;
    Sof = soft_sync;
  }
}