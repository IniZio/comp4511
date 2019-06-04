/* FILE: kernel/sched/myfifo.c

Note: Symmetric Multiprocessing (SMP) is removed to simplfy the implementation
for the kernel programming teaching and learning purpose

-> make menuconfig
-> Select Processor type and features
-> Select SMP support
-> Press "N" to disable
-> * symbol will be removed
-> Save and Exit
*/

#include "sched.h"

void init_myfifo_rq(struct myfifo_rq *fifo_rq)
{
    /* Equivalent to an empty doubly linked list:
        fifo_rq->queue.prev = &fifo_rq->queue;
        fifo_rq->queue.next = &fifo_rq->queue;  */
	INIT_LIST_HEAD(&fifo_rq->queue);
}

static void
enqueue_task_myfifo(struct rq *rq, struct task_struct *p, int flags)
{
    struct sched_myfifo_entity *myfifo_se = &p->myfifo;

    list_add_tail(&myfifo_se->list, &rq->myfifo.queue);

    printk(KERN_INFO"[SCHED_MYFIFO] ENQUEUE: p->pid=%d, p->policy=%d "
          "curr->pid=%d, curr->policy=%d, flags=%d\n",
	    p->pid, p->policy, rq->curr->pid, rq->curr->policy, flags);

}

static void
dequeue_task_myfifo(struct rq *rq, struct task_struct *p, int flags)
{
    struct sched_myfifo_entity *myfifo_se = &p->myfifo;

    list_del(&myfifo_se->list);

    printk(KERN_INFO"[SCHED_MYFIFO] DEQUEUE: p->pid=%d, p->policy=%d "
            "curr->pid=%d, curr->policy=%d, flags=%d\n",
        p->pid, p->policy, rq->curr->pid, rq->curr->policy, flags);

}

static void yield_task_myfifo(struct rq *rq)
{
    struct sched_myfifo_entity *myfifo_se = &rq->curr->myfifo;
    struct myfifo_rq *myfifo_rq = &rq->myfifo;

    // yield the current task, put it to the end of the queue
    list_move_tail(&myfifo_se->list, &myfifo_rq->queue);

    printk(KERN_INFO"[SCHED_MYFIFO] YIELD: Process-%d\n", rq->curr->pid);
}

static void
check_preempt_curr_myfifo(struct rq *rq,
			struct task_struct *p, int flags)
{
    return; // Not implemented in MyFIFO 
}

// static struct task_struct *pick_next_task_myfifo(struct rq *rq)
static struct task_struct *
pick_next_task_myfifo(struct rq *rq, struct task_struct *prev, struct rq_flags *rf)
{
    struct sched_myfifo_entity *myfifo_se = NULL;
    struct task_struct *p = NULL;
    struct myfifo_rq *myfifo_rq = &rq->myfifo;
    if (list_empty(&myfifo_rq->queue)) {
        return NULL;
    }

    put_prev_task(rq, prev);
    myfifo_se = list_entry(myfifo_rq->queue.next,
                           struct sched_myfifo_entity,
                           list);
    p = container_of(myfifo_se, struct task_struct, myfifo);
    return p;
}

static void put_prev_task_myfifo(struct rq *rq, struct task_struct *p)
{
    /* Not implemented in MyFIFO */
}

static void set_curr_task_myfifo(struct rq *rq)
{
    /* Not implemented in MyFIFO */
}

static void task_tick_myfifo(struct rq *rq, struct task_struct *p,
                             int queued)
{
    /* The function scheduler_tick() is periodically called by the kernel with the frequency HZ, 
    which is the tick rate of the system timer defined on system boot. 
    
    scheduler_tick() calls the current process's sched_class->task_tick(). For example:

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

    return; /* MyFIFO has no timeslice management */
}

unsigned int get_rr_interval_myfifo(struct rq *rq, struct task_struct *p)
{
    /* MyFIFO has no timeslice management */
    return 0;
}

static void
prio_changed_myfifo(struct rq *rq, struct task_struct *p, int oldprio)
{
    return; /* MyFIFO don't support priority */
}

static void switched_to_myfifo(struct rq *rq, struct task_struct *p)
{
    /* nothing to do */
}

static void update_curr_myfifo(struct rq *rq)
{
    /* nothing to do */
}

const struct sched_class myfifo_sched_class = {
    .next = &fair_sched_class,
    .enqueue_task = enqueue_task_myfifo,
    .dequeue_task = dequeue_task_myfifo,
    .yield_task = yield_task_myfifo,
    .check_preempt_curr = check_preempt_curr_myfifo,
    .pick_next_task = pick_next_task_myfifo,
    .put_prev_task = put_prev_task_myfifo,
    .set_curr_task = set_curr_task_myfifo,
    .task_tick = task_tick_myfifo,
    .get_rr_interval = get_rr_interval_myfifo,
    .prio_changed = prio_changed_myfifo,
    .switched_to = switched_to_myfifo,
    .update_curr = update_curr_myfifo,
}; /* Note: Remove SMP support */
