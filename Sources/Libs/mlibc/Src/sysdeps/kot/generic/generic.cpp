#include <kot/syscall.h>
#include <string.h>
#include <mlibc/debug.hpp>
#include <bits/ensure.h>
#include <mlibc/all-sysdeps.hpp>

struct KotSpecificData_t KotSpecificData;

namespace Kot{
    kot_process_t Sys_GetProcess(){
        /* Get Self Data */
        uint64_t self;
        asm("mov %%gs:0x8, %0":"=r"(self));
        return self;
    }
}

namespace mlibc{
    void sys_libc_log(const char *message){
        Syscall_16(KSys_Logs, (uint64_t)message, (uint64_t)strlen(message));
    }

    [[noreturn]] void sys_libc_panic(){
        sys_libc_log("libc panic!");
        __builtin_trap();
        for(;;);
    }

    int sys_tcb_set(void *pointer){
        return (Syscall_8(KSys_TCB_Set, (uint64_t)pointer) != KSUCCESS);
    }

    int sys_futex_tid(){
        /* Get Self Data */
        uint64_t TID = NULL;
        asm("mov %%gs:0x18, %0":"=r"(TID));
        return static_cast<int>(TID);
    }

    int sys_futex_wait(int *pointer, int expected, const struct timespec *time){
        __ensure(!"Not implemented");
    }

    int sys_futex_wake(int *pointer){
        __ensure(!"Not implemented");
    }


    int sys_anon_allocate(size_t size, void **pointer){
        *pointer = (void*)KotSpecificData.HeapLocation;
        KotSpecificData.HeapLocation += size;
        return (Syscall_48(KSys_Map, Kot::Sys_GetProcess(), (uint64_t)pointer, 0, 0, (uint64_t)&size, false) != KSUCCESS);
    }

    int sys_anon_free(void *pointer, size_t size){
        __ensure(!"Not implemented");
    }

    int sys_stat(fsfd_target fsfdt, int fd, const char *path, int flags, struct stat *statbuf){
        __ensure(!"Not implemented");
    }

    int sys_vm_map(void *hint, size_t size, int prot, int flags, int fd, off_t offset, void **window){
        __ensure(!"Not implemented");
    }

    int sys_vm_unmap(void *pointer, size_t size){
        __ensure(!"Not implemented");
    }

    int sys_vm_protect(void *pointer, size_t size, int prot){
        __ensure(!"Not implemented");
    }

    [[noreturn]] void sys_exit(int status){
        Syscall_8(KSys_Exit, status);
        __builtin_unreachable();
    }

    [[noreturn, gnu::weak]] void sys_thread_exit(){
        sys_exit(KSUCCESS);
        __builtin_unreachable();
    }
    
    int sys_open(const char *pathname, int flags, mode_t mode, int *fd){
        __ensure(!"Not implemented");
    }

    int sys_read(int fd, void *buf, size_t count, ssize_t *bytes_read){
        __ensure(!"Not implemented");
    }

    int sys_write(int fd, const void *buf, size_t count, ssize_t *bytes_written){
        Syscall_16(KSys_Logs, (uint64_t)buf, (uint64_t)count);
        *bytes_written = count;
        //__ensure(!"Not implemented");
        return 0;
    }

    int sys_seek(int fd, off_t offset, int whence, off_t *new_offset){
        //__ensure(!"Not implemented");
        return 0;
    }

    int sys_close(int fd){
        __ensure(!"Not implemented");
    }

    int sys_flock(int fd, int options){
        __ensure(!"Not implemented");
    }

    int sys_open_dir(const char *path, int *handle){
        __ensure(!"Not implemented");
    }

    int sys_read_entries(int handle, void *buffer, size_t max_size, size_t *bytes_read){
        __ensure(!"Not implemented");
    }

    int sys_pread(int fd, void *buf, size_t n, off_t off, ssize_t *bytes_read){
        __ensure(!"Not implemented");
    }

    int sys_clock_get(int clock, time_t *secs, long *nanos){
        __ensure(!"Not implemented");
    }

    int sys_clock_getres(int clock, time_t *secs, long *nanos){
        __ensure(!"Not implemented");
    }

    int sys_sleep(time_t *secs, long *nanos){
        __ensure(!"Not implemented");
    }

    // In contrast to the isatty() library function, the sysdep function uses return value
    // zero (and not one) to indicate that the file is a terminal.
    int sys_isatty(int fd){
        // __ensure(!"Not implemented");
        return 0;
    }

    int sys_rmdir(const char *path){
        __ensure(!"Not implemented");
    }

    int sys_unlinkat(int dirfd, const char *path, int flags){
        __ensure(!"Not implemented");
    }

    int sys_rename(const char *path, const char *new_path){
        __ensure(!"Not implemented");
    }

    int sys_renameat(int olddirfd, const char *old_path, int newdirfd, const char *new_path){
        __ensure(!"Not implemented");
    }

    int sys_sigprocmask(int how, const sigset_t *__restrict set, sigset_t *__restrict retrieve){
        __ensure(!"Not implemented");
    }

    int sys_sigaction(int, const struct sigaction *__restrict, struct sigaction *__restrict){
        //__ensure(!"Not implemented");
        return 0;
    }

    int sys_fork(pid_t *child){
        __ensure(!"Not implemented");
    }

    int sys_waitpid(pid_t pid, int *status, int flags, struct rusage *ru, pid_t *ret_pid){
        __ensure(!"Not implemented");
    }

    int sys_execve(const char *path, char *const argv[], char *const envp[]){
        __ensure(!"Not implemented");
    }

    pid_t sys_getpid(){
        /* Get Self Data */
        uint64_t PID = NULL;
        asm("mov %%gs:0x10, %0":"=r"(PID));
        return static_cast<pid_t>(PID);
    }

    int sys_kill(int, int){
        __ensure(!"Not implemented");
    }
}