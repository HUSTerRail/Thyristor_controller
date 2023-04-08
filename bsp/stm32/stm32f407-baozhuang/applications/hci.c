#include "rtthread.h"
#include <HCI.h>
LedKeyStru LedKey;
uint8_t LED_bit_mask[8] = { 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80 };
uint16_t ByteCode[8] = { 0xfe00, 0xfd00, 0xfb00, 0xf700, 0xef00, 0xdf00, 0xbf00,
		0x7f00 };

const uint8_t FwaitTab[] = { 0x00, 0x02, 0x0a, 0x1a, 0x1e, 0x9e, 0xde, 0xfe,
		0xff, 0xff, 0xff, 0xff };
uint8_t CODE164[] = {
Dig_0, Dig_1, Dig_2, Dig_3, Dig_4, Dig_5, Dig_6, Dig_7, Dig_8, Dig_9,
Dig_A, Dig_b, Dig_C, Dig_d, Dig_E, Dig_F,
Dig_L, Dig_r, Dig_u, Dig_P, Dig_H, Dig_n,
Dig_o, Dig_U, Dig_T, Dig_X, Dig_Dash, Dig_Equ };
//other files
//this file
void LedRefresh(LedKeyStru * ledkey);
static uint8_t BitNisSetU8(uint8_t value, uint8_t bit_num);
static void SetBitNValueU8(uint8_t * value, uint8_t bit_num);
static void ClrBitNValueU8(uint8_t * value, uint8_t bit_num);
KeyEnum LedKey_GetKey(LedKeyStru * ledkey);
KeyEnum __LedKeyRefresh(LedKeyStru * ledkey);
void DecLEDx(LedKeyStru * ledkey, int32_t data, DataShowPropStru prop);
void HexLEDx(LedKeyStru * ledkey, int32_t data, uint8_t sign, uint8_t dotpos,
		uint8_t ShiftValue);
void LedOut(uint16_t data);

// external interface
extern void key_handler(KeyInfo info);
//Led initial
void Led_Init() {
	int i;
	LedKey.LedFlashEN = 0;
	LedKey.statusLedFlashEN = 0;
	LedKey.RefreDis = 0;
	LedKey.IntRefreDis = 0;
	LedKey.ShiftValue = 0;
	LedKey.DotPos = 0;
	LedKey.LedBuf[BYT4] = Dig_Dash;
	LedKey.LedBuf[BYT3] = Dig_Dash;
	LedKey.LedBuf[BYT2] = Dig_Dash;
	LedKey.LedBuf[BYT1] = Dig_Dash;
	LedKey.LedBuf[BYT0] = Dig_Dash;
	LedKey.LedBuf[BYT5] = FwaitTab[0]; //指示灯
	LedKey.KeyPtr = K_NULL;
	LedKey.KeyIsPress.all = 0;

	for (i = 0; i < 8; i++) {
		LedKey.valuePre[i] = K_NULL;
		LedKey.keyFilCount[i] = 0;
		LedKey.keyCount[i] = 0;
		LedKey.keyLeaveCount[i] = 0;
		LedKey.keyCountRec[i] = 0;
		LedKey.handle[i].KeyClick = 0;
		LedKey.handle[i].KeyContin = 0;
		LedKey.handle[i].KeyLong = 0;
		LedKey.handle[i].KeyLeave = 0;
		LedKey.handle[i].KeyShortLeave = 0;
		LedKey.handle[i].KeyDoubleClick = 0;
	}

	//LED数据刷新 K_STP
	LedKey.LedDataRefresh = 0;
	
	//LedKey.NullKey = NullKeyHandle;
	LedKey.handle[K_ESC].KeyClick = key_handler;
	LedKey.handle[K_ESC].KeyContin = key_handler;
	LedKey.handle[K_PLUS].KeyClick = key_handler;
	LedKey.handle[K_PLUS].KeyContin = key_handler;
	LedKey.handle[K_MINUS].KeyClick = key_handler;
	LedKey.handle[K_MINUS].KeyContin = key_handler;
	LedKey.handle[K_ENT].KeyClick = key_handler;
	LedKey.handle[K_ENT].KeyLong = key_handler;

	//刷新
	LedKey.LedRefreshPrd = 1;
	LedKey.DataRefreshPrd = 200;
	LedKey.LedRefreshCnt = 0;
	LedKey.DataRefreshCnt = 0;
	LedKey.LedDataFlg = 0;
	LedKey.LedFlg = 0;
	LedKey.DoubleClickItv = 90; //双击最大间隔时间
	//闪烁控制
	LedKey.LedFlashPrd = 1; //闪烁周期是DataRefreshPrd数据刷新周期的倍数
	LedKey.NullKeyPrd = 120;
	LedKey.LongKeyPrd = 100;
	//按键加速
	LedKey.Level1AccPrd = 80; //一级加速开始时间    ms*8
	LedKey.Level2AccPrd = 350; //二级加速开始时间   ms*8
	LedKey.Level1AccItv = 20; //一级加速间隔 ms*8
	LedKey.Level2AccItv = 10; //一级加速间隔 ms*8
	//按键滤波
	LedKey.KeyFilTim = 2;

	LedKey.sysTimerRec = 0;
	LedOut(0x1f00 + Dig_Dash);
}

