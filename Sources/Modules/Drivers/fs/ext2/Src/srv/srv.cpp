#include <srv/srv.h>

static file_dispatch_t FileDispatcher[File_Function_Count] = { 
    [File_Function_Close] = Closefile,
    [File_Function_GetSize] = Getfilesize,
    [File_Function_Read] = Readfile,
    [File_Function_Write] = Writefile,
};

static dir_dispatch_t DirDispatcher[Dir_Function_Count] = { 
    [Dir_Function_Close] = Closedir,
    [Dir_Function_GetCount] = Getdircount,
    [Dir_Function_Read] = Readdir,
};



KResult MountToVFS(mount_info_t* MountInfo, kot_process_t VFSProcess, kot_thread_t VFSMountThread){
    kot_srv_storage_fs_server_functions_t FSServerFunctions;

    kot_process_t proc = kot_Sys_GetProcess();

    /* ChangeUserData */
    kot_thread_t ChangeUserDataThread = NULL;
    kot_Sys_CreateThread(proc, (void*)&ChangeUserData, PriviledgeDriver, (uint64_t)MountInfo, &ChangeUserDataThread);
    FSServerFunctions.ChangeUserData = kot_MakeShareableThreadToProcess(ChangeUserDataThread, VFSProcess);

    /* Removefile */
    kot_thread_t RemovefileThread = NULL;
    kot_Sys_CreateThread(proc, (void*)&Removefile, PriviledgeDriver, (uint64_t)MountInfo, &RemovefileThread);
    FSServerFunctions.Removefile = kot_MakeShareableThreadToProcess(RemovefileThread, VFSProcess);

    /* Openfile */
    kot_thread_t OpenfileThread = NULL;
    kot_Sys_CreateThread(proc, (void*)&Openfile, PriviledgeDriver, (uint64_t)MountInfo, &OpenfileThread);
    FSServerFunctions.Openfile = kot_MakeShareableThreadToProcess(OpenfileThread, VFSProcess);

    /* Rename */
    kot_thread_t RenameThread = NULL;
    kot_Sys_CreateThread(proc, (void*)&Rename, PriviledgeDriver, (uint64_t)MountInfo, &RenameThread);
    FSServerFunctions.Rename = kot_MakeShareableThreadToProcess(RenameThread, VFSProcess);

    /* Mkdir */
    kot_thread_t MkdirThread = NULL;
    kot_Sys_CreateThread(proc, (void*)&Mkdir, PriviledgeDriver, (uint64_t)MountInfo, &MkdirThread);
    FSServerFunctions.Mkdir = kot_MakeShareableThreadToProcess(MkdirThread, VFSProcess);

    /* Rmdir */
    kot_thread_t RmdirThread = NULL;
    kot_Sys_CreateThread(proc, (void*)&Rmdir, PriviledgeDriver, (uint64_t)MountInfo, &RmdirThread);
    FSServerFunctions.Rmdir = kot_MakeShareableThreadToProcess(RmdirThread, VFSProcess);

    /* Opendir */
    kot_thread_t OpendirThread = NULL;
    kot_Sys_CreateThread(proc, (void*)&Opendir, PriviledgeDriver, (uint64_t)MountInfo, &OpendirThread);
    FSServerFunctions.Opendir = kot_MakeShareableThreadToProcess(OpendirThread, VFSProcess);

    kot_Srv_Storage_MountPartition(VFSMountThread, &FSServerFunctions, true);

    return KSUCCESS;
}


/* VFS access */
KResult ChangeUserData(kot_thread_t Callback, uint64_t CallbackArg, uint64_t UID, uint64_t GID, char* UserName){
    mount_info_t* MountInfo = (mount_info_t*)kot_Sys_GetExternalDataThread();
    
    MountInfo->UID = UID;
    MountInfo->GID = GID;
    memcpy(MountInfo->UserName, UserName, strlen(UserName));

    kot_arguments_t arguments{
        .arg[0] = KSUCCESS,         /* Status */
        .arg[1] = CallbackArg,      /* CallbackArg */
        .arg[2] = NULL,             /* GP0 */
        .arg[3] = NULL,             /* GP1 */
        .arg[4] = NULL,             /* GP2 */
        .arg[5] = NULL,             /* GP3 */
    };

    kot_Sys_ExecThread(Callback, &arguments, ExecutionTypeQueu, NULL);
    kot_Sys_Close(KSUCCESS);
}

/* Files */

