/*
 * can_com_task.c
 *
 *  Created on: 2020年5月20日
 *      Author: yu953
 */
#include "rtthread.h"
#include "rtdevice.h"
#include "ulog.h"
#include "can_com_task.h"
#include "main.h"
#include "stdio.h"
#include "CanToWifi.h"
uint32_t softVersionTime(void);
//#pragma location = "IAP_SECTION"
volatile uint16_t iap_flags;
static int32_t ExtCANUniqID;
static uint8_t ChipUID[6];

static struct rt_semaphore rx_sem;     /* can int receive */
static struct rt_semaphore rx_handle_sem;     /* can handle semaphore */
static struct rt_semaphore tx_sem;
static rt_device_t can_dev;            /* CAN 设备句柄 */




static void CanDataHandle(CANFrame *frame);
static void IAPMsg(uint32_t cmd);
void uploadAdcData(void);
void uploadHwVersion(void);
void uploadSwVersion(void);


/************************************************************can receive thread and transmit thread*************************************************************************/

/* 接收数据回调函数 */
static rt_err_t can_rx_call(rt_device_t dev, rt_size_t size)
{
    rt_sem_release(&rx_sem);
    return RT_EOK;
}


static void can_rx_thread(void *parameter)
{
    rt_err_t res;
    struct rt_can_msg rxmsg = {0};

    /* 设置接收回调函数 */
    rt_device_set_rx_indicate(can_dev, can_rx_call);

#ifdef RT_CAN_USING_HDR
    struct rt_can_filter_item items[9] =
    {
        //RT_CAN_FILTER_ITEM_INIT(0x100, 0, 0, 1, 0x700, RT_NULL, RT_NULL), /* std,match ID:0x100~0x1ff，hdr 为 - 1，设置默认过滤表 */
        //RT_CAN_FILTER_ITEM_INIT(0x300, 0, 0, 1, 0x700, RT_NULL, RT_NULL), /* std,match ID:0x300~0x3ff，hdr 为 - 1 */
        //RT_CAN_FILTER_ITEM_INIT(0x211, 0, 0, 1, 0x7ff, RT_NULL, RT_NULL), /* std,match ID:0x211，hdr 为 - 1 */
        //RT_CAN_FILTER_STD_INIT(0x486, RT_NULL, RT_NULL),                  /* std,match ID:0x486，hdr 为 - 1 */
        //{0x555, 0, 0, 1, 0x7ff, 7,}                                       /* std,match ID:0x555，hdr 为 7，指定设置 7 号过滤表 */
        RT_CAN_FILTER_ITEM_INIT(0x59, 1, 0, 1, 0xff, RT_NULL, RT_NULL),
        RT_CAN_FILTER_EXT_INIT(0x5000000, RT_NULL, RT_NULL),
        RT_CAN_FILTER_EXT_INIT(0x6000000, RT_NULL, RT_NULL),
        RT_CAN_FILTER_EXT_INIT(0x7000000, RT_NULL, RT_NULL),
        RT_CAN_FILTER_EXT_INIT(0x5000000 + ExtCANUniqID, RT_NULL, RT_NULL),
        RT_CAN_FILTER_EXT_INIT(0x6000000 + ExtCANUniqID, RT_NULL, RT_NULL),
        RT_CAN_FILTER_EXT_INIT(0x7000000 + ExtCANUniqID, RT_NULL, RT_NULL),
        RT_CAN_FILTER_STD_INIT(0x08, RT_NULL, RT_NULL),
        {0, 1, 0, 1, 0, 8,},
    };
    struct rt_can_filter_config cfg = {8, 1, items}; /* 一共有 5 个过滤表 */
    /* 设置硬件过滤表 */
    res = rt_device_control(can_dev, RT_CAN_CMD_SET_FILTER, &cfg);
    RT_ASSERT(res == RT_EOK);
#endif
    CANFrame rxframe;
    while (1)
    {
        rxmsg.hdr = -1;
        rt_sem_take(&rx_sem, RT_WAITING_FOREVER);
        int size = rt_device_read(can_dev, 0, &rxmsg, sizeof(rxmsg));
        int i;
        if (size)
        {
            /*
            rt_kprintf("ID:%x ", rxmsg.id);
            for (i = 0; i < 8; i++)
            {
                    rt_kprintf("%2x", rxmsg.data[i]);
            }
            rt_kprintf("\n");
            */
            rxframe.id32 = 0;
            rxframe.ext = rxmsg.ide;
            rxframe.id = rxmsg.id;
            rxframe.dlc = rxmsg.len;
            for (i = 0 ; i < rxframe.dlc && i < 8 ; i++)
            {
                rxframe.data[i] = rxmsg.data[i];
            }
            canrx_msg_push(rxframe);
            rt_sem_release(&rx_handle_sem);
        }
    }
}

