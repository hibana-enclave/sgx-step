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

#ifndef SGX_STEP_CONFIG
#define SGX_STEP_CONFIG

// TODO add coarser-grained logging levels
#define LIBSGXSTEP_DEBUG            0

#define PSTATE_PCT                  100
#define SINGLE_STEP_ENABLE          1
#define USER_IDT_ENABLE             1
#define IRQ_VECTOR                  45
#define IRQ_PRIV_VECTOR             49
#define GDT_VECTOR                  13
#if (M32 != 1)
	#define APIC_CONFIG_MSR         1
#else
	#define APIC_CONFIG_MSR         0
#endif

#define VICTIM_CPU                  3

#endif
