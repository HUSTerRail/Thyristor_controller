#include "controller.h"
#include "string.h"

#define LOG_TAG      "state.control"
#define LOG_LVL      LOG_LVL_DBG
#include <rtthread.h>
#include "state_machine.h"
#include <ulog.h>

enum event_ctrl_type
{
    EVENT_CONTROL_FN01Pressed,
	EVENT_CONTROL_FN01Release,
	EVENT_CONTROL_FN10,
	EVENT_CONTROL_EMG,
	EVENT_CONTROL_STOP,

	EVENT_CONTROL_Period,
};



static struct state ctrl_state_root, ctrl_state_idle, ctrl_state_acc, ctrl_state_dec,
ctrl_state_stable,ctrl_state_stop,ctrl_state_error;

static bool ctrl_guard_run_conditon(void *condition, struct event *event);
static bool ctrl_guard_stabled_conditon(void *condition, struct event *event); //速度
static bool ctrl_guard_speeddown_conditon(void *condition, struct event *event);
static bool ctrl_guard_stoped_conditon(void *condition, struct event *event);

static void ctrl_entry_acc( void *state_data, struct event *event );
static void ctrl_entry_dec( void *state_data, struct event *event );
static void ctrl_entry_stable( void *state_data, struct event *event );
static void ctrl_entry_error( void *state_data, struct event *event );

static void ctrl_action_emg( void *oldstate_data, struct event *event, void *state_new_data );
static void ctrl_action_acc( void *oldstate_data, struct event *event, void *state_new_data );
static void ctrl_action_dec( void *oldstate_data, struct event *event, void *state_new_data );
static void ctrl_action_stable( void *oldstate_data, struct event *event, void *state_new_data );
static void ctrl_action_stop( void *oldstate_data, struct event *event, void *state_new_data );

static struct state ctrl_state_root = {
    .state_parent = NULL,
    .state_entry = NULL,
    .transitions = (struct transition[]){
        /* event_type, condition, guard, action, next, state  */
        { EVENT_CONTROL_Period, NULL, NULL, NULL, &ctrl_state_idle },
    },
    .transition_nums = 1,
    .data = "ROOT",
    .action_entry = NULL,
    .action_exti = NULL,
};

static struct state ctrl_state_idle = {
    .state_parent = NULL,
    .state_entry = NULL,
    .transitions = (struct transition[]){
        { EVENT_CONTROL_FN01Pressed,     0,  &ctrl_guard_run_conditon,   NULL, &ctrl_state_acc },
        { EVENT_CONTROL_FN10,  (void*)1, &ctrl_guard_run_conditon,    NULL,  &ctrl_state_acc },
    },
    .transition_nums = 2,
    .data = "idle",
    .action_entry = NULL,
    .action_exti = NULL,
};


static struct state ctrl_state_acc = {
    .state_parent = NULL,
    .state_entry = NULL,
    .transitions = (struct transition[]){
    	{ EVENT_CONTROL_Period,  (void*)0, ctrl_guard_stabled_conditon,    &ctrl_action_acc,  &ctrl_state_stable },
		{ EVENT_CONTROL_EMG,  (void*)0, NULL,    &ctrl_action_emg,  &ctrl_state_dec },
		{ EVENT_CONTROL_STOP,  (void*)0, NULL,    &ctrl_action_emg,  &ctrl_state_dec },
        { EVENT_CONTROL_Period,     0,  NULL,   &ctrl_action_acc, &ctrl_state_acc },
    },
    .transition_nums = 4,
    .data = "acc",
    .action_entry = &ctrl_entry_acc,
    .action_exti = NULL,
};

static struct state ctrl_state_stable = {
    .state_parent = NULL,
    .state_entry = NULL,
    .transitions = (struct transition[]){
    	{ EVENT_CONTROL_Period,  (void*)0, ctrl_guard_speeddown_conditon,    &ctrl_action_stable,  &ctrl_state_dec },
		{ EVENT_CONTROL_EMG,  (void*)0, NULL,    &ctrl_action_emg,  &ctrl_state_dec },
		{ EVENT_CONTROL_STOP,  (void*)0, NULL,    &ctrl_action_emg,  &ctrl_state_dec },
		{ EVENT_CONTROL_Period,  (void*)0, NULL,    &ctrl_action_stable,  &ctrl_state_stable },
    },
    .transition_nums = 4,
    .data = "stable",
    .action_entry = &ctrl_entry_stable,
    .action_exti = NULL,
};

static struct state ctrl_state_dec = {
    .state_parent = NULL,
    .state_entry = NULL,
    .transitions = (struct transition[]){
		{ EVENT_CONTROL_EMG,  (void*)0, NULL,    &ctrl_action_emg,  &ctrl_state_dec },
		{ EVENT_CONTROL_Period,  (void*)0, &ctrl_guard_stoped_conditon,    &ctrl_action_dec,  &ctrl_state_stop },
		{ EVENT_CONTROL_Period,  (void*)0, NULL,    &ctrl_action_dec,  &ctrl_state_dec },
    },
    .transition_nums = 3,
    .data = "dec",
    .action_entry = &ctrl_entry_dec,
    .action_exti = NULL,
};

