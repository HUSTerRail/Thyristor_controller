/*****************************************
 **CanToWifi.h
 **CanTowifi.h
 **      1. void CanToWifiInit();    initialization
 **      2. void ReceptionHandler(CANframePtr frame);       can reception Interrupt Handler
 **
 **      3. void ScopeTimerHandler();
 **
 **      4. extern void CanframeTransmit(CANframePtr frame_p, uint8 canboxID);

 **      5.CANtrans_mbox0,CANtrans_mbox1
 **      //two can mboxs, one for sending status ,
 **       the other for sending scope data
 **      #define CANtrans_mbox0          24
 **         #define CANtrans_mbox1          25

 ******************************************/

#include "stdint.h"
#include "stdbool.h"
#ifndef __CANTOWIFI_H__
#define __CANTOWIFI_H__

typedef struct
{
    uint8_t data[8];
    uint8_t dlc;
    uint8_t ideflg;
    uint32_t id;
} CANframe, *CANframePtr;

//typedef Can_Std_Frame CANframe;
//typedef Can_Std_Frame * CANframePtr;

typedef struct
{
    uint16_t CH1Enable : 1;
    uint16_t CH1width_32 : 1;
    uint16_t CH2Enable : 1;
    uint16_t CH2width_32 : 1;
    uint16_t CH3Enable : 1;
    uint16_t CH3width_32 : 1;
    uint16_t CH4Enable : 1;
    uint16_t CH4width_32 : 1;
    uint16_t CH5Enable : 1;
    uint16_t CH5width_32 : 1;
    uint16_t CH6Enable : 1;
    uint16_t CH6width_32 : 1;
    uint16_t CH7Enable : 1;
    uint16_t CH7width_32 : 1;
    uint16_t CH8Enable : 1;
    uint16_t CH8width_32 : 1;
    uint16_t ScopeEnable : 1;
} ScopeCtrBits;

typedef union
{
    uint32_t all;
    ScopeCtrBits bit;
} ScopeCtrReg;

typedef struct
{
    ScopeCtrReg scopeCtrReg;
    uint16_t CH1Select;
    uint16_t CH2Select;
    uint16_t CH3Select;
    uint16_t CH4Select;
    uint16_t CH5Select;
    uint16_t CH6Select;
    uint16_t CH7Select;
    uint16_t CH8Select;
    uint16_t DisplayIntervalTimer;  //示波器数据时间间隔计时器
    uint16_t DisplayIntervalSet;    //示波器数据时间间隔
    uint16_t TimeoutTimer;          //超时计时器
    uint16_t TimeoutSet;            //超时时间
    uint16_t TryTimes;              //超时后，尝试连接次数
    uint16_t TryLimit;              //尝试次数，当超时次数等于限定值时，宣告连接失败
    uint8_t staNumber;              //站号
} ScopeSetting;


#define PARAMSGROUPStart 0x0000
#define PARAMSGROUPEnd   0x0126

#define CANCMD_START_ADDR 0x012C //300
#define CANCMD_END_ADDR   0x108F

#define MONITORSETStart  0x1090
#define MONITORSETEnd    0x1096

#define STATUSGROUPStart 0x2000
#define STATUSGROUPEnd   0x2100

#define MODE_CMD_ADDR    0x1070
#define COMIO_Addr       0x1071
#define COMIO_SETCLR_ADDR 0x1072
#define POSI_CMD_ADDR_H 0x1073
#define POSI_CMD_ADDR_L 0x1074
#define SPD_CMD_ADDR 0x1075
#define TORQ_CMD_ADDR 0x1076
#define POSISYNC_CMD_ADDR_H    0x1077
#define POSISYNC_CMD_ADDR_L    0x1078
#define FNOperateAddr    0x1086

//HandHeld
#define READADDR_DEVICETYPE  300
#define READADDR_HWVERSION   301
#define READADDR_SWVERSION   302
#define READADDR_SNVERSION   303
#define READADDR_ERRORCODE   304