void SetRefreshCtrl(uint8_t data) {
	if (data & 1)
		LedKey.RefreDis = 1;
	else
		LedKey.RefreDis = 0;
	if (data & 2)
		LedKey.IntRefreDis = 1;
	else
		LedKey.IntRefreDis = 0;
}
//boot led
void LedWaitTab(int no) {
	int i;
	LedKey.LedBuf[BYT5] = FwaitTab[no]; //指示灯
	for (i = 0; i <= 4; i++) {
		LedKey.LedBuf[i] = Dig_Dash;
	}
}
//
void SetStatusLed(uint8_t statusLed, uint8_t isFlash) {
	if (statusLed) //开启某个或者某几个状态灯
		LedKey.LedBuf[BYT5] |= statusLed;
	if (isFlash) //状态灯是否需要闪烁
		LedKey.statusLedFlashEN |= statusLed;
	else
		LedKey.statusLedFlashEN &= ~statusLed;
}
//
void ClrStatusLed(uint8_t statusLed) {
	if (statusLed) { //清除状态灯以及其闪烁状态
		LedKey.LedBuf[BYT5] &= ~statusLed;
		LedKey.statusLedFlashEN &= ~statusLed;
	}
}
//
void SetLedFlash(uint8_t statusLed) {
	LedKey.LedFlashEN = statusLed;
}
//
void ToggleHLView() {
	if (LedKey.OnUpDwnSel >= LedKey.HLviewFlg)
		LedKey.OnUpDwnSel = 0;
	else
		LedKey.OnUpDwnSel++;
}
//
void SetHLViewValue(uint8_t value) {
	LedKey.OnUpDwnSel = value;
}
//Led timer and fresh
//place in interrupt routine
void LedKeyTick() {
	//如果由于背景查询很长时间不刷新LED，则在中断中刷新 ,只刷新LED，不读按键
	if ((LedKey.sysTimer - LedKey.sysTimerRec) > LedKey.LedRefreshPrd) {
		if (LedKey.IntRefreDis == 0)
			LedRefresh(&LedKey);
		LedKey.sysTimerRec = LedKey.sysTimer;
		LedKey.LedRefreshCnt = 0;
		LedKey.LedFlg = 0;
	}
	LedKey.LedRefreshCnt++;
	LedKey.DataRefreshCnt++;
	LedKey.sysTimer++;
	if (LedKey.LedRefreshCnt >= LedKey.LedRefreshPrd) {
		LedKey.LedRefreshCnt = 0;
		LedKey.LedFlg = 1;
	}
	if (LedKey.DataRefreshCnt >= LedKey.DataRefreshPrd) {
		LedKey.DataRefreshCnt = 0;
		LedKey.LedDataFlg = 1;
	}
}

//key detect and handle
//place in background thread
KeyEnum LedKeyRefresh() {
	return __LedKeyRefresh(&LedKey);
}

//
void DecHexLEDx(int32_t data, DataShowPropStru prop) {
	if (prop.isHex)
		HexLEDx(&LedKey, data, prop.sign, prop.dotpos, prop.ShiftValue);
	else
		DecLEDx(&LedKey, data, prop);

}

void LedBuffWrite(uint8_t * buff, uint8_t startPos, uint8_t num) {
	int i;
	if (startPos + num > 8)
		return;
	for (i = startPos; i < startPos + num; i++) {
		LedKey.LedBuf[i] = buff[i - startPos];
	}
}
//external interface end

static uint8_t BitNisSetU8(uint8_t value, uint8_t bit_num) {
	uint8_t temp = bit_num;
	if (temp > 7)
		temp = 7;
	if ((value & LED_bit_mask[temp]) != 0) {
		return (1);
	} else {
		return (0);
	}
}

static void SetBitNValueU8(uint8_t * value, uint8_t bit_num) {
	uint8_t temp = bit_num;
	if (temp > 7)
		temp = 7;
	*value |= LED_bit_mask[temp];
}

static void ClrBitNValueU8(uint8_t * value, uint8_t bit_num) {
	uint8_t temp = bit_num;
	if (temp > 7)
		temp = 7;
	*value &= ~LED_bit_mask[temp];
}

KeyEnum __LedKeyRefresh(LedKeyStru * ledkey) {
	static uint32_t timer = 0;
	static uint32_t timerFlash = 0;
	KeyEnum key = K_NULL;
	if (ledkey->LedFlg) {
		timer++;
		ledkey->LedFlg = 0;
		//刷新LED和获取按键，并执行按键响应函数
		ledkey->IntRefreDis = 1;
		key = LedKey_GetKey(ledkey);
		ledkey->IntRefreDis = 0;
		if (key != K_NULL) { //有按键按下立即刷新显示
			timer = 0;
			if (ledkey->LedDataRefresh != 0)
				ledkey->LedDataRefresh();
		} else if (ledkey->LedDataFlg) { //刷新数据
			ledkey->LedDataFlg = 0;
			if (ledkey->LedDataRefresh != 0)
				ledkey->LedDataRefresh();
			timerFlash++;
			if (timerFlash >= ledkey->LedFlashPrd) { //闪烁控制
				ledkey->LFlashCtr = ledkey->LFlashCtr + 1;
				timerFlash = 0;
			}
		}
		//无按键动作周期执行函数
		if (timer >= ledkey->NullKeyPrd) {
			timer = 0;
			if (ledkey->NullKey != 0)
				ledkey->NullKey();
		}
		//记录LED刷新时间
		ledkey->sysTimerRec = ledkey->sysTimer;
	}
	return key;
}

RT_WEAK int get_key_status(int index)
{
	return 1;
}

void KeySample(LedKeyStru * ledkey) {
	static KeyEnum KeyDownFlag = K_NULL; //按键锁定，以防多个按键同时按下
	if (get_key_status(ledkey->Ledptr) == 0) {
		//按下滤波
		if (ledkey->keyFilCount[ledkey->Ledptr] < ledkey->KeyFilTim)
			ledkey->keyFilCount[ledkey->Ledptr]++;
		if (ledkey->keyFilCount[ledkey->Ledptr] >= ledkey->KeyFilTim) {
			if (KeyDownFlag == K_NULL) { //当前没有按键按下 ， 响应按键并锁定
				KeyDownFlag = (KeyEnum) ledkey->Ledptr;
			}
			if ((uint8_t) KeyDownFlag == ledkey->Ledptr) {
				ledkey->KeyPtr = (KeyEnum) ledkey->Ledptr;
			} else {
				ledkey->KeyPtr = ledkey->valuePre[ledkey->Ledptr];
			}
			SetBitNValueU8(&ledkey->KeyIsPress.all, ledkey->Ledptr); //记录按键按下
		} else {
			ledkey->KeyPtr = ledkey->valuePre[ledkey->Ledptr];
		}
	} else {
		//松开滤波
		if (ledkey->keyFilCount[ledkey->Ledptr] == 0) {
			ledkey->KeyPtr = K_NULL;
			ClrBitNValueU8(&ledkey->KeyIsPress.all, ledkey->Ledptr); //记录按键松开
			if ((uint8_t) KeyDownFlag == ledkey->Ledptr) { //锁定的按键松开了
				KeyDownFlag = K_NULL;
			}
		} else {
			ledkey->keyFilCount[ledkey->Ledptr]--;
			ledkey->KeyPtr = ledkey->valuePre[ledkey->Ledptr];
		}
	}
}

