#include "controller.h"
#include "string.h"

#define LOG_TAG      "state.menu"
#define LOG_LVL      LOG_LVL_DBG
#include <rtthread.h>
#include "state_machine.h"
#include <ulog.h>
#include "hci.h"

enum event_menu_type {
	EVENT_MENU_PLUS,
	EVENT_MENU_MINUS,
	EVENT_MENU_ENT,
	EVENT_MENU_ESC,
	EVENT_MENU_SHIFT,
	EVENT_MENU_ALM,
	EVENT_MENU_PERIOD,
};

#define MAX_ON_NUM 10
#define MAX_FN_NUM 10
#define MAX_PN_NUM 10

typedef struct _MenuStatus {
	int on_index;
	int pn_index;
	int fn_index;
	int root_index;
	int cursor;
	int level ;
	void *data;
	void *parm;
} MenuStatus;

MenuStatus menu_data;

static struct state menu_state_init, menu_state_root, menu_state_on,
		menu_state_on_show, menu_state_fn, menu_state_fn_edit,
		menu_state_fn_end, menu_state_pn, menu_state_pn_edit, menu_state_error;

__weak void menu_show();
__weak void menu_on_show();
__weak void menu_pn_show();
__weak void menu_fn_show();

static bool menu_guard_fn_conditon(void *condition, struct event *event);
static bool menu_guard_rootenter_conditon(void *condition, struct event *event);
static void menu_entry_error(void *state_data, struct event *event);
static void menu_exit_init(void *state_data, struct event *event);
static void menu_entry_root(void *state_data, struct event *event);
static void menu_exit_root(void *state_data, struct event *event);
static void menu_entry_pn_read(void *state_data, struct event *event);

static void menu_action_root_plus(void *oldstate_data, struct event *event,
		void *state_new_data);
static void menu_action_root_minus(void *oldstate_data, struct event *event,
		void *state_new_data);
static void menu_action_root_enter(void *oldstate_data, struct event *event,
		void *state_new_data);
static void menu_action_on_plus(void *oldstate_data, struct event *event,
		void *state_new_data);
static void menu_action_on_minus(void *oldstate_data, struct event *event,
		void *state_new_data);
static void menu_action_on_show(void *oldstate_data, struct event *event,
		void *state_new_data);
static void menu_action_on_alarm(void *oldstate_data, struct event *event,
		void *state_new_data);
static void menu_action_on_show_modi(void *oldstate_data, struct event *event,
		void *state_new_data);
static void menu_action_pn_plus(void *oldstate_data, struct event *event,
		void *state_new_data);
static void menu_action_pn_minus(void *oldstate_data, struct event *event,
		void *state_new_data);
static void menu_action_pn_shift(void *oldstate_data, struct event *event,
		void *state_new_data);
static void menu_action_pn_edit(void *oldstate_data, struct event *event,
		void *state_new_data);
static void menu_action_pn_edit_plus(void *oldstate_data, struct event *event,
		void *state_new_data);
static void menu_action_pn_edit_minus(void *oldstate_data, struct event *event,
		void *state_new_data);
static void menu_action_pn_edit_shift(void *oldstate_data, struct event *event,
		void *state_new_data);
static void menu_action_pn_write(void *oldstate_data, struct event *event,
		void *state_new_data);
static void menu_action_fn_plus(void *oldstate_data, struct event *event,
		void *state_new_data);
static void menu_action_fn_minus(void *oldstate_data, struct event *event,
		void *state_new_data);
static void menu_action_fn_edit(void *oldstate_data, struct event *event,
		void *state_new_data);
static void menu_action_fn_edit_plus(void *oldstate_data, struct event *event,
		void *state_new_data);
static void menu_action_fn_edit_minus(void *oldstate_data, struct event *event,
		void *state_new_data);
static void menu_action_fn_edit_shift(void *oldstate_data, struct event *event,
		void *state_new_data);
static void menu_action_fn_exe(void *oldstate_data, struct event *event,
		void *state_new_data);

static struct state menu_state_init = {
	.state_parent = NULL,
	.state_entry = NULL,
	.transitions = (struct transition[] ){
		/* event_type, condition, guard, action, next, state  */
		{ EVENT_MENU_PERIOD, NULL, NULL, NULL, &menu_state_root },
		{ EVENT_MENU_ALM, (void*) 0, NULL,&menu_action_on_alarm, &menu_state_on },
	},
	.transition_nums = 2,
	.data = "init",
	.action_entry = NULL,
	.action_exti = &menu_exit_init,
};

