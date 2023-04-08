
#ifndef _MONITOR_H_
#define _MONITOR_H_

typedef struct _SystemStatus
{
	int32_t modPosi ;
	int32_t meterialPosi ;
	int32_t cutterPosi ;
	int16_t modDeg ;
	int16_t meterialDeg ;
	int16_t cutterDeg ;
	int16_t colorCodeErr ;
	int16_t meterialErr ;
	int16_t status ; // 0 : stop , 1 : jog  2:normal
	int16_t accStatus; // 0 : acc 1 : dec
}SystemStatus ;

#endif