/* VFS access */
KResult Removefile(kot_thread_t Callback, uint64_t CallbackArg, char* Path, kot_permissions_t Permissions){
    mount_info_t* MountInfo = (mount_info_t*)kot_Sys_GetExternalDataThread();
    KResult Status = MountInfo->RemoveFile(Path, Permissions);
    
    kot_arguments_t arguments{
        .arg[0] = Status,           /* Status */
        .arg[1] = CallbackArg,      /* CallbackArg */
        .arg[2] = NULL,             /* GP0 */
        .arg[3] = NULL,             /* GP1 */
        .arg[4] = NULL,             /* GP2 */
        .arg[5] = NULL,             /* GP3 */
    };

    kot_Sys_ExecThread(Callback, &arguments, ExecutionTypeQueu, NULL);
    kot_Sys_Close(KSUCCESS);
}

/* VFS access */
KResult Openfile(kot_thread_t Callback, uint64_t CallbackArg, char* Path, kot_permissions_t Permissions, kot_process_t Target){
    mount_info_t* MountInfo = (mount_info_t*)kot_Sys_GetExternalDataThread();
    KResult Status = KFAIL;

    ext_file_t* File = NULL;

    if(File = MountInfo->OpenFile(Path, Permissions)){
        File->Target = Target;
        Status = KSUCCESS;
    }

    
    kot_arguments_t arguments{
        .arg[0] = Status,           /* Status */
        .arg[1] = CallbackArg,      /* CallbackArg */
        .arg[2] = NULL,             /* Data */
        .arg[3] = NULL,             /* GP1 */
        .arg[4] = NULL,             /* GP2 */
        .arg[5] = NULL,             /* GP3 */
    };

    if(Status == KSUCCESS){
        kot_srv_storage_fs_server_open_file_data_t SrvOpenFileData;
        kot_thread_t DispatcherThread;

        kot_Sys_CreateThread(kot_Sys_GetProcess(), (void*)&FileDispatch, PriviledgeDriver, (uint64_t)File, &DispatcherThread);

        SrvOpenFileData.Dispatcher = kot_MakeShareableThreadToProcess(DispatcherThread, Target);

        SrvOpenFileData.FSDriverProc = ProcessKey;
        
        kot_ShareDataWithArguments_t ShareDataWithArguments{
            .Data = &SrvOpenFileData,
            .Size = sizeof(kot_srv_storage_fs_server_open_file_data_t),
            .ParameterPosition = 0x2,
        };
        kot_Sys_ExecThread(Callback, &arguments, ExecutionTypeQueu, &ShareDataWithArguments);
    }else{
        kot_Sys_ExecThread(Callback, &arguments, ExecutionTypeQueu, NULL);
    }
    kot_Sys_Close(KSUCCESS);
}

/* Direct access */
KResult FileDispatch(kot_thread_t Callback, uint64_t CallbackArg, uint64_t GP0, uint64_t GP1, uint64_t GP2, uint64_t GP3){
    uint64_t Function = GP0;

    if(Function >= File_Function_Count){
        kot_arguments_t arguments{
            .arg[0] = KFAIL,            /* Status */
            .arg[1] = CallbackArg,      /* CallbackArg */
            .arg[2] = NULL,             /* GP0 */
            .arg[3] = NULL,             /* GP1 */
            .arg[4] = NULL,             /* GP2 */
            .arg[5] = NULL,             /* GP3 */
        };

        kot_Sys_ExecThread(Callback, &arguments, ExecutionTypeQueu, NULL);
        kot_Sys_Close(KSUCCESS);
    }

    ext_file_t* File = (ext_file_t*)kot_Sys_GetExternalDataThread();
    kot_Sys_Close(FileDispatcher[Function](Callback, CallbackArg, File, GP1, GP2, GP3)); // It'll call the callback in the function
}

/* Direct access */
KResult Closefile(kot_thread_t Callback, uint64_t CallbackArg, ext_file_t* File, uint64_t GP0, uint64_t GP1, uint64_t GP2){
    KResult Status = File->CloseFile();
    
    kot_arguments_t arguments{
        .arg[0] = Status,           /* Status */
        .arg[1] = CallbackArg,      /* CallbackArg */
        .arg[2] = NULL,             /* GP0 */
        .arg[3] = NULL,             /* GP1 */
        .arg[4] = NULL,             /* GP2 */
        .arg[5] = NULL,             /* GP3 */
    };

    kot_Sys_ExecThread(Callback, &arguments, ExecutionTypeQueu, NULL);
    
    if(Status == KSUCCESS){
        kot_Sys_Exit(KSUCCESS);
    }

    return KSUCCESS;
}

