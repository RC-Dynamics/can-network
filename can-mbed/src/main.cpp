#include <mbed.h>

#define PIN_TX D9
#define PIN_RX D8
#define PIN_SP D7
#define PIN_RD_PT D6
#define PIN_WRT D5
#define PIN_WRT_PT D3

DigitalIn BT(BUTTON1);

Serial pc(USBTX, USBRX, 115200);

// #define F_OSC 16000000
#define TIME_QUANTA_S 1.00
// #define TIME_QUANTA_S (2 / F_OSC)
// #define BIT_RATE 500000

#define SJW 1
#define SYNC_SEG 1
#define PROP_SEG 1
#define PHASE_SEG1 7
#define PHASE_SEG2 7

bool hard_sync = 0;
bool soft_sync = 0;
bool idle = 0;

// BitStuffing <-> Timing
bool RX_bit = 0;
bool TX_bit = 0;

// BitStuffing <-> 
bool stuff_en = 0;
bool stuff_error = 0;


bool arbitration_area = 0;
bool arbitration_lost = 0;
bool bit_error = 0;
bool writing_flag = 0;

InterruptIn RX(PIN_RX);
DigitalOut TX(PIN_TX);

// Bit Timing -> Bit Stuffing
DigitalOut sample_pt(PIN_SP);
InterruptIn sample_pt_int(PIN_SP);

// Bit Timing -> Bit Stuffing
DigitalOut wrt_sp_pt(PIN_WRT);
InterruptIn wrt_sp_pt_int(PIN_WRT);

// Bit Stuffing -> Arbitration + Decoder
DigitalOut read_pt(PIN_RD_PT);
InterruptIn read_pt_int(PIN_RD_PT);

// Bit Stuffing -> Encoder 
DigitalOut write_pt(PIN_WRT_PT);
InterruptIn write_pt_int(PIN_WRT_PT);


Ticker tq_clock;

enum  states_bit_timing {
  SYNC_ST = 0,
  PHASE1_ST,
  PHASE2_ST
} states_bit_timing;

enum  states_bit_stuffing {
  START = 0,
  COUNT
} states_bit_stuffing;



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
  // pc.printf("State: %d, Count: %d, Wrt: %d, Sample: %d\n", state, count, wrt_pt, sample_pt);
  switch(state){
    case SYNC_ST:
      count++;
      wrt_sp_pt = 1;
      if(count == 1){
        state = PHASE1_ST;
        count = 0;
      }
    break;
    case PHASE1_ST:
      wrt_sp_pt = 0;
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
      wrt_sp_pt = 0;
      sample_pt = 0;
      if(hard_sync){
        hard_sync = 0;
        count = 0;
        state = PHASE1_ST;
        wrt_sp_pt = 1;
        break;
      } else if (soft_sync && (SJW > PHASE_SEG2 - count)){
        count = 0;
        soft_sync = 0;
        state = PHASE1_ST;
        wrt_sp_pt = 1;
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
  static int last_rx = RX;

  last_rx = RX_bit;
  RX_bit = RX;

  switch(state)
  {
    case(START):
      read_pt = 1;
      if(stuff_en)
      {
        count++;
        state = COUNT;
      }
      break;

    case(COUNT):
      if(RX == last_rx && count == 5 && stuff_en)
      {
        stuff_error = 1;
        read_pt = 1;
        state = START;
      } else {
        if(!stuff_en)
        {
          count = 0;
          read_pt = 1;
          state = START;
        }
        else if(RX == last_rx)
        {
          count++;
          read_pt = 1;
        }
        else if(RX != last_rx && count == 5) // STUFF
        {
          count = 0;
          state = START;
        }
        else if(RX != last_rx && count != 5)
        {
          count = 0;
          read_pt = 1;
          state = START;
        }
      }
      break;
  }
  read_pt = 0;
}

void bitstuffWRITE()
{
  static int count = 0;
  static int state = 0;
  static int last_tx = TX_bit;

  last_tx = TX;
  switch(state)
  {
    case(START):
      TX = TX_bit;
      write_pt = 1;
      if(stuff_en)
      {
        count++;
        state = COUNT;
      }
      break;

    case(COUNT):
      if(!stuff_en)
      {
        count = 0;
        TX = TX_bit;
        write_pt = 1;
        state = START;
      }
      else if(count == 5 && TX_bit == last_tx) // STUFF
      {
        TX = !TX_bit;
        count = 0;
        state = START;
      }
      else if(TX_bit == last_tx)
      {
        count++;
        TX = TX_bit;
        write_pt = 1;
      }
      else if(TX_bit != last_tx && count != 5)
      {
        count = 0;
        TX = TX_bit;
        write_pt = 1;
        state = START;
      }
      break;
  }
  write_pt = 0;
}

void arbitration()
{
  if(writing_flag && arbitration_area && RX_bit != TX_bit)
  {
    arbitration_lost = 1;
  }
  else if (writing_flag && !arbitration_area && RX_bit != TX_bit)
  {
    bit_error = 1;
  }
  arbitration_lost = 0;
  bit_error = 0;
}

void decoder(){
  static int state = 0;

  switch(){
    
  }
}

void encoder(){

}

int main() {
  
  RX.fall(&edgeDetector);
  tq_clock.attach(bitTimingSM, TIME_QUANTA_S);
  
  sample_pt_int.rise(&bitstuffREAD);
  wrt_sp_pt_int.rise(&bitstuffWRITE);

  read_pt_int.rise(&arbitration);
  read_pt_int.rise(&decoder);

  write_pt_int.rise(&encoder);


  

  while(1) {
    if(BT){
      idle = !idle;
      wait(0.3);
      while(BT);
    }
    // pc.printf("%d %d %d %d %d\n", Tog.read() * 10, St2.read() + 4, St1.read() + 6, hard_sync+2, soft_sync+1);
  }
}