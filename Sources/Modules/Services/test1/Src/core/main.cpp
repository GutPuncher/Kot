#include <core/main.h>

extern "C" int main(int argc, char* argv[]){
    kot_Printlog("[Test1] Hello world");

    process_t proc = Sys_GetProcess();

    uintptr_t addressReceive = GetFreeAlignedSpace(sizeof(uisd_test_t));
    GetControllerUISD(ControllerTypeEnum_Test, &addressReceive, true);

    uisd_test_t* TestSrv = (uisd_test_t*)addressReceive;

    arguments_t arguments;
    kot_key_mem_t MemoryShare = Sys_ExecThread(TestSrv->GetMemory, &arguments, ExecutionTypeQueuAwait, NULL);
    uintptr_t addressReceiveShare = GetFreeAlignedSpace(0x1000);
    Sys_AcceptMemoryField(proc, MemoryShare, &addressReceiveShare);
    kot_Printlog((char*)addressReceiveShare);

    kot_Printlog("[Test1] End");
}