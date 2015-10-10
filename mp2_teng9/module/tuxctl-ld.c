/* tuxctl-ld.c
 * The line discipline for the MP2 Tuxcontroller wrapper. 
 * Mark Murphy 2006
 * Andrew Ofisher 2007
 * Puskar Naha 2013
 */

#include <linux/tty.h>
#include <linux/tty_ldisc.h>

#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>

#include <linux/init.h>
#include "tuxctl-ld.h"

#define uhoh(str, ...) printk(KERN_EMERG "%s " str, __FUNCTION__, ##__VA_ARGS__)
#define debug(str, ...) printk(KERN_DEBUG "%s " str, __FUNCTION__,\
							 ## __VA_ARGS__)

/* Here's an interesting tidbit: tty_struct has no synchronization available
 * to us to protect against races involving the tty->disc_data field. So,
 * rather than do something tricky, we'll employ the 'one-big-freakin-lock'
 * strategy for synchronizing the methods in this file. This can't be a
 * semaphore because of the following chain of function calls:
 *
 * rs_interrupt()  				(serial.c)
 *	tty_flip_buffer_push() 			(tty_io.c)
 * 		flush_to_ldisc() 		(tty_io.c)
 *			ldisc.receive_buf() 	(this file)
 *
 * Specifically, rs_interrupt is, well, an interrupt. Sleeping in an 
 * interrupt is a recipe for breaking things, so a spinlock it is.
 */
static spinlock_t tuxctl_ldisc_lock = SPIN_LOCK_UNLOCKED;


/* Line Discipline specific stuff */
#define TUXCTL_MAGIC 0x74757863

#define sanity(data) if((data)->magic != TUXCTL_MAGIC){\
			uhoh("MAGIC MISMATCH!\n");\
			BUG();}
			

static int tuxctl_ldisc_open(struct tty_struct*);
static void tuxctl_ldisc_close(struct tty_struct*);
static void tuxctl_ldisc_rcv_buf(struct tty_struct*, const unsigned char *, 
					char *, int);
static void tuxctl_ldisc_write_wakeup(struct tty_struct*);
static void tuxctl_ldisc_data_callback(struct tty_struct *tty);

#define TUXCTL_BUFSIZE 64
typedef struct tuxctl_ldisc_data {
	unsigned long magic;

	char rx_buf[TUXCTL_BUFSIZE];
	int rx_start, rx_end;

	char tx_buf[TUXCTL_BUFSIZE];
	int tx_start, tx_end;

} tuxctl_ldisc_data_t;


static struct tty_ldisc tuxctl_ldisc  = {
	.magic = TUXCTL_MAGIC,
	.name = "tuxcontroller",
	.open = tuxctl_ldisc_open,
	.close = tuxctl_ldisc_close,
        .ioctl = tuxctl_ioctl,
	.receive_buf = tuxctl_ldisc_rcv_buf,
	.write_wakeup = tuxctl_ldisc_write_wakeup,
};

int __init
tuxctl_ldisc_init(void)
{
	int err = 0;
	if((err = tty_register_ldisc(N_MOUSE, &tuxctl_ldisc))){
		debug("tuxctl line discipline register failed\n");
	}else{
		printk("tuxctl line discipline registered\n");
	}
	return err;
}
module_init(tuxctl_ldisc_init);

void __exit
tuxctl_ldisc_exit(void)
{
	tty_unregister_ldisc(N_MOUSE);
	printk("tuxctl line discipline removed\n");
}
module_exit(tuxctl_ldisc_exit);


#define buf_used(start,end) ((start) <= (end) ? \
				  ((end) - (start))  \
				: (TUXCTL_BUFSIZE + (end) - (start)))

#define buf_room(start,end) (TUXCTL_BUFSIZE - buf_used(start,end) - 1)

#define buf_empty(start, end) ((start) == (end))
#define buf_full(start, end) ((((end)+1)%TUXCTL_BUFSIZE) == (start))
#define buf_incidx(idx) ((idx) = ((idx)+1) % TUXCTL_BUFSIZE)


static int 
tuxctl_ldisc_open(struct tty_struct *tty)
{
	tuxctl_ldisc_data_t *data;
	unsigned long flags;

	if(!(data = kmalloc(sizeof(*data), GFP_KERNEL))){
		uhoh("kmalloc failed!\n");
		return -ENOMEM;
	}
	
	spin_lock_irqsave(&tuxctl_ldisc_lock, flags);

	data->magic = TUXCTL_MAGIC;

	data->rx_start = 0;
	data->rx_end = 0;

	data->tx_start = 0;
	data->tx_end = 0;
	tty->disc_data = data;

	spin_unlock_irqrestore(&tuxctl_ldisc_lock, flags);

	return 0;
}

static void 
tuxctl_ldisc_close(struct tty_struct *tty)
{
	tuxctl_ldisc_data_t *data; 
	unsigned long flags;

	spin_lock_irqsave(&tuxctl_ldisc_lock, flags);

	data = tty->disc_data;
	tty->disc_data = 0;

	spin_unlock_irqrestore(&tuxctl_ldisc_lock, flags);

	kfree(data);
}


