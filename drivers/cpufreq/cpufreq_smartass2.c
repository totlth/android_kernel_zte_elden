/*
 * drivers/cpufreq/cpufreq_smartass2.c
 *
 * Copyright (C) 2010 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Author: Erasmux
 *
 * Based on the interactive governor By Mike Chan (mike@android.com)
 * which was adaptated to 2.6.29 kernel by Nadlabak (pavel@doshaska.net)
 *
 * SMP support based on mod by faux123
 *
 * For a general overview of smartassV2 see the relavent part in
 * Documentation/cpu-freq/governors.txt
 *
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>  //tcd end

#include <linux/cpu.h>
#include <linux/jiffies.h>


#include <linux/cpumask.h>
#include <linux/cpufreq.h>
#include <linux/sched.h>
#include <linux/slab.h>  //tcd
#include <linux/hrtimer.h> //tcd
#include <linux/tick.h>
#include <linux/ktime.h> //tcd

//#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/moduleparam.h>
//#include <asm/cputime.h>
#include <linux/earlysuspend.h>
#include <linux/rq_stats.h>

#define cputime64_add(__a, __b)         ((__a) + (__b))
#define cputime64_sub(__a, __b)         ((__a) - (__b))

/******************** Tunable parameters: ********************/

/*
 * The "ideal" frequency to use when awake. The governor will ramp up faster
 * towards the ideal frequency and slower after it has passed it. Similarly,
 * lowering the frequency towards the ideal frequency is faster than below it.
 */
#define DEFAULT_AWAKE_IDEAL_FREQ 918000
static unsigned int awake_ideal_freq;

/*
 * The "ideal" frequency to use when suspended.
 * When set to 0, the governor will not track the suspended state (meaning
 * that practically when sleep_ideal_freq==0 the awake_ideal_freq is used
 * also when suspended).
 */
#define DEFAULT_SLEEP_IDEAL_FREQ 486000
static unsigned int sleep_ideal_freq;

/*
 * Freqeuncy delta when ramping up above the ideal freqeuncy.
 * Zero disables and causes to always jump straight to max frequency.
 * When below the ideal freqeuncy we always ramp up to the ideal freq.
 */
#define DEFAULT_RAMP_UP_STEP 108000
static unsigned int ramp_up_step;

/*
 * Freqeuncy delta when ramping down below the ideal freqeuncy.
 * Zero disables and will calculate ramp down according to load heuristic.
 * When above the ideal freqeuncy we always ramp down to the ideal freq.
 */
//#define DEFAULT_RAMP_DOWN_STEP 216000 //tcd
#define DEFAULT_RAMP_DOWN_STEP 0
static unsigned int ramp_down_step;

/*
 * CPU freq will be increased if measured load > max_cpu_load;
 */
#define DEFAULT_MAX_CPU_LOAD 50
static unsigned long max_cpu_load;

/*
 * CPU freq will be decreased if measured load < min_cpu_load;
 */
#define DEFAULT_MIN_CPU_LOAD 25
static unsigned long min_cpu_load;

static unsigned long max_cpu_load_hifreq = 80;
static unsigned long min_cpu_load_hifreq = 40;

/*
 * The minimum amount of time to spend at a frequency before we can ramp up.
 * Notice we ignore this when we are below the ideal frequency.
 */
#define DEFAULT_UP_RATE_US 48000
static unsigned long up_rate_us;

//tcd add start todo
#define DEFAULT_FREQ_UP_REATE_FPS 40
//static unsigned long freq_up_fps = 40;

#define DEFAULT_FREQ_DOWN_REATE_FPS 50
//static unsigned long freq_down_fps = 50;

//tcd add end

/*
 * The minimum amount of time to spend at a frequency before we can ramp down.
 * Notice we ignore this when we are above the ideal frequency.
 */
#define DEFAULT_DOWN_RATE_US 39000; //tcd change from 99->39
static unsigned long down_rate_us;

/*
 * The frequency to set when waking up from sleep.
 * When sleep_ideal_freq=0 this will have no effect.
 */
#define DEFAULT_SLEEP_WAKEUP_FREQ 99999999
static unsigned int sleep_wakeup_freq;

/*
 * Sampling rate, I highly recommend to leave it at 2.
 */
#define DEFAULT_SAMPLE_RATE_JIFFIES 2
static unsigned int sample_rate_jiffies;

//tcd add start
#define MAX_HOTPLUG_RATE			(30u)

#define DEF_CPU_UP_RATE				(10)
#define DEF_CPU_DOWN_RATE			(15)
static unsigned int cpu_up_rate;
static unsigned int cpu_down_rate;

unsigned int max_cpu_lock = 0;
unsigned int min_cpu_lock = 0;
static int early_suspend;

#if 1
static int hotplug_rq[4][2] = {
	{0, 199}, {199, 200}, {200, 300}, {300, 0}
};

static int hotplug_freq[4][2] = {
	{0, 1026000},
	{918000, 500000},
	{200000, 500000},
	{200000, 0}
};
#if 0
static int hotplug_fps[4][2] = {
	{0, 40},
	{50,40},
	{50,40},
	{50,0}
};
#endif
#endif

#define HOTPLUG_DOWN_INDEX			(0)
#define HOTPLUG_UP_INDEX			(1)
#define EARLYSUSPEND_HOTPLUGLOCK 1

//tcd end
/*************** End of tunables ***************/


static void (*pm_idle_old)(void);
static atomic_t active_count = ATOMIC_INIT(0);
//tcd add start
static atomic_t g_hotplug_lock = ATOMIC_INIT(0);
static atomic_t g_hotplug_count = ATOMIC_INIT(0);

static atomic_t g_cpudown_lock = ATOMIC_INIT(1);

static atomic_t hotplug_lock = ATOMIC_INIT(0);
static atomic_t fps_lock = ATOMIC_INIT(0);
//tcd add end
struct smartass_info_s {
	struct cpufreq_policy *cur_policy;
	struct cpufreq_frequency_table *freq_table;
	struct timer_list timer;
	u64 time_in_idle;
	u64 idle_exit_time;
	u64 freq_change_time;
	u64 freq_change_time_in_idle;
	int cur_cpu_load;
	int old_freq;
	int ramp_dir;
	unsigned int enable;
	int ideal_speed;
};
static DEFINE_PER_CPU(struct smartass_info_s, smartass_info);

/* Workqueues handle frequency scaling */
static struct workqueue_struct *up_wq;
static struct workqueue_struct *down_wq;
static struct work_struct freq_scale_work;
//tcd add
static struct workqueue_struct *updown_wq;
static struct work_struct up_work;
static struct work_struct down_work;