static struct state menu_state_root = {
	.state_parent = NULL,
	.state_entry = NULL,
	.transitions = (struct transition[] ){
		/* event_type, condition, guard, action, next, state  */
		{ EVENT_MENU_ESC, NULL, NULL, &menu_action_root_plus, &menu_state_root },
		{ EVENT_MENU_PLUS, (void*) 0, &menu_guard_rootenter_conditon, &menu_action_root_enter,&menu_state_on },
		{ EVENT_MENU_PLUS, (void*) 1, &menu_guard_rootenter_conditon, &menu_action_root_enter,&menu_state_fn },
		{ EVENT_MENU_PLUS, (void*) 2, &menu_guard_rootenter_conditon, &menu_action_root_enter,&menu_state_pn },
		{ EVENT_MENU_MINUS, (void*) 0, &menu_guard_rootenter_conditon, &menu_action_root_enter,&menu_state_on },
		{ EVENT_MENU_MINUS, (void*) 1, &menu_guard_rootenter_conditon, &menu_action_root_enter,&menu_state_fn },
		{ EVENT_MENU_MINUS, (void*) 2, &menu_guard_rootenter_conditon, &menu_action_root_enter,&menu_state_pn },
		{ EVENT_MENU_SHIFT, (void*) 0, &menu_guard_rootenter_conditon, &menu_action_root_enter,&menu_state_on },
		{ EVENT_MENU_SHIFT, (void*) 1, &menu_guard_rootenter_conditon, &menu_action_root_enter,&menu_state_fn },
		{ EVENT_MENU_SHIFT, (void*) 2, &menu_guard_rootenter_conditon, &menu_action_root_enter,&menu_state_pn },
		{ EVENT_MENU_ALM, (void*) 0, NULL,&menu_action_on_alarm, &menu_state_on },
	},
	.transition_nums = 11,
	.data = "ROOT",
	.action_entry = &menu_entry_root,
	.action_exti = &menu_exit_root,
};

static struct state menu_state_on = {
	.state_parent = NULL,
	.state_entry = NULL,
	.transitions = (struct transition[] ) {
		{ EVENT_MENU_PLUS, (void*) 0, NULL, &menu_action_on_plus, &menu_state_on },
		{ EVENT_MENU_MINUS, (void*) 0, NULL, &menu_action_on_minus, &menu_state_on },
		{ EVENT_MENU_ENT, (void*) 0, NULL, &menu_action_on_show, &menu_state_on_show },
		{ EVENT_MENU_PERIOD, (void*) 0, NULL, &menu_action_on_show, &menu_state_on_show },
		{ EVENT_MENU_ESC, (void*) 0, NULL, NULL, &menu_state_root },
		{ EVENT_MENU_ALM, (void*) 0, NULL, &menu_action_on_alarm, &menu_state_on },
	},
	.transition_nums = 6,
	.data = "On",
	.action_entry = NULL,
	.action_exti = NULL,
};

static struct state menu_state_on_show = {
	.state_parent = NULL,
	.state_entry = NULL,
	.transitions = (struct transition[] ) {
		{ EVENT_MENU_PERIOD, (void*) 0, NULL, &menu_action_on_show, &menu_state_on_show },
		{ EVENT_MENU_ENT, (void*) 0, NULL, &menu_action_on_show_modi, &menu_state_on_show },
		{ EVENT_MENU_ESC, (void*) 0, NULL, NULL, &menu_state_on },
		{ EVENT_MENU_ALM, (void*) 0, NULL, &menu_action_on_alarm, &menu_state_on },
	},
	.transition_nums = 4,
	.data = "idle",
	.action_entry = NULL,
	.action_exti = NULL,
};

static struct state menu_state_pn = {
	.state_parent = NULL,
	.state_entry = NULL,
	.transitions = (struct transition[] ) {
		{ EVENT_MENU_PLUS, (void*) 0, NULL, &menu_action_pn_plus, &menu_state_pn },
		{ EVENT_MENU_MINUS, (void*) 0, NULL, &menu_action_pn_minus, &menu_state_pn },
		{ EVENT_MENU_SHIFT, (void*) 0, NULL, &menu_action_pn_shift, &menu_state_pn },
		{ EVENT_MENU_ENT, (void*) 0, NULL, &menu_action_pn_edit, &menu_state_pn_edit },
		{ EVENT_MENU_ESC, (void*) 0, NULL, NULL, &menu_state_root },
		{ EVENT_MENU_ALM, (void*) 0, NULL, &menu_action_on_alarm, &menu_state_on },
	},
	.transition_nums = 6,
	.data = "idle",
	.action_entry = NULL,
	.action_exti = NULL,
};

