/*****************************************
 **
 **
 **
 **
 **
 ******************************************/
#include "rtthread.h"
#include "CanToWifi.h"

ScopeSetting scopeSetting;
int16_t (*CanReadPara)(int16_t);
int16_t (*CanWritePara)(int16_t, int16_t);

void writeMemComCAN(uint16_t addr, uint16_t datx);
void ScopedataTransmit(void);
void canSYN(void);
void canACK(CANframePtr framePtr);
void setErrframe(CANframePtr framePtr, uint16_t err_no, uint16_t err_code);
void scopeSwitch(uint16_t data);

#ifdef __ICCARM__
    __weak int32_t GetScopeData(uint16_t addr);
#elif defined(__GNUC__)
    int32_t GetScopeData(uint16_t addr)__attribute__((weak));
#elif defined(__CC_ARM)
    int32_t GetScopeData(uint16_t addr)__attribute__((weak));
#else
    error "Tool Chain error"
#endif

#ifdef __ICCARM__
    __weak int CanframeTransmit(CANframePtr frame_p);
#elif defined(__GNUC__)
    int CanframeTransmit(CANframePtr frame_p)__attribute__((weak));
#elif defined(__CC_ARM)
    int CanframeTransmit(CANframePtr frame_p)__attribute__((weak));
#else
    error "Tool Chain error"
#endif


#ifdef __ICCARM__
    __weak int16_t SCIRead_Par(int16_t nno);
#elif defined(__GNUC__)
    int16_t SCIRead_Par(int16_t nno)__attribute__((weak));
#elif defined(__CC_ARM)
    int16_t SCIRead_Par(int16_t nno)__attribute__((weak));
#else
    error "Tool Chain error"
#endif

#ifdef __ICCARM__
    __weak int16_t SCIWrite_Par(int16_t nno, int16_t data);
#elif defined(__GNUC__)
    int16_t SCIWrite_Par(int16_t nno, int16_t data)__attribute__((weak));
#elif defined(__CC_ARM)
    int16_t SCIWrite_Par(int16_t nno, int16_t data)__attribute__((weak));
#else
    error "Tool Chain error"
#endif


void CanToWifiInit(uint8_t stationNumber)
{
    scopeSetting.TryLimit = 5;
    scopeSetting.TimeoutSet = 5000;
    CanReadPara = SCIRead_Par;
    CanWritePara = SCIWrite_Par;
    scopeSetting.staNumber = stationNumber;
}

uint8_t ScopeisEnable()
{
    return scopeSetting.scopeCtrReg.bit.ScopeEnable;
}

void ReceptionHandler(CANframePtr framePtr)
{

    //uint16_t i;
    uint16_t addr;
    uint16_t data;
    CANframe txframe = { .id = CAN_DEBUG_ID_MISO, .dlc = 0x08, .ideflg = 0 };

    /*station number*/
    if ((framePtr->data[0] != 0)
            && (framePtr->data[0] != scopeSetting.staNumber))
        return;

    for (int i = 0; i < framePtr->dlc; i++)
        txframe.data[i] = framePtr->data[i];
    txframe.data[0] = scopeSetting.staNumber;

    addr = framePtr->data[2] << 8 | framePtr->data[3];
    switch (framePtr->data[1])
    {
    case 0x03:
    case 0x64:
        txframe.data[4] = 0x02; //回复字节个数
        if (addr >= PARAMSGROUPStart && addr <= PARAMSGROUPEnd)   //
        {
            if (CanReadPara)
                data = CanReadPara(addr);
            else
                data = 0;
            txframe.data[6] = data & 0xFF;
            txframe.data[5] = (data >> 8) & 0xFF;
        }
        else if (addr >= STATUSGROUPStart && addr <= STATUSGROUPEnd)
        {
            data = GetScopeData(addr - STATUSGROUPStart);
            txframe.data[6] = data & 0xFF;
            txframe.data[5] = (data >> 8) & 0xFF;
        }
        else if (addr >= CANCMD_START_ADDR && addr <= CANCMD_END_ADDR)
        {
            data = 0;//CANCmdRead(addr);
            txframe.data[6] = data & 0xFF;
            txframe.data[5] = (data >> 8) & 0xFF;
        }
        else if (addr < MONITORSETStart)
        {
            data = 110;
            txframe.data[6] = data & 0xFF;
            txframe.data[5] = (data >> 8) & 0xFF;
        }
        else
        {
            //setErrframe(&txframe, 0x83, 0x02);
            setErrframe(&txframe, 0x64, 0x02);
        }
        CanframeTransmit(&txframe);
        break;
    case 0x06:
        data = framePtr->data[4] << 8 | framePtr->data[5];
        if (addr >= PARAMSGROUPStart && addr <= PARAMSGROUPEnd)
        {
            if (CanWritePara)
                CanWritePara(addr, data);
        }
        else if (addr >= CANCMD_START_ADDR && addr <= CANCMD_END_ADDR)
        {
            //CANCmdHandle(addr, data);
        }
        else
        {
            writeMemComCAN(addr, data);
        }
        CanframeTransmit(&txframe);
        break;
    case 0x65:
        //heart-beat response or scope start
        if ((framePtr->data[2] == 0x01)
                && ((framePtr->data[3] == 0x02) || (framePtr->data[3] == 0x03)))
        {
            canACK(&txframe);
            CanframeTransmit(&txframe);
        }
        break;
    default:
        break;
    }
}