static void can_tx_thread(void *parameter)
{
    CANFrame txframe;
    struct rt_can_msg msg = {0};           /* CAN 消息 */
    while (1)
    {
        rt_sem_take(&tx_sem, RT_WAITING_FOREVER);
        if (cantx_msg_pop(&txframe) == RT_EOK)
        {

            msg.id = txframe.id;              /* ID 为 0x78 */
            msg.ide = txframe.ext ? RT_CAN_EXTID : RT_CAN_STDID;     /* 标准格式 */
            msg.rtr = RT_CAN_DTR;       /* 数据帧 */
            msg.len = txframe.dlc;                /* 数据长度为 8 */

            for (int i = 0 ; i < 8 ; i++)
            {
                msg.data[i] = txframe.data[i];
            }

            while (rt_device_write(can_dev, 0, &msg, sizeof(msg)) <= 0)
            {
                rt_thread_mdelay(1);
            }
        }

    }
}

int canbus_init()
{
    rt_err_t result;
    rt_thread_t thread;
    uint32_t uid1 = 0x12345678;
    uint32_t uid2 = 0x25967470;
    ChipUID[0] = ((uid1 + uid2) >> 0) & 0xff;
    ChipUID[1] = ((uid1 + uid2) >> 8) & 0xff;
    ChipUID[2] = ((uid1 + uid2) >> 16) & 0xff;
    ChipUID[3] = ((uid1 + uid2) >> 24) & 0xff;
    ChipUID[4] = ((uid1) >> 0) & 0xff;
    ChipUID[5] = ((uid2) >> 0) & 0xff;
    ExtCANUniqID = (uid1 + uid2 + (uid1 >> 16) + (uid2 >> 16)) & 0xffffff; //构建一个24bit的UID

    /* 查找 CAN 设备 */
    can_dev = rt_device_find("can1");
    if (!can_dev)
    {
        rt_kprintf("find %s failed!\n", "can1");
        return RT_ERROR;
    }

    /* 初始化 CAN 接收信号量 */
    rt_sem_init(&rx_sem, "rx_sem", 0, RT_IPC_FLAG_FIFO);
    rt_sem_init(&tx_sem, "tx_sem", 0, RT_IPC_FLAG_FIFO);

    /* 以中断接收及发送方式打开 CAN 设备 */
    result = rt_device_open(can_dev, RT_DEVICE_FLAG_INT_TX | RT_DEVICE_FLAG_INT_RX);
    RT_ASSERT(result == RT_EOK);
    //result = rt_device_control(can_dev, RT_CAN_CMD_SET_BAUD, (void *)CAN1MBaud);
    RT_ASSERT(result == RT_EOK);
    //result = rt_device_control(can_dev, RT_CAN_CMD_SET_MODE, (void *)RT_CAN_MODE_NORMAL);
    RT_ASSERT(result == RT_EOK);
    /* create receive thread */
    thread = rt_thread_create("can_rx", can_rx_thread, RT_NULL, 1024, 4, 1);
    rt_kprintf("create can_rx thread!\n");
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {
        rt_kprintf("create can_rx thread failed!\n");
    }
    /* create transmit thread */
    thread = rt_thread_create("can_tx", can_tx_thread, RT_NULL, 1024, 5, 1);
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {
        rt_kprintf("create can_rx thread failed!\n");
    }

    return result;
}

/************************************************can handler*********************************************************/
struct rt_messagequeue can_tx_queue ;
struct rt_messagequeue can_rx_queue ;

static uint8_t cantx_queue_pool[sizeof(CANFrame) * 20];
static uint8_t canrx_queue_pool[sizeof(CANFrame) * 20];

rt_err_t cantx_msg_push(CANFrame frame)
{
    rt_err_t status = rt_mq_send(&can_tx_queue, &frame, sizeof(CANFrame));
    rt_sem_release(&tx_sem);
    return status;
}

rt_err_t cantx_msg_pop(CANFrame *frame)
{
    return rt_mq_recv(&can_tx_queue, frame, sizeof(CANFrame), 0);
}

rt_err_t canrx_msg_push(CANFrame frame)
{
    return rt_mq_send(&can_rx_queue, &frame, sizeof(CANFrame));
}

rt_err_t canrx_msg_pop(CANFrame *frame)
{
    return rt_mq_recv(&can_rx_queue, frame, sizeof(CANFrame), 0);
}