/***************************************
 *
 */
RT_WEAK void LedOut(uint16_t data) {

}

/***************************************
 *
 */
void LedOut_Slow(uint16_t data) {

}

//led 刷新，1ms
void LedRefresh(LedKeyStru * ledkey) {
	//LedOutDat移位显示，指示灯与数码管。高八位表示正在显示的数码管编号或者指示灯编码（总共BYT6）
	uint8_t i = 0, temp_code = 0;
	ledkey->Ledptr++;
	ledkey->Ledptr = ledkey->Ledptr & 0x7;
	//闪烁控制
	if (ledkey->Ledptr <= 4) {
		if ((ledkey->LFlashCtr & 0x02) && (ledkey->FlashDis == 0)) {
			if (BitNisSetU8(ledkey->LedFlashEN, ledkey->Ledptr))
				temp_code = Dig_None;
			else
				temp_code = ledkey->LedBuf[ledkey->Ledptr];
		} else
			temp_code = ledkey->LedBuf[ledkey->Ledptr];
	} else if ((ledkey->Ledptr == 5)) { //status Led
		if (ledkey->LFlashCtr & 0x01) {
			temp_code = ledkey->LedBuf[5];
			for (i = 0; i < 8; i++) {
				if (ledkey->statusLedFlashEN & LED_bit_mask[i]) {
					temp_code = temp_code
							& (ledkey->LedBuf[5] & (~LED_bit_mask[i]));
				}
			}
		} else {
			temp_code = ledkey->LedBuf[5];
		}

	} else {

	}
	ledkey->LedOutDat = ByteCode[ledkey->Ledptr] + temp_code;
	LedOut(ledkey->LedOutDat);
//	Spt0a_Send(ledkey->LedOutDat);
}

//按键采样和计时计数,1ms调用一次
KeyEnum LedKey_GetKey(LedKeyStru * ledkey) {
	int re = 0;
	KeySample(ledkey);
	KeyInfo inf;
	//按下与松开
	if ((ledkey->valuePre[ledkey->Ledptr] != ledkey->KeyPtr)) {
		if ((ledkey->KeyPtr == K_NULL)) { //松开
			inf.KeyNo = ledkey->valuePre[ledkey->Ledptr];
			//短按松开
			if (ledkey->keyCount[ledkey->Ledptr] < ledkey->LongKeyPrd) {
				inf.KeyAct = K_SHORT_LEAVE;
				if (ledkey->handle[ledkey->Ledptr].KeyShortLeave != 0) { //单击松开
					ledkey->handle[ledkey->Ledptr].KeyShortLeave(inf);
					re = 1;
				}
				if (ledkey->keyLeaveCount[ledkey->Ledptr] > 0) { //双击
					inf.KeyAct = K_DOUBLE_CLICK;
					if (ledkey->handle[ledkey->Ledptr].KeyDoubleClick != 0) {
						ledkey->handle[ledkey->Ledptr].KeyDoubleClick(inf);
						re = 1;
					}
				}
				ledkey->keyLeaveCount[ledkey->Ledptr] = ledkey->DoubleClickItv;
			}
			//不管松开的时间
			inf.KeyAct = K_LEAVE;
			if (ledkey->handle[ledkey->Ledptr].KeyLeave != 0) {
				ledkey->handle[ledkey->Ledptr].KeyLeave(inf);
				re = 1;
			}
			ledkey->FlashDis = 0;
			ledkey->keyCount[ledkey->Ledptr] = 0;
			ledkey->keyCountRec[ledkey->Ledptr] = 0;
		} else { //按下
			if (ledkey->keyCount[ledkey->Ledptr] == 0) { //上跳
				ledkey->keyCount[ledkey->Ledptr]++;
				inf.KeyAct = K_CLICK;
				inf.KeyNo = ledkey->KeyPtr;
				if (ledkey->handle[ledkey->Ledptr].KeyClick != 0)
					ledkey->handle[ledkey->Ledptr].KeyClick(inf);
				re = 1;
			}

		}
	}
	ledkey->valuePre[ledkey->Ledptr] = ledkey->KeyPtr;
	if (ledkey->keyLeaveCount[ledkey->Ledptr] > 0)
		ledkey->keyLeaveCount[ledkey->Ledptr]--;
	//按下计数
	if (ledkey->KeyPtr != K_NULL) {
		ledkey->keyCount[ledkey->Ledptr]++;
		if (ledkey->keyCount[ledkey->Ledptr] == ledkey->LongKeyPrd) { //长按
			inf.KeyAct = K_LONGKEY;
			inf.KeyNo = ledkey->KeyPtr;
			if (ledkey->handle[ledkey->Ledptr].KeyLong != 0) {
				ledkey->handle[ledkey->Ledptr].KeyLong(inf);
				re = 1;
			}
		}
		if (ledkey->keyCount[ledkey->Ledptr] > ledkey->Level2AccPrd) { //二级加速
			if ((ledkey->keyCount[ledkey->Ledptr] - ledkey->Level2AccPrd
					- ledkey->keyCountRec[ledkey->Ledptr])
					> ledkey->Level2AccItv) { //一级加速周期到
				ledkey->keyCountRec[ledkey->Ledptr] =
						ledkey->keyCount[ledkey->Ledptr] - ledkey->Level2AccPrd;
				//连按
				inf.KeyAct = K_CONTINUE;
				inf.KeyNo = ledkey->KeyPtr;
				if (ledkey->handle[ledkey->Ledptr].KeyContin != 0) {
					ledkey->handle[ledkey->Ledptr].KeyContin(inf);
					ledkey->FlashDis = 1; //常亮不闪烁
					re = 1;
				}
			}
		} else if (ledkey->keyCount[ledkey->Ledptr] > ledkey->Level1AccPrd) { //一级加速
			if ((ledkey->keyCount[ledkey->Ledptr] - ledkey->Level1AccPrd
					- ledkey->keyCountRec[ledkey->Ledptr])
					> ledkey->Level1AccItv) { //一级加速周期到
				ledkey->keyCountRec[ledkey->Ledptr] =
						ledkey->keyCount[ledkey->Ledptr] - ledkey->Level1AccPrd;
				if (ledkey->keyCount[ledkey->Ledptr] + 1 > ledkey->Level2AccPrd)
					ledkey->keyCountRec[ledkey->Ledptr] = 0;
				//连按
				inf.KeyAct = K_CONTINUE;
				inf.KeyNo = ledkey->KeyPtr;
				if (ledkey->handle[ledkey->Ledptr].KeyContin != 0) {
					ledkey->handle[ledkey->Ledptr].KeyContin(inf);
					ledkey->FlashDis = 1; //常亮不闪烁
					re = 1;
				}
			}
		}
	} else {

	}

	if (ledkey->RefreDis == 0) {
		LedRefresh(ledkey);
	}

	if (re)
		return inf.KeyNo;
	else
		return K_NULL;
}

