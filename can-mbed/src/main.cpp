#include <inttypes.h>
#include <mbed.h>

#define DEBUG
#ifdef DEBUG
#define debug(x) x
#else
#define debug(x)
#endif

#define PIN_RX_IN D2
#define PIN_TX_OUT D3
#define PIN_TX_IN A1 // DEBUG

#define PIN_SP_OUT D5
#define PIN_WRT_OUT D6

#define PIN_SP_IN A5
#define PIN_WRT_IN A4

DigitalIn BT(BUTTON1);

Serial pc(USBTX, USBRX, 1843200);

// #define F_OSC 16000000
#define TIME_QUANTA_S 0.01
// #define TIME_QUANTA_S (2 / F_OSC)

#define SJW 1
#define SYNC_SEG 1
#define PROP_SEG 1
#define PHASE_SEG1 7
#define PHASE_SEG2 7

typedef struct CAN_FRAME
{
  bool SOF;
  uint16_t ID;
  bool SRR;
  bool RTR;
  bool IDE;
  bool R0;
  uint32_t IDB;
  bool R1;
  bool R2;
  uint8_t DLC;
  uint64_t DATA;
  bool data_b = false;
  uint16_t CRC_V;
  bool CRC_D;
  bool ACK_S;
  bool ACK_D;
  uint8_t EOFRAME;
} CAN_FRAME;

bool hard_sync = 0;
bool soft_sync = 0;
bool idle = 0;

// BitStuffing <-> Timing
bool RX_bit = 1;
bool TX_bit = 1;

// BitStuffing <-> 
bool stuff_en = 0;
bool stuff_error = 0;

// Decoder
CAN_FRAME frame_recv;
CAN_FRAME frame_send;
int CRC_CALC = 0;

// Debug        
// Frame Wikipedia Original
// bool frame[] = {1,1,1,1,0,0,0,0,0,1,0,0,1,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,0,1,0,1,0,0,0,0,1,1,0,0,0,0,0,1,0,0,1,0,1,1,1,1,1,1,1,1,1,1,1};
// Frame Wikipedia sem bit stuff
// bool frame[] = {0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,0,1,1,1,1,1,1,1,1,1,1,1};
// Frame crc certo
// bool frame[] = {0,0,0,0,0,1,0,0,1,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,0,1,1,1,1,0,1,1,1,0,1,0,1,0,0,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1};
// Frame crc ext certo
// bool frame[] = {0,0,0,0,0,1,0,0,0,1,0,0,1,0,1,0,0,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,0,0,0,0,1,1,0,1,0,0,0,0,0,1,1,1,0,1,0,1,1,0,0,0,1,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1,1,1,1};
// alain - 1
// bool frame[] = {0,1,1,0,0,1,1,1,0,0,1,0,0,0,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,0,0,1,0,0,0,0,1,0,1,0,0,0,1,1,0,1,1,1,1,1,1,1,1,1,1};
// alain - 9
// bool frame[] = {0,1,1,0,0,1,1,1,0,0,1,0,0,0,0,0,1,0,0,0,0,1,1,0,0,1,0,1,1,0,1,0,1,0,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
// alain - 12
// bool frame [] = {0,1,0,0,0,1,0,0,1,0,0,1,1,1,1,1,0,0,0,0,0,1,0,0,0,0,0,1,1,1,1,1,0,0,1,0,0,0,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,1,0,1,1,1,1,1,0,1,0,1,0,1,1,0,1,1,1,1,1,1,1,1,1,1};
// alain - 17 - ack error
// bool frame[] = {0,1,1,0,0,1,1,1,0,0,1,0,0,0,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,0,0,1,0,0,0,0,1,0,1,0,0,0,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1};
// alain - 18 - stuff error
// bool frame[] = {0,1,1,0,0,1,1,1,0,0,1,0,0,0,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1};
// alain - 19 - crc error
// bool frame[] = {0,1,1,0,0,1,1,1,0,0,1,0,0,0,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,0,0,1,0,0,0,0,1,1,1,0,0,0,1,1,0,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1};
// alain - 24 - idle grande
// bool frame[] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,0,1,0,1,1,0,0,0,0,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,0,1,0,1,1,0,0,0,0,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1};
bool frame[] = {1,1,1,1,0,0,0,0,0,1,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,0,1,0,1,1,0,0,0,0,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,0,1,0,1,1,0,0,0,0,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1};