/* Direct access */
KResult Getfilesize(kot_thread_t Callback, uint64_t CallbackArg, ext_file_t* File, uint64_t GP0, uint64_t GP1, uint64_t GP2){
    kot_arguments_t arguments{
        .arg[0] = KSUCCESS,             /* Status */
        .arg[1] = CallbackArg,          /* CallbackArg */
        .arg[2] = File->GetSize(),      /* FileSize */
        .arg[3] = NULL,                 /* GP1 */
        .arg[4] = NULL,                 /* GP2 */
        .arg[5] = NULL,                 /* GP3 */
    };
    kot_Sys_ExecThread(Callback, &arguments, ExecutionTypeQueu, NULL);
    return KSUCCESS;
}

/* Direct access */
KResult Readfile(kot_thread_t Callback, uint64_t CallbackArg, ext_file_t* File, uint64_t GP0, uint64_t GP1, uint64_t GP2){
    size64_t Size = GP1;
    kot_process_t TargetDataProc = static_cast<kot_process_t>(GP2);

    if((GP0 + Size) > File->GetSize()){
        Size = File->GetSize() - GP0;
    }
    
    kot_key_mem_t BufferKey;
    KResult Status = File->ReadFile(&BufferKey, GP0, Size);

    kot_arguments_t arguments{
        .arg[0] = Status,           /* Status */
        .arg[1] = CallbackArg,      /* CallbackArg */
        .arg[2] = NULL,             /* Key to buffer */
        .arg[3] = NULL,             /* GP1 */
        .arg[4] = NULL,             /* GP2 */
        .arg[5] = NULL,             /* GP3 */
    };

    if(Status == KSUCCESS){
        kot_Sys_Keyhole_CloneModify(BufferKey, &arguments.arg[2], TargetDataProc, KeyholeFlagPresent | KeyholeFlagCloneable | KeyholeFlagEditable, PriviledgeApp);
        kot_Sys_ExecThread(Callback, &arguments, ExecutionTypeQueu, NULL);
    }else{
        kot_Sys_ExecThread(Callback, &arguments, ExecutionTypeQueu, NULL);
    }

    return KSUCCESS;
}

/* Direct access */
KResult Writefile(kot_thread_t Callback, uint64_t CallbackArg, ext_file_t* File, uint64_t GP0, uint64_t GP1, uint64_t GP2){
    KResult Status = KFAIL;
    
    uint64_t TypePointer;
    uint64_t Size;
    if(kot_Sys_GetInfoMemoryField(GP0, &TypePointer, &Size) == KSUCCESS){
        if(TypePointer == MemoryFieldTypeSendSpaceRO){            
            void* Buffer = malloc(Size);
            assert(kot_Sys_AcceptMemoryField(kot_Sys_GetProcess(), GP0, &Buffer) == KSUCCESS);

            Status = File->WriteFile(Buffer, GP1, Size, GP2);
        }
    }
    
    kot_arguments_t arguments{
        .arg[0] = Status,            /* Status */
        .arg[1] = CallbackArg,      /* CallbackArg */
        .arg[2] = NULL,             /* GP0 */
        .arg[3] = NULL,             /* GP1 */
        .arg[4] = NULL,             /* GP2 */
        .arg[5] = NULL,             /* GP3 */
    };

    kot_Sys_ExecThread(Callback, &arguments, ExecutionTypeQueu, NULL);

    return KSUCCESS;
}


/* Files and directories */

/* VFS access */
KResult Rename(kot_thread_t Callback, uint64_t CallbackArg, kot_srv_storage_fs_server_rename_t* RenameData, kot_permissions_t Permissions){
    char* OldPath = (char*)((uint64_t)RenameData + RenameData->OldPathPosition);
    char* NewPath = (char*)((uint64_t)RenameData + RenameData->NewPathPosition);
    
    mount_info_t* MountInfo = (mount_info_t*)kot_Sys_GetExternalDataThread();

    KResult Status = MountInfo->Rename(OldPath, NewPath, Permissions);
    
    kot_arguments_t arguments{
        .arg[0] = Status,            /* Status */
        .arg[1] = CallbackArg,      /* CallbackArg */
        .arg[2] = NULL,             /* GP0 */
        .arg[3] = NULL,             /* GP1 */
        .arg[4] = NULL,             /* GP2 */
        .arg[5] = NULL,             /* GP3 */
    };

    kot_Sys_ExecThread(Callback, &arguments, ExecutionTypeQueu, NULL);
    kot_Sys_Close(KSUCCESS);
}


