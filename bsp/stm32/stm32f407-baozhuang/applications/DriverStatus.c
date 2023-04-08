/*
 * DriverStatus.c
 *
 *  Created on: 2019.4.28
 *      Author: yubq
 */
#include "main.h"
#include "CanToWifi.h"
#include "rtthread.h"
#include "rtdevice.h"

#define SW_VERSION 0x01
uint32_t generateDate(void);

int32_t GetScopeData(int32_t addr)
{
    switch (addr)
    {
    case DrivSt_Status:
        return 0;
        break;
    case DrivSt_SpdRef:
        return 0; //
        break;
    case DrivSt_SpdFbd:
        return (int32_t)0; //
        break;
    case DrivSt_DcVolt:
        return (int32_t)0;
        break;
    case DrivSt_Torq:
        return 0;
        break;
    case DrivSt_EncPosi:
        return (int32_t)0; //
        break;
    case DrivSt_Reverse1:
        return (int32_t)0; //;
        break;
    case DrivSt_Reverse2:
        return 0;//GetCodeMaxRunTime(CodeSeg_MISRRunt); //
        break;
    case DrivSt_EleAngle:
        return (int32_t)0; //;
        break;
    case DrivSt_PhaseACur:
#ifndef show_tatbtc
        return 0;
#else
        return ta;
#endif
        break;
    case DrivSt_PhaseBCur:
#ifndef show_tatbtc
        return 0;
#else
        return tb;
#endif
        break;
    case DrivSt_PhaseCCur:
#ifndef show_tatbtc
        return 0;
#else
        return tc;
#endif
        break;
    case DrivSt_axisDCurRef:
        return 0;
        break;
    case DrivSt_axisDCurFbd:
        return 0;
        break;
    case DrivSt_axisQCurRef:
        return 0;
        break;
    case DrivSt_axisQCurFbd:
        return 0;
        break;
    case DrivSt_axisDVoltRef:
        return (int16_t) 0;
    case DrivSt_axisQVoltRef:
        return (int16_t) 0;
        break;
    case DrivSt_EcCnt:
        return 0;//sysStatus.encoderErrCnt;   //RunCtrl.RunCom;
        break;
    case DrivSt_RunMode:
        return 0;
        break;
    case DrivSt_MInput:
        return 0;
        break;
    case DrivSt_MOutput:
        return 0;
        break;
    case DrivSt_Rev32_1: //32bit
        return  0;//getSzData(18);//
        break;
    case DrivSt_Rev32_2: //32bit
        return 0;//getSzData(19);
        break;
    case DrivSt_CANErrRecCnt:
        return 0;
        break;
    case DrivSt_CANErrTraCnt:
        return 0;
        break;
    case DrivSt_Rev32_3:
        return 0;//;//getSzData(0);
        break;
    case DrivSt_Rev32_4:
        return 0;//CanPosiLoop.CANPosiGD;
        break;
    case DrivSt_Rev32_5:
        return 0;//getSpdOb()->posiSum_m;//MotorModeCtrl.TorqGD;
        break;
    case DrivSt_Rev32_6:
        return 0; //CanPosiLoop.CAN_PosiGD_Inc;
        break;
    case DrivSt_GenYear:
        return generateDate() / 10000;
        break;
    case DrivSt_GenMonDay:
        return generateDate() % 10000;
        break;
    case DriSt_SoftVersion:
        return SW_VERSION;
    default:
        break;
    }
    return 0;
}

/*
 *显示软件编译的时间
 */