int frame_index = 0;

bool CRC_en;
bool TX_decod;
bool TX_en;
bool TX_ack;

bool write_en;

bool bit_error = 0;
bool writing_flag = 0;

bool last_tx = 1;

// DEBUG
DigitalIn TX_IN(PIN_TX_IN);

// INTERFACES
InterruptIn RX(PIN_RX_IN);
DigitalOut TX(PIN_TX_OUT);

// Bit Timing -> Bit Stuffing
DigitalOut sample_pt(PIN_SP_OUT);
InterruptIn sample_pt_int(PIN_SP_IN);

// Bit Timing -> Bit Stuffing
DigitalOut wrt_sp_pt(PIN_WRT_OUT);
InterruptIn wrt_sp_pt_int(PIN_WRT_IN);

Ticker tq_clock;


enum  states_bit_timing {
  SYNC_ST = 0,
  PHASE1_ST,
  PHASE2_ST
} states_bit_timing;

enum  states_bit_stuffing {
  START = 0,
  COUNT,
  STUFF
} states_bit_stuffing;

enum states_coder
{
  IDLE = 0,
  ID,
  SRR,
  RTR,
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
  EOFRAME,
  INTERFRAME,
  OVERLOAD,
  OVERLOAD_D,
  ERROR_FLAG,
  ERROR_D,
  SOF
} states_coder;

char* print_state(int st){
  switch(st){
    case IDLE:
      return  "IDLE";
    break;
    case ID:
      return "ID"; 
    break;
    case SRR:
      return "SRR"; 
    break;
    case RTR:
      return "RTR"; 
    break;
    case IDE:
      return "IDE"; 
    break;
    case R0:
      return "R0"; 
    break;
    case IDB:
      return "IDB"; 
    break;
    case R1:
      return "R1"; 
    break;
    case R2:
      return "R2"; 
    break;
    case DLC:
      return "DLC"; 
    break;
    case DATA:
      return  "DATA";
    break;
    case CRC_V:
      return  "CRC_V";
    break;
    case CRC_D:
      return  "CRC_D";
    break;
    case ACK_S:
      return  "ACK_S";
    break;
    case ACK_D:
      return  "ACK_D";
    break;
    case EOFRAME:
      return  "EOFRAME";
    break;
    case INTERFRAME:
      return  "INTERFRAME";
    break;
    case OVERLOAD:
      return  "OVERLOAD";
    break;
    case OVERLOAD_D:
      return  "OVERLOAD_D";
    break;
    case ERROR_FLAG:
      return  "ERROR_FLAG";
    break;
    case ERROR_D:
      return  "ERROR_D";
    break;
    case SOF:
      return "SOF"; 
    break;
  }

}


void edgeDetector(){
  if (idle)
  {
    soft_sync = 0;
    hard_sync = 1;
    idle = 0;
  }
  else
  {
    hard_sync = 0;
    soft_sync = 1;
  }
  debug(pc.printf("Edge: Soft: %d, Hard: %d\n", soft_sync, hard_sync));
}

