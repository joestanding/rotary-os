#include <rotary/cpuid.h>

/* ========================================================================= */

uint32 leaf0_eax, leaf0_ebx, leaf0_ecx, leaf0_edx;
uint32 leaf1_eax, leaf1_ebx, leaf1_ecx, leaf1_edx;
uint32 leaf2_eax, leaf2_ebx, leaf2_ecx, leaf2_edx;
uint32 leaf3_eax, leaf3_ebx, leaf3_ecx, leaf3_edx;

/* ========================================================================= */

uint32 cpuid_init() {
    __get_cpuid(0, &leaf0_eax, &leaf0_ebx, &leaf0_ecx, &leaf0_edx);
    __get_cpuid(1, &leaf1_eax, &leaf1_ebx, &leaf1_ecx, &leaf1_edx);
    __get_cpuid(2, &leaf2_eax, &leaf2_ebx, &leaf2_ecx, &leaf2_edx);
    __get_cpuid(3, &leaf3_eax, &leaf3_ebx, &leaf3_ecx, &leaf3_edx);
    char cpu_name[24];
    cpuid_get_cpu_name(cpu_name);
    printk(LOG_DEBUG, "CPU: %s\n", cpu_name);
    return 1;
}

/* ========================================================================= */

uint32 cpuid_get_cpu_name(char * dest_buf) {
    uint32 eax, ebx, ecx, edx;
    __get_cpuid(0, &eax, &ebx, &ecx, &edx);

    sprintf(dest_buf, "%c%c%c%c%c%c%c%c%c%c%c%c\0", (ebx & 0xFF), (ebx >> 8 & 0xFF), (ebx >> 16 & 0xFF), (ebx >> 24 & 0xFF),
                                            (edx & 0xFF), (edx >> 8 & 0xFF), (edx >> 16 & 0xFF), (edx >> 24 & 0xFF),
                                            (ecx & 0xFF), (ecx >> 8 & 0xFF), (ecx >> 16 & 0xFF), (ecx >> 24 & 0xFF));
    return 1; 
}

/* ========================================================================= */

uint32 cpuid_check_pse() {
    return leaf1_edx & CPUID_FEAT_EDX_PSE;
}

/* ========================================================================= */

uint32 cpuid_check_apic() {
    return leaf1_edx & CPUID_FEAT_EDX_APIC;
}

/* ========================================================================= */

uint32 cpuid_check_x2apic() {
    return leaf1_edx & CPUID_FEAT_ECX_X2APIC;
}

/* ========================================================================= */