static struct state menu_state_pn_edit = {
	.state_parent = NULL,
	.state_entry = NULL,
	.transitions = (struct transition[] ) {
		{ EVENT_MENU_PLUS, (void*) 0, NULL, &menu_action_pn_edit_plus, &menu_state_pn_edit },
		{ EVENT_MENU_MINUS, (void*) 0, NULL, &menu_action_pn_edit_minus, &menu_state_pn_edit },
		{ EVENT_MENU_SHIFT, (void*) 0, NULL, &menu_action_pn_edit_shift, &menu_state_pn_edit },
		{ EVENT_MENU_ENT, (void*) 0, NULL, &menu_action_pn_write, &menu_state_pn },
		{ EVENT_MENU_ESC, (void*) 0, NULL, NULL, &menu_state_pn },
		{ EVENT_MENU_ALM, (void*) 0, NULL, &menu_action_on_alarm, &menu_state_on },
	},
	.transition_nums = 6,
	.data = "idle",
	.action_entry = &menu_entry_pn_read,
	.action_exti = NULL,
};

static struct state menu_state_fn = {
	.state_parent = NULL,
	.state_entry = NULL,
	.transitions = (struct transition[] ) {
		{ EVENT_MENU_PLUS, (void*) 0, NULL, &menu_action_fn_plus, &menu_state_fn },
		{ EVENT_MENU_MINUS, (void*) 0, NULL, &menu_action_fn_minus, &menu_state_fn },
		{ EVENT_MENU_ENT, (void*) 0, NULL, NULL, &menu_state_fn_edit },
		{ EVENT_MENU_ESC, (void*) 0, NULL, NULL, &menu_state_root },
		{ EVENT_MENU_ALM, (void*) 0, NULL, &menu_action_on_alarm, &menu_state_on },
	},
	.transition_nums = 5,
	.data = "idle",
	.action_entry = NULL,
	.action_exti = NULL,
};

static struct state menu_state_fn_edit = {
	.state_parent = NULL,
	.state_entry = NULL,
	.transitions = (struct transition[] ) {
		{ EVENT_MENU_PLUS, (void*) 0, NULL, &menu_action_fn_edit_plus, &menu_state_fn_edit },
		{ EVENT_MENU_MINUS, (void*) 0, NULL, &menu_action_fn_edit_minus, &menu_state_fn_edit },
		{ EVENT_MENU_SHIFT, (void*) 0, NULL, &menu_action_fn_edit_shift, &menu_state_fn_edit },
		{ EVENT_MENU_ENT, (void*) 0, &menu_guard_fn_conditon, &menu_action_fn_exe, &menu_state_fn_end },
		{ EVENT_MENU_ESC, (void*) 0, NULL, NULL, &menu_state_fn },
		{ EVENT_MENU_ALM, (void*) 0, NULL, &menu_action_on_alarm, &menu_state_on },
	},
	.transition_nums = 6,
	.data = "fn_edit",
	.action_entry = &menu_entry_pn_read,
	.action_exti = NULL,
};

static struct state menu_state_fn_end = {
	.state_parent = NULL,
	.state_entry = NULL,
	.transitions = (struct transition[] ) {
		{ EVENT_MENU_ENT, (void*) 0, NULL, NULL, &menu_state_fn },
		{ EVENT_MENU_ESC, (void*) 0, NULL, NULL, &menu_state_fn },
		{ EVENT_MENU_ALM, (void*) 0, NULL, &menu_action_on_alarm, &menu_state_on },
	},
	.transition_nums = 3,
	.data = "idle",
	.action_entry = NULL,
	.action_exti = NULL,
};

static struct state menu_state_error = { .data = "error", .action_entry =
		&menu_entry_error, };

static bool menu_guard_fn_conditon(void *condition, struct event *event) {
	return true;
}
static bool menu_guard_rootenter_conditon(void *condition, struct event *event) {
	if (condition == event->data)
		return true;
	else
		return false;
}
static void menu_entry_error(void *state_data, struct event *event) {
	log_e("enter menu error");
}
static void menu_exit_init(void *state_data, struct event *event) {
	menu_data.data = 0;
	menu_data.parm = 0;
	menu_data.fn_index = 0;
	menu_data.on_index = 0;
	menu_data.pn_index = 0;
	menu_data.root_index = 0;
	menu_data.cursor = 0;
	menu_data.level = 0;
	rt_kprintf("exit init\n");
	menu_show();
}