void ScopeTimerHandler()
{
    if (!scopeSetting.scopeCtrReg.bit.ScopeEnable) //if scope is disabled,return
        return;
    scopeSetting.DisplayIntervalTimer++;          //timer runs
    scopeSetting.TimeoutTimer++;
    if (scopeSetting.DisplayIntervalTimer >= scopeSetting.DisplayIntervalSet)   //data send timer
    {
        scopeSetting.DisplayIntervalTimer = 0;
        ScopedataTransmit();
    }
    if (scopeSetting.TimeoutTimer >= scopeSetting.TimeoutSet)   //beat heart timer,
    {
        scopeSetting.TimeoutTimer = 0;
        canSYN();   //timeout and send syn to ask for ack
        if (scopeSetting.TryTimes >= scopeSetting.TryLimit)   //disconnected, scope stoped
        {
            scopeSwitch(0x5555);
        }
        else
        {
            scopeSetting.TryTimes++;
        }
    }
}

void writeMemComCAN(uint16_t addr, uint16_t data)
{
    switch (addr)
    {
    case MONITORSETStart:     //scope switch
        scopeSwitch(data);
        break;
    case MONITORSETStart + 0x01:   //channel enable
        scopeSetting.scopeCtrReg.all = data;
        break;
    case MONITORSETStart + 0x02:
        scopeSetting.DisplayIntervalSet = data;
        break;
    case MONITORSETStart + 0x03:
        scopeSetting.CH1Select = data;
        break;
    case MONITORSETStart + 0x04:
        scopeSetting.CH2Select = data;
        break;
    case MONITORSETStart + 0x05:
        scopeSetting.CH3Select = data;
        break;
    case MONITORSETStart + 0x06:
        scopeSetting.CH4Select = data;
        break;
    case MONITORSETStart + 0x07:
        scopeSetting.CH5Select = data;
        break;
    case MONITORSETStart + 0x08:
        scopeSetting.CH6Select = data;
        break;
    case MONITORSETStart + 0x09:
        scopeSetting.CH7Select = data;
        break;
    case MONITORSETStart + 0x0A:
        scopeSetting.CH8Select = data;
        break;
    default:
        break;
    }
}

void ScopedataTransmit()
{
    static CANframe frame = { .id = CAN_DEBUG_ID_MISO, .dlc = 0x08, .ideflg = 0 };
    //static CANframe frame_ext = { .id = 0x08, .dlc = 0x08, .ideflg = 0 };
    static int32_t data;
    static uint16_t Scope_ID = 0x0010;
    static uint16_t Scope_ID_ext = 0x0020;
    if (!scopeSetting.scopeCtrReg.bit.ScopeEnable) //if scope is disabled,return
        return;
    if (scopeSetting.scopeCtrReg.bit.CH5Enable)
    {
        data = GetScopeData(scopeSetting.CH5Select - STATUSGROUPStart);
        frame.data[0] = (data >> 8) & 0xFF;
        frame.data[1] = data & 0xFF;
        if (scopeSetting.scopeCtrReg.bit.CH5width_32)
        {
            frame.data[2] = (data >> 8) & 0xFF;
            frame.data[3] = data & 0xFF;
            frame.data[1] = (data >> 16) & 0xFF;
            frame.data[0] = (data >> 24) & 0xFF;
        }
    }
    if (scopeSetting.scopeCtrReg.bit.CH6Enable)
    {
        data = GetScopeData(scopeSetting.CH6Select - STATUSGROUPStart);
        frame.data[2] = (data >> 8) & 0xFF;
        frame.data[3] = data & 0xFF;
    }
    if (scopeSetting.scopeCtrReg.bit.CH7Enable)
    {
        data = GetScopeData(scopeSetting.CH7Select - STATUSGROUPStart);
        frame.data[4] = (data >> 8) & 0xFF;
        frame.data[5] = data & 0xFF;
        if (scopeSetting.scopeCtrReg.bit.CH7width_32)
        {
            frame.data[6] = (data >> 8) & 0xFF;
            frame.data[7] = data & 0xFF;
            frame.data[5] = (data >> 16) & 0xFF;
            frame.data[4] = (data >> 24) & 0xFF;
        }
    }
    if (scopeSetting.scopeCtrReg.bit.CH8Enable)
    {
        data = GetScopeData(scopeSetting.CH8Select - STATUSGROUPStart);
        frame.data[6] = (data >> 8) & 0xFF;
        frame.data[7] = data & 0xFF;
    }
    frame.id = Scope_ID_ext;
    //if channel 5~8 is enable
    if (scopeSetting.scopeCtrReg.bit.CH5Enable
            || scopeSetting.scopeCtrReg.bit.CH6Enable
            || scopeSetting.scopeCtrReg.bit.CH7Enable
            || scopeSetting.scopeCtrReg.bit.CH8Enable)
        CanframeTransmit(&frame);
    if ((Scope_ID_ext & 0x07) == 0x07)     //id cycle
        Scope_ID_ext &= 0xff8;
    else
        Scope_ID_ext++;

    if (scopeSetting.scopeCtrReg.bit.CH1Enable)
    {
        data = GetScopeData(scopeSetting.CH1Select - STATUSGROUPStart);
        frame.data[0] = (data >> 8) & 0xFF;
        frame.data[1] = data & 0xFF;
        if (scopeSetting.scopeCtrReg.bit.CH1width_32)
        {
            frame.data[2] = (data >> 8) & 0xFF;
            frame.data[3] = data & 0xFF;
            frame.data[1] = (data >> 16) & 0xFF;
            frame.data[0] = (data >> 24) & 0xFF;
        }
    }
    if (scopeSetting.scopeCtrReg.bit.CH2Enable)
    {
        data = GetScopeData(scopeSetting.CH2Select - STATUSGROUPStart);
        frame.data[2] = (data >> 8) & 0xFF;
        frame.data[3] = data & 0xFF;
    }
    if (scopeSetting.scopeCtrReg.bit.CH3Enable)
    {
        data = GetScopeData(scopeSetting.CH3Select - STATUSGROUPStart);
        frame.data[4] = (data >> 8) & 0xFF;
        frame.data[5] = data & 0xFF;
        if (scopeSetting.scopeCtrReg.bit.CH3width_32)
        {
            frame.data[6] = (data >> 8) & 0xFF;
            frame.data[7] = data & 0xFF;
            frame.data[5] = (data >> 16) & 0xFF;
            frame.data[4] = (data >> 24) & 0xFF;
        }
    }
    if (scopeSetting.scopeCtrReg.bit.CH4Enable)
    {
        data = GetScopeData(scopeSetting.CH4Select - STATUSGROUPStart);
        frame.data[6] = (data >> 8) & 0xFF;
        frame.data[7] = data & 0xFF;
    }
    frame.id = Scope_ID;
    //if channel 5~8 is enable
    if (scopeSetting.scopeCtrReg.bit.CH1Enable
            || scopeSetting.scopeCtrReg.bit.CH2Enable
            || scopeSetting.scopeCtrReg.bit.CH3Enable
            || scopeSetting.scopeCtrReg.bit.CH4Enable)
        CanframeTransmit(&frame);
    if ((Scope_ID & 0x07) == 0x07)     //id cycle
        Scope_ID &= 0xff8;
    else
        Scope_ID++;
}