int candev_send_frame(CANFrame frame)
{
    struct rt_can_msg msg = {0};           /* CAN 消息 */
    msg.id = frame.id;              /* ID 为 0x78 */
    msg.ide = frame.ext ? RT_CAN_EXTID : RT_CAN_STDID;     /* 标准格式 */
    msg.rtr = RT_CAN_DTR;       /* 数据帧 */
    msg.len = frame.dlc;                /* 数据长度为 8 */

    for (int i = 0 ; i < 8 ; i++)
    {
        msg.data[i] = frame.data[i];
    }

    return rt_device_write(can_dev, 0, &msg, sizeof(msg));
}


static rt_err_t timeout_cb(rt_device_t dev, rt_size_t size)
{
    ScopeTimerHandler();
    return 0;
}

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       512
#define THREAD_TIMESLICE        1
static rt_thread_t tid_can = RT_NULL;


void can_task_entry(void *argument)
{

    rt_err_t result;

    /* 初始化消息队列 */
    result = rt_mq_init(&can_tx_queue,
                        "can_tx_queue",
                        &cantx_queue_pool[0],             /* 内存池指向 msg_pool */
                        sizeof(CANFrame),                          /* 每个消息的大小是 1 字节 */
                        sizeof(cantx_queue_pool),        /* 内存池的大小是 msg_pool 的大小 */
                        RT_IPC_FLAG_PRIO);       /* 如果有多个线程等待，优先级大小的方法分配消息 */
    RT_ASSERT(result == RT_EOK);
    result = rt_mq_init(&can_rx_queue,
                        "can_rx_queue",
                        &canrx_queue_pool[0],             /* 内存池指向 msg_pool */
                        sizeof(CANFrame),                          /* 每个消息的大小是 1 字节 */
                        sizeof(canrx_queue_pool),        /* 内存池的大小是 msg_pool 的大小 */
                        RT_IPC_FLAG_PRIO);       /* 如果有多个线程等待，优先级大小的方法分配消息 */
    RT_ASSERT(result == RT_EOK);

    canbus_init();
    CanToWifiInit(1);
    rt_err_t ret = RT_EOK;
    rt_hwtimerval_t timeout_s;      /* 定时器超时值 */
    rt_device_t hw_dev = RT_NULL;   /* 定时器设备句柄 */
    rt_hwtimer_mode_t mode;         /* 定时器模式 */
    hw_dev = rt_device_find("timer11");
    RT_ASSERT(hw_dev != RT_NULL)
    ret = rt_device_open(hw_dev, RT_DEVICE_OFLAG_RDWR);
    RT_ASSERT(ret == RT_EOK)
    rt_device_set_rx_indicate(hw_dev, timeout_cb);
    mode = HWTIMER_MODE_PERIOD;
    ret = rt_device_control(hw_dev, HWTIMER_CTRL_MODE_SET, &mode);
    RT_ASSERT(ret == RT_EOK);
    timeout_s.sec = 0;
    timeout_s.usec = 100;
    if (rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s)) != sizeof(timeout_s))
    {
        rt_kprintf("set timeout value failed\n");
        RT_ASSERT(0);
    }
/*
    tid_gpio = rt_thread_create("gpio_task",
                                gpio_entry, RT_NULL,
                                512,
                                3, 1);

    RT_ASSERT(tid_gpio != RT_NULL);
    rt_thread_startup(tid_gpio);
*/
    rt_sem_init(&rx_handle_sem, "rx_handle_sem", 0, RT_IPC_FLAG_FIFO);

    CANFrame frame;
    extern void uploadSwVersion();

    while (RT_TRUE)
    {
        rt_sem_take(&rx_handle_sem, RT_WAITING_FOREVER);
        if (canrx_msg_pop(&frame) == RT_EOK)
        {
            CanDataHandle(&frame);
        }
        //uploadSwVersion();
        //rt_thread_mdelay(10);
    }
}


int can_task_start(void)
{

    tid_can = rt_thread_create("can_task",
                               can_task_entry, RT_NULL,
                               1024,
                               21, 1);
    rt_kprintf("can_task excute\n");

    if (tid_can != RT_NULL)
        rt_thread_startup(tid_can);

    return 0;
}
//INIT_APP_EXPORT(can_task_start);



/**
 * @brief CAN数据处理分发
 */