/* Directories */

/* VFS access */
KResult Mkdir(kot_thread_t Callback, uint64_t CallbackArg, char* Path, mode_t Mode, kot_permissions_t Permissions){
    mount_info_t* MountInfo = (mount_info_t*)kot_Sys_GetExternalDataThread();

    KResult Status = MountInfo->CreateDir(Path, Mode, Permissions);
    
    kot_arguments_t arguments{
        .arg[0] = Status,            /* Status */
        .arg[1] = CallbackArg,      /* CallbackArg */
        .arg[2] = NULL,             /* GP0 */
        .arg[3] = NULL,             /* GP1 */
        .arg[4] = NULL,             /* GP2 */
        .arg[5] = NULL,             /* GP3 */
    };

    kot_Sys_ExecThread(Callback, &arguments, ExecutionTypeQueu, NULL);
    kot_Sys_Close(KSUCCESS);
}

/* VFS access */
KResult Rmdir(kot_thread_t Callback, uint64_t CallbackArg, char* Path, kot_permissions_t Permissions){
    mount_info_t* MountInfo = (mount_info_t*)kot_Sys_GetExternalDataThread();
    KResult Status = MountInfo->RemoveDir(Path, Permissions);
    
    kot_arguments_t arguments{
        .arg[0] = Status,            /* Status */
        .arg[1] = CallbackArg,      /* CallbackArg */
        .arg[2] = NULL,             /* GP0 */
        .arg[3] = NULL,             /* GP1 */
        .arg[4] = NULL,             /* GP2 */
        .arg[5] = NULL,             /* GP3 */
    };

    kot_Sys_ExecThread(Callback, &arguments, ExecutionTypeQueu, NULL);
    kot_Sys_Close(KSUCCESS);
}

/* VFS access */
KResult Opendir(kot_thread_t Callback, uint64_t CallbackArg, char* Path, kot_permissions_t Permissions, kot_process_t Target){
    mount_info_t* MountInfo = (mount_info_t*)kot_Sys_GetExternalDataThread();
    KResult Status = KFAIL;

    ext_directory_t* Directory = NULL;
    
    if(Directory = MountInfo->OpenDir(Path, Permissions)){
        Status = KSUCCESS;
    }
    
    kot_arguments_t arguments{
        .arg[0] = Status,            /* Status */
        .arg[1] = CallbackArg,      /* CallbackArg */
        .arg[2] = NULL,             /* Data */
        .arg[3] = NULL,             /* GP1 */
        .arg[4] = NULL,             /* GP2 */
        .arg[5] = NULL,             /* GP3 */
    };

    if(Status == KSUCCESS){
        kot_srv_storage_fs_server_open_dir_data_t SrvOpenDirData;
        kot_thread_t DispatcherThread;

        kot_Sys_CreateThread(kot_Sys_GetProcess(), (void*)&DirDispatch, PriviledgeDriver, (uint64_t)Directory, &DispatcherThread);

        kot_Sys_Keyhole_CloneModify(DispatcherThread, &SrvOpenDirData.Dispatcher, Target, KeyholeFlagPresent | KeyholeFlagDataTypeThreadIsExecutableWithQueue, PriviledgeApp);

        SrvOpenDirData.FSDriverProc = ProcessKey;
        
        kot_ShareDataWithArguments_t ShareDataWithArguments{
            .Data = &SrvOpenDirData,
            .Size = sizeof(kot_srv_storage_fs_server_open_dir_data_t),
            .ParameterPosition = 0x2,
        };
        kot_Sys_ExecThread(Callback, &arguments, ExecutionTypeQueu, &ShareDataWithArguments);
    }else{
        kot_Sys_ExecThread(Callback, &arguments, ExecutionTypeQueu, NULL);
    }
    kot_Sys_Close(KSUCCESS);
}

/* Direct access */
KResult DirDispatch(kot_thread_t Callback, uint64_t CallbackArg, uint64_t GP0, uint64_t GP1, uint64_t GP2, uint64_t GP3){
    uint64_t Function = GP0;

    if(Function >= Dir_Function_Count){
        kot_arguments_t arguments{
            .arg[0] = KFAIL,            /* Status */
            .arg[1] = CallbackArg,      /* CallbackArg */
            .arg[2] = NULL,             /* GP0 */
            .arg[3] = NULL,             /* GP1 */
            .arg[4] = NULL,             /* GP2 */
            .arg[5] = NULL,             /* GP3 */
        };

        kot_Sys_ExecThread(Callback, &arguments, ExecutionTypeQueu, NULL);
        kot_Sys_Close(KSUCCESS);
    }

    ext_directory_t* Directory = (ext_directory_t*)kot_Sys_GetExternalDataThread();

    kot_Sys_Close(DirDispatcher[Function](Callback, CallbackArg, Directory, GP1, GP2, GP3));
}