#define WRITE_UPDATA_COM    300
#define CLEAR_ERROR_HISTORY 301
#define WRITE_DEFAULT_PAR   350
#define AUTO_LEARN_PHASE    351
#define FACTORY_TEST_MODE   352
#define RUN_TEST            353
#define PULLPHASE_MODE      354


enum EnumDriverStatus
{
    //error
    DrivSt_CurErr = 0x02,
    DrivSt_ErrNumber = 0xa0,
    DrivSt_ErrRec1 = 0xa1,
    DrivSt_ErrRec2 = 0xa2,
    DrivSt_ErrRec3 = 0xa3,
    DrivSt_ErrRec4 = 0xa4,
    DrivSt_ErrRec5 = 0xa5,
    DrivSt_ErrRec6 = 0xa6,
    DrivSt_ErrRec7 = 0xa7,
    DrivSt_ErrRec8 = 0xa8,
    DrivSt_ErrRec9 = 0xa9,
    DrivSt_ErrRec10 = 0xaa,
    //
    DrivSt_Status = 0x01,
    DrivSt_SpdFbd = 0x03,
    DrivSt_SpdRef = 0x04,
    DrivSt_Torq = 0x05,
    DrivSt_DcVolt = 0x06,
    DrivSt_EncPosi = 0x0c,
    DrivSt_Reverse1 = 0x0f,
    DrivSt_Reverse2 = 0x10,
    DrivSt_Temperature = 0x13,
    DrivSt_AnaVoltRef = 0x14,
    DrivSt_EleAngle = 0x30,
    DrivSt_PhaseACur = 0x31,
    DrivSt_PhaseBCur = 0x32,
    DrivSt_PhaseCCur = 0x33,
    DrivSt_axisDCurRef = 0x34,
    DrivSt_axisDCurFbd = 0x36,
    DrivSt_axisQCurRef = 0x35,
    DrivSt_axisQCurFbd = 0x37,
    DrivSt_axisDVoltRef = 0x38,
    DrivSt_axisQVoltRef = 0x39,
    DrivSt_EcCnt = 0x40,
    DrivSt_RunMode = 0x41,
    DrivSt_MInput = 0x42,
    DrivSt_MOutput = 0x43,
    DrivSt_MISt1 = 0x44,
    DrivSt_MISt2 = 0x45,
    DrivSt_MISt3 = 0x46,
    DrivSt_MISt4 = 0x47,
    DrivSt_MISt5 = 0x48,
    DrivSt_MISt6 = 0x49,
    DrivSt_MISt7 = 0x4A,
    DrivSt_MISt8 = 0x4B,
    DrivSt_Rev32_1 = 0x4C,
    DrivSt_Rev32_2 = 0x4E,
    DrivSt_Error = 0x50,
    DrivSt_DEAddr = 0x51,
    DrivSt_CANErrRecCnt = 0x52,
    DrivSt_CANErrTraCnt = 0x53,
    DrivSt_Error1 = 0x54,
    DrivSt_Error2 = 0x55,
    DrivSt_Error3 = 0x56,
    DrivSt_Error4 = 0x57,
    DrivSt_Rev32_3 = 0x60,
    DrivSt_Rev32_4 = 0x62,
    DrivSt_Rev32_5 = 0x64,
    DrivSt_Rev32_6 = 0x66,
    DrivSt_GenYear = 0x68,
    DrivSt_GenMonDay = 0x69,
    DriSt_SoftVersion = 0x6A
};

#define CAN_DEBUG_ID_MOSI 0x08
#define CAN_DEBUG_ID_MISO 0x09

void CanToWifiInit(uint8_t);
uint8_t ScopeisEnable(void);
void ReceptionHandler(CANframePtr frame);
void ScopeTimerHandler(void);
void ScopedataTransmit(void);
int CanframeTransmit(CANframePtr frame_p);
#endif
