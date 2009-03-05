				/* v_cmul uses _t0 and _t1, but xlate ops that
				   involve cmul only uses t0 and t1, so t2
				   can be set to _t1 (or _t0). */
enum { t0=_t2, t1=_t3, t2=_t1, ft0=_f4, ft1=_f6 };

enum { num_ireg=32, num_freg=32, cachesize=32768 /* bytes */ };
