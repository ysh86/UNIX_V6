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
#define GET_D_MINOR(__data__) ( (__data__)    &0xff)
#define GET_D_MAJOR(__data__) (((__data__)>>8)&0xff)
#define SET_D_MINOR(__dst__,__src__) ((__dst__) = ((__dst__)&0xff00) | ( (__src__)    &0x00ff))
#define SET_D_MAJOR(__dst__,__src__) ((__dst__) = ((__dst__)&0x00ff) | (((__src__)<<8)&0xff00))

/*
 * Declaration of block device
 * switch. Each entry (row) is
 * the only link between the
 * main unix code and the driver.
 * The initialization of the
 * device switches is in the
 * file conf.c.
 */
struct	bdevsw
{
	int	(*d_open)();
	int	(*d_close)();
	int	(*d_strategy)();
	int	*d_tab;
} *bdevsw;

/*
 * Character device switch.
 */
struct	cdevsw
{
	int	(*d_open)();
	int	(*d_close)();
	int	(*d_read)();
	int	(*d_write)();
	int	(*d_sgtty)();
} *cdevsw;
