/*
 * Used to dissect integer device code
 * into major (driver designation) and
 * minor (driver parameter) parts.
 */
/*
struct
{
	char	d_minor;
	char	d_major;
};
 */
#define GET_MINOR(__data__) ((char)(((__data__)>>8)&0xFF))
#define GET_MAJOR(__data__) ((char)( (__data__)    &0xFF))
#define SET_MINOR(__dst__,__src__) ((__dst__) = ((__dst__)&0x00FF) | (((__src__)<<8)&0xFF00))
#define SET_MAJOR(__dst__,__src__) ((__dst__) = ((__dst__)&0xFF00) | ( (__src__)    &0x00FF))

/*
 * Declaration of block device
 * switch. Each entry (row) is
 * the only link between the
 * main unix code and the driver.
 * The initialization of the
 * device switches is in the
 * file conf.c.
 */
extern struct	bdevsw
{
	int	(*d_open)();
	int	(*d_close)();
	int	(*d_strategy)();
	int	*d_tab;
} *bdevsw;

/*
 * Nblkdev is the number of entries
 * (rows) in the block switch. It is
 * set in binit/bio.c by making
 * a pass over the switch.
 * Used in bounds checking on major
 * device numbers.
 */
int	nblkdev;

/*
 * Character device switch.
 */
extern struct	cdevsw
{
	int	(*d_open)();
	int	(*d_close)();
	int	(*d_read)();
	int	(*d_write)();
	int	(*d_sgtty)();
} *cdevsw;

/*
 * Number of character switch entries.
 * Set by cinit/tty.c
 */
int	nchrdev;
