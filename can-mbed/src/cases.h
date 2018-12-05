// Debug
#include <inttypes.h>

#define DEBUG_CASE 3


typedef struct FRAME_DEBUG
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
    bool data_b;
    uint16_t CRC_V;
    bool CRC_D;
    bool ACK_S;
    bool ACK_D;
    uint8_t EOFRAME;
} FRAME_DEBUG;

FRAME_DEBUG frame_dbg;

void init_frame_debug(){
    frame_dbg.SOF = 0;
    frame_dbg.ID = 0;
    frame_dbg.SRR = 0;
    frame_dbg.RTR = 0;
    frame_dbg.IDE = 0;
    frame_dbg.R0 = 0;
    frame_dbg.IDB = 0;
    frame_dbg.R1 = 0;
    frame_dbg.R2 = 0;
    frame_dbg.DLC = 0;
    frame_dbg.DATA = 0;
    frame_dbg.data_b = false;
    frame_dbg.CRC_V = 0;
    frame_dbg.CRC_D = 1;
    frame_dbg.ACK_S = 0;
    frame_dbg.ACK_D = 1;
    frame_dbg.EOFRAME = 127;
}

// alain - 1
#if DEBUG_CASE == 1
    #define DEBUG_SEND
    bool frame[] = {0,1,1,0,0,1,1,1,0,0,1,0,0,0,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,0,0,1,0,0,0,0,1,0,1,0,0,0,1,1,0,1,1,1,1,1,1,1,1,1,1};
    void fill_frame_debug()
    {
        init_frame_debug();
        frame_dbg.ID = 0x0672;
        frame_dbg.RTR = 0;
        frame_dbg.IDE = 0;
        frame_dbg.DLC = 8;
        frame_dbg.DATA = 0xAAAAAAAAAAAAAAAA;
        frame_dbg.CRC_V = 0b000000001010001;
    }
