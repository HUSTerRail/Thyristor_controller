#include "stdint.h"
#ifndef _HCI_H_
#define _HCI_H_

#define LedSega 0xef
#define LedSegb 0xdf
#define LedSegc 0xbf
#define LedSegd 0xf7
#define LedSege 0xfb
#define LedSegf	0xfe
#define LedSegg 0xfd
#define LedSegh 0x7f

#define Dig_0 (LedSega & LedSegb & LedSegc & LedSegd & LedSege & LedSegf)
#define Dig_1 (LedSegb & LedSegc)
#define Dig_2 (LedSega & LedSegb & LedSege & LedSegd & LedSegg)
#define Dig_3 (LedSega & LedSegb & LedSegc & LedSegd & LedSegg)
#define Dig_4 (LedSegb & LedSegc & LedSegf & LedSegg)
#define Dig_5 (LedSega & LedSegc & LedSegd & LedSegf & LedSegg)
#define Dig_6 (LedSega & LedSegc & LedSegd & LedSege & LedSegf & LedSegg)
#define Dig_7 (LedSega & LedSegb & LedSegc)
#define Dig_8 (LedSega & LedSegb & LedSegc & LedSegd & LedSege & LedSegf & LedSegg)
#define Dig_9 (LedSega & LedSegb & LedSegc & LedSegd & LedSegf & LedSegg)
#define Dig_A (LedSega & LedSegb & LedSegc & LedSege & LedSegf & LedSegg)
#define Dig_b (LedSegc & LedSegd & LedSege & LedSegf & LedSegg)
#define Dig_C (LedSega & LedSege & LedSegd & LedSegf)
#define Dig_d (LedSegb & LedSegc & LedSegd & LedSege & LedSegg)
#define Dig_E (LedSega & LedSegd & LedSege & LedSegf & LedSegg)
#define Dig_F (LedSega & LedSege & LedSegf & LedSegg)
#define Dig_G (LedSega & LedSegc & LedSegd & LedSege & LedSegf & LedSegg)
#define Dig_H (LedSegb & LedSegc & LedSege & LedSegf & LedSegg)
#define Dig_L (LedSegd & LedSege & LedSegf)
#define Dig_n (LedSegc & LedSege & LedSegg)
#define Dig_o (LedSegc & LedSegd & LedSege & LedSegg)
#define Dig_O (LedSega & LedSegb & LedSegc & LedSegd & LedSege & LedSegf)
#define Dig_P (LedSega & LedSegb & LedSege & LedSegf & LedSegg)
#define Dig_r (LedSege & LedSegf & LedSegg)
#define Dig_S (LedSega & LedSegc & LedSegd & LedSegf & LedSegg)
#define Dig_T (LedSega & LedSege & LedSegf)
#define Dig_u (LedSegc & LedSegd & LedSege)
#define Dig_U (LedSegb & LedSegc & LedSegd & LedSege & LedSegf)

#define Dig_Dash LedSegg
#define Dig_Top LedSega
#define Dig_Equ  (LedSegg  & LedSegd)
#define Dig_0C 0xc3
#define PointAdd 0x10
#define Dig_h 0x27
#define Dig_None 0xff
#define Dig_X 0x00
#define Dot        LedSegh

#define BYT0 0
#define BYT1 1
#define BYT2 2
#define BYT3 3
#define BYT4 4
#define BYT5 5
#define BYT6 6
#define BYT7 7



/*
 #define LED_V           0x01
 #define LED_A           0x02
 #define LED_HZ          0x04
 #define LED_LOC_REM     0x08
 #define LED_FWD_REV     0x10
 #define LED_TRIP        0x20
 #define LED_RUN         0x40
 */


#define KeyDataValid       0//((ADI_PORTE.DATA&ADI_GPIO_PIN_3) == ADI_GPIO_PIN_3)
#define KeyDataLo       0//((ADI_PORTE.DATA&ADI_GPIO_PIN_3) == 0)

#define SET_LEDCS()	//(ADI_PORTE.DATA_SET = ADI_GPIO_PIN_1)/*片選线置位*/
#define CLR_LEDCS() //(ADI_PORTE.DATA_CLR = ADI_GPIO_PIN_1)/*片選线值0*/
#define SET_LEDDA()	//(ADI_PORTE.DATA_SET = ADI_GPIO_PIN_2)/*數據綫线置位*/
#define CLR_LEDDA() //(ADI_PORTE.DATA_CLR = ADI_GPIO_PIN_2)/*數據綫线值0*/
#define SET_LEDCK()	//(ADI_PORTE.DATA_SET = ADI_GPIO_PIN_0)/*時鐘线置位*/
#define CLR_LEDCK() //(ADI_PORTE.DATA_CLR = ADI_GPIO_PIN_0)/*時鐘线值0*/
//键值
typedef enum _KeyEnum {
	K_ESC = 0,
	K_PLUS = 1,
	K_MINUS = 2,
	K_ENT = 3,
	K_NULL = 0x10
} KeyEnum;
//动作
typedef enum _KeyAction {
	K_NULLACT = 0,
	K_LONGKEY,
	K_CLICK,
	K_CONTINUE,
	K_LEAVE,
	K_SHORT_LEAVE,
	K_DOUBLE_CLICK
} KeyAction;