//tcd end
static cpumask_t work_cpumask;
static spinlock_t cpumask_lock;

static unsigned int suspended;

#define dprintk(flag,msg...) do { \
	if (debug_mask & flag) printk(KERN_DEBUG msg); \
	} while (0)

enum {
	SMARTASS_DEBUG_JUMPS=1,
	SMARTASS_DEBUG_LOAD=2,
	SMARTASS_DEBUG_ALG=4,
	SMARTASS_DEBUG_UPDOWN=8,
};

#define LATENCY_MULTIPLIER			(1000)
#define MIN_LATENCY_MULTIPLIER			(100)
#define TRANSITION_LATENCY_LIMIT		(10 * 1000 * 1000)	

/*
 * Combination of the above debug flags.
 */
static unsigned long debug_mask;

static int cpufreq_governor_smartass(struct cpufreq_policy *policy,
		unsigned int event);
extern void set_fps_ok(void);
extern int is_fps_ok(void);

#ifndef CONFIG_CPU_FREQ_DEFAULT_GOV_SMARTASS2
static
#endif
struct cpufreq_governor cpufreq_gov_smartass2 = {
	.name = "smartassV2",
	.governor = cpufreq_governor_smartass,
	.max_transition_latency = TRANSITION_LATENCY_LIMIT,
	.owner = THIS_MODULE,
};

inline static void smartass_update_min_max(struct smartass_info_s *this_smartass, struct cpufreq_policy *policy, int suspend) {
	if (suspend) {
		this_smartass->ideal_speed = // sleep_ideal_freq; but make sure it obeys the policy min/max
			policy->max > sleep_ideal_freq ?
			(sleep_ideal_freq > policy->min ? sleep_ideal_freq : policy->min) : policy->max;
	} else {
		this_smartass->ideal_speed = // awake_ideal_freq; but make sure it obeys the policy min/max
			policy->min < awake_ideal_freq ?
			(awake_ideal_freq < policy->max ? awake_ideal_freq : policy->max) : policy->min;
	}
}

inline static void smartass_update_min_max_allcpus(void) {
	unsigned int i;
	for_each_online_cpu(i) {
		struct smartass_info_s *this_smartass = &per_cpu(smartass_info, i);
		if (this_smartass->enable)
			smartass_update_min_max(this_smartass,this_smartass->cur_policy,suspended);
	}
}

inline static unsigned int validate_freq(struct cpufreq_policy *policy, int freq) {
	if (freq > (int)policy->max)
		return policy->max;
	if (freq < (int)policy->min)
		return policy->min;
	return freq;
}

inline static void reset_timer(unsigned long cpu, struct smartass_info_s *this_smartass) {
	this_smartass->time_in_idle = get_cpu_idle_time_us(cpu, &this_smartass->idle_exit_time);
//	dprintk(SMARTASS_DEBUG_LOAD,"reset_timer %u\n",sample_rate_jiffies); 
	if(cpu_online(cpu))
	mod_timer(&this_smartass->timer, jiffies + sample_rate_jiffies);
}

inline static void work_cpumask_set(unsigned long cpu) {
	unsigned long flags;
	spin_lock_irqsave(&cpumask_lock, flags);
	cpumask_set_cpu(cpu, &work_cpumask);
	spin_unlock_irqrestore(&cpumask_lock, flags);
}

inline static int work_cpumask_test_and_clear(unsigned long cpu) {
	unsigned long flags;
	int res = 0;
	spin_lock_irqsave(&cpumask_lock, flags);
	res = cpumask_test_and_clear_cpu(cpu, &work_cpumask);
	spin_unlock_irqrestore(&cpumask_lock, flags);
	return res;
}

inline static int target_freq(struct cpufreq_policy *policy, struct smartass_info_s *this_smartass,
			      int new_freq, int old_freq, int prefered_relation) {
	int index, target;
	struct cpufreq_frequency_table *table = this_smartass->freq_table;

	if (new_freq == old_freq)
		return 0;
	new_freq = validate_freq(policy,new_freq);
	if (new_freq == old_freq)
		return 0;

	if (table &&
	    !cpufreq_frequency_table_target(policy,table,new_freq,prefered_relation,&index))
	{
		target = table[index].frequency;
		if (target == old_freq) {
			// if for example we are ramping up to *at most* current + ramp_up_step
			// but there is no such frequency higher than the current, try also
			// to ramp up to *at least* current + ramp_up_step.
			if (new_freq > old_freq && prefered_relation==CPUFREQ_RELATION_H
			    && !cpufreq_frequency_table_target(policy,table,new_freq,
							       CPUFREQ_RELATION_L,&index))
				target = table[index].frequency;
			// simlarly for ramping down:
			else if (new_freq < old_freq && prefered_relation==CPUFREQ_RELATION_L
				&& !cpufreq_frequency_table_target(policy,table,new_freq,
								   CPUFREQ_RELATION_H,&index))
				target = table[index].frequency;
		}

		if (target == old_freq) {
			// We should not get here:
			// If we got here we tried to change to a validated new_freq which is different
			// from old_freq, so there is no reason for us to remain at same frequency.
			printk(KERN_WARNING "Smartass: frequency change failed: %d to %d => %d\n",
			       old_freq,new_freq,target);
			return 0;
		}
	}
	else target = new_freq;

	__cpufreq_driver_target(policy, target, prefered_relation);

	dprintk(SMARTASS_DEBUG_JUMPS,"SmartassQ: jumping from %d to %d => %d (%d)\n",
		old_freq,new_freq,target,policy->cur);

	return target;
}

//tcd add start
/*
 * History of CPU usage
 */
struct cpu_usage {
	unsigned int freq;
	unsigned int rq;  //tcd add start --end
	unsigned int load[NR_CPUS];
	unsigned int rq_avg;
	int64_t time;
};

struct cpu_usage_history {
	struct cpu_usage usage[MAX_HOTPLUG_RATE];
	unsigned int num_hist;
	int flag;
};

struct cpu_usage_history *hotplug_history;
static int fps_fail_freq_up = 0;
static int fps_ok_cpu_up_fail = 0;
static int fps_ok_cpu_down_ok = 0;
static int total_cpu_up = 0;
static int total_freq_up = 0;


