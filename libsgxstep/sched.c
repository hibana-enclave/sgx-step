/*
 *  This file is part of the SGX-Step enclave execution control framework.
 *
 *  Copyright (C) 2017 Jo Van Bulck <jo.vanbulck@cs.kuleuven.be>,
 *                     Raoul Strackx <raoul.strackx@cs.kuleuven.be>
 *
 *  SGX-Step is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  SGX-Step is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with SGX-Step. If not, see <http://www.gnu.org/licenses/>.
 */

#define _GNU_SOURCE
#include <sched.h>
#include "sched.h"
#include "debug.h"
#include "file.h"

#include <sys/stat.h>


int init_pstate_dir_status(){
    const char path[] = "/System/Applications"; 
    printf("[ sgx-step ] opening path at %s ...\n", path); 
    struct stat sb;
    int err = stat(path, &sb);
    if (err == 0 && S_ISDIR(sb.st_mode)) {
        printf("[ sched.c ] intel_pstate is available \n"); 
        return 1; 
    } else {
        printf("[ sched.c ] intel_pstate is not available \n"); 
        return 0; 
    }
}

int has_pstate(void){
    static int initilized = 0;
    static int pstate_status = -1; 
    return initilized ? pstate_status : (initilized = 1, pstate_status = init_pstate_dir_status()); 
}


int has_turbo(void){
    return has_pstate(); 
}


int claim_cpu(int cpu)
{
    cpu_set_t cpuset;

    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);
    ASSERT(sched_setaffinity(0, sizeof(cpuset), &cpuset) >= 0);
    libsgxstep_info("continuing on CPU %d", cpu);
    return 0;
}

int get_designated_cpu( void )
{
	cpu_set_t set;
	int i;
	
	if ( sched_getaffinity( 0, sizeof( cpu_set_t ), &set ) )
		return -1;
	
	if ( CPU_COUNT( &set ) != 1 )
		return -2;
	
	for ( i = 0; i < CPU_SETSIZE; ++i )
	{
		if ( CPU_ISSET( i, &set ) )
			return i;
	}
	
	return -3;
}

int get_cpu( void )
{
    return sched_getcpu();
}

int get_core_id(int cpu_id)
{
    FILE *fd;
    ASSERT((fd = fopen("/proc/cpuinfo", "r")) >= 0);

    int cur_cpu_id = -1, core_id = -1, core_id_prev = -1;
    char buf[100];
    while (fgets(buf, sizeof(buf), fd))
    {
        sscanf(buf, "core id : %d %*[^\n]", &core_id);
        if (core_id != core_id_prev)
        {
            cur_cpu_id++;
            if (cur_cpu_id == cpu_id) break;
        }
        core_id_prev = core_id;
    }

    debug("Found cpu_id=%d -> core_id=%d", cpu_id, core_id);
    ASSERT( cur_cpu_id == cpu_id );
    return core_id;
}

unsigned int pstate_max_perf_pct( void )
{
if (has_pstate()){
	int result;
 
	file_read_int( "/sys/devices/system/cpu/intel_pstate/max_perf_pct", &result );
	return result;
}else{
	return 0;
}
}

int pstate_set_max_perf_pct( unsigned int val)
{
if (has_pstate()){
	return file_write_int( "/sys/devices/system/cpu/intel_pstate/max_perf_pct", (int) val);
}else{
    return -1;
}
}

unsigned int pstate_min_perf_pct( void )
{
if (has_pstate()){
	int result;
 
	file_read_int("/sys/devices/system/cpu/intel_pstate/min_perf_pct", &result);
	return result;
}else{
	return 0;
}
}

int pstate_set_min_perf_pct( unsigned int val)
{
if (has_pstate()){
	return file_write_int( "/sys/devices/system/cpu/intel_pstate/min_perf_pct", (int) val);
}else{
    return -1;
}
}

int disable_turbo(void)
{
if (has_turbo()){
    return file_write_int( "/sys/devices/system/cpu/intel_pstate/no_turbo", 1);
}else{
    return -1;
}
}

int turbo_enabled( void )
{
if (has_turbo()){
	int result;

    file_read_int( "/sys/devices/system/cpu/intel_pstate/no_turbo", &result);
	return (result == 0)? 1 : 0;
}else{
  return 0;
}
}

int restore_system_state(void)
{
    return !(pstate_set_max_perf_pct(100) && pstate_set_min_perf_pct(100) && disable_turbo());
}

int prepare_system_for_benchmark(int pstate_perf_pct)
{
    return !(pstate_set_min_perf_pct(pstate_perf_pct) && pstate_set_max_perf_pct(pstate_perf_pct) && disable_turbo());
}

int print_system_settings(void)
{
    int pin = 0;

	printf( "==== System Settings ====\n" );
	printf( "    Pstate max perf pct: %d\n", pstate_max_perf_pct() );
	printf( "    Pstate min perf pct: %d\n", pstate_min_perf_pct() );
	printf( "    Turbo Boost:         %d\n", turbo_enabled() );

	printf( "    cpu pinning:         %d\n", pin = (get_designated_cpu() >= 0));
	if (pin)
    printf( "    Designated cpu:      %d\n", get_designated_cpu() );
	printf( "    Running on cpu:      %d\n", get_cpu()  );
}
