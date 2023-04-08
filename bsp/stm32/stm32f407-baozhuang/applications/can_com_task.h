/*
 * can_com_task.h
 *
 *  Created on: 2020年5月20日
 *      Author: yu953
 */

#ifndef SRC_USER_CAN_COM_TASK_H_
#define SRC_USER_CAN_COM_TASK_H_
#include "stdint.h"
#include "rtthread.h"
#if defined(__CC_ARM) || defined(__GNUC__)
    #pragma anon_unions
#endif
typedef struct _can_frame_struct
{
    uint8_t dlc;
    union
    {
        uint8_t data[8];
        uint16_t data16[4];
        struct
        {
            uint32_t l32data;
            uint32_t h32data;
        };
    };
    union
    {
        uint32_t id32;
        struct
        {
            uint32_t id: 29;
            uint32_t res: 2;
            uint32_t ext: 1;
        };
        struct
        {
            uint8_t dev_id;
            uint8_t main_cmd;
            uint8_t sub_cmd;
            uint8_t iap_cmd: 5;
            uint8_t res1: 3;
        };
    };
} CANFrame;

#define CAN_REC_THREAD_FALG 0x01

#define CAN_EMG_TRANSMIT_THREAD_FALG 0x02
#define CAN_SENSOR_REC_THREAD_FLAG 0x04

#define CAN_NORMAL_TRANSMIT_MB 11   /*CAN发送邮箱*/
#define CAN_EMG_TRANSMIT_MB 10      /*CAN发送邮箱*/

#define MAIN_ENGINE_ADDR 0x01 /*主机地址*/
#define DEVICE_ADDR 0x59
#define CAN_ACK 0x80
#define DEVICE_ACK_ADDR (DEVICE_ADDR | CAN_ACK)

enum StepperIndex
{
    StepperIndex_0 = 0,
    StepperIndex_1 = 1,
    StepperIndex_2 = 2,
    StepperIndex_3 = 3,
    StepperIndex_4 = 4,
    StepperIndex_5 = 5,
    StepperIndex_6 = 6,
    StepperIndex_7 = 7,
    StepperIndex_8 = 8,
    StepperIndex_9 = 9,
    StepperIndex_10 = 10,
    StepperIndex_11 = 11,
    StepperIndex_12 = 12,
    StepperIndex_13 = 13,
};

enum Actuator
{
    ActNull = 0,
    STMotor_MainHook = 0x11,
    STMotor_ForeDensi = 0x12,
    STMotor_RightHook = 0x13,
    STMotor_BackDensi = 0x14,
    STMotor_Scissor = 0x15,
    STMotor_Rubber = 0x16,
    STMotor_DoubleColor = 0x17,
    STMotor_X2 = 0x18,
    STMotor_X3 = 0x19,
    STMotor_X4 = 0x1A,
    STMotor_X5 = 0x1B,
    STMotor_X6 = 0x1C,
    STMotor_NeedleDrum = 0x1D,      /*针筒*/
    STMotor_Fork = 0x1E,
    /*电磁阀*/
    EleMag_RubberScissor = 0x41, /*橡筋剪刀*/
    EleMag_XYarnScissor = 0x42, /*X纱剪刀*/
    EleMag_X2 = 0x43,
    EleMag_X3 = 0x44,
    EleMag_AddOil = 0x45, /*加油*/
    Elemag_AlarmLight = 0x46, /*报警灯*/
    EleMag_ColorYarn1 = 0x47, /*色纱1*/
    EleMag_ColoeYarn2 = 0x48, /*色纱2*/
    EleMag_XYarn = 0x49,    /*X纱*/
    EleMag_MainStop1 = 0x4A, /*主停1*/
    EleMag_RubberYarn = 0x4B, /*橡筋纱*/
    EleMag_U3 = 0x4C, /*U3*/
    EleMag_MainStop2 = 0x4D, /*主停2*/
    EleMag_Color1Scissor = 0x4E, /*色纱1剪刀*/
    EleMag_Color2Scissor = 0x4F, /*色纱2剪刀*/
    EleMag_AirValve = 0x50, /*气阀*/

};

enum EleMagCmd
{
    EleMag_Off = 0,
    EleMag_On = 1,
    EleMag_OnWithTime = 2
};