/* Direct access */
KResult Readdir(kot_thread_t Callback, uint64_t CallbackArg, ext_directory_t* Directory, uint64_t GP0, uint64_t GP1, uint64_t GP2){
    uint64_t IndexCount = GP1;
    uint64_t IndexStart = GP0;

    read_dir_data** ReadirData = (read_dir_data**)malloc(sizeof(read_dir_data*) * IndexCount);

    KResult Status = KSUCCESS;

    uint64_t EntryCount = 0;
    size64_t DataSize = sizeof(kot_directory_entries_t);
    for(uint64_t i = 0; i < IndexCount; i++){
        ReadirData[i] = Directory->ReadDir(IndexStart + i);
        if(ReadirData[i] == NULL){
            Status = KFAIL;
            break;
        } 
        DataSize += sizeof(kot_directory_entry_t);
        DataSize += ReadirData[i]->NameLength + 1;
        EntryCount++;
    }

    kot_directory_entries_t* Data = (kot_directory_entries_t*)malloc(DataSize);
    Data->EntryCount = EntryCount;

    uint64_t NextEntryPosition = 0;
    kot_directory_entry_t* Entry = &Data->FirstEntry;
    for(uint64_t i = 0; i < EntryCount; i++){
        NextEntryPosition += sizeof(kot_directory_entry_t) + ReadirData[i]->NameLength + 1;
        Entry->NextEntryPosition = NextEntryPosition;
        Entry->IsFile = ReadirData[i]->IsFile;
        memcpy(&Entry->Name, ReadirData[i]->Name, ReadirData[i]->NameLength + 1);
        free(ReadirData[i]->Name);
        free(ReadirData[i]);
        Entry = (kot_directory_entry_t*)((uint64_t)&Data->FirstEntry + (uint64_t)NextEntryPosition);
    }

    Entry->NextEntryPosition = NULL;

    free(ReadirData);

    
    kot_arguments_t arguments{
        .arg[0] = Status,           /* Status */
        .arg[1] = CallbackArg,      /* CallbackArg */
        .arg[2] = NULL,             /* GP0 */
        .arg[3] = DataSize,         /* GP1 */
        .arg[4] = NULL,             /* GP2 */
        .arg[5] = NULL,             /* GP3 */
    };

    kot_ShareDataWithArguments_t ShareDataWithArguments{
        .Data = Data,
        .Size = DataSize,
        .ParameterPosition = 0x2,
    };
    kot_Sys_ExecThread(Callback, &arguments, ExecutionTypeQueu, &ShareDataWithArguments);
    free(Data);
    kot_Sys_Close(KSUCCESS);
}

/* Direct access */
KResult Getdircount(kot_thread_t Callback, uint64_t CallbackArg, ext_directory_t* Directory, uint64_t GP0, uint64_t GP1, uint64_t GP2){
    kot_arguments_t arguments{
        .arg[0] = KSUCCESS,                 /* Status */
        .arg[1] = CallbackArg,              /* CallbackArg */
        .arg[2] = Directory->GetDirCount(), /* DirCount */
        .arg[3] = NULL,                     /* GP1 */
        .arg[4] = NULL,                     /* GP2 */
        .arg[5] = NULL,                     /* GP3 */
    };

    kot_Sys_ExecThread(Callback, &arguments, ExecutionTypeQueu, NULL);
    return KSUCCESS;
}

/* Direct access */
KResult Closedir(kot_thread_t Callback, uint64_t CallbackArg, ext_directory_t* Directory, uint64_t GP0, uint64_t GP1, uint64_t GP2){
    KResult Status = Directory->CloseDir();
    
    kot_arguments_t arguments{
        .arg[0] = Status,            /* Status */
        .arg[1] = CallbackArg,      /* CallbackArg */
        .arg[2] = NULL,             /* GP0 */
        .arg[3] = NULL,             /* GP1 */
        .arg[4] = NULL,             /* GP2 */
        .arg[5] = NULL,             /* GP3 */
    };

    kot_Sys_ExecThread(Callback, &arguments, ExecutionTypeQueu, NULL);
    kot_Sys_Exit(KSUCCESS);
}