static int check_up(void)
{
	int num_hist = hotplug_history->num_hist;
	//struct cpu_usage *usage;
	int freq;
	int up_rate = cpu_up_rate;
	int up_freq, up_rq;
	int min_freq = INT_MAX;
	int online;
	int hotplug_lock = atomic_read(&g_hotplug_lock);
//tcd add start
	unsigned int avg_load = 0;
	unsigned int rq = 0;
	cputime64_t time_diff = 0;
//tcd end	
	if (hotplug_lock > 0)
		return 0;
	
	online = num_online_cpus();
	up_freq = hotplug_freq[online - 1][HOTPLUG_UP_INDEX];
	up_rq = hotplug_rq[online - 1][HOTPLUG_UP_INDEX];
	
	if (online == num_possible_cpus())
		return 0;
	
	if (max_cpu_lock != 0
		&& online >= max_cpu_lock)
		return 0;
	
	if (min_cpu_lock != 0
		&& online < min_cpu_lock)
		return 1;

	if (hotplug_history->num_hist <= 1 )
		return 0;

	min_freq = hotplug_history->usage[num_hist-1].freq;
	avg_load = hotplug_history->usage[num_hist-1].load[0];
	rq = hotplug_history->usage[num_hist-1].rq;

	for(up_rate = 1; num_hist - 1 - up_rate >= 0; up_rate++){
		int prev = 0;
		prev = num_hist -1 - up_rate;

		time_diff = cputime64_sub((cputime64_t)hotplug_history->usage[num_hist-1].time , 
			(cputime64_t)hotplug_history->usage[prev].time);
		
		min_freq = min(min_freq, freq);
		avg_load+= hotplug_history->usage[prev].load[0];
		rq+=hotplug_history->usage[prev].rq;

		if ( time_diff > 120*1000 )//200ms
			break;
	}
	if ((time_diff <= 120*1000) || (up_rate == cpu_up_rate))
		return 0;

	avg_load = avg_load/(up_rate+1);
	rq = rq*100/(up_rate+1);

	if (is_fps_ok()==0){
	    if ((avg_load > 80) && (rq > up_rq)) {
		dprintk(SMARTASS_DEBUG_UPDOWN, "[HOTPLUG IN OK] %s %d<=%d && %u>%d && load %u\n",
			__func__, min_freq, up_freq, rq, up_rq, avg_load);
		hotplug_history->num_hist = 0;
		set_fps_ok();//tcd
		return 1;
	    } else
		dprintk(SMARTASS_DEBUG_ALG, "[HOTPLUG IN FAIL %d] %s %d<=%d && %u>%d && load %u, fps_deny_cpu_up %d times\n",
			is_fps_ok(), __func__, min_freq, up_freq, rq, up_rq, avg_load, fps_ok_cpu_up_fail);

	} else {
		fps_ok_cpu_up_fail++;
	}
	hotplug_history->num_hist = 0;

	return 0;
}


static int check_down(void)
{
	int num_hist = hotplug_history->num_hist;
	//struct cpu_usage *usage;
	int freq;//, rq_avg;
	//int i;
	int down_rate = cpu_down_rate;
	int down_freq, down_rq;
	int max_freq = 0;
	unsigned int avg_load = 0;
//	int max_rq_avg = 0;
	int online;
	int hotplug_lock = atomic_read(&g_hotplug_lock);
	static int cnt=0;

//tcd add start
	unsigned int rq = 0;
	cputime64_t time_diff = 0;
//tcd end	
	cnt++;
	if (hotplug_lock > 0)
		return 0;
	
	online = num_online_cpus();
	down_freq = hotplug_freq[online - 1][HOTPLUG_DOWN_INDEX];
	down_rq = hotplug_rq[online - 1][HOTPLUG_DOWN_INDEX];

	if (online == 1)
		return 0;

	if (max_cpu_lock != 0
		&& online > max_cpu_lock)
		return 1;

	if (min_cpu_lock != 0
		&& online <= min_cpu_lock)
		return 0;

	if (hotplug_history->num_hist <= 1 )
		return 0;

	max_freq = hotplug_history->usage[num_hist-1].freq;
	avg_load = hotplug_history->usage[num_hist-1].load[0];
	rq = hotplug_history->usage[num_hist-1].rq;

	for(down_rate = 1; num_hist - 1 - down_rate >= 0; down_rate++){
		int prev = num_hist -1 - down_rate;

		time_diff = cputime64_sub((cputime64_t)hotplug_history->usage[num_hist-1].time , 
			(cputime64_t)hotplug_history->usage[prev].time);
		
		max_freq = max(max_freq, freq);
		avg_load+= hotplug_history->usage[prev].load[0];
		rq+=hotplug_history->usage[prev].rq;

		if ( time_diff > 220*1000 )//200ms
			break;
	}
	if ((time_diff <= 200*1000) || (down_rate == cpu_down_rate))
		return 0;

//tcd add start
	avg_load = avg_load/(down_rate+1);
	rq = rq*100/(down_rate+1);

	if ((rq < down_rq) || ( max_freq <= down_freq) || (avg_load < 30)) {
//tcd end
		dprintk(SMARTASS_DEBUG_UPDOWN, "[HOTPLUG OUT OK] %s %d<=%d && %u<%d && load %u\n",
			__func__, max_freq, down_freq, rq, down_rq, avg_load);
		hotplug_history->num_hist = 0;
		return 1;
	}
	if (is_fps_ok() > 0){
		fps_ok_cpu_down_ok++;
		dprintk(SMARTASS_DEBUG_UPDOWN, "[HOTPLUG OUT fps(%d)] %s %d<=%d && %u<%d fps_allow_cpu_down %d times, load %u, timediff=%llu\n",
			is_fps_ok(), __func__, max_freq, down_freq, rq, down_rq, fps_ok_cpu_down_ok, avg_load, time_diff);
		hotplug_history->num_hist = 0;
		return 1;
	
	}
	set_fps_ok();//tcd
	hotplug_history->num_hist = 0;

	dprintk(SMARTASS_DEBUG_UPDOWN, "[HOTPLUG OUT FAIL %d] %s %d>=%d && [%u/%llu] load %u\n",
			cnt, __func__, max_freq, down_freq, rq, time_diff, avg_load);
	return 0;
}

//tcd add end
static void apply_hotplug_lock(void)
{
	int online, possible, lock, flag;
	struct work_struct *work;

	/* do turn_on/off cpus */
	online = num_online_cpus();
	possible = num_possible_cpus();
	lock = atomic_read(&g_hotplug_lock);
	flag = lock - online;

	if (flag == 0)
		return;

	work = flag > 0 ? &up_work : &down_work;

	pr_debug("%s online %d possible %d lock %d flag %d %d\n",
		 __func__, online, possible, lock, flag, (int)abs(flag));
	if ( flag > 0)
		queue_work_on(0, up_wq, work);
	else
		queue_work_on(0, down_wq, work);
}

/*
  * we add several fps condition to help when adjust freq 
  * 1. > 918(ideal freq), if this freq could achieve fps interval ms < fps_lock ms or 
  * 2. if no screen update for 500ms , switch to 384M and stop freq adjustment until fps starts
  * 3. Advance : for specific app, remember high freq avg fps interval, 
  *     when go low freq, if its interval is same, adjust ideal freq to this ideal freq, after adjust if high-freq get better interval, adjust ideal freq up
  */