#endif
// alain - 2
#if DEBUG_CASE == 2
    #define DEBUG_SEND
    bool frame[] = {0,1,1,0,0,1,1,1,0,0,1,0,0,0,0,0,1,1,1,1,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,0,1,1,0,0,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
    void fill_frame_debug()
    {
        init_frame_debug();
        frame_dbg.ID = 0x0672;
        frame_dbg.RTR = 0;
        frame_dbg.IDE = 0;
        frame_dbg.DLC = 7;
        frame_dbg.DATA = 0xAAAAAAAAAAAAAA;
        frame_dbg.CRC_V = 0b101100110011101;
    }
#endif
// alain - 6
#if DEBUG_CASE == 3
    #define DEBUG_SEND
    bool frame[] = {0,1,1,0,0,1,1,1,0,0,1,0,0,0,0,0,1,0,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,0,1,0,1,1,1,0,0,0,0,0,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1};
    void fill_frame_debug()
    {
        init_frame_debug();
        frame_dbg.ID = 0x0672;
        frame_dbg.RTR = 0;
        frame_dbg.IDE = 0;
        frame_dbg.DLC = 3;
        frame_dbg.DATA = 0xAAAAAA;
        frame_dbg.CRC_V = 0b010010111000001;
    }
#endif
// alain - 9
#if DEBUG_CASE == 4
    #define DEBUG_SEND
    bool frame[] = {0,1,1,0,0,1,1,1,0,0,1,0,0,0,0,0,1,0,0,0,0,1,1,0,0,1,0,1,1,0,1,0,1,0,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
    void fill_frame_debug()
    {
        init_frame_debug();
        frame_dbg.ID = 0x0672;
        frame_dbg.RTR = 0;
        frame_dbg.IDE = 0;
        frame_dbg.DLC = 0;
        frame_dbg.DATA = 0;
        frame_dbg.CRC_V = 0b011001011010101;
    }
#endif
// alain - 10
#if DEBUG_CASE == 5
    #define DEBUG_SEND
    bool frame[] = {0,1,1,0,0,1,1,1,0,0,1,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0,1,1,0,0,0,1,0,0,0,0,1,0,1,1,1,1,1,1,1,1,1,1};
    void fill_frame_debug()
    {
        init_frame_debug();
        frame_dbg.ID = 0x0672;
        frame_dbg.RTR = 1;
        frame_dbg.IDE = 0;
        frame_dbg.DLC = 0;
        frame_dbg.DATA = 0;
        frame_dbg.CRC_V = 0b100000100010000;
    }
#endif
// alain - 11
#if DEBUG_CASE == 6
    #define DEBUG_SEND
    bool frame[] = {0,1,1,0,0,1,1,1,0,0,1,0,1,0,0,0,0,0,1,1,0,0,0,0,1,0,0,1,0,0,0,1,0,0,1,1,0,1,1,1,1,1,1,1,1,1,1,1};
    void fill_frame_debug()
    {
        init_frame_debug();
        frame_dbg.ID = 0x0672;
        frame_dbg.RTR = 1;
        frame_dbg.IDE = 0;
        frame_dbg.DLC = 1;
        frame_dbg.DATA = 0;
        frame_dbg.CRC_V = 0b000010010001001;
    }
#endif
// alain - 12
#if DEBUG_CASE == 7
    #define DEBUG_SEND
    bool frame [] = {0,1,0,0,0,1,0,0,1,0,0,1,1,1,1,1,0,0,0,0,0,1,0,0,0,0,0,1,1,1,1,1,0,0,1,0,0,0,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,1,0,1,1,1,1,1,0,1,0,1,0,1,1,0,1,1,1,1,1,1,1,1,1,1};
    void fill_frame_debug()
    {
        init_frame_debug();
        frame_dbg.ID = 0x0449;
        frame_dbg.SRR = 1;
        frame_dbg.RTR = 0;
        frame_dbg.IDE = 1;
        frame_dbg.IDB = 0x3007A;
        frame_dbg.DLC = 8;
        frame_dbg.DATA = 0xAAAAAAAAAAAAAAAA;
        frame_dbg.CRC_V = 0b111101111110101;
    }
#endif
// alain - 13
#if DEBUG_CASE == 8
    #define DEBUG_SEND
    bool frame [] = {0,1,0,0,0,1,0,0,1,0,0,1,1,1,1,1,0,0,0,0,0,1,0,0,0,0,0,1,1,1,1,1,0,0,1,0,1,0,0,1,0,0,0,0,1,0,1,0,0,1,1,1,1,1,0,0,1,1,0,1,0,1,1,1,1,1,1,1,1,};
    void fill_frame_debug()
    {
        init_frame_debug();
        frame_dbg.ID = 0x0449;
        frame_dbg.SRR = 1;
        frame_dbg.RTR = 1;
        frame_dbg.IDE = 1;
        frame_dbg.IDB = 0x3007A;
        frame_dbg.DLC = 8;
        frame_dbg.DATA = 0;
        frame_dbg.CRC_V = 0b010100111110110;
    }
#endif
// alain - 24 - idle grande
#if DEBUG_CASE == 9
    #define DEBUG_SEND
    bool frame[] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,0,1,0,1,1,0,0,0,0,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,0,1,0,1,1,0,0,0,0,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1};
    void fill_frame_debug()
    {
        init_frame_debug();
        frame_dbg.ID = 0b11;
        frame_dbg.SRR = 1;
        frame_dbg.RTR = 0;
        frame_dbg.IDE = 1;
        frame_dbg.IDB = 0;
        frame_dbg.DLC = 15;
        frame_dbg.DATA = 0xFFFFFFFFFFFFFFF;
        frame_dbg.CRC_V = 0b100111011110110;
    }
#endif
// alain - 17 - ack error
#if DEBUG_CASE == 10
    #define DEBUG_RECEIVE 
    bool frame[] = {0,1,1,0,0,1,1,1,0,0,1,0,0,0,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,0,0,1,0,0,0,0,1,0,1,0,0,0,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1};
#endif
// alain - 18 - stuff error
#if DEBUG_CASE == 11
    #define DEBUG_RECEIVE
    bool frame[] = {0,1,1,0,0,1,1,1,0,0,1,0,0,0,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1};
#endif
// alain - 19 - crc error
#if DEBUG_CASE == 12
    #define DEBUG_RECEIVE
    bool frame[] = {0,1,1,0,0,1,1,1,0,0,1,0,0,0,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,0,0,1,0,0,0,0,1,1,1,0,0,0,1,1,0,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1};
#endif