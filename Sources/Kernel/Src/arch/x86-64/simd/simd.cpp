#include <arch/x86-64/simd/simd.h>


void simdInit(){
    ASMWriteCr0(ASMReadCr0() & ~((uint64_t)CR0_EMULATION)); 
    ASMWriteCr0(ASMReadCr0() | CR0_MONITOR_CO_PROCESSOR);
    ASMWriteCr0(ASMReadCr0() | CR0_NUMERIC_ERROR_ENABLE);

    ASMWriteCr4(ASMReadCr4() | CR4_FXSR_ENABLE);
    ASMWriteCr4(ASMReadCr4() | CR4_SIMD_EXCEPTION_SUPPORT);
    
    ASMFninit();
}

void* simdCreateSaveSpace(){
    return kcalloc(512);
}

void simdSave(void* location){
    asm volatile("fxsave (%0) "::"r"(location));
}

void simdRestore(void* location){
    asm volatile("fxrstor (%0) "::"r"(location));
}