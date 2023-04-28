#ifndef _SRV_HID_H
#define _SRV_HID_H 1

#include <kot/sys.h>
#include <kot/uisd.h>
#include <kot/types.h>
#include <kot/uisd/srvs/storage.h>

#if defined(__cplusplus)
extern "C" {
#endif

event_t GetMouseRelativeEvent();
event_t GetMouseAbsoluteEvent();
event_t GetKeyboardEvent();

KResult BindMouseRelative(thread_t Task, bool IgnoreMissedEvents);
KResult BindMouseAbsolute(thread_t Task, bool IgnoreMissedEvents);
KResult BindKeyboardEvent(thread_t Task, bool IgnoreMissedEvents);

KResult GetTableConverter(char* Path, char** TableConverter, size64_t* TableConverterCharCount);
KResult GetCharFromScanCode(uint64_t ScanCode, char* TableConverter, size64_t TableConverterCharCount, char* Char, bool* IsPressed, uint64_t* PressedCache);

#if defined(__cplusplus)
}
#endif


#endif