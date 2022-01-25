#include	"../param.h"
#include	"../systm.h"
#include	"../user.h"
#include	"../userx.h"
#include	"../inode.h"

/*
 * Perform process accounting functions.
 */

sysacct()
{
	extern uchar();
	register struct inode *ip;

	if (suser()) {
		if (u.u_arg[0]==0) {
			if (acctp) {
				plock(acctp);
				iput(acctp);
				acctp = NULL;
			}
			return;
		}
		if (acctp) {
			u.u_error = EBUSY;
			return;
		}
		if ((ip = namei(&uchar, 0))==NULL)
			return;
		if((ip->i_mode & IFMT) != 0) {
			u.u_error = EACCES;
			iput(ip);
			return;
		}
		acctp = ip;
		prele(ip);
	}
}

/*
 * On exit, write a record on the accounting file.
 */
acct()
{
	register struct inode *ip;
	register i, j;

	if ((ip=acctp)==NULL)
		return;
	plock(ip);
	for (i=0; i<DIRSIZ; i++)
		acctbuf.ac_comm[i] = u.u_comm[i];
	acctbuf.ac_utime = u.u_utime;
	acctbuf.ac_stime = u.u_stime;
	acctbuf.ac_etime = time - u.u_start;
	acctbuf.ac_uid = u.u_ruid;
	acctbuf.ac_flag = u.u_acflag;
	i = ip->i_size0&0377;
	j = ip->i_size1;
	u.u_offset[0] = i;
	u.u_offset[1] = j;
	u.u_base = &acctbuf;
	u.u_count = sizeof(acctbuf);
	u.u_segflg = 1;
	u.u_error = 0;
	writei(ip);
	if(u.u_error) {
		ip->i_size0 = i;
		ip->i_size1 = j;
	}
	prele(ip);
}
