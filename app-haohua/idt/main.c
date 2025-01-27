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

#include "libsgxstep/idt.h"
#include "libsgxstep/gdt.h"
#include "libsgxstep/apic.h"
#include "libsgxstep/cpu.h"
#include "libsgxstep/sched.h"
#include "libsgxstep/config.h"

#define DO_APIC_SW_IRQ              1
#define DO_APIC_TMR_IRQ             1
#define DO_EXEC_PRIV                1
#define NUM                         100
#define INFINITE_LOOP               1
#define NEMESIS_HIGH                1

/* ------------------------------------------------------------ */
/* This code may execute with ring0 privileges */
int my_cpl = -1;
uint64_t my_flags = 0;
extern uint64_t nemesis_tsc_aex, nemesis_tsc_eresume;


void do_irq_tmr(void)
{
    apic_timer_irq(10);
}

/* ------------------------------------------------------------ */

void post_irq(char *s)
{
    info("returned from %s IRQ: my_cpl=%d; irq_cpl=%d; my_flags=%p; count=%02d; nemesis=%d", s,
            my_cpl, __ss_irq_cpl, my_flags, __ss_irq_count, nemesis_tsc_aex - nemesis_tsc_eresume);
}

void do_irq_test(int do_exec_priv)
{
    #if DO_APIC_TMR_IRQ
        printf("\n");
        info_event("Triggering ring-3 APIC timer interrupts..");
        apic_timer_oneshot(IRQ_VECTOR);

        for (int i=0; i < NUM; i++)
        {
            do_irq_tmr();
            while (!__ss_irq_fired)
            {
                asm("nop\n\t");
            }   
            __ss_irq_fired = 0; 
            post_irq("APIC timer");
        }
        apic_timer_deadline();
    #endif
}

int main( int argc, char **argv )
{
    idt_t idt = {0};
    ASSERT( !claim_cpu(VICTIM_CPU) );
    ASSERT( !prepare_system_for_benchmark(PSTATE_PCT) );
    print_system_settings();
    map_idt(&idt);

    #if 0
    /* ring 3 timer IRQ handlers may #GP when interrupting the kernel.. */
    info_event("Installing and testing ring3 IDT handler");
    install_user_irq_handler(&idt, __ss_irq_handler, IRQ_VECTOR);
    do_irq_test(/*do_exec_priv=*/ 0);
    #endif

    info_event("Installing and testing ring-0 IDT handler");
    install_kernel_irq_handler(&idt, __ss_irq_handler, IRQ_VECTOR);

    //#if INFINITE_LOOP
    //while(1)
    //#endif
        do_irq_test(/*do_exec_priv=*/ DO_EXEC_PRIV);

    info("all is well; irq_count=%d; exiting..", __ss_irq_count);
    return 0;
}