static void cpufreq_smartass_timer(unsigned long cpu)
{
	cputime64_t delta_idle;
	cputime64_t delta_time;
	int cpu_load;
	int old_freq;
	u64 update_time;
	u64 now_idle;
	int queued_work = 0;
	struct smartass_info_s *this_smartass = &per_cpu(smartass_info, cpu);
	struct cpufreq_policy *policy = this_smartass->cur_policy;

	int max_hotplug_rate = max(cpu_up_rate, cpu_down_rate);
//tcd add start
	
	now_idle = get_cpu_idle_time_us(cpu, &update_time);

	delta_idle = cputime64_sub((cputime64_t)now_idle, (cputime64_t)this_smartass->time_in_idle);
	delta_time = cputime64_sub((cputime64_t)update_time, (cputime64_t)this_smartass->idle_exit_time);

//tcd add end

	old_freq = policy->cur;

	if (this_smartass->idle_exit_time == 0 || update_time == this_smartass->idle_exit_time){
		return;
	}

	// If timer ran less than 1ms after short-term sample started, retry.
	if (delta_time < 1000) {
		if (!timer_pending(&this_smartass->timer))
			reset_timer(cpu,this_smartass);
		return;
	}

	if (delta_idle > delta_time)
		cpu_load = 0;
	else
		cpu_load = 100 * (unsigned int)(delta_time - delta_idle) / (unsigned int)delta_time;

if (cpu == 0) {
	int num_hist = hotplug_history->num_hist;

	hotplug_history->usage[num_hist].freq = policy->cur;
	hotplug_history->usage[num_hist].rq = nr_running();
	hotplug_history->usage[num_hist].time = ktime_to_us(ktime_get());
	hotplug_history->usage[num_hist].load[0] = cpu_load;
	
	++hotplug_history->num_hist;
}


	dprintk(SMARTASS_DEBUG_LOAD,"(%lu)smartassT @ %d: load %d (delta_time %llu)\n",cpu,
		old_freq,cpu_load,delta_time);

	this_smartass->cur_cpu_load = cpu_load;
	this_smartass->old_freq = old_freq;
//tcd add start
	/* Check for CPU hotplug */
if (cpu == 0){
	if (check_up()) {
		total_cpu_up++;
		queue_work_on(0, updown_wq, &up_work);
	} else if (check_down()) {
		queue_work_on(0, updown_wq, &down_work);
	}
	if (hotplug_history->num_hist == max_hotplug_rate)
		hotplug_history->num_hist = 0;
}
//tcd end		

	// Scale up if load is above max or if there where no idle cycles since coming out of idle,
	// additionally, if we are at or above the ideal_speed, verify we have been at this frequency
	// for at least up_rate_us:
	
	if ((cpu_load > max_cpu_load && old_freq < this_smartass->ideal_speed) || 
	   (cpu_load > max_cpu_load_hifreq && old_freq >= this_smartass->ideal_speed) || delta_idle == 0)
//	if (cpu_load > max_cpu_load || delta_idle == 0)
	{
		if (old_freq < policy->max &&
			 (old_freq < this_smartass->ideal_speed || delta_idle == 0 ||
			  cputime64_sub(update_time, this_smartass->freq_change_time) >= up_rate_us))
		{
		    if (is_fps_ok() == 0){//tcd
			dprintk(SMARTASS_DEBUG_ALG,"smartassT @ %d ramp up: load %d (delta_idle %llu)\n",
				old_freq,cpu_load,delta_idle);
			this_smartass->ramp_dir = 1;
			work_cpumask_set(cpu);
			queue_work(up_wq, &freq_scale_work);
			queued_work = 1;
		    } else {//tcd
			this_smartass->ramp_dir = 0;
			fps_fail_freq_up++;
		    }
		    total_freq_up++;
			
		}
		else this_smartass->ramp_dir = 0;
	}
	// Similarly for scale down: load should be below min and if we are at or below ideal
	// frequency we require that we have been at this frequency for at least down_rate_us:
	else if ((cpu_load < min_cpu_load_hifreq && old_freq > policy->min && (old_freq > this_smartass->ideal_speed)) ||
		(( cputime64_sub(update_time, this_smartass->freq_change_time) >= down_rate_us) && ((cpu_load < min_cpu_load) && (old_freq <= this_smartass->ideal_speed))) ||
		((is_fps_ok() > 0) && (old_freq > this_smartass->ideal_speed)))
	{
		dprintk(SMARTASS_DEBUG_ALG,"smartassT @ %d ramp down: load %d (delta_idle %llu)\n",
			old_freq,cpu_load,delta_idle);
		this_smartass->ramp_dir = -1;
		work_cpumask_set(cpu);
		queue_work(down_wq, &freq_scale_work);
		queued_work = 1;
	}
	else this_smartass->ramp_dir = 0;

	// To avoid unnecessary load when the CPU is already at high load, we don't
	// reset ourselves if we are at max speed. If and when there are idle cycles,
	// the idle loop will activate the timer.
	// Additionally, if we queued some work, the work task will reset the timer
	// after it has done its adjustments.
	if (!queued_work && old_freq < policy->max)
		reset_timer(cpu,this_smartass);
}

static void cpufreq_idle(void)
{
	struct smartass_info_s *this_smartass = &per_cpu(smartass_info, smp_processor_id());
	struct cpufreq_policy *policy = this_smartass->cur_policy;

	if (!this_smartass->enable) {
		pm_idle_old();
		return;
	}

	if (policy->cur == policy->min && timer_pending(&this_smartass->timer)){
		del_timer(&this_smartass->timer);
	}

	pm_idle_old();

	if (!timer_pending(&this_smartass->timer)){
		reset_timer(smp_processor_id(), this_smartass);
	}
}

//tcd add start
static void __ref cpu_up_work(struct work_struct *work)
{
//		struct smartass_info_s *this_smartass = &per_cpu(smartass_info, 1);

		dprintk(SMARTASS_DEBUG_UPDOWN, "CPU_UP 1 start\n");
		cpu_up(1);
//		printk(SMARTASS_DEBUG_UPDOWN, "CPU_UP end\n");
		set_fps_ok();//tcd
#if 0
	int cpu;
	int online = num_online_cpus();
	int nr_up = 1;
	int min_cpu_lock = min_cpu_lock;
	int hotplug_lock = atomic_read(&g_hotplug_lock);

	if (hotplug_lock && min_cpu_lock)
		nr_up = max(hotplug_lock, min_cpu_lock) - online;
	else if (hotplug_lock)
		nr_up = hotplug_lock - online;
	else if (min_cpu_lock)
		nr_up = max(nr_up, min_cpu_lock - online);

	if (online == 1) {
		printk(SMARTASS_DEBUG_UPDOWN, "CPU_UP 3\n");
		cpu_up(num_possible_cpus() - 1);
		nr_up -= 1;
	}

	for_each_cpu_not(cpu, cpu_online_mask) {
		if (nr_up-- == 0)
			break;
		if (cpu == 0)
			continue;
		printk(SMARTASS_DEBUG_UPDOWN, "CPU_UPP %d\n", cpu);
		cpu_up(cpu);
	}
#endif
}

