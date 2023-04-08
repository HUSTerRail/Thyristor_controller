#ifndef _PARAMS_H_
#define _PARAMS_H_


typedef struct _OperateParams
{
	uint16_t speed ;
	uint16_t bags ;
	uint16_t modLen ; // 0.1 mm
	uint16_t colorInd ; // 0.1 mm
	uint16_t materialLen ;// 0.1 mm
	uint16_t modEn ;
	uint16_t meterialEn ;
	uint16_t cutterEn ;
	uint16_t cutterSync ; //切刀同步
	uint16_t stopPosi ;
	uint16_t counter;
	uint16_t counterMax ;
	uint16_t brushEn ;
	uint16_t modJointStart ;
	uint16_t modJointStop ;
	uint16_t jointSpeed ;
	uint16_t jogSpeed;
	uint16_t xuntingSpeed ;
	uint16_t accTime ; //s
	uint16_t decTime ; //s
	uint16_t cutPrevent ; //防止切料
	uint16_t cutPrevStart ;
	uint16_t cutPrevStop ;
	uint16_t cutPrevShiftBags ;
	uint16_t emptyDetect ;
	uint16_t emptyDetStart ;
	uint16_t emptyDetStop ;
	uint16_t emptyDetShiftBags ;
	uint16_t emptyBagsStopEn ;
	uint16_t emptyBagsStopNum ;
	uint16_t emptyBagsStopDec ;
	uint16_t modRemoveBlowEn ;
	uint16_t modRemoveBlowPosi ;
	uint16_t emptyBagsStopTime ; //s
}OperateParams ;

#endif