uint32_t generateDate()
{
    static char datestr[20] = __DATE__;  //获取编译日期
    static uint16_t year = 0, mon = 0, day = 0, firstFlag = 0;
    static uint32_t ymd = 0;
    uint16_t temp, i, j;
    if (firstFlag == 0)
    {
        firstFlag = 1;
        j = 0;
        for (i = 0; i < 12; i++)
        {
            if ((datestr[i] == ' ') && (j == 0))  /*找第1个空格后的日期*/
            {
                j = 1;
                temp = datestr[i + 1] - '0';
                if ((temp <= 9) || (datestr[i + 1] == ' '))
                {
                    if (temp <= 9)
                    {
                        day = temp * 10;
                    }
                    temp = datestr[i + 2] - '0';
                    if (temp <= 9)
                        day = day + temp;
                    else
                        day = 0;
                }
                else
                {
                    day = 0;
                }
                if (datestr[i + 1] == ' ') //如果连续两个空格，跳过
                    i++;
            }
            else if ((datestr[i] == ' ') && (j == 1))    /*找第2个空格后的年份*/
            {
                for (int _ = 0; _ < 4; _++)
                {
                    temp = datestr[i + _ + 1] - '0';
                    year = year * 10;
                    if (temp <= 9)
                    {
                        year += temp;
                    }
                }
                break;
            }
        }

        if ((datestr[0] == 'J') && (datestr[1] == 'a'))   //Jan
        {
            mon = 1;
        }
        else if ((datestr[0] == 'F') && (datestr[1] == 'e'))     //
        {
            mon = 2;
        }
        else if ((datestr[0] == 'M') && (datestr[1] == 'a'))
        {
            mon = 3;
        }
        else if ((datestr[0] == 'A') && (datestr[1] == 'p'))
        {
            mon = 4;
        }
        else if ((datestr[0] == 'M') && (datestr[1] == 'a'))
        {
            mon = 5;
        }
        else if ((datestr[0] == 'J') && (datestr[1] == 'u')
                 && (datestr[2] == 'n'))
        {
            mon = 6;
        }
        else if ((datestr[0] == 'J') && (datestr[1] == 'u')
                 && (datestr[2] == 'l'))
        {
            mon = 7;
        }
        else if ((datestr[0] == 'A') && (datestr[1] == 'u'))
        {
            mon = 8;
        }
        else if ((datestr[0] == 'S') && (datestr[1] == 'e'))
        {
            mon = 9;
        }
        else if ((datestr[0] == 'O') && (datestr[1] == 'c'))
        {
            mon = 10;
        }
        else if ((datestr[0] == 'N') && (datestr[1] == 'o'))
        {
            mon = 11;
        }
        else if ((datestr[0] == 'D') && (datestr[1] == 'e'))
        {
            mon = 12;
        }
        else
        {
            mon = 0;
        }

        if (year != 0)
        {
            ymd = year * 10000 + mon * 100 + day;
        }
        else
        {
            ymd = 0;
        }
    }

    return ymd;
}

#if 0
#define CAL_SPEED_FREQ    100 //1000Hz
#define SPEED_FILTER_MS   2  //2ms滤波
void SpeedMeasure(int32_t QepValue)
{
    int32_t deta_angle;
    static float speed_coe = 0.5f, speed_filter_coe = 0.5f;
    static int32_t lastPosi = 0;
    static uint8_t sp_window_full = 0 ;
    int32_t encoderPosi = QepValue;
    sysStatus.encPosi = encoderPosi;

    /************************Speed calculation********************************/
    //speed measure
    if (sp_window_full)
    {
        sysStatus.servoPosi += (int32_t)(encoderPosi - lastPosi);
        deta_angle = (int32_t)(encoderPosi - lastPosi);  //
        sysStatus.servoSpeed = deta_angle * speed_coe
                               + sysStatus.servoSpeed * speed_filter_coe;
    }
    else
    {
        sysStatus.servoSpeed = 0;
        sysStatus.servoPosi = 0;
        sp_window_full = 1;
        speed_coe = (CAL_SPEED_FREQ * 60.0 / 10000); //pulse / tick -> rpm
        speed_filter_coe = SPEED_FILTER_MS
                           / ((1000.0 / CAL_SPEED_FREQ) + SPEED_FILTER_MS);
    }
    lastPosi = encoderPosi;
}

static rt_timer_t timer_speed_1ms;

/* 定时器 2 超时函数 */
#include "drv_gpio.h"
static void timeout_speedcal(void *parameter)
{
    if (sysStatus.bootTimer < 0xffffff) sysStatus.bootTimer += 10;
    sysStatus.msTimer += 10;
    SpeedMeasure(ENC_GetPositionValue(ENC1));
}

int timer_sample(void)
{
    timer_speed_1ms = rt_timer_create("timer1", timeout_speedcal,
                                      RT_NULL, 10,
                                      RT_TIMER_FLAG_PERIODIC);


    if (timer_speed_1ms != RT_NULL) rt_timer_start(timer_speed_1ms);

    return 0;
}
INIT_APP_EXPORT(timer_sample);
#endif