static void cpu_down_work(struct work_struct *work)
{
	int cpu;
	int online = num_online_cpus();
	int nr_down = 1;
	int hotplug_lock = atomic_read(&g_hotplug_lock);

	if (hotplug_lock)
		nr_down = online - hotplug_lock;

	if (atomic_dec_and_test(&g_cpudown_lock)){
//		printk(KERN_ERR "CPU_DOWN start\n");
		for_each_online_cpu(cpu) {
		if (cpu == 0)
			continue;
		dprintk(SMARTASS_DEBUG_UPDOWN, "CPU_DOWN %d\n", cpu);
		cpu_down(cpu);
		if (--nr_down == 0)
			break;
		}
//		printk(KERN_ERR "CPU_DOWN end\n");
		set_fps_ok();//tcd
	} else {
//		printk(KERN_ERR "tcd cpu down failed of freq\n");
	}
	atomic_inc(&g_cpudown_lock);
}


/* We use the same work function to sale up and down */
static void cpufreq_smartass_freq_change_time_work(struct work_struct *work)
{
	unsigned int cpu;
	int new_freq;
	int old_freq;
	int ramp_dir;
	struct smartass_info_s *this_smartass;
	struct cpufreq_policy *policy;
	unsigned int relation = CPUFREQ_RELATION_L;

	for_each_possible_cpu(cpu) {
		this_smartass = &per_cpu(smartass_info, cpu);
		if (!work_cpumask_test_and_clear(cpu))
			continue;
//tcd add this
		if (cpu == 1 && !atomic_dec_and_test(&g_cpudown_lock)){
			atomic_inc(&g_cpudown_lock);
//			printk(KERN_ERR "tcd cpu freq failed due to cpu is down\n");
			continue;
		}
		if (!cpu_online(cpu)){
		     	if(cpu==1) atomic_inc(&g_cpudown_lock);
//			printk(KERN_ERR "tcd do nothing for a offline cpu\n");
			continue;
		}
///tcd add end	

		ramp_dir = this_smartass->ramp_dir;
		this_smartass->ramp_dir = 0;

		old_freq = this_smartass->old_freq;
		policy = this_smartass->cur_policy;

		if (old_freq != policy->cur) {
			// frequency was changed by someone else?
			printk(KERN_WARNING "Smartass: frequency changed by 3rd party: %d to %d\n",
			       old_freq,policy->cur);
			new_freq = old_freq;
		}
		else if (ramp_dir > 0 && nr_running() > 1) {
			// ramp up logic:
			if (old_freq < this_smartass->ideal_speed)
				new_freq = this_smartass->ideal_speed;
			else if (ramp_up_step) {
				new_freq = old_freq + ramp_up_step;
				relation = CPUFREQ_RELATION_H;
				if (num_online_cpus() == 1)
					set_fps_ok();//tcd
			}
			else {
				new_freq = policy->max;
				relation = CPUFREQ_RELATION_H;
				if (num_online_cpus() == 1)
					set_fps_ok();//tcd
			}
			dprintk(SMARTASS_DEBUG_ALG,"smartassQ @ %d ramp up: ramp_dir=%d ideal=%d\n",
				old_freq,ramp_dir,this_smartass->ideal_speed);
		}
		else if (ramp_dir < 0) {
			// ramp down logic:
			if (old_freq > this_smartass->ideal_speed) {
				new_freq = this_smartass->ideal_speed;
				relation = CPUFREQ_RELATION_H;
				if (num_online_cpus() == 1)
					set_fps_ok();//tcd
			}
			else if (ramp_down_step)
				new_freq = old_freq - ramp_down_step;
			else {
				// Load heuristics: Adjust new_freq such that, assuming a linear
				// scaling of load vs. frequency, the load in the new frequency
				// will be max_cpu_load:
				new_freq = old_freq * this_smartass->cur_cpu_load / max_cpu_load;
				if (new_freq > old_freq) // min_cpu_load > max_cpu_load ?!
					new_freq = old_freq -1;
			}
			dprintk(SMARTASS_DEBUG_ALG,"smartassQ @ %d ramp down: ramp_dir=%d ideal=%d\n",
				old_freq,ramp_dir,this_smartass->ideal_speed);
		}
		else { // ramp_dir==0 ?! Could the timer change its mind about a queued ramp up/down
		       // before the work task gets to run?
		       // This may also happen if we refused to ramp up because the nr_running()==1
			new_freq = old_freq;
			dprintk(SMARTASS_DEBUG_ALG,"smartassQ @ %d nothing: ramp_dir=%d nr_running=%lu\n",
				old_freq,ramp_dir,nr_running());
		}

		// do actual ramp up (returns 0, if frequency change failed):
		new_freq = target_freq(policy,this_smartass,new_freq,old_freq,relation);
		if (new_freq)
			this_smartass->freq_change_time_in_idle =
				get_cpu_idle_time_us(cpu,&this_smartass->freq_change_time);

		// reset timer:
		if (new_freq < policy->max)
			reset_timer(cpu,this_smartass);
		// if we are maxed out, it is pointless to use the timer
		// (idle cycles wake up the timer when the timer comes)
		else if (timer_pending(&this_smartass->timer))
			del_timer(&this_smartass->timer);

		if (cpu == 1)
			atomic_inc(&g_cpudown_lock);
	}
}

static ssize_t show_debug_mask(struct kobject *kobj, struct attribute *attr, char *buf)
{
	return sprintf(buf, "%lu\n", debug_mask);
}

static ssize_t store_debug_mask(struct kobject *kobj, struct attribute *attr, const char *buf, size_t count)
{
	ssize_t res;
	unsigned long input;
	
	res = strict_strtoul(buf, 0, &input);
	if (res >= 0)
		debug_mask = input;
	return count;
}

static ssize_t show_up_rate_us(struct kobject *kobj, struct attribute *attr, char *buf)
{
	return sprintf(buf, "%lu\n", up_rate_us);
}

