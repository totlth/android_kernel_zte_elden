/******************************************************************************
 *
 * 	D E C I S I O N . C
 *
 * GENERAL DESCRIPTION
 * 	Multicore Decision Engine based on MP-DCVS v10.5

 Copyright (c) 2010-2011 QUALCOMM Incorporated.  All Rights Reserved.
 QUALCOMM Proprietary and Confidential.
 ******************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <time.h>
#include <getopt.h>
#include <pthread.h>
#include <poll.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <private/android_filesystem_config.h>
#include <linux/un.h>
#include <cutils/uevent.h>

#include "decision.h"

#ifdef MPCTL_SERVER
extern int  mpctl_server_init(void);
extern void mpctl_server_exit(void);
#endif

#define __NR_sched_setaffinity  (__NR_SYSCALL_BASE+241) /* ARM arch specific */

int debug_output = 1;
int advance = 0;

#ifdef USE_ANDROID_LOG
#define LOG_TAG "MP-Decision"
#include "cutils/log.h"
#    define msg(format, ...)   LOGE(format, ## __VA_ARGS__)
#else
#    define msg(format, ...)   printf(format, ## __VA_ARGS__)
#endif

#define dbgmsg(format, ...) \
	if (debug_output) \
		msg(format, ## __VA_ARGS__)

#define MAX_BUF (30)
#define MAX_UP_WNDW_SIZE (3)
#define MAX_DW_WNDW_SIZE (5)


/* Status of core1 */
static enum CORE_STATUS core1_status;

#define DEF_Nw 1.99
#define DEF_Tw 140
#define DEF_Ns 1.1
#define DEF_Ts 190
#define DEF_CPU_HIGH_UTILS_AND 50
#define DEF_CPU_HIGH_UTILS_OR 80
#define DEF_CPU_LOW_UTILS  40
#define RQ_DEPTH "/sys/devices/system/cpu/cpu0/rq-stats/run_queue_avg"
#define RQ_POLL_MS "/sys/devices/system/cpu/cpu0/rq-stats/run_queue_poll_ms"
#define DEF_TIMER_MS "/sys/devices/system/cpu/cpu0/rq-stats/def_timer_ms"
#define CPU0_UTILS "/sys/devices/system/cpu/cpu0/cpufreq/cpu_utilization"
#define CPU1_UTILS "/sys/devices/system/cpu/cpu1/cpufreq/cpu_utilization"

#define CPU0_FREQ "/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq"
#define FRAME_COUNT "/sys/devices/system/cpu/cpu0/rq-stats/fps"
#define WAITTIME "/sys/devices/system/cpu/cpu0/rq-stats/run_waittime"
#define CPU_BOOST_MS 500

#define LATENCY_SAMPLE_MS 900
#define MAX_LATENCY 15
#define MAX_DOWN_LATENCY 10
#define LATENCY_SAMPLE 20
#define LATENCY_DOWN_SAMPLE 30

#define BIT(x)	(1<<(x%LATENCY_SAMPLE))
#define BIT_D(x)(1<<(x%LATENCY_DOWN_SAMPLE))

static float Nw = DEF_Nw;
static float Ns = DEF_Ns;
static unsigned int Tw = DEF_Tw;
static unsigned int Ts = DEF_Ts;
static unsigned int poll_ms = 9;
static unsigned int decision_ms = 50;
static unsigned int old_decision_ms = 20;
static unsigned int util_high_and = DEF_CPU_HIGH_UTILS_AND;
static unsigned int util_high_or = DEF_CPU_HIGH_UTILS_OR;
static unsigned int util_low = DEF_CPU_LOW_UTILS;
static unsigned int max_up_wndw_size = MAX_UP_WNDW_SIZE;
static unsigned int max_dw_wndw_size = MAX_DW_WNDW_SIZE;
static int valid_in_wndw = 0;
static int stall_decision = 0;
static int stall_decision_external = 0;
static int def_timer_fd = 0;
static unsigned int boost_time = 0;

static int control_sleep_modes = 0;
static int adjust_average = 0;
static int saved_sapc_idle_en_c0 = -1;
static int saved_sapc_idle_en_c1 = -1;
static int saved_pc_idle_en_c0 = -1;
static int saved_pc_idle_en_c1 = -1;

static pthread_t poll_rq, mp_decision, hotplug_thread;
static float avg_rq_depth, total_time;
static pthread_mutex_t rq_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t hotplug_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t hotplug_condition = PTHREAD_COND_INITIALIZER;
static int thermal_condition = 0;

#define MAX_UTIL_WNDW 20
static unsigned int cpu_avg_load_window[MAX_UTIL_WNDW]={0};
static unsigned int cpu_load_index=0;

static int get_cpu_status(int cpu);
static int act_on_decision(unsigned int status);

int disable_decision_engine(void)
{
	if (stall_decision_external)
		return -1;

	pthread_mutex_lock(&hotplug_mutex);
	if (thermal_condition) {
		msg("%s: failed, system is in thermal condition", __func__);
		pthread_mutex_unlock(&hotplug_mutex);
		return -1;
	}

	stall_decision_external = 1;
	pthread_mutex_unlock(&hotplug_mutex);

	return 0;
}

int enable_decision_engine(void)
{
	if (!stall_decision_external)
		return -1;

	pthread_mutex_lock(&hotplug_mutex);
	core1_status = get_cpu_status(1);
	stall_decision_external = 0;
	pthread_mutex_unlock(&hotplug_mutex);

	return 0;
}

int core_boost(int tasks)
{
	boost_time = CPU_BOOST_MS;
	if (core1_status != CORE_UP)
		act_on_decision(CORE_UP);
	return 0;
}

static int read_file_double(char *file, double *val)
{
	int ret = -ENODEV;
	int fd = 0;
	char buf[MAX_BUF] = {0};

	fd = open(file, O_RDONLY);
	if (fd > 0) {
		/* ensure NULL terminated, sizeof(buf) - 1 */
		if (read(fd, buf, sizeof(buf) - 1) == -1)
			ret = -EINVAL;
		else {
			*val = strtod(buf, NULL);
			ret = 0;
		}
		close(fd);
	}

	return ret;
}

static int write_file_uint32(char *file, unsigned int val)
{
	int ret = -ENODEV;
	int fd = 0;
	char buf[MAX_BUF] = {0};

	snprintf(buf, MAX_BUF, "%u\n", val);

	fd = open(file, O_WRONLY);
	if (fd > 0) {
		if (write(fd, buf, sizeof(buf)) == -1)
			ret = -EINVAL;
		else
			ret = 0;
		close(fd);
	}

	return ret;
}

int get_cpu_utils(unsigned int * hotplug_avg_load)
{
	int ret = -ENODEV;
	int fd = 0;
	char buf[MAX_BUF] = {0};
	unsigned int val0 = 0;
	unsigned int val1 = 0;
	int i,j;
	int count = 0;
	int max_wndw;

	*hotplug_avg_load = 0;
	if (core1_status == CORE_UP) {
		max_wndw = max_dw_wndw_size ;
	} else {
		max_wndw = max_up_wndw_size;
	}
	if (valid_in_wndw < max_wndw) {
		valid_in_wndw++;
	}

	/* get cpu0 utlization at max*/
	fd = open(CPU0_UTILS, O_RDONLY);
	if (fd > 0) {
		if (read(fd, buf, sizeof(buf)) == -1)
			ret = -EINVAL;
		else {
			val0 = strtol(buf, NULL,0);
			ret = 0;
		}
		close(fd);
	}
	memset(buf, 0, MAX_BUF);
	/* get cpu1 utilization at max */
	fd = open(CPU1_UTILS, O_RDONLY);
	if (fd > 0) {
		if (read(fd, buf, sizeof(buf)) == -1)
			ret = -EINVAL;
		else {
			val1 = strtol(buf, NULL,0);
			ret = 0;
		}
		close(fd);
	}
	/* store the aggregated load of cpu0 and cpu1  in the circular buffer */
	cpu_avg_load_window[cpu_load_index]= (val0 + val1);

	/* compute average load across in & out sampling periods */
	for (i = 0, j = cpu_load_index; i < valid_in_wndw; i++, j--) {
		*hotplug_avg_load += cpu_avg_load_window[j];
		if (j == 0)
			j = MAX_UTIL_WNDW;
	}
	/* compute avg loads across the whole window */
	*hotplug_avg_load  = *hotplug_avg_load/valid_in_wndw;

	/* return to first element if we're at the circular buffer's end */
	if (++cpu_load_index == MAX_UTIL_WNDW)
		cpu_load_index = 0;

	dbgmsg ("CPU LOAD : IN = %u, val0=%d, val1=%d\n, valid=%d", *hotplug_avg_load, val0, val1, valid_in_wndw);

	return ret;
}


static int get_power_mode(int cpu, char *dir, char *filename)
{
	int fd;
	char str[MAX_BUF * 3] = {0};
	char buf[MAX_BUF] = {0};
	int ret = -1;

	snprintf(str, MAX_BUF * 3, "/sys/module/pm_8x60/modes/cpu%d/%s/%s", cpu, dir, filename);
	fd = open(str, O_RDONLY);
	if (fd > 0) {
		/* ensure NULL terminated, sizeof(buf) - 1 */
		if (read(fd, buf, sizeof(buf) - 1) != -1)
			ret = atoi(buf);
		close(fd);
	}

	return ret;
}

static int set_power_mode(int cpu, char *dir, char *filename, int enable)
{
	int ret = 0;
	int fd;
	char str[MAX_BUF *3] = {0};
	char val[MAX_BUF] = {0};

	snprintf(str, MAX_BUF * 3, "/sys/module/pm_8x60/modes/cpu%d/%s/%s", cpu, dir, filename);
	snprintf(val, MAX_BUF, "%d\n", enable);

	fd = open(str, O_RDWR);
	if (fd > 0) {
		if (write(fd, val, sizeof(val)) == -1)
			ret = -EINVAL;
		close(fd);
	}
	else
		ret = -ENODEV;

	return ret;
}

static void print_power_modes()
{
	if(control_sleep_modes == 0)
		return;

	dbgmsg("Current power modes:\n");
	dbgmsg("sapc/idle_en_c0 = %d\n",
			get_power_mode(0, "standalone_power_collapse", "idle_enabled"));
	dbgmsg("sapc/idle_en_c1 = %d\n",
			get_power_mode(1, "standalone_power_collapse", "idle_enabled"));
	dbgmsg("pc/idle_en_c0 = %d\n",
		get_power_mode(0, "power_collapse", "idle_enabled"));
	dbgmsg("pc/idle_en_c1 = %d\n",
		get_power_mode(1, "power_collapse", "idle_enabled"));
}

static void save_power_modes()
{
	if(control_sleep_modes == 0)
		return;

	dbgmsg("Saving power modes:\n");
	saved_sapc_idle_en_c0 = get_power_mode(0, "standalone_power_collapse", "idle_enabled");
	saved_sapc_idle_en_c1 = get_power_mode(1, "standalone_power_collapse", "idle_enabled");
	saved_pc_idle_en_c0 = get_power_mode(0, "power_collapse", "idle_enabled");
	saved_pc_idle_en_c1 = get_power_mode(1, "power_collapse", "idle_enabled");

	dbgmsg("saved_sapc_idle_en_c0: %d\n", saved_sapc_idle_en_c0);
	dbgmsg("saved_sapc_idle_en_c1: %d\n", saved_sapc_idle_en_c1);
	dbgmsg("saved_pc_idle_en_c0: %d\n", saved_pc_idle_en_c0);
	dbgmsg("saved_pc_idle_en_c1: %d\n", saved_pc_idle_en_c1);
}

static void restore_power_modes()
{
	if(control_sleep_modes == 0)
		return;

	dbgmsg("Restoring power modes\n");
	if(saved_sapc_idle_en_c0  != -1)
		set_power_mode(0, "standalone_power_collapse", "idle_enabled", saved_sapc_idle_en_c0 );
	if(saved_sapc_idle_en_c1  != -1)
		set_power_mode(1, "standalone_power_collapse", "idle_enabled", saved_sapc_idle_en_c1 );
	if(saved_pc_idle_en_c0 != -1)
		set_power_mode(0, "power_collapse", "idle_enabled", saved_pc_idle_en_c0);
	if(saved_pc_idle_en_c1 != -1)
		set_power_mode(1, "power_collapse", "idle_enabled", saved_pc_idle_en_c1);
}

static int cpu_up(int cpu, int on)
{
	int ret = 0;
	int fd;
	char str[MAX_BUF * 3] = {0};
	char val[MAX_BUF] = {0};

	snprintf(str, MAX_BUF * 3, "/sys/devices/system/cpu/cpu%d/online", cpu);
	snprintf(val, MAX_BUF, "%d\n", (on ? 1 : 0));

	fd = open(str, O_RDWR);
	if (fd > 0) {
		if (write(fd, val, sizeof(val)) == -1)
			ret = -EINVAL;
		close(fd);
	}
	else
		ret = -ENODEV;

	return ret;
}

static int get_cpu_status(int cpu)
{
	int status = CORE_DOWN;
	int fd;
	char str[MAX_BUF * 3] = {0};
	char buf[MAX_BUF] = {0};

	snprintf(str, MAX_BUF * 3, "/sys/devices/system/cpu/cpu%d/online", cpu);
	fd = open(str, O_RDONLY);
	if (fd > 0) {
		/* ensure NULL terminated, sizeof(buf) - 1 */
		if (read(fd, buf, sizeof(buf) - 1) == -1)
			buf[0] = '0';
		buf[1] = '\0';
		status = atoi(buf);
		close(fd);
	}

	return status;
}

static inline int decision_engine_stalled(void)
{
	return (stall_decision || stall_decision_external);
}

static int act_on_decision(unsigned int status)
{
	int ret = 0;

	if (status == core1_status) {
		msg("No action required to change core status(%d).", status);
		return 0;
	}

	pthread_mutex_lock(&hotplug_mutex);

	if (decision_engine_stalled()) {
		msg("MP decision to set core status to %d ignored\n", status);
		pthread_mutex_unlock(&hotplug_mutex);
		return -ENODEV;
	}

	if (thermal_condition) {
		msg("MP decision in thermal condition, "
		    "request to set core status to %d ignored\n", status);
		pthread_mutex_unlock(&hotplug_mutex);
		return -ENODEV;
	}

	switch (status) {
	case CORE_UP:
		if(control_sleep_modes==1) {
			int err=0;
			// 1) Remember the individual core independent power collapse setting for both cores
			save_power_modes();
			// 2) Disable individual core independent power collapse
			dbgmsg("Setting power modes before CORE_UP\n");
			err += set_power_mode(0, "standalone_power_collapse", "idle_enabled", 0);
			err += set_power_mode(1, "standalone_power_collapse", "idle_enabled", 0);
			err += set_power_mode(0, "power_collapse", "idle_enabled", 0);
			err += set_power_mode(1, "power_collapse", "idle_enabled", 0);
			if(err) {
				dbgmsg("Error setting a sleep mode - %d\n", err);
			}
			//print_power_modes();
		}
		// 3) Power up the 2nd core
		core1_status = CORE_UP;
		ret = cpu_up(1, 1);
		break;
	case CORE_DOWN:
		// 1) Power down the 2nd core
		core1_status = CORE_DOWN;
		ret = cpu_up(1, 0);
		// 2) Restore the original individual core independent power collapse setting for both cores
		if(control_sleep_modes==1)  {
			dbgmsg("Restoring power modes after CORE_DOWN\n");
			restore_power_modes();
			//print_power_modes();
		}
		break;
	default:
		break;
	}

	if (ret) {
		msg("Error(%d) changing core status to %s\n", ret,
				status ? "online" : "offline");
	}
	pthread_mutex_unlock(&hotplug_mutex);

	return ret;
}

static int wait_for_cpu_online(void)
{
	if (!decision_engine_stalled())
		return 0;

	pthread_mutex_lock(&hotplug_mutex);
	dbgmsg("Waiting for cpu to be online\n");
	while (stall_decision) {
		pthread_cond_wait(&hotplug_condition, &hotplug_mutex);
	}
	pthread_mutex_unlock(&hotplug_mutex);

	return 1;
}

static int deferrable_sleep(int sleep_ms)
{
	int err = 0;
	int time_ms = 0;
	struct pollfd fds;
	char buf[20] = {0};

	write_file_uint32(DEF_TIMER_MS, sleep_ms);

	while (1) {
		fds.fd = def_timer_fd;
		fds.events = POLLERR|POLLPRI;
		fds.revents = 0;

		err = poll(&fds, 1, -1);
		if (err == -1) {
			msg("Error waiting for timer\n");
			close(fds.fd);
			return 0;
		}

		/* ensure NULL terminated, sizeof(buf) - 1 */
		if (read(def_timer_fd, buf, sizeof(buf) - 1) != -1) {
			lseek(def_timer_fd, 0, SEEK_SET);

			time_ms = atoi(buf);
		}
		if (time_ms)
			break;
	}

	return time_ms;
}

//#define tv2mS(tv) ((tv).tv_sec * 1000LL + ((tv).tv_usec + 500) / 1000)
//		 	gettimeofday(&tv, NULL);
//       	 	mS = tv2mS(tv) - tv2mS(tv_start);/*no screen update for 1sec*/

void *do_mp_decision(void *data)
{
	unsigned int new_state = 0;
	double rq_depth = 0;
	double time = 0;
	double total_time = 0;
	unsigned int hotplug_avg_load = 0;

	//tcd start
	int waittime_fd;
	int frame_fd;
	int freq_fd;

	double total_up_time = 0;
	int fc_still_flag = 0;
	int fc_cnt = 0;
	int fc_prev = 0;
	int fc = 0;

	char buf[MAX_BUF] = {0};

	int waittime_ms;
	int waittime_flag = 0;
	int w_cnt = 0;

	
	if (advance == 1){
	    waittime_fd = open(WAITTIME, O_RDONLY);
	    frame_fd = open(FRAME_COUNT,O_RDONLY);
	}
	//tcd end
	while (1) {

		time = deferrable_sleep(decision_ms);

		/* Block if hotplugged externally. */
		/* We cannot account for the hotplugged time correctly, reset and continue */
		if (wait_for_cpu_online()) {
			total_time = 0;
			dbgmsg("MP-Decision was stalled. Will reset and continue.\n");
			continue;
		}

		if (thermal_condition == 1) {
			total_time = 0;
			dbgmsg("MP-Decision is disabled in thermal condition. Will reset and continue.\n");
			continue;
		}

		if (read_file_double(RQ_DEPTH, &rq_depth) < 0) {
			msg("Eror reading run queue depth.\n");
			continue;
		}

		if (rq_depth < 0) {
			msg("Invalid run queue depth :%f\n", rq_depth);
			continue;
		}

		if(get_cpu_utils(&hotplug_avg_load) < 0) {
			msg("Eror reading cpu load.\n");
			continue;
		} 
//tcd start
		if (advance == 1){
			if (frame_fd > 0){
				if (read(frame_fd, buf, sizeof(buf) - 1)!=-1){
		              	        lseek(frame_fd, 0, SEEK_SET);
					fc = atoi(buf);

					fc_still_flag &= ~BIT(fc_cnt);
					if (fc!=fc_prev)
						fc_still_flag |= BIT(fc_cnt);
					fc_cnt++;
					fc_cnt = fc_cnt%LATENCY_SAMPLE;
					fc_prev = fc;
				} else
				    fc_still_flag = -1;
			} else
				fc_still_flag = -1;

			if (waittime_fd	>0){
				if (read(waittime_fd, buf, sizeof(buf)-1)!=-1){
		              	    lseek(waittime_fd, 0, SEEK_SET);
				    waittime_ms = atoi(buf);

				    if (core1_status == CORE_DOWN) 
				        waittime_flag &= ~BIT(w_cnt);
				    else
				        waittime_flag &= ~BIT_D(w_cnt);

				    if ((core1_status == CORE_DOWN) && (waittime_ms > MAX_LATENCY))
					waittime_flag |= BIT(w_cnt);
				    else if ((waittime_ms > MAX_DOWN_LATENCY) && (core1_status == CORE_UP))
					waittime_flag |= BIT_D(w_cnt);

				    w_cnt++;

				    if (core1_status == CORE_DOWN)
				        w_cnt = w_cnt%LATENCY_SAMPLE;
				    else
				        w_cnt = w_cnt%LATENCY_DOWN_SAMPLE;
				} else 
				    waittime_flag = -1;
			} else {
				waittime_flag = -1;
			}
		}
//tcd end
		/*if the timer has been deferred, ignore the sample when deciding to bringing up core1*/
		if( adjust_average==1 && time > decision_ms && core1_status == CORE_DOWN )
		{
			dbgmsg("Runqueue depth :%f ld :%d time :%f (ignored)\n", rq_depth, hotplug_avg_load, time);
			total_time = 0;
			continue;
		}
		dbgmsg("Runqueue depth :%f ld :%d time :%f boost :%dms st :%d\n",
		    rq_depth, hotplug_avg_load, time, boost_time, core1_status);

		total_time += time;

		if (core1_status == CORE_DOWN) {
			int condition1 = ((rq_depth > Nw) && total_time >= Tw);
		        int condition2 = (hotplug_avg_load > util_high_or);
			int condition3 = (advance == 1)?((fc_still_flag!=0) && (waittime_flag!=0)):1;
			int condition4 = 1;
			int freq;

			if (fc_still_flag == -1 || waittime_flag == -1)
				condition3 = 1;

			freq_fd = open(CPU0_FREQ, O_RDONLY);
			if (freq_fd > 0){
				if (read(freq_fd, buf, sizeof(buf)-1)!=-1){
					freq = atoi(buf);
					condition4 = (freq == 1512000);
				}
				close(freq_fd);
			}

		        if((condition1 && (hotplug_avg_load > util_high_and)) && condition3 && condition4) {
			    if (advance == 0)
				  msg("UP Ld:%u Nw:%f Tw:%d rq:%f seq:%f \n",
				     hotplug_avg_load, Nw, Tw, rq_depth, total_time);
			    else
				  msg("UP Ld:%u Nw:%f Tw:%d rq:%f seq:%f waitflag:0x%x fc_still_flag:0x%x fps:%d freq:%d\n",
				     hotplug_avg_load, Nw, Tw, rq_depth, total_time, waittime_flag, fc_still_flag, fc, freq);

			          new_state = CORE_UP;

				if (advance == 1){
				  decision_ms = 30;//tcd
				  waittime_flag = 0;
				  total_up_time = 0;
				  w_cnt = 0;
				}
			}
			if (rq_depth <= Nw)
				total_time = 0;
		} else if (core1_status == CORE_UP) {
			int condition4 = (advance == 1)?((waittime_flag==0) && ( total_up_time > LATENCY_SAMPLE_MS))
                                || ((fc_still_flag == 0) && (total_up_time > LATENCY_SAMPLE_MS)):1;
			
			if (fc_still_flag == -1 || waittime_flag == -1)
				condition4 = 0;

			total_up_time += time;
			if (boost_time > 0) {
				if (boost_time < decision_ms)
					boost_time = 0;
				else
					boost_time -= decision_ms;
				new_state = CORE_UP;
			} else if (((rq_depth <= Ns) && (total_time >= Ts)) || (hotplug_avg_load < util_low) 
				|| condition4){
			    if (advance == 0)
			        msg("DOWN Ld:%u Ns:%f Ts:%d rq:%f seq:%f",
				   hotplug_avg_load, Ns, Ts, rq_depth, total_time);
			    else
			        msg("DOWN Ld:%u Ns:%f Ts:%d rq:%f seq:%f waitflag:0x%x fc_still_flag:0x%x fps:%d",
				   hotplug_avg_load, Ns, Ts, rq_depth, total_time, waittime_flag, fc_still_flag, fc);
				new_state = CORE_DOWN;

				if (advance == 1){
					decision_ms = 50;//tcd
					waittime_flag = 0;
					w_cnt = 0;
				}
			}

			if (rq_depth > Ns) {
				total_time = 0;
			}
		}

		/* Change core state */
		if (new_state != core1_status) {
			total_time = 0;
			valid_in_wndw = 0;
			act_on_decision(new_state);
		}
	}

	return NULL;
}

void *do_hotplug(void *data)
{
	int err = 0;
	int cpu = (int) data;
	struct pollfd fds;
	char buf[1024] = {0};
	char cpu_online[MAX_BUF * 2] = {0};
	char cpu_offline[MAX_BUF * 2] = {0};
	unsigned int online;
	int recv_bytes = 0;

	/* Looking for online@/devices/system/cpu/cpu1 or
	 * offline@/devices/system/cpu/cpu1 */

	snprintf(cpu_online, MAX_BUF * 2, "online@/devices/system/cpu/cpu1" );
	snprintf(cpu_offline, MAX_BUF * 2, "offline@/devices/system/cpu/cpu1");

	fds.events = POLLIN;
	fds.fd = uevent_open_socket(64*1024, true);
	if (fds.fd == -1) {
		msg("Error opening socket for hotplug uevent.\n");
		return NULL;
	}

	while (1) {

		err = poll(&fds, 1, -1);
		if (err == -1) {
			msg("Error in hotplug CPU[%d] poll.\n", cpu);
			break;
		}

		recv_bytes = uevent_kernel_multicast_recv(fds.fd, buf, sizeof(buf));
		if (recv_bytes == -1)
			continue;
		if (recv_bytes >= 1024)
			buf[1023] = '\0';
		else
			buf[recv_bytes] = '\0';

		if (!strcmp(buf, cpu_online))
			online = 1;
		else if (!strcmp(buf, cpu_offline))
			online = 0;
		else
			continue;

		pthread_mutex_lock(&hotplug_mutex);

		if (stall_decision_external && !thermal_condition) {
			pthread_mutex_unlock(&hotplug_mutex);
			continue;
		}

		dbgmsg("Online %u core1_status %d stalled %d\n",
		       online, core1_status, stall_decision);
		if (thermal_condition && online == 1) {
			/* hotplug core back off, since we are in thermal state */
			msg("System in thermal condition, bring core back down.");
			core1_status = CORE_DOWN;
			cpu_up(1, CORE_DOWN);
		} else if (!stall_decision && online != core1_status && boost_time == 0) {
			msg("CPU[%d] has been hotplugged outside MP-Decision.", cpu);
			msg("MP-Decision will be stalled until enabled.");
			core1_status = online ? CORE_UP : CORE_DOWN;
			stall_decision = 1;
		} else if (stall_decision) {
			msg("Enabling MP decision for CPU[%d]\n", cpu);
			stall_decision = 0;
			core1_status = online ? CORE_UP : CORE_DOWN;
			pthread_cond_broadcast(&hotplug_condition);
		}
		pthread_mutex_unlock(&hotplug_mutex);
	}

	return NULL;
}

static void parse_args(int argc, char *argv[])
{

	while (1) {
		int idx = 0;
		int c = 0;
		static struct option long_options[] = {
			{"Nw", required_argument, NULL, 1},
			{"Tw", required_argument, NULL, 2},
			{"Ns", required_argument, NULL, 3},
			{"Ts", required_argument, NULL, 4},
			{"poll_ms", required_argument, NULL, 5},
			{"decision_ms", required_argument, NULL, 6},
			{"no_sleep", no_argument, NULL, 7},
			{"avg_comp", no_argument, NULL, 8},
			{"debug", no_argument, NULL, 9},
			{"util_h_and", required_argument, NULL, 10},
			{"util_h_or", required_argument, NULL, 11},
			{"util_l", required_argument, NULL, 12},
			{"max_up_wndw_size", required_argument, NULL, 13},
			{"max_dw_wndw_size", required_argument, NULL, 14},
			{"advance", no_argument, NULL, 15},//tcd
			{NULL, 0, 0, 0},
		};

		c = getopt_long_only(argc, argv, "", long_options, &idx);
		if (c == -1)
			break;

		switch (c) {
		case 1:
			sscanf(optarg, "%f", &Nw);
			if (Nw < 0 && Nw > 4)
				Nw = DEF_Nw;
			break;
		case 2:
			Tw = atoi(optarg);
			if (Tw == 0  && Tw > 1000000)
				Tw = DEF_Tw;
			break;
		case 3:
			sscanf(optarg, "%f", &Ns);
			if (Ns < 0 && Ns > 4)
				Ns = DEF_Ns;
			break;
		case 4:
			Ts = atoi(optarg);
			if (Ts == 0  && Ts > 1000000)
				Ts = DEF_Ts;
			break;
		case 5:
			poll_ms = atoi(optarg);
			break;
		case 6:
			decision_ms = atoi(optarg);
			break;
		case 7:
			control_sleep_modes = 1;
			msg("OPTION ENABLED: Control sleep modes");
			break;
		case 8:
			adjust_average = 1;
			msg("OPTION ENABLED: Adjusting average");
			break;
		case 9:
			debug_output = 1;
			break;
		case 10:
			util_high_and = atoi(optarg);
			break;
		case 11:
			util_high_or = atoi(optarg);
			break;
		case 12:
			util_low = atoi(optarg);
			break;
		case 13:
			max_up_wndw_size = atoi(optarg);
			break;
		case 14:
			max_dw_wndw_size = atoi(optarg);
			break;
		case 15:
			advance = 1;
			break;
		default:
		case '?':
			msg("\n MP Decision Options:\n");
			msg("--Nw --Tw --Ns --Ts --poll_ms --decision_ms --no_sleep --avg_comp --util_h_and --utli_h_or --util_l --debug\n");
			exit(-1);
			break;
		}
	}
	msg("Decision parameters Nw=%f, Tw=%d, Ns=%f, Ts=%d, DI=%d, poll=%d, util_h_and=%d, util_h_or=%d, util_l=%d, max_up_wndw_size=%d, max_dw_wndw_size=%d\n",
			Nw, Tw, Ns, Ts, decision_ms, poll_ms, util_high_and, util_high_or, util_low, max_up_wndw_size, max_dw_wndw_size);
}

void thermal_force_coredown(int enable)
{
	if ((enable == 0 && thermal_condition == 1) ||
	    (enable == 1 && thermal_condition == 0)) {
		msg("%s: invalid request for thermal", __func__);
		return;
	}

	pthread_mutex_lock(&hotplug_mutex);

	if (enable == 0) {
		msg("%s: Request received to force core 1 down", __func__);
		core1_status = CORE_DOWN;
		cpu_up(1, CORE_DOWN);
		thermal_condition = 1;
	} else {
		msg("%s: Request received to remove thermal condition", __func__);
		thermal_condition = 0;
	}
	pthread_mutex_unlock(&hotplug_mutex);
}

static pthread_t mpdecision_server_thread;
static struct sockaddr_un addr;
static int comsoc = -1;
#define MPDECISION_SOCKET       "/dev/socket/mpdecision"
#define NUM_LISTEN_QUEUE        (2)

static void *mpdecision_server(void *data)
{
    int rc, len;
    int online;
    int conn_socket = 0;
    char msgbuf[MAX_BUF] = {0};

    if (comsoc < 0)
        return NULL;

    for (;;) {
	    len = sizeof(struct sockaddr_un);
	    conn_socket = accept(comsoc, (struct sockaddr *) &addr, &len);
	    if (conn_socket == -1) {
		    msg("%s: accept failed", __func__);
		    continue;
	    }

	    memset(msgbuf, 0, MAX_BUF);
	    rc = recv(conn_socket, msgbuf, MAX_BUF, 0);
	    if (rc <= 0) {
		    msg("%s: recv failed", __func__);
		    goto close_conn;
	    }
	    dbgmsg("%s: received msg %s", __func__, msgbuf);

	    if (rc >= MAX_BUF)
		    msgbuf[MAX_BUF - 1] = '\0';
	    else
		    msgbuf[rc] = '\0';

	    rc = sscanf(msgbuf, "hotplug %d", &online);
	    if (rc != 1 || !(online == 0 || online == 1)) {
		    msg("%s: msg format unexpected", __func__);
	    } else {
		    thermal_force_coredown(online);
	    }

close_conn:
	    close(conn_socket);
    }

    return NULL;
}

int mpdecision_server_init()
{
    int rc = 0, stage = 0;
    int len = 0;
    msg("MPDecision server starting");

    comsoc = socket(PF_UNIX, SOCK_STREAM, 0);
    if (comsoc < 0) {
	    stage = 1;
	    goto error;
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    snprintf(addr.sun_path, UNIX_PATH_MAX, MPDECISION_SOCKET);
    addr.sun_family = AF_UNIX;

    /* Delete existing socket file if necessary */
    unlink(addr.sun_path);

    rc = bind(comsoc, (struct sockaddr *)&addr, sizeof(struct sockaddr_un));
    if (rc != 0) {
	    stage = 2;
	    goto error;
    }

    chown(addr.sun_path, AID_ROOT, AID_SYSTEM);
    chmod(addr.sun_path, 0660);

    rc = listen(comsoc, NUM_LISTEN_QUEUE);
    if (rc != 0) {
	    stage = 3;
	    goto error;
    }

    rc = pthread_create(&mpdecision_server_thread, NULL, mpdecision_server, NULL);
    if (rc != 0) {
	    stage = 4;
	    goto error;
    }
    return 1;

error:
    msg("Unable to create control service (stage=%d, rc=%d)", stage, rc);
    if (comsoc != -1) {
	    close(comsoc);
	    comsoc = -1;
    }
    return 0;
}

void mpdecision_server_exit()
{
	if (comsoc != -1) {
		close(comsoc);

		pthread_join(mpdecision_server_thread, NULL);
		comsoc = -1;
	}
}

int main(int argc, char *argv[])
{
	int ret = 0;
	int cpu = 1;
	unsigned int cpu_set = 1; /* Run on core0 */

#ifdef MPCTL_SERVER
	mpctl_server_init();
#endif

	mpdecision_server_init();

	ret = syscall(__NR_sched_setaffinity,0, sizeof(cpu_set), &cpu_set);
	if (ret < 0) {
		msg("Cannot set cpu affinity: %s\n", strerror(-ret));
		return ret;
	}

	core1_status = CORE_DOWN;
	cpu_up(1, core1_status);

	msg("Core1 status: %s\n", core1_status ? "online" : "offline");

	/* Priority ? */
	setpriority(PRIO_PROCESS, getpid(), -20);

	/* Command line overrides */
	parse_args(argc, argv);

	/* Enable kernel calculation of rq depth */
	write_file_uint32(RQ_POLL_MS, poll_ms);

	def_timer_fd = open(DEF_TIMER_MS, O_RDONLY);
	if (def_timer_fd < 0) {
		msg("Unable to open deferrable timer file\n");
		return -1;
	}

	pthread_create(&mp_decision, NULL, do_mp_decision, NULL);
	pthread_create(&hotplug_thread, NULL, do_hotplug, (void *)cpu);

	pthread_join(hotplug_thread, NULL);
	pthread_join(mp_decision, NULL);

	close(def_timer_fd);

	mpdecision_server_exit();

#ifdef MPCTL_SERVER
	mpctl_server_exit();
#endif
	return ret;
}