static void menu_entry_root(void *state_data, struct event *event)
{
	menu_data.level = 0;
	menu_show();
	rt_kprintf("enter root\n");
}
static void menu_exit_root(void *state_data, struct event *event) {
	menu_data.level = 1;
	menu_show();
	rt_kprintf("exit root\n");
}

static void menu_entry_pn_read(void *state_data, struct event *event) {
	menu_data.parm = 0;
	menu_data.data = 0;
	menu_pn_show();
}

static void menu_action_root_plus(void *oldstate_data, struct event *event,
		void *state_new_data) {
	menu_data.root_index++;
	if (menu_data.root_index > 2)
		menu_data.root_index = 0;
	menu_show();
}
static void menu_action_root_minus(void *oldstate_data, struct event *event,
		void *state_new_data) {
	menu_data.root_index--;
	if (menu_data.root_index < 0)
		menu_data.root_index = 2;
	menu_show();
}
static void menu_action_root_enter(void *oldstate_data, struct event *event,
		void *state_new_data) {
	menu_show();
}
static void menu_action_on_plus(void *oldstate_data, struct event *event,
		void *state_new_data) {
	menu_data.on_index++;
	if (menu_data.on_index >= MAX_ON_NUM)
		menu_data.on_index = 0;
	menu_show();
}
static void menu_action_on_minus(void *oldstate_data, struct event *event,
		void *state_new_data) {
	menu_data.on_index--;
	if (menu_data.on_index < 0)
		menu_data.on_index = MAX_ON_NUM - 1;
	menu_show();
}
static void menu_action_on_show(void *oldstate_data, struct event *event,
		void *state_new_data) {
	menu_on_show();
}
static void menu_action_on_alarm(void *oldstate_data, struct event *event,
		void *state_new_data) {
	menu_data.on_index = 0;
	menu_on_show();
}
static void menu_action_on_show_modi(void *oldstate_data, struct event *event,
		void *state_new_data) {

}
static void menu_action_pn_plus(void *oldstate_data, struct event *event,
		void *state_new_data) {
	menu_data.pn_index++;
	if (menu_data.pn_index >= MAX_PN_NUM)
		menu_data.pn_index = 0;
	menu_show();
	rt_kprintf("pn plus\n");
}

static void menu_action_pn_minus(void *oldstate_data, struct event *event,
		void *state_new_data) {
	menu_data.pn_index--;
	if (menu_data.on_index < 0)
		menu_data.on_index = MAX_ON_NUM - 1;
	menu_show();
	rt_kprintf("pn minus\n");
}
static void menu_action_pn_shift(void *oldstate_data, struct event *event,
		void *state_new_data) {

}
static void menu_action_pn_edit(void *oldstate_data, struct event *event,
		void *state_new_data) {
	menu_pn_show();
}
static void menu_action_pn_edit_plus(void *oldstate_data, struct event *event,
		void *state_new_data) {
	int *dataptr = (int*) menu_data.data;
	*dataptr += 1;
	if (*dataptr > 100)
		*dataptr = 100;
	menu_pn_show();
}

static void menu_action_pn_edit_minus(void *oldstate_data, struct event *event,
		void *state_new_data) {
	int *dataptr = (int*) menu_data.data;
	*dataptr -= 1;
	if (*dataptr < -100)
		*dataptr = -100;
	menu_pn_show();
}
static void menu_action_pn_edit_shift(void *oldstate_data, struct event *event,
		void *state_new_data) {

}

static void menu_action_pn_write(void *oldstate_data, struct event *event,
		void *state_new_data) {

}
static void menu_action_fn_plus(void *oldstate_data, struct event *event,
		void *state_new_data) {
	menu_data.fn_index++;
	if (menu_data.fn_index >= MAX_FN_NUM)
		menu_data.fn_index = 0;
	menu_show();

}
static void menu_action_fn_minus(void *oldstate_data, struct event *event,
		void *state_new_data) {
	menu_data.fn_index--;
	if (menu_data.fn_index < 0)
		menu_data.fn_index = 0;
	menu_show();
}
static void menu_action_fn_edit(void *oldstate_data, struct event *event,
		void *state_new_data) {
	menu_fn_show();
}
static void menu_action_fn_edit_plus(void *oldstate_data, struct event *event,
		void *state_new_data) {
	int *dataptr = (int*) menu_data.data;
	*dataptr += 1;
	if (*dataptr > 100)
		*dataptr = 100;
	menu_fn_show();

}
static void menu_action_fn_edit_minus(void *oldstate_data, struct event *event,
		void *state_new_data) {
	int *dataptr = (int*) menu_data.data;
	*dataptr -= 1;
	if (*dataptr < -100)
		*dataptr = -100;
	menu_pn_show();
}

