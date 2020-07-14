#ifndef __API_DBG_MODULE__
#define __API_DBG_MODULE__

#ifdef __cplusplus
#define	VARG_PROTO	...
#else
#define	VARG_PROTO
#endif	/* __cplusplus */

#ifdef __cplusplus
extern "C" {
#endif

extern int			dbgprint		(const char *format , ... );
extern int			tprint0n		(const char *format , ... );
extern int			tprint1n		(const char *format , ... );
extern int			rprint0n		(const char *format , ... );
extern int			rprint1n		(const char *format , ... );

#ifdef __cplusplus
}
#endif

#endif/*__API_DBG_MODULE__*/
