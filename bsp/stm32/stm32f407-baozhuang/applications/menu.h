#ifndef _MENU_HANDLERS_H_
#define _MENU_HANDLERS_H_

typedef struct _FN_Operate_Func {
	int type ; // 0: exec only , 1 : with a "int" param
	char * desc ;
	int32_t parm ;
	void (* handler)(void * parameters);
}FN_Operate_Func;
#endif
