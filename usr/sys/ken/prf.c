#
#include "../param.h"
#include "../seg.h"
#include "../buf.h"
#include "../bufx.h"
#include "../conf.h"
#include "../systm.h"

/*
 * Address and structure of the
 * KL-11 console device registers.
 */
struct
{
	int	rsr;
	int	rbr;
	int	xsr;
	int	xbr;
};
char	*msgbufp msgbuf;	/* Next saved printf character */

/*
 * In case console is off,
 * panicstr contains argument to last
 * call to panic.
 */

char	*panicstr;

/*
 * Scaled down version of C Library printf.
 * Only %s %l %d (==%l) %o are recognized.
 * Used to print diagnostic information
 * directly on console tty.
 * Since it is not interrupt driven,
 * all system activities are pretty much
 * suspended.
 * Printf should not be used for chit-chat.
 */
printf(fmt,x1)
char fmt[];
{
	register char *s;
	register *adx, c;

	adx = &x1;
loop:
	while((c = *fmt++) != '%') {
		if(c == '\0')
			return;
		putchar(c);
	}
	c = *fmt++;
	if(c == 'd' || c == 'l' || c == 'o')
		printn(*adx, c=='o'? 8: 10);
	if(c == 's') {
		s = *adx;
		while(c = *s++)
			putchar(c);
	}
	adx++;
	goto loop;
}

/*
 * Print an unsigned integer in base b.
 */
printn(n, b)
{
	register a;

	if(a = ldiv(n, b))
		printn(a, b);
	putchar(lrem(n, b) + '0');
}

/*
 * Print a character on console.
 * Attempts to save and restore device
 * status.
 * If the switches are 0, all
 * printing is inhibited.
 *
 * Whether or not printing is inhibited,
 * the last MSGBUFS characters
 * are saved in msgbuf for inspection later.
 */
putchar(c)
{
	register rc, s, timo;

	rc = c;
	if (rc!='\0' && rc!='\r' && rc!=0177) {
		*msgbufp++ = rc;
		if (msgbufp >= &msgbuf[MSGBUFS])
			msgbufp = msgbuf;
	}
	if(SW->integ == 0)
		return;
	timo = 30000;
	/*
	 * Try waiting for the console tty to come ready,
	 * otherwise give up after a reasonable time.
	 */
	while((KL->xsr&0200)==0 && --timo!=0)
		;
	if(rc == 0)
		return;
	s = KL->xsr;
	KL->xsr = 0;
	KL->xbr = rc;
	if(rc == '\n') {
		putchar('\r');
		putchar(0177);
		putchar(0177);
	}
	putchar(0);
	KL->xsr = s;
}

/*
 * Panic is called on unresolvable
 * fatal errors.
 * It syncs, prints "panic: mesg" and
 * then loops.
 */
panic(s)
char *s;
{
	panicstr = s;
	update();
	printf("panic: %s\n", s);
	for(;;)
		idle();
}

/*
 * prdev prints a warning message of the
 * form "mesg on dev x/y".
 * x and y are the major and minor parts of
 * the device argument.
 */
prdev(str, dev)
{

	printf("%s on dev %l/%l\n", str, dev.d_major, dev.d_minor);
}

/*
 * deverr prints a diagnostic from
 * a device driver.
 * It prints the device, block number,
 * and an octal word (usually some error
 * status register) passed as argument.
 */
deverror(bp, o1, o2)
int *bp;
{
	register *rbp;

	rbp = bp;
	prdev("err", rbp->b_dev);
	printf("bn%l er%o %o\n", rbp->b_blkno, o1, o2);
}