static ssize_t store_up_rate_us(struct kobject *kobj, struct attribute *attr, const char *buf, size_t count)
{
	ssize_t res;
	unsigned long input;
	res = strict_strtoul(buf, 0, &input);
	if (res >= 0 && input >= 0 && input <= 100000000)
		up_rate_us = input;
	return count;
}

static ssize_t show_down_rate_us(struct kobject *kobj, struct attribute *attr, char *buf)
{
	return sprintf(buf, "%lu\n", down_rate_us);
}

static ssize_t store_down_rate_us(struct kobject *kobj, struct attribute *attr, const char *buf, size_t count)
{
	ssize_t res;
	unsigned long input;
	res = strict_strtoul(buf, 0, &input);
	if (res >= 0 && input >= 0 && input <= 100000000)
		down_rate_us = input;
	return count;
}

static ssize_t show_sleep_ideal_freq(struct kobject *kobj, struct attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", sleep_ideal_freq);
}

static ssize_t store_sleep_ideal_freq(struct kobject *kobj, struct attribute *attr, const char *buf, size_t count)
{
	ssize_t res;
	unsigned long input;
	res = strict_strtoul(buf, 0, &input);
	if (res >= 0 && input >= 0) {
		sleep_ideal_freq = input;
		if (suspended)
			smartass_update_min_max_allcpus();
	}
	return count;
}

static ssize_t show_sleep_wakeup_freq(struct kobject *kobj, struct attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", sleep_wakeup_freq);
}

static ssize_t store_sleep_wakeup_freq(struct kobject *kobj, struct attribute *attr, const char *buf, size_t count)
{
	ssize_t res;
	unsigned long input;
	res = strict_strtoul(buf, 0, &input);
	if (res >= 0 && input >= 0)
		sleep_wakeup_freq = input;
	return count;
}

static ssize_t show_awake_ideal_freq(struct kobject *kobj, struct attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", awake_ideal_freq);
}

static ssize_t store_awake_ideal_freq(struct kobject *kobj, struct attribute *attr, const char *buf, size_t count)
{
	ssize_t res;
	unsigned long input;
	res = strict_strtoul(buf, 0, &input);
	if (res >= 0 && input >= 0) {
		awake_ideal_freq = input;
		if (!suspended)
			smartass_update_min_max_allcpus();
	}
	return count;
}

static ssize_t show_sample_rate_jiffies(struct kobject *kobj, struct attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", sample_rate_jiffies);
}

static ssize_t store_sample_rate_jiffies(struct kobject *kobj, struct attribute *attr, const char *buf, size_t count)
{
	ssize_t res;
	unsigned long input;
	res = strict_strtoul(buf, 0, &input);
	if (res >= 0 && input > 0 && input <= 1000)
		sample_rate_jiffies = input;
	return count;
}

static ssize_t show_ramp_up_step(struct kobject *kobj, struct attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", ramp_up_step);
}

static ssize_t store_ramp_up_step(struct kobject *kobj, struct attribute *attr, const char *buf, size_t count)
{
	ssize_t res;
	unsigned long input;
	res = strict_strtoul(buf, 0, &input);
	if (res >= 0 && input >= 0)
		ramp_up_step = input;
	return count;
}

static ssize_t show_ramp_down_step(struct kobject *kobj, struct attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", ramp_down_step);
}

static ssize_t store_ramp_down_step(struct kobject *kobj, struct attribute *attr, const char *buf, size_t count)
{
	ssize_t res;
	unsigned long input;
	res = strict_strtoul(buf, 0, &input);
	if (res >= 0 && input >= 0)
		ramp_down_step = input;
	return count;
}

static ssize_t show_max_cpu_load(struct kobject *kobj, struct attribute *attr, char *buf)
{
	return sprintf(buf, "%lu\n", max_cpu_load);
}

static ssize_t store_max_cpu_load(struct kobject *kobj, struct attribute *attr, const char *buf, size_t count)
{
	ssize_t res;
	unsigned long input;
	res = strict_strtoul(buf, 0, &input);
	if (res >= 0 && input > 0 && input <= 100)
		max_cpu_load = input;
	return count;
}

static ssize_t show_min_cpu_load(struct kobject *kobj, struct attribute *attr, char *buf)
{
	return sprintf(buf, "%lu\n", min_cpu_load);
}

static ssize_t store_min_cpu_load(struct kobject *kobj, struct attribute *attr, const char *buf, size_t count)
{
	ssize_t res;
	unsigned long input;
	res = strict_strtoul(buf, 0, &input);
	if (res >= 0 && input > 0 && input < 100)
		min_cpu_load = input;
	return count;
}

//tcd add start
void cpufreq_pegasusq_min_cpu_lock(unsigned int num_core)
{
	int online, flag;

	min_cpu_lock = min(num_core, num_possible_cpus());

	
	online = num_online_cpus();
	flag = (int)num_core - online;
	if (flag <= 0)
		return;
	queue_work_on(0, up_wq, &up_work);
}

void cpufreq_pegasusq_min_cpu_unlock(void)
{
	int online, lock, flag;

	min_cpu_lock = 0;

	online = num_online_cpus();
	lock = atomic_read(&g_hotplug_lock);
	if (lock == 0)
		return;
	flag = lock - online;
	if (flag >= 0)
		return;
	queue_work_on(0, down_wq, &down_work);
}

int cpufreq_pegasusq_cpu_lock(int num_core)
{
	int prev_lock;

	if (num_core < 1 || num_core > num_possible_cpus())
		return -EINVAL;

	prev_lock = atomic_read(&g_hotplug_lock);

	if (prev_lock != 0 && prev_lock < num_core)
		return -EINVAL;
	else if (prev_lock == num_core)
		atomic_inc(&g_hotplug_count);

	atomic_set(&g_hotplug_lock, num_core);
	atomic_set(&g_hotplug_count, 1);
	apply_hotplug_lock();

	return 0;
}

int cpufreq_pegasusq_cpu_unlock(int num_core)
{
	int prev_lock = atomic_read(&g_hotplug_lock);

	if (prev_lock < num_core)
		return 0;
	else if (prev_lock == num_core)
		atomic_dec(&g_hotplug_count);

	if (atomic_read(&g_hotplug_count) == 0)
		atomic_set(&g_hotplug_lock, 0);

	return 0;
}

static ssize_t show_max_cpu_lock(struct kobject *kobj, struct attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", max_cpu_lock);
}


static ssize_t store_max_cpu_lock(struct kobject *a, struct attribute *b,
				  const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);
	if (ret != 1)
		return -EINVAL;
	max_cpu_lock = min(input, num_possible_cpus());
	return count;
}

static ssize_t show_min_cpu_lock(struct kobject *kobj, struct attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", min_cpu_lock);
}