enum STMotorCmd
{
    STMotorCmd_Reset = 0,
    STMotorCmd_Run = 1,         /*动作，绝对位置+频率*/
    STMotorCmd_Stop = 2,
    STMotorCmd_Forward = 3,     /*向前动作，增量位置+频率*/
    STMotorCmd_Backward = 4,    /*向后动作，增量位置+频率*/
    STMotorCmd_ForwardToAOrigin = 5,   /*到传感器位置A,Origin 如果当前已经在传感器位置，则不用运行*/
    STMotorCmd_ForwardToB = 6,          /*到传感器位置B,B point 如果当前已经在传感器位置，则不用运行*/
    STMotorCmd_ForwardToC = 7,
    STMotorCmd_BackwardToAOrigin = 8,   /*到传感器位置A,Origin 如果当前已经在传感器位置，则不用运行*/
    STMotorCmd_BackwardToB = 9,          /*到传感器位置B,B point 如果当前已经在传感器位置，则不用运行*/
    STMotorCmd_BackwardToC = 0x0A,
    STMotorCmd_ForwardToAOriginNext = 0x0B,   /*到传感器位置A,Origin , 如果当前已经在传感器位置，继续走到下一个传感器位置*/
    STMotorCmd_ForwardToBNext = 0x0C,          /*到传感器位置B,B point 如果当前已经在传感器位置，继续走到下一个传感器位置*/
    STMotorCmd_ForwardToCNext = 0x0D,
    STMotorCmd_BackwardToAOriginNext = 0x0E,   /*到传感器位置A,Origin 如果当前已经在传感器位置，继续走到下一个传感器位置*/
    STMotorCmd_BackwardToBNext = 0x0F,          /*到传感器位置B,B point 如果当前已经在传感器位置，继续走到下一个传感器位置*/
    STMotorCmd_BackwardToCNext = 0x10,
    STMotorCmd_ResetForward = 0x11,
    STMotorCmd_ResetBackward = 0x12,
    STMotorCmd_ForwardToAOriginComp = 0x13,
    STMotorCmd_BackwardToAOriginComp = 0x14,
    STMotorCmd_ForwardToAOriginNextComp = 0x15,
    STMotorCmd_BackwardToAOriginNextComp = 0x16,
    STMotorCmd_ForwardToPosiAfterA = 0x17,
    STMotorCmd_ForwardToPosiAfterANext = 0x18,
    STMotorCmd_BackwardToPosiAfterA = 0x19,
    STMotorCmd_BackwardToPosiAfterANext = 0x1A,
    STMotorCmd_ClearNRun = 0x1B,
    STMotorCmd_ForwardResetToPosiAfterANext = 0x1C,  /*复位到原点A后的X位置 到达X位置后位置清零*/
    STMotorCmd_BackwardResetToPosiAfterANext = 0x1D, /*反向复位到原点A后的X位置*/

    STMotorCmd_SetPosi = 0x50,
    STMotorCmd_SetOffVref = 0x51,
    STMotorCmd_SetOnVref = 0x52,
    STMotorCmd_SetSubdiv = 0x53,
    STMotorCmd_SetRstDir = 0x54,
    STMotorCmd_SetRunDir = 0x55,
    STMotorCmd_SetCompPulse = 0x56, /*设置复位补偿脉冲*/
    STMotorCmd_SetACompPulse = 0x57, /*设置A补偿脉冲*/
    STMotorCmd_SetBCompPulse = 0x58, /*设置B补偿脉冲*/
    STMotorCmd_SetCCompPulse = 0x59, /*设置C补偿脉冲*/
    STMotorCmd_SetRstMaxPulse = 0x5A,/*设置复位时最大运行脉冲*/
};



#define STEPPER_CAN_MAIN_CMD_BROC 0x10 //步进电机
#define STEPPER_CAN_MAIN_CMD_MIN 0x11 //步进电机编号空间
#define STEPPER_CAN_MAIN_CMD_MAX 0x3f //步进电机编号空间

#define ELEMAG_OUTPUT_MAIN_CMD_BROC 0x40   //电磁铁输出
#define ELEMAG_OUTPUT_MAIN_CMD_MIN 0x41   //电磁铁输出
#define ELEMAG_OUTPUT_MAIN_CMD_MAX 0x6f   //电磁铁输出

#define OTHERS_OUTPUT_MAIN_CMD_BROC 0x70   //其他输出
#define OTHERS_OUTPUT_MAIN_CMD_MIN 0x71   //其他输出
#define OTHERS_OUTPUT_MAIN_CMD_MAX 0x91   //其他输出

#define SENSOR_INPUT_MAIN_CMD_BROC 0xA0    //接近开关传感器输入
#define ADC_INPUT_MIAN_CMD_BROC 0xA1        //ADC 输入
#define HW_VERSION_READ_CMD 0xA2
#define SW_VERSION_READ_CMD 0xA3

#define SERVO_FORWARD_TO 0xff
/**
 * 上传数据到主机的地址
 */
typedef enum
{
    UPLOAD_CMD_ALARM = 0x01,     // 报警
    UPLOAD_CMD_STEPPER = 0x02,   // 步进电机状态
    UPLOAD_CMD_ELEMAG = 0x03,    // 电磁铁状态
    UPLOAD_CMD_OTHERSOUT = 0x04, // 其他输出状态
    UPLOAD_CMD_SENSORS = 0x05,   // 传感器状态
    UPLOAD_CMD_OTHERSIN = 0x06,  // 其他输入状态
    UPLOAD_CMD_ADC_IN = 0x07,    // ADC输入状态
    UPLOAD_CMD_HWVERSION = 0x08, // 硬件版本号
    UPLOAD_CMD_SWVERSION = 0x09, // 软件版本号
} upload_command_t;



void StartCanTask(void *argument);
void CanTask(void);
void CanEmgTask(void *argument);
void uploadSensorsData(void);

rt_err_t cantx_msg_push(CANFrame frame);
rt_err_t cantx_msg_pop(CANFrame *frame);
rt_err_t canrx_msg_push(CANFrame frame);
rt_err_t canrx_msg_pop(CANFrame *frame);

extern struct rt_messagequeue stepper_queue;

#endif /* SRC_USER_CAN_COM_TASK_H_ */
