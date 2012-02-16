/*
 * KT-11 addresses and bits.
 */

int *UISD = (int*)(0177600);		/* first user I-space descriptor register */
int *UISA = (int*)(0177640);		/* first user I-space address register */
int *UDSA = (int*)(0177660);		/* first user D-space address register */
#define	RO	02		/* access abilities */
#define	WO	04
#define	RW	06
#define	ED	010		/* extend direction */

/*
 * structure used to address
 * a sequence of integers.
 */
/*
struct
{
	int	r[];
};
 */
int	*ka6;		/* 11/40 KISA6; 11/45 KDSA6 */

/*
 * address to access 11/70 UNIBUS map
 */
int *UBMAP = (int*)(0170200);
