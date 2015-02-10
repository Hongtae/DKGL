#ifndef __ANDROID__
#error "__ANDROID__ not defined. this file should be used with android only!"
#endif

#ifndef ANDROID_LOCALE_H
#define ANDROID_LOCALE_H

#ifdef HAVE_SYS_SYSCALL_H
#include "android_syscall.h"
#endif

#include <locale.h>

struct android_lconv
{
	char	*decimal_point;
	char	*thousands_sep;
	char	*grouping;
	char	*int_curr_symbol;
	char	*currency_symbol;
	char	*mon_decimal_point;
	char	*mon_thousands_sep;
	char	*mon_grouping;
	char	*positive_sign;
	char	*negative_sign;
	char	int_frac_digits;
	char	frac_digits;
	char	p_cs_precedes;
	char	p_sep_by_space;
	char	n_cs_precedes;
	char	n_sep_by_space;
	char	p_sign_posn;
	char	n_sign_posn;
	char	int_p_cs_precedes;
	char	int_n_cs_precedes;
	char	int_p_sep_by_space;
	char	int_n_sep_by_space;
	char	int_p_sign_posn;
	char	int_n_sign_posn;
};
struct android_lconv* android_localeconv(void);
#define lconv android_lconv
#define localeconv android_localeconv

#ifdef HAVE_LOCKF
#define F_ULOCK         0      /* unlock locked section */
#define	F_LOCK          1      /* lock a section for exclusive use */
#define	F_TLOCK         2      /* test and lock a section for exclusive use */
#define	F_TEST          3      /* test a section for locks by other procs */
#endif // ifdef HAVE_LOCKF

#endif // ifndef ANDROID_LOCALE_H