/*
 * 无显示
 * */
void BLANK_LED(LedKeyStru * ledkey) {
	int i;
	for (i = 0; i < 4; i++) {
		ledkey->LedBuf[i] = Dig_None;
	}
}

/*
 *把RunCtrl.LEDDAT
 *用BCD编码写到ledkey->LedBuf[]中
 *用BCD编码写到ledkey->LedBuf[]中 RunCtrl.LEDDAT
 * */
void DectoLed(LedKeyStru * ledkey, int32_t data, int16_t sign) {
	int16_t i, j;
	int32_t ldat;
	ldat = data;
	if (sign == 0)
		ldat = ldat & 0xffff;
	if (ldat < 0) {
		ldat = -ldat;
		j = 1;
	} else
		j = 0;

	if (ldat > 99999l)
		ldat = 99999l;
	else if (ldat < 0)
		ldat = 0;
	i = ldat / 10000;
	ledkey->LedBuf[BYT0] = CODE164[i];
	if (j == 1) {	//negative value
		if (i == 0)
			ledkey->LedBuf[BYT0] = Dig_Dash;
		else
			ledkey->LedBuf[BYT0] |= PointAdd;
	}
	ldat = ldat % 10000;
	i = ldat / 1000;
	ledkey->LedBuf[BYT1] = CODE164[i];
	ldat = ldat % 1000;
	i = ldat / 100;
	ledkey->LedBuf[BYT2] = CODE164[i];
	ldat = ldat % 100;
	i = ldat / 10;
	ledkey->LedBuf[BYT3] = CODE164[i];
	i = ldat % 10;
	ledkey->LedBuf[BYT4] = CODE164[i];

}
/*
 *把RunCtrl.LEDDAT
 *用BCD编码写到ledkey->LedBuf[]中
 * */
void HextoLed(LedKeyStru * ledkey, int32_t data) {
	int32_t ldat;
	ldat = data;
	ldat = ldat & 0xffff;
	ledkey->LedBuf[BYT4] = CODE164[ldat & 0xf];
	ldat = ldat >> 4;
	ledkey->LedBuf[BYT3] = CODE164[ldat & 0xf];
	ldat = ldat >> 4;
	ledkey->LedBuf[BYT2] = CODE164[ldat & 0xf];
	ldat = ldat >> 4;
	ledkey->LedBuf[BYT1] = CODE164[ldat & 0xf];
	ledkey->LedBuf[BYT0] = Dig_X;
}

#if 0
/*
 * 无光标的数据显示
 * 用函数void HexToLed(int16_t sign);
 *把RunCtrl.LEDDAT
 *用BCD编码写到ledkey->LedBuf[]中
 *去掉数据显示前面的0 ， 并加上小数点
 * */
