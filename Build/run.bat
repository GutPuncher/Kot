  
set OSNAME=kot
set BUILDDIR=%0/../bin
set OVMFDIR=%0/../OVMFbin

qemu-system-x86_64 -d cpu_reset -D log.txt -cpu host -smp 1 -usb -machine q35 -drive file=../bin/debugDisk.vhd -drive file=../Data/debugDisk.vhd -m 4G -cpu qemu64 -drive if=pflash,format=raw,unit=0,file=../OVMFbin/OVMF_CODE-pure-efi.fd,readonly=on -drive if=pflash,format=raw,unit=1,file=../OVMFbin/OVMF_VARS-pure-efi.fd -net none

pause