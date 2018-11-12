#include <mbed.h>
#include <math.h>

#define PIN_RX PB_15

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

enum  states {
  SYNC_ST = 0,
  PHASE1_ST,
  PHASE2_ST
} states;


InterruptIn int_rx(PIN_RX);

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
}

void bitTimingSM(){
  static int state = PHASE1_ST;
  static int count = 0;
  switch(state){
    case SYNC_ST:
      count++;
      wrt_pt = 1;
      if(count == 1){
        state = PHASE1_ST;
        count = 0;
      }
      break;
    case PHASE1_ST:
      wrt_pt = 0;
      sample_pt = 0;
      if(hard_sync){
        count = 0;
      } else if (soft_sync){
        count -= fmin(SJW, count);
        soft_sync = 0;
      } else if(count < PROP_SEG + PHASE_SEG1){
        count++;
      } 
      if(count == PROP_SEG + PHASE_SEG1){
        count = 0;
        sample_pt = 1;
        state = PHASE2_ST;
      }
      break;
    case PHASE2_ST:
      wrt_pt = 0;
      sample_pt = 0;
      if(hard_sync){
        hard_sync = 0;
        count = 0;
        state = PHASE1_ST;
      } else if (soft_sync && (SJW > PHASE_SEG2 - count)){
        count = 0;
        state = PHASE1_ST;
      } 
      if(count == PHASE_SEG2){
        count = 0;
        state = SYNC_ST;
      } else if (soft_sync && PHASE_SEG2 - count == SJW){
        count = 0;
        state = SYNC_ST;
      }
      if(soft_sync && PHASE_SEG2 - count > SJW){
        count += SJW;
      } else if(count < PHASE_SEG2){
        count++;
      }

      break;
  }
}



int main() {
  int_rx.fall(&edgeDetector);

  

  while(1) {
  
  }
}