void DecLED(LedKeyStru * ledkey, int32_t data, uint8_t sign, uint8_t dotpos,
		uint8_t HLFlag) {
	int32_t ldat;
	ledkey->DotPos = dotpos;
	if (HLFlag)
	ledkey->HLviewFlg = 1;
	else
	ledkey->HLviewFlg = 0;
	if (ledkey->HLviewFlg) {
		ldat = (int32_t) (data * 0.0001f);
		if (!ledkey->OnUpDwnSel)
		ldat = data - ldat * 10000;
	}

	DectoLed(ledkey, ldat, sign);
	/*去掉数据显示前面的0 ， 加小数点*/
	if (ledkey->DotPos == 0) {
		if (ledkey->LedBuf[BYT0] == Dig_0) {
			ledkey->LedBuf[BYT0] = Dig_X;
			if (ledkey->LedBuf[BYT1] == Dig_0) {
				ledkey->LedBuf[BYT1] = Dig_X;
				if (ledkey->LedBuf[BYT2] == Dig_0) {
					ledkey->LedBuf[BYT2] = Dig_X;
					if (ledkey->LedBuf[BYT3] == Dig_0)
					ledkey->LedBuf[BYT3] = Dig_X;
				}
			}
		}
	}
	if (ledkey->DotPos == 1) {
		if (ledkey->LedBuf[BYT0] == Dig_0) {
			ledkey->LedBuf[BYT0] = Dig_X;
			if (ledkey->LedBuf[BYT1] == Dig_0) {
				ledkey->LedBuf[BYT1] = Dig_X;
				if (ledkey->LedBuf[BYT2] == Dig_0)
				ledkey->LedBuf[BYT2] = Dig_X;
			}
		}
		ledkey->LedBuf[BYT3] |= PointAdd;
	}
	if (ledkey->DotPos == 2) {
		if (ledkey->LedBuf[BYT0] == Dig_0) {
			ledkey->LedBuf[BYT0] = Dig_X;
			if (ledkey->LedBuf[BYT1] == Dig_0)
			ledkey->LedBuf[BYT1] = Dig_X;
		}
		ledkey->LedBuf[BYT2] |= PointAdd;
	}
	if (ledkey->DotPos == 3) {
		if (ledkey->LedBuf[BYT0] == Dig_0)
		ledkey->LedBuf[BYT0] = Dig_X;
		ledkey->LedBuf[BYT1] |= PointAdd;
	}
	if (ledkey->DotPos == 4)
	ledkey->LedBuf[BYT0] |= PointAdd;
	//加上 HL字符 和 符号
	if (ledkey->HLviewFlg) {
		if (ledkey->OnUpDwnSel)
		ledkey->LedBuf[BYT0] = Dig_H;
		else
		ledkey->LedBuf[BYT0] = Dig_L;
		if (data < 0)
		ledkey->LedBuf[BYT0] |= PointAdd;
	}
	SetLedFlash(0);
}

#endif

/*
 * 十进制数据显示
 * 用函数void HexToLed(int16_t sign);
 * 去掉数据显示前面的0 ， 并加上小数点
 * */
void LDecLEDx(LedKeyStru * ledkey, int32_t data, DataShowPropStru prop) {
	uint8_t temp;
	int32_t tempdata;
	if (prop.HLFlag == 0) {
		tempdata = data;
		ledkey->ShiftValue = prop.ShiftValue;
		ledkey->DotPos = prop.dotpos;
		ledkey->HLviewFlg = prop.HLFlag;
		DectoLed(ledkey, tempdata, prop.sign);

		if (prop.flashFlag) { //光标位闪烁 ShiftValue
			/*
			 *根据小数点位置和当前光标点 ，熄灭左侧为0的LED数码管
			 * 也就是：同时在小数点和光标点左侧的数字0,不显示
			 * */
			if (ledkey->DotPos == 0) {
				if ((ledkey->LedBuf[BYT0] == Dig_0)
						&& (ledkey->ShiftValue < 4)) {
					ledkey->LedBuf[BYT0] = Dig_X;
					if ((ledkey->LedBuf[BYT1] == Dig_0)
							&& (ledkey->ShiftValue < 3)) {
						ledkey->LedBuf[BYT1] = Dig_X;
						if ((ledkey->LedBuf[BYT2] == Dig_0)
								&& (ledkey->ShiftValue < 2)) {
							ledkey->LedBuf[BYT2] = Dig_X;
							if ((ledkey->LedBuf[BYT3] == Dig_0)
									&& (ledkey->ShiftValue < 1))
								ledkey->LedBuf[BYT3] = Dig_X;
						}
					}
				}
			}
			if (ledkey->DotPos == 1) {
				if ((ledkey->LedBuf[BYT0] == Dig_0)
						&& (ledkey->ShiftValue < 4)) {
					ledkey->LedBuf[BYT0] = Dig_X;
					if ((ledkey->LedBuf[BYT1] == Dig_0)
							&& (ledkey->ShiftValue < 3)) {
						ledkey->LedBuf[BYT1] = Dig_X;
						if ((ledkey->LedBuf[BYT2] == Dig_0)
								&& (ledkey->ShiftValue < 2))
							ledkey->LedBuf[BYT2] = Dig_X;
					}
				}
				ledkey->LedBuf[BYT3] |= PointAdd;
			}
			if (ledkey->DotPos == 2) {
				if ((ledkey->LedBuf[BYT0] == Dig_0)
						&& (ledkey->ShiftValue < 4)) {
					ledkey->LedBuf[BYT0] = Dig_X;
					if ((ledkey->LedBuf[BYT1] == Dig_0)
							&& (ledkey->ShiftValue < 3))
						ledkey->LedBuf[BYT1] = Dig_X;
				}
				ledkey->LedBuf[BYT2] |= PointAdd;
			}
			if (ledkey->DotPos == 3) {
				if ((ledkey->LedBuf[BYT0] == Dig_0) && (ledkey->ShiftValue < 4))
					ledkey->LedBuf[BYT0] = Dig_X;
				ledkey->LedBuf[BYT1] |= PointAdd;
			}
			if (ledkey->DotPos == 4)
				ledkey->LedBuf[BYT0] |= PointAdd;

			temp = BYT4 - ledkey->ShiftValue;
			if (temp > 5)
				temp = 5;
			SetLedFlash(LED_bit_mask[temp]);
		} else { //无需闪烁
			/*去掉数据显示前面的0 ， 加小数点*/
			if (ledkey->DotPos == 0) {
				if (ledkey->LedBuf[BYT0] == Dig_0) {
					ledkey->LedBuf[BYT0] = Dig_X;
					if (ledkey->LedBuf[BYT1] == Dig_0) {
						ledkey->LedBuf[BYT1] = Dig_X;
						if (ledkey->LedBuf[BYT2] == Dig_0) {
							ledkey->LedBuf[BYT2] = Dig_X;
							if (ledkey->LedBuf[BYT3] == Dig_0)
								ledkey->LedBuf[BYT3] = Dig_X;
						}
					}
				}
			}
			if (ledkey->DotPos == 1) {
				if (ledkey->LedBuf[BYT0] == Dig_0) {
					ledkey->LedBuf[BYT0] = Dig_X;
					if (ledkey->LedBuf[BYT1] == Dig_0) {
						ledkey->LedBuf[BYT1] = Dig_X;
						if (ledkey->LedBuf[BYT2] == Dig_0)
							ledkey->LedBuf[BYT2] = Dig_X;
					}
				}
				ledkey->LedBuf[BYT3] |= PointAdd;
			}
			if (ledkey->DotPos == 2) {
				if (ledkey->LedBuf[BYT0] == Dig_0) {
					ledkey->LedBuf[BYT0] = Dig_X;
					if (ledkey->LedBuf[BYT1] == Dig_0)
						ledkey->LedBuf[BYT1] = Dig_X;
				}
				ledkey->LedBuf[BYT2] |= PointAdd;
			}
			if (ledkey->DotPos == 3) {
				if (ledkey->LedBuf[BYT0] == Dig_0)
					ledkey->LedBuf[BYT0] = Dig_X;
				ledkey->LedBuf[BYT1] |= PointAdd;
			}
			if (ledkey->DotPos == 4)
				ledkey->LedBuf[BYT0] |= PointAdd;
			SetLedFlash(0);
		}
	}
}

