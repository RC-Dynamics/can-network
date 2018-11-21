#include <mbed.h>


// ######## INIT - Debug ########
#define PIN_RX D8
#define Toggle D7
#define HardSync D6
#define SoftSync D5
#define State1 D3
#define State2 D4

DigitalIn BT(BUTTON1);
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut Tog(Toggle);
DigitalOut Har(HardSync);
DigitalOut Sof(SoftSync);
DigitalOut St1(State1);
DigitalOut St2(State2);

Serial pc(USBTX, USBRX, 115200);
// ######## END - Debug ########

// #define F_OSC 16000000
#define TIME_QUANTA_S 1.01
// #define TIME_QUANTA_S (2 / F_OSC)
// #define BIT_RATE 500000

#define SJW 1
#define SYNC_SEG 1
#define PROP_SEG 1
#define PHASE_SEG1 7
#define PHASE_SEG2 7

bool sample_pt = 0;
bool hard_sync = 0;
bool soft_sync = 0;
bool idle = 0;
bool wrt_pt = 1;

InterruptIn RX(PIN_RX);
Ticker tq_clock;

enum  states {
  SYNC_ST = 0,
  PHASE1_ST,
  PHASE2_ST
} states;

enum  states1 {
  START = 0,
  COUNT_0,
  COUNT_1,
  ERROR
} states1;



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
  Tog = !Tog;
  pc.printf("State: %d, Count: %d, Wrt: %d, Sample: %d\n", state, count, wrt_pt, sample_pt);
  // End - Debug
  switch(state){
    case SYNC_ST:
    // Debug
      led1 = St1 = 0;
      led2 = St2 = 0;
    // End - Debug
      count++;
      wrt_pt = 1;
      if(count == 1){
        state = PHASE1_ST;
        count = 0;
      }
    break;
    case PHASE1_ST:
    // Debug
      led1 = St1 = 1;
      led2 = St2 = 0;
    // End - Debug
      wrt_pt = 0;
      sample_pt = 0;
      if(hard_sync){
        hard_sync = 0;
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
      led1 = St1 = 0;
      led2 = St2 = 1;
    // End - Debug
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

void bitstuffREAD()
{
  static int count = 0;
  static int state = 0;

  switch(state)
  {
    case(START):
      // RX_bit = RX;
      // rd_pt == CLK
      //Implementar stuff_en == 0

      if(RX == 0 && stuff_en)
      {
        count++;
        RX_bit = 0;
        rd_pt = 1;
        state = COUNT_0;
      }
      else if(RX == 1 && stuff_en)
      {
        count++;
        RX_bit = 1;
        rd_pt = 1;
        state = COUNT_1;
      }
      break;

    case(COUNT_0):
      rd_pt = 0;

      if(!stuff_en)
      {
        count = 0;
        RX_bit = RX;
        rd_pt = 1;
        state = START;
      }
      else if(RX == 0 && stuff_en)
      {
        count ++;
        RX_bit = RX;
        rd_pt = 1;
      }
      else if(RX == 1 && count != 5)
      {
        count = 0;
        RX_bit = RX;
        rd_pt = 1;
        state = COUNT_1;
      }
      else if(RX == 1 && count == 5)
      {
        count = 0;
        RX_bit = RX;
        state = COUNT_1;
      }
      else if(RX == 0 && count == 5 && stuff_en)
      {
        stuff_error = 1;
        rd_pt = 1;
        state = ERROR;
      }
      break;

    case(COUNT_1):
      rd_pt = 0;

      if(!stuff_en)
      {
        count = 0;
        RX_bit = RX;
        rd_pt = 1;
        state = START;
      }
      else if(RX == 1 && stuff_en)
      {
        count ++;
        RX_bit = RX;
        rd_pt = 1;
      }
      else if(RX == 0 && count != 5)
      {
        count = 0;
        RX_bit = RX;
        rd_pt = 1;
        state = COUNT_0;
      }
      else if(RX == 0 && count == 5)
      {
        count = 0;
        RX_bit = RX;
        state = COUNT_0;
      }
      else if(RX == 1 && count == 5 && stuff_en)
      {
        rd_pt = 1;
        stuff_error = 1;
        state = ERROR;
      }
      break;

    case(ERROR):
      stuff_error = 0;
      count = 0;
      RX_bit = RX;
      rd_pt = 1;
      state = START;
      break;
  }
}


int main() {
// Debug
  Tog = 1;
  St1 = 0;
  St2 = 0;
  Har = 0;
  Sof = 0;
// End - Debug

  RX.fall(&edgeDetector);
  tq_clock.attach(bitTimingSM, TIME_QUANTA_S);
  

  while(1) {
  // Debug
    Har = hard_sync;
    Sof = soft_sync;
    if(BT){
      idle = !idle;
      led3 = idle;
      wait(0.3);
      while(BT);
    }
    // pc.printf("%d %d %d %d %d\n", Tog.read() * 10, St2.read() + 4, St1.read() + 6, hard_sync+2, soft_sync+1);
  // End Debug
  }
}