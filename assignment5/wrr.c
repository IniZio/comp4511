/* Location: kernel/sched/wrr.c */
/* include necessary header files */
#include "sched.h"

#define WRR_TIMESLICE  (100 * HZ /1000)

/* implementation of init_wrr_rq */
void init_wrr_rq(struct wrr_rq *wrr)
{
	INIT_LIST_HEAD(&wrr->queue);
}

/* implementation of the weight round-robin scheduler functions(i.e. XXX_wrr functions*/
static void
enqueue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
    struct sched_wrr_entity *wrr_se = &p->wrr;

    list_add_tail(&wrr_se->list, &rq->wrr.queue);

    // printk(KERN_INFO"[SCHED_WRR] ENQUEUE: p->pid=%d, p->policy=%d "
    //       "curr->pid=%d, curr->policy=%d, flags=%d\n",
	//     p->pid, p->policy, rq->curr->pid, rq->curr->policy, flags);

}

static void
dequeue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
    struct sched_wrr_entity *wrr_se = &p->wrr;

    list_del(&wrr_se->list);

    printk(KERN_INFO"[SCHED_WRR] DEQUEUE: p->pid=%d, p->policy=%d "
            "curr->pid=%d, curr->policy=%d, flags=%d\n",
        p->pid, p->policy, rq->curr->pid, rq->curr->policy, flags);

}

static void yield_task_wrr(struct rq *rq)
{
    struct sched_wrr_entity *wrr_se = &rq->curr->wrr;
    struct wrr_rq *wrr_rq = &rq->wrr;

    // yield the current task, put it to the end of the queue
    list_move_tail(&wrr_se->list, &wrr_rq->queue);

    printk(KERN_INFO"[SCHED_WRR] YIELD: Process-%d\n", rq->curr->pid);
}

static void
check_preempt_curr_wrr(struct rq *rq,
			struct task_struct *p, int flags)
{
    return; // Not implemented in WRR 
}

static struct task_struct *
pick_next_task_wrr(struct rq *rq, struct task_struct *prev, struct rq_flags *rf)
{
    struct sched_wrr_entity *wrr_se = NULL;
    struct task_struct *p = NULL;
    struct wrr_rq *wrr_rq = &rq->wrr;
    if (list_empty(&wrr_rq->queue)) {
        return NULL;
    }
    put_prev_task(rq, prev);
    wrr_se = list_entry(wrr_rq->queue.next,
                           struct sched_wrr_entity,
                           list);
    p = container_of(wrr_se, struct task_struct, wrr);
    return p;
}

static void put_prev_task_wrr(struct rq *rq, struct task_struct *p)
{
    /* Not implemented in WRR */
}

static void set_curr_task_wrr(struct rq *rq)
{
    /* Not implemented in WRR */
}

static void task_tick_wrr(struct rq *rq, struct task_struct *p,
                             int queued)
{
    if (p->policy != SCHED_WRR)
        return ;

    /* The function scheduler_tick() is periodically called by the kernel with the frequency HZ, 
    which is the tick rate of the system timer defined on system boot. 
    
    scheduler_tick() calls the current process's sched_class->task_tick(). 
    This function is implemented in the core scheduler (kernel/sched/core.c) 

        void scheduler_tick(void)
        {
            int cpu = smp_processor_id();
            struct rq *rq = cpu_rq(cpu);
            struct task_struct *curr = rq->curr;
            ...
            update_rq_clock(rq);
            curr->sched_class->task_tick(rq, curr, 0);
            ...
        } 
    */

   /* Round-robin has a time slice management here... */
    printk(KERN_INFO"[SCHED_WRR] Task Tick: Process-%d = %d\n", p->pid, p->wrr.time_slice);

    /* Case 1: 
    Decrease time_slice first. If time_slice > 0, keep running */
	if(--p->wrr.time_slice)
		return;

    /* Case 2:
    time_slice equals to 0

		1. replenish the time_slice with a default time slice (assume: 10)
		2. remove the current one to the tail of the queue
		3. if there is another task in the queue, then set the need_reched flag

	*/
	p->wrr.time_slice = WRR_TIMESLICE * p->wrr.weight;
	list_move_tail(&p->wrr.list, &rq->wrr.queue);
	set_tsk_need_resched(p);

}

unsigned int get_rr_interval_wrr(struct task_struct *task){
        /*
 *          * Time slice is 0 for SCHED_FIFO tasks
 *                   */
        if (task->policy == SCHED_WRR)
                return WRR_TIMESLICE;
        else
                return 0;
}

static void
prio_changed_wrr(struct rq *rq, struct task_struct *p, int oldprio)
{
    return; /* WRR don't support priority */
}

static void switched_to_wrr(struct rq *rq, struct task_struct *p)
{
    /* nothing to do */
}

static void update_curr_wrr(struct rq *rq)
{
    /* nothing to do */
}


/* WRR scheduler class definition */
const struct sched_class wrr_sched_class = {
  .next = &wrr_sched_class,
  .enqueue_task = enqueue_task_wrr,
  .dequeue_task = dequeue_task_wrr,
  .yield_task = yield_task_wrr,
  .check_preempt_curr = check_preempt_curr_wrr,
  .pick_next_task = pick_next_task_wrr,
  .put_prev_task = put_prev_task_wrr,
  .set_curr_task = set_curr_task_wrr,
  .task_tick = task_tick_wrr,
  .get_rr_interval = get_rr_interval_wrr,
  .prio_changed = prio_changed_wrr,
  .switched_to = switched_to_wrr,
  .update_curr = update_curr_wrr,
};// /* Note: Remove SMP support */// It is recommended to use SYSCALL_DEFINEn marcoSYSCALL_DEFINE2(set_wrr_weight, pid_t, pid, int, weight){/* implementation of set_wrr_weight syscall */}


SYSCALL_DEFINE2(set_wrr_weight, pid_t, pid, int, weight) {
    struct sched_wrr_entity *wrr_se = &find_task_by_vpid(pid)->wrr;

    wrr_se->wrr_weight = weight;
    return 0;
}