static struct state ctrl_state_stop = {
    .state_parent = NULL,
    .state_entry = NULL,
    .transitions = (struct transition[]){
		{ EVENT_CONTROL_Period,  (void*)0, NULL,    &ctrl_action_stop,  &ctrl_state_idle },
    },
    .transition_nums = 1,
    .data = "stop",
    .action_entry = NULL,
    .action_exti = NULL,
};

static struct state ctrl_state_error = {
    .data = "error",
    .action_entry = &ctrl_entry_error,
};


static int count  = 0;
static int maxc = 10;
static int stcount = 0;
static int deccount = 0;
static bool ctrl_guard_run_conditon(void *condition, struct event *event)
{
	return true;
}
static bool ctrl_guard_stabled_conditon(void *condition, struct event *event)
{
	if(count + 1 >= maxc)
		return true;
	else
		return false;
}
static bool ctrl_guard_speeddown_conditon(void *condition, struct event *event)
{
	if(stcount + 1 >= maxc)
		return true;
	else
		return false;
}
static bool ctrl_guard_stoped_conditon(void *condition, struct event *event)
{
	if(deccount == 0)
		return true;
	else
		return false;
}

static void ctrl_entry_acc( void *state_data, struct event *event )
{
	count = 0;
	stcount = 0;
	rt_kprintf("enter acc\n");
}

static void ctrl_entry_dec( void *state_data, struct event *event )
{
	//deccount = maxc;
	rt_kprintf("enter dec\n");
}

static void ctrl_entry_stable( void *state_data, struct event *event )
{
	rt_kprintf("enter stable\n");
}

static void ctrl_entry_error( void *state_data, struct event *event )
{
	log_e("control state machine error!");
}

static void ctrl_action_emg( void *oldstate_data, struct event *event, void *state_new_data )
{
	log_e("action error");
}
static void ctrl_action_acc( void *oldstate_data, struct event *event, void *state_new_data )
{
	count ++ ;
	if(count >= maxc){
		count  = maxc;
		stcount = 0 ;
		deccount = maxc;
	}
	rt_kprintf("acc : %d\n",count);
}
static void ctrl_action_dec( void *oldstate_data, struct event *event, void *state_new_data )
{
	deccount -- ;
	if(deccount <= 0)
		deccount = 0 ;
	rt_kprintf("dec : %d\n",deccount);
}
static void ctrl_action_stable( void *oldstate_data, struct event *event, void *state_new_data )
{
	stcount ++ ;
	if(stcount >= maxc)
		stcount = maxc ;
	rt_kprintf("stabled : %d\n",stcount);
}
static void ctrl_action_stop( void *oldstate_data, struct event *event, void *state_new_data )
{
	rt_kprintf("enter stoped\n");
}


static rt_mq_t mq_event_ctrl;
static struct state_machine m_post;

int state_ctrl_event_set(enum event_ctrl_type event, void* data)
{
    struct event e;
    e.type = event;
    e.data = data;
    return rt_mq_send(mq_event_ctrl, &e, sizeof(struct event));
}

static void state_process(void *parameter)
{
    struct event e;
    statem_init( &m_post, &ctrl_state_root, &ctrl_state_error );

    while(1)
    {
        if(RT_EOK == rt_mq_recv(mq_event_ctrl, &e, sizeof(struct event), 500))
        {
            statem_handle_event( &m_post, &(struct event){ e.type, e.data } );
        }
        statem_handle_event( &m_post, &(struct event){ EVENT_CONTROL_Period, 0 } );
    }
}

static int control_state_init(void)
{
    rt_thread_t tid = RT_NULL;

    mq_event_ctrl = rt_mq_create("event_post",sizeof(struct event), 16, RT_IPC_FLAG_FIFO);

    tid = rt_thread_create("state_post", state_process, RT_NULL, 1024, 10, 100);
    if (tid == RT_NULL)
    {
        rt_kprintf("control progress initialize failed! thread create failed!\r\n");

        return -RT_ENOMEM;
    }
    rt_thread_startup(tid);

    return RT_EOK;
}
INIT_APP_EXPORT(control_state_init);

static void state_ctrl_set(uint8_t argc, char **argv)
{
	if(argc == 2)
	{
		if(argv[1][0] == 's')
		{
			state_ctrl_event_set(EVENT_CONTROL_FN10,0);
		}
		else if(argv[1][0] == 'r')
		{
			state_ctrl_event_set(EVENT_CONTROL_EMG,0);
		}
	}
}
MSH_CMD_EXPORT(state_ctrl_set, state key set<a-z>.);