/*
 * 十进制数据显示 高低位显示
 * 用BCD编码写到ledkey->LedBuf[]中
 * 去掉数据显示前面的0 ， 并加上小数点
 * */
void HLDecLEDx(LedKeyStru * ledkey, int32_t data, DataShowPropStru prop) {
	uint8_t temp;
	uint32_t tempdata, absdata;
	uint8_t flag = 0;
	if (prop.HLFlag == 1) {
		if (prop.sign && (data < 0)) {
			absdata = -data;
		} else {
			absdata = data;
		}
		if (absdata > 99999999)
			tempdata = 99999999;
		else
			tempdata = absdata;
		ledkey->HLviewFlg = prop.HLFlag;
		if ((ledkey->OnUpDwnSel & 1) == 0) { //L
			tempdata = tempdata % 10000;
			if (prop.ShiftValue < 4)
				ledkey->ShiftValue = prop.ShiftValue;
			else
				ledkey->ShiftValue = 0;
			if (prop.dotpos < 4)
				ledkey->DotPos = prop.dotpos;
			else
				ledkey->DotPos = 0;
		} else { //H
			tempdata = tempdata / 10000;
			if (prop.ShiftValue >= 4)
				ledkey->ShiftValue = prop.ShiftValue - 4;
			else
				ledkey->ShiftValue = 0;
			if (prop.dotpos >= 4)
				ledkey->DotPos = prop.dotpos - 4;
			else
				ledkey->DotPos = 0;
		}
		DectoLed(ledkey, tempdata, prop.sign);
		if ((ledkey->OnUpDwnSel & 1) == 0) { //L
			ledkey->LedBuf[BYT0] = Dig_L;
			if (prop.sign && (data < 0))
				ledkey->LedBuf[BYT0] |= PointAdd;
			if (absdata < 9999)
				flag = 1;
		} else {
			ledkey->LedBuf[BYT0] = Dig_H;
			if (prop.sign && (data < 0))
				ledkey->LedBuf[BYT0] |= PointAdd;
			if (prop.dotpos == 4)
				ledkey->LedBuf[BYT4] |= PointAdd;
			flag = 1;
		}

		if (flag == 1) { //去掉显示前面的0 , 如果是负数，加上负号
			if (prop.flashFlag) { //光标位闪烁 ShiftValue
				/*
				 *根据小数点位置和当前光标点 ，熄灭左侧为0的LED数码管
				 * 也就是：同时在小数点和光标点左侧的数字0,不显示
				 * */
				if (ledkey->DotPos == 0) {
					if ((ledkey->LedBuf[BYT1] == Dig_0)
							&& (ledkey->ShiftValue < 3)) {
						ledkey->LedBuf[BYT1] = Dig_X;
						if ((ledkey->LedBuf[BYT2] == Dig_0)
								&& (ledkey->ShiftValue < 2)) {
							ledkey->LedBuf[BYT2] = Dig_X;
							if ((ledkey->LedBuf[BYT3] == Dig_0)
									&& (ledkey->ShiftValue < 1))
								ledkey->LedBuf[BYT3] = Dig_X;
						}
					}
				}
				if (ledkey->DotPos == 1) {
					if ((ledkey->LedBuf[BYT1] == Dig_0)
							&& (ledkey->ShiftValue < 3)) {
						ledkey->LedBuf[BYT1] = Dig_X;
						if ((ledkey->LedBuf[BYT2] == Dig_0)
								&& (ledkey->ShiftValue < 2))
							ledkey->LedBuf[BYT2] = Dig_X;
					}
				}
				if (ledkey->DotPos == 2) {
					if ((ledkey->LedBuf[BYT1] == Dig_0)
							&& (ledkey->ShiftValue < 3))
						ledkey->LedBuf[BYT1] = Dig_X;
					ledkey->LedBuf[BYT2] |= PointAdd;
				}
			} else { //无需闪烁
				/*去掉数据显示前面的0 ， 加小数点*/
				if (ledkey->DotPos == 0) {
					if (ledkey->LedBuf[BYT1] == Dig_0) {
						ledkey->LedBuf[BYT1] = Dig_X;
						if (ledkey->LedBuf[BYT2] == Dig_0) {
							ledkey->LedBuf[BYT2] = Dig_X;
							if (ledkey->LedBuf[BYT3] == Dig_0)
								ledkey->LedBuf[BYT3] = Dig_X;
						}
					}
				}
				if (ledkey->DotPos == 1) {

					if (ledkey->LedBuf[BYT1] == Dig_0) {
						ledkey->LedBuf[BYT1] = Dig_X;
						if (ledkey->LedBuf[BYT2] == Dig_0)
							ledkey->LedBuf[BYT2] = Dig_X;
					}
				}
				if (ledkey->DotPos == 2) {

					if (ledkey->LedBuf[BYT1] == Dig_0)
						ledkey->LedBuf[BYT1] = Dig_X;

					ledkey->LedBuf[BYT2] |= PointAdd;
				}
				if (ledkey->DotPos == 3) {
					ledkey->LedBuf[BYT1] |= PointAdd;
				}
				if (ledkey->DotPos == 4)
					ledkey->LedBuf[BYT0] |= PointAdd;
			}
			if (prop.sign && (data < 0)) { //加上负号
				for (temp = 4; temp >= 1; temp--) {
					if (ledkey->LedBuf[temp] == Dig_X) {
						ledkey->LedBuf[temp] = Dig_Dash;
						break;
					}
				}
			}
		}
		if (prop.flashFlag) { //闪烁位置
			temp = BYT4 - ledkey->ShiftValue;
			if (temp > 5)
				temp = 5;
			SetLedFlash(LED_bit_mask[temp]);
		} else {
			SetLedFlash(0);
		}
		if ((ledkey->DotPos > 0) && (ledkey->DotPos < 4)) { //小数点位置
			temp = BYT4 - ledkey->DotPos;
			if (temp > 4)
				temp = 4;
			ledkey->LedBuf[temp] |= PointAdd;
		}

	}
}