/* tuxctl_ldisc_rcv_buf
 * The receive_buf() method of our line discipline. It receives count bytes
 * from cp. fp points to some flag/error bytes which I conveniently ignore. 
 * This is called when there are bytes received from the serial driver, and
 * is called from an interrupt handler.
 */
static void 
tuxctl_ldisc_rcv_buf(struct tty_struct *tty, const unsigned char *cp, 
			char *fp, int count)
{
	int call = 0, c=0;
	unsigned long flags;
	tuxctl_ldisc_data_t *data;

	spin_lock_irqsave(&tuxctl_ldisc_lock, flags);
	if(0 != (data = tty->disc_data)){
		call = 1;

		while(count-- > 0 && !buf_full(data->rx_start, data->rx_end)) {
			data->rx_buf[data->rx_end] = *cp++;
			buf_incidx(data->rx_end);
			c++;
		}
	}

	spin_unlock_irqrestore(&tuxctl_ldisc_lock, flags);

	if(call){
		tuxctl_ldisc_data_callback(tty);
	}
}

/* tuxctl_ldisc_write_wakeup()
 * Called by the lower level serial driver when it can accept more 
 */
static void 
tuxctl_ldisc_write_wakeup(struct tty_struct *tty)
{
	tuxctl_ldisc_data_t *data;
	int sent, n = 0, room;
	char buf[TUXCTL_BUFSIZE];
	unsigned long flags;

	/* I hope that this doesn't need synchronization. */
	room = tty->driver->write_room(tty);
	
	spin_lock_irqsave(&tuxctl_ldisc_lock, flags);
	data = tty->disc_data;

	while(n <= room && !buf_empty(data->tx_start, data->tx_end)){
		buf[n++] = data->tx_buf[data->tx_start];
		buf_incidx(data->tx_start);
	}

	spin_unlock_irqrestore(&tuxctl_ldisc_lock, flags);

	sent = tty->driver->write(tty, buf, n);

	if(sent != n){
		debug("driver lied to us? We lost some data");
	}
}

/*********** Interface to the char driver ********************/


/* tuxctl_ldisc_get()
 * Read bytes that the line-discipline has received from the controller.
 * Returns the number of bytes actually read, or  -1 on error (if, for
 * example, the first argument is invalid.
 */
int 
tuxctl_ldisc_get(struct tty_struct *tty, char *buf, int n)
{
	tuxctl_ldisc_data_t *data;
	unsigned long flags;
	int r = 0;

	spin_lock_irqsave(&tuxctl_ldisc_lock, flags);
	data = tty->disc_data;
	while(n-- > 0 && !buf_empty(data->rx_start, data->rx_end)){
		*buf++ = data->rx_buf[data->rx_start];
		buf_incidx(data->rx_start);
		r++;
	}
	spin_unlock_irqrestore(&tuxctl_ldisc_lock, flags);

	return r;
}

/* tuxctl_ldisc_put()
 * Write bytes out to the device. Returns the number of bytes *not* written.
 * This means, 0 on success and >0 if the line discipline's internal buffer
 * is full.
 */
int 
tuxctl_ldisc_put(struct tty_struct *tty, char const *buf, int n)
{
	tuxctl_ldisc_data_t *data;
	unsigned long flags;

	spin_lock_irqsave(&tuxctl_ldisc_lock, flags);

	data = tty->disc_data;

	while (n > 0 && !buf_full(data->tx_start, data->tx_end)) {
		data->tx_buf[data->tx_end] = *buf++;
		buf_incidx(data->tx_end);
		--n;
	}

	spin_unlock_irqrestore(&tuxctl_ldisc_lock, flags);

	/* Potential race conditions here ...  ?*/

	tuxctl_ldisc_write_wakeup(tty);

	return n;
}

/* tuxctl_ldisc_data_callback()
 * This is the function called from the line-discipline when data is
 * available from the device. This is how responses to polling the buttons
 * and ACK's for setting the LEDs will be transmitted to the tuxctl driver.
 * The tuxctl driver must implement this function. The 'tty' parameter
 * can be passed back to the tuxctl_ldisc_read function to read
 * data from the lower-level buffers.
 *
 * IMPORTANT: This function is called from an interrupt context, so it 
 *            cannot acquire any semaphores or otherwise sleep, or access
 *            the 'current' pointer. It also must not take up too much time.
 */
static void tuxctl_ldisc_data_callback(struct tty_struct *tty)
{
	/* Should probably synchronize these */
	static unsigned char saved[2];
	static int n_saved = 0;
	
	unsigned char packet[12], *p;
	int n, i = 0, j;

	for(i=0; i< n_saved; i++)
		packet[i] = saved[i];
	
	n = n_saved + tuxctl_ldisc_get(tty, packet + n_saved,  12 - n_saved);

	/* Look at all bytes as potential packet beginnings except for 
	 * the last two bytes. Save them for next time. */
	for(i = 0; i < n-2; ){
		p = packet + i;

		/* Check the framing bits to detect lost bytes */
		if(!(p[0]&0x80) && p[1]&0x80 && p[2]&0x80){
			tuxctl_handle_packet(tty, p);
			i += 3;
		}else{
			i++;
		}
	}

	/* Save the leftovers - extra bytes for breakfast */
	n_saved = n - i;
	for(j = 0; j < n_saved; j++)
		saved[j] = packet[i+j];
}