void bitTimingSM(){
  static int state = PHASE1_ST;
  static int count = 0;

  // if(wrt_sp_pt_int.read() || sample_pt_int.read())
    // debug(pc.printf("--> Timing: State: %d, Count: %d, Wrt: %d, Sample: %d\n", state, count, wrt_sp_pt_int.read(), sample_pt_int.read()));

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


void calculateCRC(bool bit)
{
  if (CRC_en) {
    CRC_CALC <<= 1;
    if ((CRC_CALC >= (1 << 15)) ^ bit) { // um smente no bit mais significativo
      CRC_CALC ^= 0x4599;
    }
    CRC_CALC &= 0x7fff; // zero no bit mais significativo e um no resto
  }
}

void decoder(){
  static int state = 0;
  static int bit_cnt = 0;

  bool bit = RX_bit;

  // Debug
  debug(pc.printf("%d %d   --> DECODER State: %s, bit_cnt: %d\n\n", TX_bit, bit, print_state(state), bit_cnt));
  

  if(stuff_error || bit_error)
  {
    stuff_en = 0;
    bit_cnt = 0;
    TX_decod = 0;
    TX_en = 1;
    state = ERROR_FLAG;
    debug(pc.printf("Error Detected: %s\n", (stuff_error)?"STUFF_ERROR": "BIT_ERROR"));
    stuff_error = 0;
    bit_error = 0;
  }
  switch(state)
  {
    case(IDLE):
      idle = 1;
      debug(pc.printf("IDLE\n"));
      if(bit == 0)
      {
        stuff_en = 1;
        frame_recv.SOF = bit;
        debug(pc.printf("START OF FRAME\n"));
        bit_cnt = 0;
        frame_recv.ID = 0;
        CRC_CALC = 0;
        CRC_en = 1;
        idle = 0;
        state = ID;
      }
     break;
    case(ID):
      frame_recv.ID = (frame_recv.ID << 1) ^ bit;
      bit_cnt++;
      if(bit_cnt == 11)   
      {
        debug(pc.printf("ID: %x \n", frame_recv.ID));
        state = SRR;
       }
      break;  
    case(SRR):
      frame_recv.RTR = bit;
      debug(pc.printf("RTR/SRR: %d \n", frame_recv.RTR));
      state = IDE;
      break;
    case(IDE):
      frame_recv.IDE = bit;
      debug(pc.printf("IDE: %d \n", frame_recv.IDE));
      bit_cnt = 0;
      if(bit == 0) 
      {
        state = R0;
      }
      else if(bit == 1 && frame_recv.RTR == 1) 
      {
        frame_recv.SRR = frame_recv.RTR;
        debug(pc.printf("SRR: %d \n", frame_recv.SRR));
        frame_recv.IDB = 0;
        state = IDB;
      }
      else if(bit == 1 && frame_recv.RTR == 0) // RTR = SRR
      { // OLHAR ENCODER
        stuff_en = 0;
        TX_decod = 0;
        TX_en = 1;
        state = ERROR_FLAG;
      }
      break;
    case(R0):
      frame_recv.R0 = bit;
      debug(pc.printf("R0: %d \n", frame_recv.R0));
      frame_recv.DLC = 0;
      state = DLC;
     break;
    case(IDB):
      frame_recv.IDB = (frame_recv.IDB << 1) ^ bit;
      bit_cnt++;
      if(bit_cnt == 18)
      {
        debug(pc.printf("IDB: %x \n", frame_recv.IDB));
        state = RTR;
      }
      break;
    case(RTR):
      frame_recv.RTR = bit;
      debug(pc.printf("RTR: %d \n", frame_recv.RTR));
      state = R1;
      break;
    case(R1):
      frame_recv.R1 = bit;
      debug(pc.printf("R1: %d \n", frame_recv.R1));
      state = R2;
      break;
    case(R2):
      frame_recv.R2 = bit;
      frame_recv.DLC = 0;
      debug(pc.printf("R2: %d \n", frame_recv.R2));
      bit_cnt = 0;
      state = DLC;
      break;
    case(DLC):
      frame_recv.DLC = (frame_recv.DLC << 1) ^ bit;
      bit_cnt++;
      if(bit_cnt == 4)
      {
        bit_cnt = 0;
        debug(pc.printf("DLC: %d ", frame_recv.DLC));
        if(frame_recv.DLC > 8)
        {
          frame_recv.DLC = 8;
          debug(pc.printf("--> DLC: %d", frame_recv.DLC));
        }
        debug(pc.printf("\n"));
        if(frame_recv.RTR == 1 || frame_recv.DLC == 0)
        {
          // CRC_en = 0;
          frame_recv.CRC_V = 0;
          state = CRC_V;
        }
        else if (frame_recv.RTR == 0)
        {
          frame_recv.DATA = 0;
          state = DATA;
        }
      }
     break;
    case(DATA):
      frame_recv.DATA = (frame_recv.DATA << 1) ^ bit;
      bit_cnt++;
      if(bit_cnt == (frame_recv.DLC * 8))
      {
        debug(pc.printf("DATA: %" PRIx64 "\n", frame_recv.DATA));
        bit_cnt = 0;
        frame_recv.CRC_V = 0;
        state = CRC_V;
      } 
     break;
    case(CRC_V):
      CRC_en = 0;
      frame_recv.CRC_V = (frame_recv.CRC_V << 1) ^ bit;
      bit_cnt++;
      if(bit_cnt == 15)
      {
        debug(pc.printf("CRC_Value: %d \n", frame_recv.CRC_V));
        debug(pc.printf("CRC_CALC: %d \n", CRC_CALC));
        stuff_en = 0;
        state = CRC_D;
      }
      break;
    case(CRC_D):
      frame_recv.CRC_D = bit;
      TX_decod = 0; // OLHAR ENCODER
      debug(pc.printf("CRC_D: %d\n", bit));
      if(bit == 1)
      {
        TX_ack = 1;
        state = ACK_S;
      }
      else if(bit == 0)
      {
        TX_en = 1;
        bit_cnt = 0;
        stuff_en = 0;
        state = ERROR_FLAG;
      }
      break;
    case(ACK_S):
      frame_recv.ACK_S = bit;
      debug(pc.printf("ACK_S: %d\n", bit));
      TX_decod = 1; // OLHAR ENCODER
      state = ACK_D;
      break;  
    case(ACK_D):
      frame_recv.ACK_D = bit;
      debug(pc.printf("ACK_D: %d \n", frame_recv.ACK_D));
      if(frame_recv.CRC_V != CRC_CALC || bit == 0)
      {
        debug(if(frame_recv.CRC_V != CRC_CALC))
          debug(pc.printf("CRC_V =! CRC_CALC\n"));
        debug(else)
          debug(pc.printf("ACK Error\n"));
        stuff_en = 0;
        bit_cnt = 0;
        TX_decod = 0; // OLHAR ENCODER
        TX_en = 1;
        state = ERROR_FLAG;
      }
      else if(bit == 1)
      {
        bit_cnt = 0;
        frame_recv.EOFRAME = 0;
        state = EOFRAME;
      }
      break;
    case(EOFRAME):
      frame_recv.EOFRAME = (frame_recv.EOFRAME << 1) ^ bit;
      bit_cnt++;
      if(bit == 1 && bit_cnt == 7)
      {
        bit_cnt = 0;
        debug(pc.printf("EOFRAME\n"));
        state = INTERFRAME;
      }
      else if(bit == 0)
      {
        stuff_en = 0;
        bit_cnt = 0;
        TX_decod = 0; // OLHAR ENCODER
        TX_en = 1;
        state = ERROR_FLAG;
      }
     break;
    case(INTERFRAME):
      bit_cnt++;
      
      if(bit == 0)
      {
        TX_decod = 0;
        TX_en = 1; // OLHAR ENCODER
        bit_cnt = 0;
        debug(pc.printf("INTERFRAME\n"));
        state = OVERLOAD;
      }
      else if(bit_cnt == 2)
      {
        debug(pc.printf("INTERFRAME\n"));
        state = IDLE;
      }
     break;
    case(OVERLOAD):
      bit_cnt++;
      if(bit_cnt == 6)
      {
        bit_cnt = 0;
        TX_decod = 1; // OLHAR ENCODER
        TX_en = 1;
        debug(pc.printf("OVERLOAD\n"));
        state = OVERLOAD_D;
      }
      break;
    case(OVERLOAD_D):
      bit_cnt++;
      if(bit == 0)
      {
        bit_cnt = 0;
      }
      else if (bit_cnt == 8)
      {
        bit_cnt = 0;
        TX_en = 0;
        debug(pc.printf("OVERLOAD_DELIMITER\n"));
        state = INTERFRAME;
      }
      break;
    case(ERROR_FLAG):
      bit_cnt++;
      if(bit_cnt == 5)
      {
        CRC_en = 0;
        bit_cnt = 0;
        TX_decod = 1; 
        TX_en = 1;
        debug(pc.printf("ERROR_FLAG\n"));
        state = ERROR_D;
      }
     break;
   case(ERROR_D):
      bit_cnt++;
      if(bit == 0)
      {
        bit_cnt = 0;
      }
      else if (bit_cnt == 8)
      {
        bit_cnt = 0;
        TX_en = 0;
        debug(pc.printf("ERROR_DELIMITER\n"));
        state = INTERFRAME;
      }
      break;
  }
  calculateCRC(bit);
}

void encoder(){
  static int state = 0;
  static int bit_cnt = 0;
  // DEBUG
  // RX_bit = TX_bit;

  // debug(pc.printf("%d %d   --> ENCODER State: %s, bit_ctn: %d\n", TX_bit, RX_bit, print_state(state), bit_cnt));

  if(TX_en){
    state = IDLE;
  }
  switch(state){
    case IDLE:
      if(TX_ack)
      {
        TX_bit = TX_decod;
        TX_ack = 0;
      }
      else if(TX_en)
      {
        TX_bit = TX_decod;
      }
      else 
      {
        TX_bit = 1;
      }
      if(write_en){
        TX_bit = frame_send.SOF;
        write_en = 0;
        stuff_en = 1;
        state = SOF;
        bit_cnt = 0;
      }
      break;

    case SOF:
      if(RX_bit != TX_bit) //BIT_ERROR
      {
        state = IDLE;
        TX_bit = 1;
        bit_error = 1;
        break;
      }
      TX_bit = (frame_send.ID >> (10-bit_cnt)) & 1;
      bit_cnt++;
      state = ID;
      break;

    case ID:
      if(RX_bit != TX_bit) // ARBITRATION
      {
        state = IDLE;
        TX_bit = 1;
        break;
      }
      if(bit_cnt == 11){
        TX_bit = frame_send.SRR;
        state = SRR;
      } else {
        TX_bit = (frame_send.ID >> (10-bit_cnt)) & 1;
        bit_cnt++;
      }
      break;

    case SRR:
      if(RX_bit != TX_bit) // ARBITRATION
      {
        state = IDLE;
        TX_bit = 1;
        break;
      }
      TX_bit = frame_send.IDE;
      bit_cnt = 0;
      state = IDE;
      break;

    case IDE:
      if(frame_send.IDE == 1){
        if(RX_bit != TX_bit) // ARBITRATION verifica se nao for extendido?
        {
          state = IDLE;
          TX_bit = 1;
          break;
        }
        TX_bit = (frame_send.IDB >> (17-bit_cnt)) & 1;
        bit_cnt++;
        state = IDB;
      } else if(frame_send.IDE == 0){
        if(RX_bit != TX_bit) // BIT_ERROR
        {
          state = IDLE;
          TX_bit = 1;
          bit_error = 1;
          break;
        }
        TX_bit = frame_send.R0;
        state = R0;
      }
      break;

    case R0:
      if(RX_bit != TX_bit) //BIT_ERROR
      {
        state = IDLE;
        TX_bit = 1;
        bit_error = 1;
        break;
      }
      TX_bit = (frame_send.DLC >> 3)&1;
      bit_cnt = 1;
      state = DLC;
      break;

    case IDB:
      if(RX_bit != TX_bit) // ARBITRATION
      {
        state = IDLE;
        TX_bit = 1;
        break;
      }
      if(bit_cnt == 18){
        TX_bit = frame_send.RTR;
        state = RTR;
      }else {
        TX_bit = (frame_send.IDB >> (17-bit_cnt)) & 1;
        bit_cnt++;
      }
      break;

    case RTR:
      if(RX_bit != TX_bit) // ARBITRATION
      {
        state = IDLE;
        TX_bit = 1;
        break;
      }
      TX_bit = frame_send.R1;
      state = R1;
      break;

    case R1:
      if(RX_bit != TX_bit) //BIT_ERROR
      {
        state = IDLE;
        TX_bit = 1;
        bit_error = 1;
        break;
      }
      TX_bit = frame_send.R2;
      state = R2;
      break;

    case R2:
      if(RX_bit != TX_bit) //BIT_ERROR
      {
        state = IDLE;
        TX_bit = 1;
        bit_error = 1;
        break;
      }
      bit_cnt = 0;
      TX_bit = (frame_send.DLC >> (3 - bit_cnt)) & 1;
      bit_cnt++;
      state = DLC;
      break;

    case DLC:
      if(RX_bit != TX_bit) //BIT_ERROR
      {
        state = IDLE;
        TX_bit = 1;
        bit_error = 1;
        break;
      }
      if(bit_cnt == 4){
        if(frame_send.RTR == 1){
          CRC_en = 0;
          bit_cnt = 1;
          TX_bit = CRC_CALC >> (15 - bit_cnt);
          state = CRC_V;
        } else {
          bit_cnt = 0;
          TX_bit = (frame_send.DATA >> ((frame_send.DLC * 8) - bit_cnt - 1)) & 1;
          bit_cnt++;
          state = DATA;
        }
      } else {
        TX_bit = frame_send.DLC >> (3 - bit_cnt) & 1;
        bit_cnt++;
      }
      
      break;

    case DATA:
      if(RX_bit != TX_bit) //BIT_ERROR
      {
        state = IDLE;
        TX_bit = 1;
        bit_error = 1;
        break;
      }
      if(bit_cnt == frame_send.DLC * 8){
        bit_cnt = 0;
        TX_bit = CRC_CALC >> (14 - bit_cnt);
        bit_cnt = 1;
        state = CRC_V;
      } else {
        TX_bit = (frame_send.DATA >> ((frame_send.DLC * 8) - bit_cnt - 1)) & 1;
        bit_cnt++;
      }
      break;

    case CRC_V:
      if(RX_bit != TX_bit) //BIT_ERROR
      {
        state = IDLE;
        TX_bit = 1;
        bit_error = 1;
        break;
      }
      if(bit_cnt == 15){
        stuff_en = 0;
        TX_bit = frame_send.CRC_D;
        state = CRC_D;
      } else {
        TX_bit = (CRC_CALC >> (14 - bit_cnt)) & 1;
        bit_cnt++;
      }
      break;

    case CRC_D:
      if(RX_bit != TX_bit) //BIT_ERROR
      {
        state = IDLE;
        TX_bit = 1;
        bit_error = 1;
        break;
      }
      TX_bit = frame_send.ACK_S;
      state = ACK_S;
      break;

    case ACK_S:
      TX_bit = frame_send.ACK_D;
      state = ACK_D;
      break;

    case ACK_D:
      if(RX_bit != TX_bit) //BIT_ERROR
      {
        state = IDLE;
        TX_bit = 1;
        bit_error = 1;
        break;
      }
      bit_cnt = 0;
      TX_bit = (frame_send.EOFRAME >> (6-bit_cnt) ) & 1;
      bit_cnt++;
      state = EOFRAME;
      break;

    case EOFRAME:
      if(RX_bit != TX_bit) //BIT_ERROR
      {
        state = IDLE;
        TX_bit = 1;
        bit_error = 1;
        break;
      }
      if(bit_cnt == 7){
        TX_bit = 1;
        bit_cnt = 0;
        state = INTERFRAME;
      } else {
        TX_bit = (frame_send.EOFRAME >> (6-bit_cnt)) & 1;
        bit_cnt++;
      }
      break;

    case INTERFRAME:
      if(RX_bit != TX_bit) //BIT_ERROR
      {
        state = IDLE;
        TX_bit = 1;
        bit_error = 1;
        break;
      }
      if(bit_cnt == 2){
        bit_cnt = 0;
        state = IDLE;
      } else {
        TX_bit = 1;
        bit_cnt++;
      }
      break;
  }
}

void bitstuffREAD()
{ 
  static int count = 0;
  static int state = 0;
  static int last_rx;
  
  // debug(pc.printf("         - stuff READ: status: %s - RX_bit: %d - stuff_en: %d - error: %d - read_pt: %d - count: %d\n", (state==0)?"START":"COUNT", RX_bit, stuff_en, stuff_error, read_pt_int.read(), count));

  last_rx = RX_bit;
  RX_bit = RX.read();
  // RX_bit = TX_IN.read();

  // pc.printf("att                                   %d\n", RX_bit);

  // Debug <--
  // RX_bit = frame[frame_index];
  // frame_index++;
  // // debug(pc.printf("RX_bit: %d, Frame Index: %d, \n", RX_bit, frame_index));
  // if(frame_index >= sizeof(frame)/sizeof(bool))
  //   frame_index = 0;
  // Debug -->

  switch(state)
  {
    case(START):
      stuff_error = 0;
      decoder();
      if(stuff_en)
      {
        count++;
        state = COUNT;
      }
      break;

    case(COUNT):
      if(RX_bit == last_rx && count == 5 && stuff_en)
      {
        stuff_en = 0;
        stuff_error = 1;
        count = 0;
        decoder();
        state = COUNT;
      } else {
        if(!stuff_en)
        {
          count = 0;
          decoder();
          state = START;
        }
        else if(RX_bit == last_rx)
        {
          count++;
          decoder();
        }
        else if(RX_bit != last_rx && count == 5) // STUFF
        {
          count = 1;
          // state = START;
          debug(pc.printf("stuff - read\n"));
        }
        else if(RX_bit != last_rx && count != 5)
        {
          count = 1;
          decoder();
          // state = START;
        }
      }
      break;
  }
  

}

void bitstuffWRITE()
{
  static int count = 0;
  static int state = 0;

  // debug(pc.printf("- stuff WRT: status: %s - TX_bit: %d - TX: %d - last TX: %d -  stuff_en: %d - error: %d - read_pt: %d - count: %d\n\n", (state==0)?"START":"COUNT//STUFF", TX_bit, TX_IN.read(), last_tx, stuff_en, stuff_error, write_pt_int.read(), count));

  // last_tx = TX_IN.read();
  last_tx = RX.read();

  switch(state)
  {
    case(START):
      encoder();
      TX = TX_bit;
      if(stuff_en)
      {
        count++;
        state = COUNT;
      }
      break;

    case(COUNT):
      encoder();
      if(!stuff_en)
      {
        count = 1;
        TX = TX_bit;
        state = START;
      }
      else if(TX_bit == last_tx)
      {
        count++;
        TX = TX_bit;
      }
      else if(TX_bit != last_tx)
      {
        count = 1;
        TX = TX_bit;
      }
      if(count == 5 && TX_bit == last_tx) // STUFF
      {
        state = STUFF;
      }
      break;
    case (STUFF):
      debug(pc.printf("stuff - write\n"));
      state = COUNT;
      TX = !TX_bit;
      // TX_bit = TX_IN.read();
      TX_bit = RX.read();
      count = 1;
      break;
  }
  
}

int main() {  TX = 1;

  sample_pt = 0;
  wrt_sp_pt = 0;


  frame_send.SOF = 0;
  frame_send.ID = 0x20;
  frame_send.SRR = 0;
  frame_send.RTR = 0;
  frame_send.IDE = 0;
  frame_send.R0 = 0;
  frame_send.IDB = 0;
  frame_send.R1 = 0;
  frame_send.R2 = 0;
  frame_send.DLC = 2;
  frame_send.DATA = 0xaaaa;
  frame_send.data_b = false;
  frame_send.CRC_V = 30547;
  frame_send.CRC_D = 1;
  frame_send.ACK_S = 0;
  frame_send.ACK_D = 1;
  frame_send.EOFRAME = 127;
 
  
  wrt_sp_pt_int.rise(&bitstuffWRITE);
  sample_pt_int.rise(&bitstuffREAD);

  // write_pt_int.rise(&encoder);
  // read_pt_int.rise(&decoder);

  RX.fall(&edgeDetector);

  while(!BT);
  wait(0.5);
  
  tq_clock.attach(bitTimingSM, TIME_QUANTA_S);

  write_en = 1;
  wait(0.2);
  write_en = 0;

  while(1);
}