static void CanDataHandle(CANFrame *frame)
{
    CANFrame txframe;
    if (frame->ext == 1)
    {
        if ((frame->id & 0xff0000ff) == DEVICE_ADDR)
        {
            txframe = *frame;
            txframe.dlc = 0;
            txframe.id |= CAN_ACK;
            cantx_msg_push(txframe);//ACK
        }
        else if ((frame->id & 0xff000000) != 0)
        {
            IAPMsg(frame->id);
        }
    }
    else
    {
        if (frame->id == CAN_DEBUG_ID_MOSI)
        {
            CANframe rxframe;
            rxframe.dlc = frame->dlc;
            rxframe.ideflg = 0 ;
            rxframe.id = frame->id;
            for (int i = 0 ; i < 8 ; i ++)
            {
                rxframe.data[i] = frame->data[i];
            }
            ReceptionHandler(&rxframe);
        }
        else if (frame->id == 0x53A)
        {
            if (frame->data[0] == 12)
            {
                //extern SysStatus sysStatus;
                //sysStatus.servoPosi = (frame->h32data << 16) + frame->data16[1];
            }
        }
    }
}


#include <rthw.h>
#include <rtdef.h>
#include <board.h>
static void IAPMsg(uint32_t cmd)
{
    if (cmd == (0x5000000 + ExtCANUniqID) || cmd == 0x5000000)   //read info
    {
        uint32_t flash = 0x6005FFFF - 0x60010000 + 1;
        flash = flash >> 10;
        uint16_t devno = 0x201;
        CANFrame txframe;
        txframe.id32 = 0;
        txframe.ext = 1;
        txframe.id = 0x5000000 + ExtCANUniqID;
        txframe.dlc = 8;    // 字节
        txframe.data[0] = flash & 0xff;
        txframe.data[1] = (flash >> 8) & 0xff;
        txframe.data[2] = devno & 0xff;
        txframe.data[3] = (devno >> 8) & 0xff;
        txframe.data[4] = ChipUID[0];
        txframe.data[5] = ChipUID[1];
        txframe.data[6] = ChipUID[2];
        txframe.data[7] = ChipUID[3];
        cantx_msg_push(txframe);
    }
    else if (cmd == (0x6000000 + ExtCANUniqID) || cmd == 0x6000000)     //reboot
    {
        //IOMUXC_SNVS_GPR->GPR0 = 0xCCCC;
        NVIC_SystemReset();
    }
    else if (cmd == (0x7000000 + ExtCANUniqID) || cmd == 0x7000000)    //reboot
    {
        NVIC_SystemReset();
    }
}

int CanframeTransmit(CANframePtr frame_p)
{
    CANFrame txframe;
    txframe.id32 = 0;
    txframe.ext = frame_p->ideflg;
    txframe.id = frame_p->id;
    txframe.dlc = frame_p->dlc;
    for (int i = 0 ; i < 8 ; i++)
    {
        txframe.data[i] = frame_p->data[i];
    }
    cantx_msg_push(txframe);
    return 0;
}



int hardVers = 0;
/**
 * @breif
 */
void uploadHwVersion()
{
    CANFrame frame;
    frame.id32 = 0;
    frame.dev_id = MAIN_ENGINE_ADDR;
    frame.main_cmd = DEVICE_ADDR;
    frame.sub_cmd = UPLOAD_CMD_HWVERSION;
    frame.ext = 1;
    frame.dlc = 8;
    frame.data[0] = 'h';
    frame.data[1] = 'g';
    frame.data[2] = 'l';
    frame.data[3] = 'v';
    frame.h32data = hardVers;
    cantx_msg_push(frame);
}
/*
 *@brief 软件编译的时间
 */
uint32_t softVersionTime()
{
    static char datestr[20] = __DATE__;  //获取编译日期
    static uint16_t year = 0, mon = 0, day = 0, firstFlag = 0;
    static uint32_t time = 0;
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
                temp = datestr[i + 3] - '0';
                if (temp <= 9)
                {
                    year = temp * 10;
                    temp = datestr[i + 4] - '0';
                    if (temp <= 9)
                        year = year + temp;
                    else
                        year = 0;
                }
                else
                {
                    year = 0;
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
        time = year * 10000 + mon * 100 + day;
    }
    return time;
}

/**
 * @breif
 */
void uploadSwVersion()
{
#define SUB_SOFTVERSION 0 //0~255
    CANFrame frame;
    frame.id32 = 0;
    frame.dev_id = MAIN_ENGINE_ADDR;
    frame.main_cmd = DEVICE_ADDR;
    frame.sub_cmd = UPLOAD_CMD_SWVERSION;
    frame.ext = 1;
    frame.dlc = 8;
    frame.data[0] = 's';
    frame.data[1] = 'g';
    frame.data[2] = 'l';
    frame.data[3] = 'v';
    frame.h32data = (softVersionTime() << 8) + SUB_SOFTVERSION;
    cantx_msg_push(frame);
}



