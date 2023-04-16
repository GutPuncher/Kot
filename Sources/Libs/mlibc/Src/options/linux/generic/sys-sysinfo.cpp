#include <errno.h>
#include <sys/sysinfo.h>

#include <bits/ensure.h>
#include <mlibc/debug.hpp>
#include <mlibc/linux-sysdeps.hpp>

int sysinfo(struct sysinfo *info) {
	MLIBC_CHECK_OR_ENOSYS(mlibc::sys_sysinfo, -1);
	if(int e = mlibc::sys_sysinfo(info); e) {
		errno = e;
		return -1;
	}
	return 0;
}
