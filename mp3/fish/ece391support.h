#if !defined(ECE391SUPPORT_H)
#define ECE391SUPPORT_H

extern uint32_t ece391_strlen (const uint8_t* s);
extern void ece391_strcpy (uint8_t* dst, const uint8_t* src);
extern void ece391_fdputs (int32_t fd, const uint8_t* s);
extern int32_t ece391_strcmp (const uint8_t* s1, const uint8_t* s2);
extern int32_t ece391_strncmp (const uint8_t* s1, const uint8_t* s2, 
			       uint32_t n);

#endif /* ECE391SUPPORT_H */
