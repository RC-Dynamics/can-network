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

typedef struct
{
  bool SOF;
  uint16_t ID;
  bool SRR;
  bool IDE;
  bool R0;
  uint32_t IDB;
  bool R1;
  bool R2;
  uint8_t DLC;
  uint64_t DATA;
  uint16_t CRC_V;
  bool CRC_D;
  bool ACK_S;
  bool ACK_D;
  uint8_t EOF;
} CAN_FRAME;

bool hard_sync = 0;
bool soft_sync = 0;
bool idle = 0;

// BitStuffing <-> Timing
bool RX_bit = 0;
bool TX_bit = 0;

// BitStuffing <-> 
bool stuff_en = 0;
bool stuff_error = 0;

// Decoder
CAN_FRAME frame;

bool CRC_en;
bool TX_decod;
bool TX_en;


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

enum states_decoder
{
  IDLE = 0,
  ID,
  SRR,
  IDE,
  R0,
  IDB,
  R1,
  R2,
  DLC,
  DATA,
  CRC_V,
  CRC_D,
  ACK_S,
  ACK_D,
  EOF,
  INTERFRAME,
  OVERLOAD,
  OVERLOAD_D,
  ERROR,
  ERROR_D
} states_decoder;



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
  static int bit_cnt = 0;
  bool bit = RX_bit;
  switch(state)
  {
    case(IDLE):
      if(bit == 0)
      {
        frame.SOF = bit;
        bit_cnt = 0;
        frame.ID = 0;
        CRC_en = 1;
        //idle
        state = ID;
      }
    break;
    case(ID):
      frame.ID << 1;
      frame.ID = frame.ID ^ bit;
      bit_cnt++;
      if(bit_cnt == 11)   
        {
         state = SRR;
        }
    break;  
    case(SRR):
      frame.RTR = bit;
      state = IDE;
    break;
    case(IDE):
      frame.IDE = bit;
      bit_cnt = 0;
      if(bit == 0) 
      {
        state = R0;
      }
      else if(bit == 1 && frame.RTR == 0) 
      {
        frame.SRR = frame.RTR;
        frame.IDB = 0;
        state = IDB;
      }
      else if(bit == 1 && frame.RTR == 1)
      {
        TX_decod = 0;
        TX_en = 1;
        state = ERROR;
      }
    break;
    case(R0):
      frame.R0 = bit;
      frame.DLC = 0;
      state = DLC;
    break;
    case(IDB):
      frame.IDB << 1;
      frame.IDB = frame.IDB ^ bit;
      bit_cnt++;
      if(bit_cnt == 18)
      {
        state = RTR;
      }
    break;
    case(RTR):
      frame.RTR = bit;
      state = R1;
    break;
    case(R1):
      frame.R1 = bit;
      state = R2;
    break;
    case(R2):
      frame.R2 = bit;
      frame.DLC = 0;
      bit_cnt = 0;
    break;
    case(DLC):
      frame.DLC << 1;
      frame.DLC = frame.DLC ^ bit;
      bit_cnt++;
      if(bit_cnt == 4)
      {
        bit_cnt = 0;
        frame.CRC = 0;
        if(frame.DLC >=8)
        {
          frame.DLC = 8;
        }
        if(frame.RTR == 1 || frame.DLC == 0)
        {
          bit_cnt = 0;
          CRC_en = 0;
          state = CRC_V;
        }
        else if (RTR == 0)
        {
          frame.DATA = 0;
          bit_cnt = 0;
          state = DATA;
        }
      }
    break;
    case(DATA):
      frame.DATA << 1;
      frame.DATA = frame.DATA ^ bit;
      bit_cnt++;
      if(bit_cnt == (frame.DATA * 8))
      {
        bit_cnt = 0;
        CRC_en = 0;
        state = CRC_V;
      } 
    break;
    case(CRC_V):
      frame.CRC_V << 1;
      frame.CRC_V = frame.CRC_V ^ bit;
      if(bit_cnt == 15)
      {
        state = CRC_D;
      }
    break;
    case(CRC_D):
      frame.CRC_D = bit;
      TX_decod = 0;
      if(bit == 1)
      {
        state = ACK_S;
      }
      else if(bit == 0)
      {
        TX_en = 1;
        bit_cnt = 0;
        state = ERROR;
      }
    break;
    case(ACK_S):
      frame.ACK_S = bit;
      TX_decod = 1;
      state = ACK_D;
    break;
    case(ACK_D):
    frame.ACK_D = bit;
      if(frame.CRC != CRC_CALC || bit == 0)
      {
        bit_cnt = 0;
        TX_decod = 0;
        TX_en = 1;
        state = ERROR;
      }
      else if(bit == 1)
      {
        bit_cnt = 0;
        state = EOF;
      }
    break;
    case(EOF):
      bit_cnt++;
      if(bit == 1 && bit_cnt == 7)
      {
        bit_cnt = 0;
        state = INTERFRAME;
      }
      else if(bit == 0)
      {
        bit_cnt = 0;
        TX_decod = 0;
        TX_en = 1;
        state = ERROR;
      }
    break;
    case(INTERFRAME):
      bit_cnt++;
      if(bit == 0)
      {
        TX_decod = 0;
        bit_cnt = 0;
        state = OVERLOAD;
      }
      else if(bit_cnt == 2)
      {
        state = IDLE;
      }
    break;
    case(OVERLOAD):
      bit_cnt++;
      if(bit_cnt == 6)
      {
        bit_cnt = 0;
        TX_decod = 1;
        state = OVERLOAD_D;
      }
    break;
    case(OVERLOAD_D):
      bit_cnt++;
      if(bit == 0)
      {
        bit_cnt = 1; // duvida nesse valor
      }
      else if (bit_cnt == 8)
      {
        bit_cnt = 0;
        state = INTERFRAME;
      }
    break;
    case(ERROR):
      bit_cnt++;
      if(bit_cnt == 6)
      {
            bit_cnt = 0;
            TX_decod = 1; 
            state = ERROR_D;
      }
    break;
    case(ERROR_D)
      bit_cnt++;
      if(bit == 0)
      {
        bit_cnt = 1; // duvida nesse valor
      }
      else if (bit_cnt == 8)
      {
        bit_cnt = 0;
        state = INTERFRAME;
      }
    break;
  }
}

// void encoder(){

// }

int main() {
  
  RX.fall(&edgeDetector);
  tq_clock.attach(bitTimingSM, TIME_QUANTA_S);
  
  sample_pt_int.rise(&bitstuffREAD);
  wrt_sp_pt_int.rise(&bitstuffWRITE);

  read_pt_int.rise(&arbitration);
  // read_pt_int.rise(&decoder);

  // write_pt_int.rise(&encoder);


  

  while(1) {
    if(BT){
      idle = !idle;
      wait(0.3);
      while(BT);
    }
    // pc.printf("%d %d %d %d %d\n", Tog.read() * 10, St2.read() + 4, St1.read() + 6, hard_sync+2, soft_sync+1);
  }
}