static void menu_action_fn_edit_shift(void *oldstate_data, struct event *event,
		void *state_new_data) {

}
static void menu_action_fn_exe(void *oldstate_data, struct event *event,
		void *state_new_data) {
	int index = menu_data.fn_index;
	int *dataptr = (int*) menu_data.data;
	rt_kprintf("fn exe : index@%d,data@%d\n",index,*dataptr);
}

__weak void menu_show() {

	uint8_t buff[] = { Dig_1, Dig_n, Dig_None, Dig_None, Dig_None };     //设置数码管每一位的字符
	if(menu_data.level == 0){
		if (menu_data.root_index == 0) {
			buff[0] = Dig_O;
		} else if (menu_data.root_index == 1) {
			buff[0] = Dig_F;
		} else if (menu_data.root_index == 2) {
			buff[0] = Dig_P;
		}
	}else {
		buff[2] = Dig_Dash;
		if (menu_data.root_index == 0) {
			buff[0] = Dig_O;
			buff[3] = CODE164[(menu_data.on_index / 10) % 10];
			buff[4] = CODE164[menu_data.on_index % 10];
		} else if (menu_data.root_index == 1) {
			buff[0] = Dig_F;
			buff[3] = CODE164[(menu_data.fn_index / 10) % 10];
			buff[4] = CODE164[menu_data.fn_index % 10];
		} else if (menu_data.root_index == 2) {
			buff[0] = Dig_P;
			buff[3] = CODE164[(menu_data.pn_index / 10) % 10];
			buff[4] = CODE164[menu_data.pn_index % 10];
		}
	}

	LedBuffWrite(buff, 0, 5);
}
__weak void menu_on_show() {
	DataShowPropStru prop ;
	prop.data = 0 ;
	DecHexLEDx((int)menu_data.data, prop);
}
__weak void menu_pn_show() {
	DataShowPropStru prop ;
	prop.data = 0 ;
	DecHexLEDx((int)menu_data.data, prop);
}
__weak void menu_fn_show() {
	DataShowPropStru prop ;
	prop.data = 0 ;
	DecHexLEDx((int)menu_data.data, prop);
}

static rt_mq_t mq_event_menu;
static struct state_machine m_menu;

int state_menu_event_set(enum event_menu_type event, void *data) {
	struct event e;
	e.type = event;
	e.data = data;
	return rt_mq_send(mq_event_menu, &e, sizeof(struct event));
}

static void state_process(void *parameter) {
	struct event e;
	statem_init(&m_menu, &menu_state_init, &menu_state_error);

	while (1) {
		if (RT_EOK
				== rt_mq_recv(mq_event_menu, &e, sizeof(struct event), 300)) {
			statem_handle_event(&m_menu, &(struct event ) { e.type, e.data });
		}
		statem_handle_event(&m_menu,
				&(struct event ) { EVENT_MENU_PERIOD, 0 });
	}
}

static int menu_state_machine_initial(void) {
	rt_thread_t tid = RT_NULL;

	mq_event_menu = rt_mq_create("event_menu", sizeof(struct event), 16,
			RT_IPC_FLAG_FIFO);

	tid = rt_thread_create("menu", state_process, RT_NULL, 1024, 10, 100);
	if (tid == RT_NULL) {
		rt_kprintf(
				"menu progress initialize failed! thread create failed!\r\n");

		return -RT_ENOMEM;
	}
	rt_thread_startup(tid);

	return RT_EOK;
}
INIT_APP_EXPORT(menu_state_machine_initial);


void key_handler(KeyInfo info) {
	if (info.KeyNo == K_ESC) {
		if (info.KeyAct == K_CLICK || info.KeyAct == K_CONTINUE) {
			state_menu_event_set(EVENT_MENU_ESC, 0);
		}
	} else if (info.KeyNo == K_PLUS) {
		if (info.KeyAct == K_CLICK || info.KeyAct == K_CONTINUE) {
			state_menu_event_set(EVENT_MENU_PLUS, 0);
		}
	} else if (info.KeyNo == K_MINUS) {
		if (info.KeyAct == K_CLICK || info.KeyAct == K_CONTINUE) {
			state_menu_event_set(EVENT_MENU_MINUS, 0);
		}
	} else if (info.KeyNo == K_ENT) {
		if (info.KeyAct == K_LONGKEY) {
			state_menu_event_set(EVENT_MENU_ENT, 0);
		} else if (info.KeyAct == K_CLICK       ) {
			state_menu_event_set(EVENT_MENU_SHIFT, 0);
		}
	}
}
