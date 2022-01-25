/*
 * Allocation of buffer headers
 */

struct	buf	buf[NBUF];

/*
 * Head of the available list for buffers
 */
struct	buf bfreelist;