/*
 * 十进制数据显示 高中低位
 * 用函数void HexToLed(int16_t sign);
 * 把RunCtrl.LEDDAT
 * 用BCD编码写到ledkey->LedBuf[]中
 * 去掉数据显示前面的0 ， 并加上小数点
 * */
void HMLDecLEDx(LedKeyStru * ledkey, int32_t data, DataShowPropStru prop) {
	uint8_t temp;
	int32_t tempdata, absdata;
	uint8_t flag = 0;
	if (prop.HLFlag == 2) {
		if (prop.sign && (data < 0)) {
			tempdata = -data;
			absdata = -data;
		} else {
			tempdata = data;
			absdata = data;
		}
		ledkey->HLviewFlg = prop.HLFlag;
		if (ledkey->OnUpDwnSel == 0) { //L
			tempdata = tempdata % 10000;
			if (prop.ShiftValue < 4)
				ledkey->ShiftValue = prop.ShiftValue;
			else
				ledkey->ShiftValue = 0;
			if (prop.dotpos < 4)
				ledkey->DotPos = prop.dotpos;
			else
				ledkey->DotPos = 0;
		} else if (ledkey->OnUpDwnSel == 1) { //M
			tempdata = tempdata / 10000;
			tempdata = tempdata % 10000;
			if ((prop.ShiftValue >= 4) && (prop.ShiftValue < 8))
				ledkey->ShiftValue = prop.ShiftValue - 4;
			else
				ledkey->ShiftValue = 0;
			if ((prop.dotpos >= 4) && (prop.dotpos < 8))
				ledkey->DotPos = prop.dotpos - 4;
			else
				ledkey->DotPos = 0;
		} else { //H
			tempdata = tempdata / 100000000;
			if (prop.ShiftValue >= 8)
				ledkey->ShiftValue = prop.ShiftValue - 8;
			else
				ledkey->ShiftValue = 0;
			if (prop.dotpos >= 8)
				ledkey->DotPos = prop.dotpos - 8;
			else
				ledkey->DotPos = 0;
		}
		DectoLed(ledkey, tempdata, prop.sign);
		if (ledkey->OnUpDwnSel == 0) { //L
			ledkey->LedBuf[BYT0] = Dig_L;
			if (prop.sign && (data < 0))
				ledkey->LedBuf[BYT0] |= PointAdd;
			if (absdata < 9999)
				flag = 1;
		} else if (ledkey->OnUpDwnSel == 1) { //M
			ledkey->LedBuf[BYT0] = Dig_n;
			if (prop.sign && (data < 0))
				ledkey->LedBuf[BYT0] |= PointAdd;
			if (prop.dotpos == 4)
				ledkey->LedBuf[BYT4] |= PointAdd;
			if (absdata < 99999999)
				flag = 1;
		} else { //H
			ledkey->LedBuf[BYT0] = Dig_H;
			if (prop.sign && (data < 0))
				ledkey->LedBuf[BYT0] |= PointAdd;
			if (prop.dotpos == 8)
				ledkey->LedBuf[BYT4] |= PointAdd;
			flag = 1;
		}

		if (flag) { //去掉数值前面的0
			if (prop.flashFlag) { //光标位闪烁 ShiftValue
				/*
				 *根据小数点位置和当前光标点 ，熄灭左侧为0的LED数码管
				 * 也就是：同时在小数点和光标点左侧的数字0,不显示
				 * */
				if (ledkey->DotPos == 0) {
					if ((ledkey->LedBuf[BYT1] == Dig_0)
							&& (ledkey->ShiftValue < 3)) {
						ledkey->LedBuf[BYT1] = Dig_X;
						if ((ledkey->LedBuf[BYT2] == Dig_0)
								&& (ledkey->ShiftValue < 2)) {
							ledkey->LedBuf[BYT2] = Dig_X;
							if ((ledkey->LedBuf[BYT3] == Dig_0)
									&& (ledkey->ShiftValue < 1))
								ledkey->LedBuf[BYT3] = Dig_X;
						}
					}

				}
				if (ledkey->DotPos == 1) {
					if ((ledkey->LedBuf[BYT1] == Dig_0)
							&& (ledkey->ShiftValue < 3)) {
						ledkey->LedBuf[BYT1] = Dig_X;
						if ((ledkey->LedBuf[BYT2] == Dig_0)
								&& (ledkey->ShiftValue < 2))
							ledkey->LedBuf[BYT2] = Dig_X;
					}
				}
				if (ledkey->DotPos == 2) {
					if ((ledkey->LedBuf[BYT1] == Dig_0)
							&& (ledkey->ShiftValue < 3))
						ledkey->LedBuf[BYT1] = Dig_X;
				}
			} else { //无需闪烁
				/*去掉数据显示前面的0 ， 加小数点*/
				if (ledkey->DotPos == 0) {
					if (ledkey->LedBuf[BYT1] == Dig_0) {
						ledkey->LedBuf[BYT1] = Dig_X;
						if (ledkey->LedBuf[BYT2] == Dig_0) {
							ledkey->LedBuf[BYT2] = Dig_X;
							if (ledkey->LedBuf[BYT3] == Dig_0)
								ledkey->LedBuf[BYT3] = Dig_X;
						}
					}
				}
				if (ledkey->DotPos == 1) {
					if (ledkey->LedBuf[BYT1] == Dig_0) {
						ledkey->LedBuf[BYT1] = Dig_X;
						if (ledkey->LedBuf[BYT2] == Dig_0)
							ledkey->LedBuf[BYT2] = Dig_X;
					}
				}
				if (ledkey->DotPos == 2) {
					if (ledkey->LedBuf[BYT1] == Dig_0)
						ledkey->LedBuf[BYT1] = Dig_X;
				}
			}
			if (prop.sign && (data < 0)) { //加上负号
				for (temp = 4; temp >= 1; temp--) {
					if (ledkey->LedBuf[temp] == Dig_X) {
						ledkey->LedBuf[temp] = Dig_Dash;
						break;
					}

				}
			}
		}
		if (prop.flashFlag) { //闪烁位置
			temp = BYT4 - ledkey->ShiftValue;
			if (temp > 5)
				temp = 5;
			SetLedFlash(LED_bit_mask[temp]);
		} else {
			SetLedFlash(0);
		}
		if ((ledkey->DotPos > 0) && (ledkey->DotPos < 4)) { //小数点位置
			temp = BYT4 - ledkey->DotPos;
			if (temp > 4)
				temp = 4;
			ledkey->LedBuf[temp] |= PointAdd;
		}
	}
}

