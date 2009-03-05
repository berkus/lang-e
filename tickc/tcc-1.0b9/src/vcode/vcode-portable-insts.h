#ifndef _VCODE_PORTABLE_INSTS_H_
#define _VCODE_PORTABLE_INSTS_H_

#ifndef v_alduci
        void v_alduci(v_reg_t rd, v_reg_t rs, long offset, long align);
#endif
#ifndef v_alduci
        void v_aldci(v_reg_t rd, v_reg_t rs, long offset, long align);
#endif
#ifndef v_aldusi
        void v_aldusi(v_reg_t rd, v_reg_t rs, long offset, long align);
#endif
#ifndef v_aldsi
        void v_aldsi(v_reg_t rd, v_reg_t rs, long offset, long align);
#endif

#ifndef v_aldii
        void v_aldii(v_reg_t rd, v_reg_t rs, long offset, long align);
#endif

/* Note: we assume a 32-bit machine for these 3. */
#ifndef v_aldui
#	define v_aldui v_aldii
#endif
#ifndef v_aldpi
#	define v_aldpi v_aldii
#endif
#ifndef v_aldli
#	define v_aldli v_aldii
#endif


#endif /* _VCODE_PORTABLE_INSTS_H_ */
