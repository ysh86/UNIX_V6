/*
 * tunable variables
 */

#define	NBUF	32		/* size of buffer cache */
#define	NINODE	125		/* number of in core inodes */
#define	NFILE	125		/* number of in core file structures */
#define	NMOUNT	8		/* number of mountable file systems */
#define	NEXEC	3		/* number of simultaneous exec's */
#define	MAXMEM	(64*32)		/* max core per process - first # is Kw */
#define	SSIZE	20		/* initial stack size (*64 bytes) */
#define	SINCR	20		/* increment of stack (*64 bytes) */
#define	NOFILE	20		/* max open files per process */
#define	FCLFILE	15		/* First file auto-closed on exec */
#define	CANBSIZ	256		/* max size of typewriter line */
#define	CMAPSIZ	50		/* size of core allocation area */
#define	SMAPSIZ	50		/* size of swap allocation area */
#define	NCALL	20		/* max simultaneous time callouts */
#define	NPROC	65		/* max number of processes */
#define	NTEXT	40		/* max number of pure texts */
#define	NCLIST	150		/* max total clist size */
#define	HZ	60		/* Ticks/second of the clock */
#define	MSGBUFS	128		/* Characters saved from error messages */
#define	NCARGS	5120		/* # characters in exec arglist */

/*
 * priorities
 * probably should not be
 * altered too much
 */

#define	PSWP	-100
#define	PINOD	-90
#define	PRIBIO	-50
#define	PPIPE	1
#define	PWAIT	40
#define	PSLEP	90
#define	PUSER	100

/*
 * signals
 * dont change
 */

#define	NSIG	17
/*
 * No more than 16 signals (1-17) because they are
 * stored in bits in a word.
 */
#define	SIGHUP	1	/* hangup */
#define	SIGINT	2	/* interrupt (rubout) */
#define	SIGQIT	3	/* quit (FS) */
#define	SIGINS	4	/* illegal instruction */
#define	SIGTRC	5	/* trace or breakpoint */
#define	SIGIOT	6	/* iot */
#define	SIGEMT	7	/* emt */
#define	SIGFPT	8	/* floating exception */
#define	SIGKIL	9	/* kill, uncatchable termination */
#define	SIGBUS	10	/* bus error */
#define	SIGSEG	11	/* segmentation violation */
#define	SIGSYS	12	/* bad system call */
#define	SIGPIPE	13	/* end of pipe */
#define	SIGCLK	14	/* alarm clock */
#define	SIGTRM	15	/* Catchable termination */

/*
 * fundamental constants
 * cannot be changed
 */

#define	USIZE	16		/* size of user block (*64) */
#define	NULL	0
#define	NODEV	(-1)
#define	ROOTINO	1		/* i number of all roots */
#define	DIRSIZ	14		/* max characters per directory */

/*
 * structure to access an
 * integer in bytes
 */
/*
struct
{
	char	lobyte;
	char	hibyte;
};
*/
#define GET_LOBYTE(__data__) ( (__data__)    &0xff)
#define GET_HIBYTE(__data__) (((__data__)>>8)&0xff)
#define SET_LOBYTE(__dst__,__src__) ((__dst__) = ((__dst__)&0xff00) | ( (__src__)    &0x00ff))
#define SET_HIBYTE(__dst__,__src__) ((__dst__) = ((__dst__)&0x00ff) | (((__src__)<<8)&0xff00))

/*
 * structure to access an integer
 */
/*
struct
{
	short	integ;
};
*/
#define GET_INTEG(__ptr__)         (*(short *)(__ptr__) & 0xffff)
#define SET_INTEG(__prt__,__src__) (*(short *)(__ptr__) = (__src__))


/*
 * structure to access a long as integers
 */
/*
struct {
	short	hiword;
	short	loword;
};
*/
#define GET_LOWORD(__data__) ((short)( (__data__)     &0xffff))
#define GET_HIWORD(__data__) ((short)(((__data__)>>16)&0xffff))
#define SET_LOWORD(__dst__,__src__) ((__dst__) = ((__dst__)&0xffff0000) | ( (__src__)     &0x0000ffff))
#define SET_HIWORD(__dst__,__src__) ((__dst__) = ((__dst__)&0x0000ffff) | (((__src__)<<16)&0xffff0000))


/*
 * Certain processor registers
 */
#define PS	0177776
#define KL	0177560
#define SW	0177570

/*
 * Some macros for units conversion
 */
/* Core clicks (64 bytes) to segments */
#define	ctos(x)	((x+127)>>7)

/* Core clicks (64 bytes) to blocks */
#define	ctob(x)	((x+7)>>3)