void canACK(CANframePtr framePtr)
{
    framePtr->data[2] = 0x01;
    framePtr->data[3] = 0x04;
    scopeSwitch(0xAAAA);
}

/*
 *heart beat response
 */
void canSYN()
{
    static CANframe frame = { .id = CAN_DEBUG_ID_MISO, .dlc = 0x08, .ideflg = 0 };
    for (int i = 0; i < 8; i++)
        frame.data[i] = 0;
    frame.data[0] = scopeSetting.staNumber;
    frame.data[1] = 0x65;
    frame.data[2] = 0x01;
    frame.data[3] = 0x01;
    CanframeTransmit(&frame);
}

void setErrframe(CANframePtr framePtr, uint16_t err_no, uint16_t err_code)
{
    framePtr->data[1] = err_no;
    framePtr->data[2] = (err_code >> 8) & 0xFF;
    framePtr->data[3] = err_code & 0xFF;
    framePtr->data[4] = 0;
    framePtr->data[5] = 9;
}

void scopeSwitch(uint16_t data)
{
    if (data == 0x5555)       //stop scope
    {
        scopeSetting.TryTimes = 0;
        scopeSetting.TimeoutTimer = 0;
        scopeSetting.DisplayIntervalTimer = 0;
        scopeSetting.scopeCtrReg.bit.ScopeEnable = 0;
    }
    else if (data == 0xAAAA)     //start scope or keep scope transmit
    {
        scopeSetting.TryTimes = 0;
        scopeSetting.TimeoutTimer = 0;
        scopeSetting.scopeCtrReg.bit.ScopeEnable = 1;
    }
}

#ifdef __ICCARM__
    __weak
#endif
RT_WEAK
int CanframeTransmit(CANframePtr frame_p)
{
    return 0;
}

#ifdef __ICCARM__
    __weak
#endif
RT_WEAK
int32_t GetScopeData(uint16_t addr)
{
    switch (addr)
    {
    case DrivSt_Status:
        break;
    default:
        break;
    }
    return 0;
}

#ifdef __ICCARM__
    __weak
#endif
RT_WEAK
int16_t SCIRead_Par(int16_t nno)
{
    switch (nno)
    {
    case PARAMSGROUPStart:
        break;
    default:
        break;
    }
    return 0;
}

#ifdef __ICCARM__
    __weak
#endif
int16_t SCIWrite_Par(int16_t nno, int16_t data)
{
    switch (nno)
    {
    case PARAMSGROUPStart:
        break;
    default:
        break;
    }
    return 0;
}