typedef struct _KeyInfo {
	KeyEnum KeyNo;
	KeyAction KeyAct;
} KeyInfo;

typedef struct _KeyHandle {
	void (*KeyLong)(KeyInfo info);   //长按操作
	void (*KeyClick)(KeyInfo info);	 //按下
	void (*KeyLeave)(KeyInfo info);	 //松开
	void (*KeyShortLeave)(KeyInfo info);	 //short press松开
	void (*KeyContin)(KeyInfo info); //连按操作
	void (*KeyDoubleClick)(KeyInfo info); //双击操作
} KeyHandlerType;

typedef struct _STRUCT_LEDKEY {

	//KEY LED刷新定时
	KeyEnum KeyPtr;
	KeyEnum KeyPtrLast;
	KeyEnum valuePre[8];
	uint32_t keyCount[8];
	uint32_t keyLeaveCount[8];
	uint32_t keyCountRec[8];
	uint8_t Ledptr;

	//刷新
	int16_t LedFlashPrd;
	int16_t LedRefreshPrd;
	int16_t DataRefreshPrd;
	int16_t LedRefreshCnt;
	int16_t DataRefreshCnt;
	uint8_t LedDataFlg, LedFlg;

	//按键加速
	uint32_t NullKeyPrd; //无按键时周期执行某函数
	uint32_t LongKeyPrd;
	uint32_t Level1AccPrd; //一级加速开始时间    ms
	uint32_t Level2AccPrd; //二级加速开始时间  ms
	uint32_t Level1AccItv; //一级加速间隔 ms
	uint32_t Level2AccItv; //一级加速间隔 ms

	//输出
	uint8_t LedBuf[8];
	uint16_t LedOutDat;
	uint8_t DotPos;	//
	uint8_t ShiftValue; //
	int8_t LFlashCtr;
	uint8_t OnUpDwnSel :2;
	uint8_t HLviewFlg :2;
	uint8_t FlashDis :1; //禁止LED闪烁
	uint8_t IntRefreDis :1;
	uint8_t RefreDis :1;
	uint8_t statusLedFlashEN;
	uint8_t LedFlashEN;

	//按键相关
	uint32_t keyFilCount[8];
	uint32_t KeyFilTim; //按键滤波时间
	uint32_t DoubleClickItv ;
	KeyHandlerType handle[8];
	void (*LedDataRefresh)(void);
	void (*NullKey)(void); //若没有按键按下，100ms执行一次该程序
	int32_t sysTimer;
	int32_t sysTimerRec;

	union {
		uint8_t all;
		struct {
			uint8_t Key0 :1;
			uint8_t Key1 :1;
			uint8_t Key2 :1;
			uint8_t Key3 :1;
			uint8_t Key4 :1;
			uint8_t Key5 :1;
			uint8_t Key6 :1;
			uint8_t Key7 :1;
		} bit;
	} KeyIsPress;
} LedKeyStru;

typedef union _DataShowPropStru {
	struct {
		uint32_t dotpos :8;		//小数点位置
		uint32_t ShiftValue :8; //移位值 0~9
		uint32_t HLFlag :2; //是否使用高低位显示 0：不使用 1：HL模式 2：HML模式
		uint32_t isHex :1;		//是否是十六进制
		uint32_t flashFlag :1;  //ShiftValue处是否闪烁
		uint32_t sign :1;		//有符号？
	};
	uint32_t data;
} DataShowPropStru;

#define RUNLed_On  0x02
#define TRIPLed_On  0x04
#define FWDLed_On  0x08
#define LOCLed_On  0x10
#define HzLed_On  0x20
#define A_Led_On  0x40
#define V_Led_On  0x80

#define RFLLed_Off 0xe0
#define RUNLed_Off 0xfd
#define TRIPLed_Off 0xfb
#define FWDLed_Off 0xf7
#define LOCLed_Off 0xef
#define HzLed_Off 0xdf
#define A_Led_Off 0xbf
#define V_Led_Off 0x7f
#define UnitLed_Off 0x1f
extern uint8_t CODE164[];
extern uint16_t ByteCode[8];
extern const uint8_t FwaitTab[];
void LedOut(uint16_t data);
void LedOut_Slow(uint16_t data);
void SetRefreshCtrl(uint8_t data);
void ToggleHLView();
void SetHLViewValue(uint8_t value);
void SetLedFlash(uint8_t statusLed);
void SetStatusLed(uint8_t statusLed, uint8_t isFlash);
void ClrStatusLed(uint8_t statusLed);
void LedKeyTick();
KeyEnum LedKeyRefresh();
void Led_Init();
void LedWaitTab(int no);
void DecHexLEDx(int32_t data, DataShowPropStru prop);
void LedBuffWrite(uint8_t * buff, uint8_t startPos, uint8_t num);
extern uint8_t CODE164[];
#endif