/*
 * 十进制数据显示
 * 用函数void HexToLed(int16_t sign);
 * 把RunCtrl.LEDDAT
 * 用BCD编码写到ledkey->LedBuf[]中
 * 去掉数据显示前面的0 ， 并加上小数点
 * */
void DecLEDx(LedKeyStru * ledkey, int32_t data, DataShowPropStru prop) {
	if (prop.HLFlag == 0) { //L
		LDecLEDx(ledkey, data, prop);
	} else if (prop.HLFlag == 1) { //HL
		HLDecLEDx(ledkey, data, prop);
	} else { //HML
		HMLDecLEDx(ledkey, data, prop);
	}
}

/*
 * 有光标的数据显示
 * 用函数void HexToLed(int16_t sign);
 * 把RunCtrl.LEDDAT
 * 用BCD编码写到ledkey->LedBuf[]中
 * 去掉数据显示前面的0 ， 并加上小数点
 * */
void HexLEDx(LedKeyStru * ledkey, int32_t data, uint8_t sign, uint8_t dotpos,
		uint8_t ShiftValue) {
	uint8_t temp = 0;
	ledkey->DotPos = dotpos;
	ledkey->ShiftValue = ShiftValue;
	HextoLed(ledkey, data);
	/*
	 *根据小数点位置和当前光标点 ，熄灭左侧为0的LED数码管
	 * 也就是：同时在小数点和光标点左侧的数字0,不显示
	 * */
	if (ledkey->DotPos == 0) {
		if ((ledkey->LedBuf[BYT0] == Dig_0) && (ledkey->ShiftValue < 4)) {
			ledkey->LedBuf[BYT0] = Dig_X;
			if ((ledkey->LedBuf[BYT1] == Dig_0) && (ledkey->ShiftValue < 3)) {
				ledkey->LedBuf[BYT1] = Dig_X;
				if ((ledkey->LedBuf[BYT2] == Dig_0)
						&& (ledkey->ShiftValue < 2)) {
					ledkey->LedBuf[BYT2] = Dig_X;
					if ((ledkey->LedBuf[BYT3] == Dig_0)
							&& (ledkey->ShiftValue < 1))
						ledkey->LedBuf[BYT3] = Dig_X;
				}
			}
		}
	}
	if (ledkey->DotPos == 1) {
		if ((ledkey->LedBuf[BYT0] == Dig_0) && (ledkey->ShiftValue < 4)) {
			ledkey->LedBuf[BYT0] = Dig_X;
			if ((ledkey->LedBuf[BYT1] == Dig_0) && (ledkey->ShiftValue < 3)) {
				ledkey->LedBuf[BYT1] = Dig_X;
				if ((ledkey->LedBuf[BYT2] == Dig_0) && (ledkey->ShiftValue < 2))
					ledkey->LedBuf[BYT2] = Dig_X;
			}
		}
		ledkey->LedBuf[BYT3] |= PointAdd;
	}
	if (ledkey->DotPos == 2) {
		if ((ledkey->LedBuf[BYT0] == Dig_0) && (ledkey->ShiftValue < 4)) {
			ledkey->LedBuf[BYT0] = Dig_X;
			if ((ledkey->LedBuf[BYT1] == Dig_0) && (ledkey->ShiftValue < 3))
				ledkey->LedBuf[BYT1] = Dig_X;
		}
		ledkey->LedBuf[BYT2] |= PointAdd;
	}
	if (ledkey->DotPos == 3) {
		if ((ledkey->LedBuf[BYT0] == Dig_0) && (ledkey->ShiftValue < 4))
			ledkey->LedBuf[BYT0] = Dig_X;
		ledkey->LedBuf[BYT1] |= PointAdd;
	}
	if (ledkey->DotPos == 4)
		ledkey->LedBuf[BYT0] |= PointAdd;
	temp = BYT4 - ledkey->ShiftValue;
	if (temp > 5)
		temp = 5;
	SetLedFlash(LED_bit_mask[temp]);
}