static ssize_t store_min_cpu_lock(struct kobject *a, struct attribute *b,
				  const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);
	if (ret != 1)
		return -EINVAL;
	if (input == 0)
		cpufreq_pegasusq_min_cpu_unlock();
	else
		cpufreq_pegasusq_min_cpu_lock(input);
	return count;
}

static ssize_t show_hotplug_lock(struct kobject *kobj, struct attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", 1);
}

static ssize_t store_hotplug_lock(struct kobject *a, struct attribute *b,
				  const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	int prev_lock;

	ret = sscanf(buf, "%u", &input);
	if (ret != 1)
		return -EINVAL;
	input = min(input, num_possible_cpus());
	prev_lock = atomic_read(&hotplug_lock);

	if (prev_lock)
		cpufreq_pegasusq_cpu_unlock(prev_lock);

	if (input == 0) {
		atomic_set(&hotplug_lock, 0);
		return count;
	}

	ret = cpufreq_pegasusq_cpu_lock(input);
	if (ret) {
		printk(KERN_ERR "[HOTPLUG] already locked with smaller value %d < %d\n",
			atomic_read(&g_hotplug_lock), input);
		return ret;
	}

	atomic_set(&hotplug_lock, input);

	return count;
}

static ssize_t store_fps_lock(struct kobject *a, struct attribute *b,
				  const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	
	ret = sscanf(buf, "%u", &input);
	if (ret != 1)
		return -EINVAL;
	atomic_set(&fps_lock, input);
	return count;
}


static ssize_t show_fps_lock(struct kobject *kobj, struct attribute *attr, char *buf)
{
	printk("tcd failed freq %d of %d, cpu fail up(fps) - %d of %d, fps make cpu down %d time\n", 
		fps_fail_freq_up, total_freq_up, fps_ok_cpu_up_fail, total_cpu_up + fps_ok_cpu_up_fail, fps_ok_cpu_down_ok);
	return sprintf(buf, "%u\n", (unsigned int)atomic_read(&fps_lock));
}

//tcd end

