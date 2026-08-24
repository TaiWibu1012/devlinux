#ifndef _SMARTCLOCK_COMMON_H_
#define _SMARTCLOCK_COMMON_H_

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
#include <stdbool.h>
#endif

/* Button state enumeration */
#define BTN_STATE_RELEASED  0
#define BTN_STATE_PRESSED   1

/**
 * struct button_event - Raw event packet transferred from Kernel to Userspace
 * @state: Button physical state (BTN_STATE_RELEASED / BTN_STATE_PRESSED)
 * @timestamp_ns: Monotonic kernel timestamp in nanoseconds when interrupt occurred
 */
struct button_event {
    uint8_t  state;
    uint64_t timestamp_ns;
};

/* Device node paths */
#define BTN_DEV_PATH      "/dev/btn_driver"
#define BUZZER_DEV_PATH   "/dev/buzzer_driver"
#define I2C_DEV_PATH      "/dev/i2c-1"

#endif /* _SMARTCLOCK_COMMON_H_ */