#define define_global_rw_attr(_name)		\
static struct global_attr _name##_attr =	\
	__ATTR(_name, 0644, show_##_name, store_##_name)

define_global_rw_attr(debug_mask);
define_global_rw_attr(up_rate_us);
define_global_rw_attr(down_rate_us);
define_global_rw_attr(sleep_ideal_freq);
define_global_rw_attr(sleep_wakeup_freq);
define_global_rw_attr(awake_ideal_freq);
define_global_rw_attr(sample_rate_jiffies);
define_global_rw_attr(ramp_up_step);
define_global_rw_attr(ramp_down_step);
define_global_rw_attr(max_cpu_load);
define_global_rw_attr(min_cpu_load);
//tcd add start
define_global_rw_attr(min_cpu_lock);
define_global_rw_attr(max_cpu_lock);
define_global_rw_attr(hotplug_lock);
define_global_rw_attr(fps_lock);

//tcd end



static struct attribute * smartass_attributes[] = {
	&debug_mask_attr.attr,
	&up_rate_us_attr.attr,
	&down_rate_us_attr.attr,
	&sleep_ideal_freq_attr.attr,
	&sleep_wakeup_freq_attr.attr,
	&awake_ideal_freq_attr.attr,
	&sample_rate_jiffies_attr.attr,
	&ramp_up_step_attr.attr,
	&ramp_down_step_attr.attr,
	&max_cpu_load_attr.attr,
	&min_cpu_load_attr.attr,
	&min_cpu_lock_attr.attr, //tcd add start
	&max_cpu_lock_attr.attr,
	&hotplug_lock_attr.attr, 
	&fps_lock_attr.attr,//tcd end
	NULL,
};

static struct attribute_group smartass_attr_group = {
	.attrs = smartass_attributes,
	.name = "smartass",
};

struct cpufreq_policy cpu1_policy;
struct cpufreq_policy cpu0_policy;

static int cpufreq_governor_smartass(struct cpufreq_policy *new_policy,
		unsigned int event)
{
	unsigned int cpu = new_policy->cpu;
	int rc;
	struct smartass_info_s *this_smartass = &per_cpu(smartass_info, cpu);

	switch (event) {
	case CPUFREQ_GOV_START:
		if ((!cpu_online(cpu)) || (!new_policy->cur))
			return -EINVAL;

		this_smartass->cur_policy = new_policy;

		this_smartass->enable = 1;
		//tcd add start
		if (cpu==0)
			hotplug_history->num_hist = 0;
		//tcd end
		smartass_update_min_max(this_smartass,new_policy,suspended);

		this_smartass->freq_table = cpufreq_frequency_get_table(cpu);
		if (!this_smartass->freq_table)
			printk(KERN_WARNING "Smartass: no frequency table for cpu %d?!\n",cpu);

		smp_wmb();

		// Do not register the idle hook and create sysfs
		// entries if we have already done so.
		if (cpu == 0 && atomic_inc_return(&active_count) <= 1) {
			rc = sysfs_create_group(cpufreq_global_kobject,
						&smartass_attr_group);
			if (rc){
				return rc;
			}

			smp_wmb();//tcd
			pm_idle_old = pm_idle;
			pm_idle = cpufreq_idle;
		}

		if (this_smartass->cur_policy->cur < new_policy->max && !timer_pending(&this_smartass->timer))
			reset_timer(cpu,this_smartass);

		break;

	case CPUFREQ_GOV_LIMITS:
		smartass_update_min_max(this_smartass,new_policy,suspended);

		if (this_smartass->cur_policy->cur > new_policy->max) {
			dprintk(SMARTASS_DEBUG_JUMPS,"SmartassI: jumping to new max freq: %d\n",new_policy->max);
			__cpufreq_driver_target(this_smartass->cur_policy,
						new_policy->max, CPUFREQ_RELATION_H);
		}
		else if (this_smartass->cur_policy->cur < new_policy->min) {
			dprintk(SMARTASS_DEBUG_JUMPS,"SmartassI: jumping to new min freq: %d\n",new_policy->min);
			__cpufreq_driver_target(this_smartass->cur_policy,
						new_policy->min, CPUFREQ_RELATION_L);
		}

		if (this_smartass->cur_policy->cur < new_policy->max && !timer_pending(&this_smartass->timer))
			reset_timer(cpu,this_smartass);

		break;

	case CPUFREQ_GOV_STOP:
		printk("cpu1 stop start\n");
		this_smartass->enable = 0;
		smp_wmb();
		del_timer(&this_smartass->timer);
		flush_work(&freq_scale_work);
		this_smartass->idle_exit_time = 0;

		if (cpu == 0 && (atomic_dec_return(&active_count) <= 1)) {
			printk("tcd remove sysfs\n");
			sysfs_remove_group(cpufreq_global_kobject,
					   &smartass_attr_group);
			pm_idle = pm_idle_old;
		}
		printk("cpu1 stop end\n");
		break;
	}

	return 0;
}

static void smartass_suspend(int cpu, int suspend)
{
	struct smartass_info_s *this_smartass = &per_cpu(smartass_info, smp_processor_id());
	struct cpufreq_policy *policy = this_smartass->cur_policy;
	unsigned int new_freq;

	if (!this_smartass->enable)
		return;

	smartass_update_min_max(this_smartass,policy,suspend);
	if (!suspend) { // resume at max speed:
		new_freq = validate_freq(policy,sleep_wakeup_freq);

		dprintk(SMARTASS_DEBUG_JUMPS,"SmartassS: awaking at %d\n",new_freq);

		__cpufreq_driver_target(policy, new_freq,
					CPUFREQ_RELATION_L);
	} else {
		// to avoid wakeup issues with quick sleep/wakeup don't change actual frequency when entering sleep
		// to allow some time to settle down. Instead we just reset our statistics (and reset the timer).
		// Eventually, the timer will adjust the frequency if necessary.

		this_smartass->freq_change_time_in_idle =
			get_cpu_idle_time_us(cpu,&this_smartass->freq_change_time);

		dprintk(SMARTASS_DEBUG_JUMPS,"SmartassS: suspending at %d\n",policy->cur);
	}

	reset_timer(smp_processor_id(),this_smartass);
}

static void smartass_early_suspend(struct early_suspend *handler) {
	int i;
	if (suspended || sleep_ideal_freq==0) // disable behavior for sleep_ideal_freq==0
		return;
	
#if EARLYSUSPEND_HOTPLUGLOCK
	early_suspend =	atomic_read(&g_hotplug_lock);
#endif
	suspended = 1;
	for_each_online_cpu(i)
		smartass_suspend(i,1);
	
#if EARLYSUSPEND_HOTPLUGLOCK
		atomic_set(&g_hotplug_lock,
			(min_cpu_lock) ? min_cpu_lock : 1);
		apply_hotplug_lock();
#endif
}

static void smartass_late_resume(struct early_suspend *handler) {
	int i;
	if (!suspended) // already not suspended so nothing to do
		return;
	
#if EARLYSUSPEND_HOTPLUGLOCK
		atomic_set(&g_hotplug_lock, early_suspend);
#endif
	suspended = 0;
	for_each_online_cpu(i)
		smartass_suspend(i,0);
#if EARLYSUSPEND_HOTPLUGLOCK
		apply_hotplug_lock();
#endif
}

static struct early_suspend smartass_power_suspend = {
	.suspend = smartass_early_suspend,
	.resume = smartass_late_resume,
#ifdef CONFIG_MACH_HERO
	.level = EARLY_SUSPEND_LEVEL_DISABLE_FB + 1,
#endif
};

static int __init cpufreq_smartass_init(void)
{
	unsigned int i;
	struct smartass_info_s *this_smartass;
	debug_mask = 0;
	up_rate_us = DEFAULT_UP_RATE_US;
	down_rate_us = DEFAULT_DOWN_RATE_US;
	sleep_ideal_freq = DEFAULT_SLEEP_IDEAL_FREQ;
	sleep_wakeup_freq = DEFAULT_SLEEP_WAKEUP_FREQ;
	awake_ideal_freq = DEFAULT_AWAKE_IDEAL_FREQ;
	sample_rate_jiffies = DEFAULT_SAMPLE_RATE_JIFFIES;
	ramp_up_step = DEFAULT_RAMP_UP_STEP;
	ramp_down_step = DEFAULT_RAMP_DOWN_STEP;
	max_cpu_load = DEFAULT_MAX_CPU_LOAD;
	min_cpu_load = DEFAULT_MIN_CPU_LOAD;
	//tcd add start
	cpu_up_rate = DEF_CPU_UP_RATE;
	cpu_down_rate = DEF_CPU_DOWN_RATE;	
	//tcd end
	spin_lock_init(&cpumask_lock);

	suspended = 0;

	/* Initalize per-cpu data: */
	for_each_possible_cpu(i) {
		this_smartass = &per_cpu(smartass_info, i);
		this_smartass->enable = 0;
		this_smartass->cur_policy = 0;
		this_smartass->ramp_dir = 0;
		this_smartass->time_in_idle = 0;
		this_smartass->idle_exit_time = 0;
		this_smartass->freq_change_time = 0;
		this_smartass->freq_change_time_in_idle = 0;
		this_smartass->cur_cpu_load = 0;
		// intialize timer:
		init_timer_deferrable(&this_smartass->timer);
		this_smartass->timer.function = cpufreq_smartass_timer;
		this_smartass->timer.data = i;
		work_cpumask_test_and_clear(i);
	}

	// Scale up is high priority
	printk("register smartassv2 start");
	up_wq = alloc_workqueue("ksmartass_up", WQ_HIGHPRI, 1);
	down_wq = alloc_workqueue("ksmartass_down", 0, 1);
	updown_wq = alloc_workqueue("ksmartass_cpu1", 0, 1);
	if (!up_wq || !down_wq)
		return -ENOMEM;

	INIT_WORK(&freq_scale_work, cpufreq_smartass_freq_change_time_work);
	//tcd add start
	hotplug_history = kzalloc(sizeof(struct cpu_usage_history), GFP_KERNEL);
	if (!hotplug_history) {
		printk("cannot create hotplug history array\n");
		return -ENOMEM;
	}
	
	INIT_WORK(&up_work, cpu_up_work);
	INIT_WORK(&down_work, cpu_down_work);
	//tcd add end
	register_early_suspend(&smartass_power_suspend);

	printk("register smartassv2 end");
	return cpufreq_register_governor(&cpufreq_gov_smartass2);
}

#ifdef CONFIG_CPU_FREQ_DEFAULT_GOV_SMARTASS2
fs_initcall(cpufreq_smartass_init);
#else
module_init(cpufreq_smartass_init);
#endif

static void __exit cpufreq_smartass_exit(void)
{
	//tcd start
	kfree(hotplug_history);
	//tcd end
	cpufreq_unregister_governor(&cpufreq_gov_smartass2);
	destroy_workqueue(up_wq);
	destroy_workqueue(down_wq);
}

module_exit(cpufreq_smartass_exit);

MODULE_AUTHOR ("Erasmux");
MODULE_DESCRIPTION ("'cpufreq_smartass2' - A smart cpufreq governor");
MODULE_LICENSE ("GPL");

