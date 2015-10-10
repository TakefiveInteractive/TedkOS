;/*
; mtcp.h - Mouse/TuxController Protocol
; Mark Murphy 2006
;
; This header file defines the interface between the PC and the 
; Mouse/TuxController boards. The overall philosophy was to incorporate 
; both the functionality of the PS/2 Mouse (which is used by the 'layout'
; circuit simulator) as well as extend the functionality provided by the
; parallel-port based 2-button boards used previously for MP2's like the
; 'mazegame'.
;
; Commands sent from the PC to the MTC are variable length. Most are simple
; single-byte commands, however some require additional bytes to be sent.
; For example, to set the LED display, as many as 6 bytes can be sent. See
; the descriptions of individual commands for details.
;
; Responses from the MTC to the PC come in a single, 3-byte format. The
; three byte packets have a structure which is meant to allow for as much
; compatibility as possible with the PS/2 Mouse motion packet (see below).
; A few restrictions and changes had to be made to ensure higher reliability
; in the presence of untrustworthy operating systems .... 
;
; (Don't worry that there is a lot of words in these next few paragraphs,
;  pictures follow... )
;
; The high order bit of the first byte of a 3-byte packet will always be
; '0', and the high order bit of the second and third bytes will always be
; '1'. If the packet is a PS/2 Mouse emulation packet, then bit 6 will be '0'
; and bit 3 will be '1'; otherwise bit 6 will be '1' and bit 3 will be '0'.
; This leaves 5 bits in which to encode opcodes for non PS/2 responses, and
; 2 7-bit fields for data. 
;
; This limits the X,Y movement values for the PS/2 emulation to 8 bit 2's
; compliment integers (in the actual PS/2 spec, they are 9-bits). If you are
; familiar with the PS/2 spec, notice that this also fixes the Y,X overflow 
; fields to 0. 
;
;
; In a more visual form:
;
;   PS/2 Emulation:
;   +-7-+-6-+-5--+-4--+-3-+-2-+-1-+-0-+
; 0 | 0 | 0 | YS | XS | 1 | M | R | L |
;   +---+---+----+----+---+---+---+---+
; 1 | 1 |    X Movement               |
;   +---+---+----+----+---+---+---+---+
; 2 | 1 |    Y Movement               |
;   +---+---+----+----+---+---+---+---+
;
; Where:
; YS - Sign (8th bit) of the Y-movement value
; XS - Sign (8th bit) of the X-movement value
; M  - Status of Middle button (1 when button is down)
; R  - Status of Right button (1 when button is down)
; L  - Status of Left button (1 when button is down)
;
;  TuxController Responses:
;   +-7-+-6-+-5--+-4--+-3-+--2-+--1-+--0-+
; 0 | 0 | 1 | R4 | R3 | 0 | R2 | R1 | R0 |
;   +---+---+----+----+---+----+----+----+
; 1 | 1 |             DATA               |
;   +---+---+----+----+---+----+----+----+
; 2 | 1 |             DATA               |
;   +---+---+----+----+---+----+----+----+
;
; Where R[4:0] is the command response code, and the DATA fields are
; specific to each particular response.
; 
;*/

;/*
; (For reference)
; PS/2 Mouse movement packet format:
;
;	   7    6     5    4   3   2   1   0
;        -------------------------------------
; Byte 0 | YO | XO | YS | XS | 1 | M | R | L |
;        -------------------------------------
; Byte 1 | X Movement                        |
;        -------------------------------------
; Byte 2 | Y Movement                        |
;        -------------------------------------
; YO - Y overflow. Fixed 0 for MTC mouse packets
; XO - X overflow. Fixed 0 for MTC mouse packets
; YS - Y Sign
; XS - X Sign
; M  - Middle Button pressed (active high)
; R  - Right Button pressed (active high)
; L  - Left Button pressed (active high)
;*/


#define MOUSE_Y0 	0x80
#define MOUSE_X0 	0x40
#define MOUSE_YS 	0x20
#define MOUSE_XS 	0x10
#define MOUSE_MIDDLE 	0x04
#define MOUSE_RIGHT	0x02
#define MOUSE_LEFT	0x01



;/* 
; MTC to PC first bytes. Limited to 32 opcodes, however the 
; MTCP_BUTTON_POLL takes up 4 opcodes
; */

#define MTCP_RESP(n)	(((n)&7) | (((n)&0x18)<<1) | 0x40)
#define MTCP_IS_MOUSE(n) (0x08 == ((n)&0xC8))

#define MTCP_ACK	MTCP_RESP(0x0)

#define MTCP_BIOC_EVENT	MTCP_RESP(0x1)
#define MTCP_CLK_EVENT	MTCP_RESP(0x2)
#define MTCP_OFF_EVENT	MTCP_RESP(0x3)

#define MTCP_POLL_OK	MTCP_RESP(0x4)
#define MCTP_CLK_POLL	MTCP_RESP(0x5)
#define MTCP_RESET	MTCP_RESP(0x6)

#define MTCP_LEDS_POLL0	MTCP_RESP(0x8)
#define __LEDS_POLL01 	MTCP_RESP(0x9)
#define __LEDS_POLL02	MTCP_RESP(0xa)
#define __LEDS_POLL012 	MTCP_RESP(0xb)

#define MTCP_LEDS_POLL1	MTCP_RESP(0xC)
#define __LEDS_POLL11 	MTCP_RESP(0xd)
#define __LEDS_POLL12	MTCP_RESP(0xe)
#define __LEDS_POLL112 	MTCP_RESP(0xf)

#define MTCP_ERROR 	MTCP_RESP(0x1F)

;/* 
; PC to MTC Command Opcodes - don't change these without changing the order
; of the jump table in mainloop.asm. The defines limit the number to 32.
; */

#define MTCP_CMD_MASK 0x1F
#define MTCP_CMD_CHECK_MASK 0xE0
#define MTCP_CMD_CHECK 0xC0
#define MTCP_CMD(c)  (MTCP_CMD_CHECK | (c))

#define MTCP_OFF	MTCP_CMD(0x0)
#define MTCP_RESET_DEV	MTCP_CMD(0x1)

#define MTCP_POLL 	MTCP_CMD(0x2)
#define MTCP_BIOC_ON 	MTCP_CMD(0x3)
#define MTCP_BIOC_OFF 	MTCP_CMD(0x4)
#define MTCP_DBG_OFF	MTCP_CMD(0x5)

#define MTCP_LED_SET	MTCP_CMD(0x6)
#define MTCP_LED_CLK	MTCP_CMD(0x7)
#define MTCP_LED_USR	MTCP_CMD(0x8)
#define MTCP_POLL_LEDS	MTCP_CMD(0x13)

#define MTCP_CLK_RESET	MTCP_CMD(0x9)
#define MTCP_CLK_SET	MTCP_CMD(0xa)
#define MTCP_CLK_POLL	MTCP_CMD(0xb)
#define MTCP_CLK_RUN	MTCP_CMD(0xc)
#define MTCP_CLK_STOP	MTCP_CMD(0xd)	
#define MTCP_CLK_UP	MTCP_CMD(0xe)
#define MTCP_CLK_DOWN	MTCP_CMD(0xf)
#define MTCP_CLK_MAX	MTCP_CMD(0x10)

#define MTCP_MOUSE_OFF	MTCP_CMD(0x11)
#define MTCP_MOUSE_ON	MTCP_CMD(0x12)




;/*
; ************************** Commands ******************************** 
;
; Opcode: MTCP_RESET_DEV
;	Reset the device. The device doesn't immediately respond, but
;	generates an MTCP_RESET event when it is finished initializing
;	itself. 
;
; Opcode: MTCP_OFF
;	Turn the MTC off. The RESET button on the board will need to be
;	pressed for it to resume operation. It will generate an 
;	MTCP_OFF_EVENT before going to sleep. This is only a valid command
;	when debug mode is off (i.e. the PC has sent an MTCP_DBG_OFF
;	command).
; 
; Opcode: MTCP_POLL
; 	Poll Buttons - get the current status of the 8 buttons.
;	The bitmask is active low - a bit is clear when the corresponding
;	button is pressed. The bitmask is split into two bytes.
;
; 	Response packet:
;		byte 0 - MTCP_POLL_OK
;		byte 1  __7_____4___3___2___1_____0____
;			| 1 X X X | C | B | A | START |
;			-------------------------------
;		byte 2  __7_____4_____3______2______1_____0___
;			| 1 X X X | right | down | left | up |
;			--------------------------------------
;
; Opcode: MTCP_BIOC_ON
;	Enable Button interrupt-on-change. MTCP_ACK is returned.
;
; Opcode: MTCP_BIOC_OFF
;	Disable Button interrupt-on-change. MTCP_ACK is returned.
;
; Opcode MTCP_DBG_OFF
;	Disable Debug-lockup mode. This sets the error-handling behavior;
;	when debug-lockup is on, the device displays 0xdEAd on the LEDs and
;	ceases responding to commands. MTCP_ACK is returned
;
; Opcode MTCP_LED_SET
;	Set the User-set LED display values. These will be displayed on the
;	LED displays when the LED display is in USR mode (see the MTCP_LED_USR
;	and MTCP_LED_CLK commands). The first byte of argument specifies
;	which of the LED's to set, and also determines how many bytes will
;	follow - one byte for each led to set.
;
; 
; 	Mapping from 7-segment to bits
; 	The 7-segment display is:
;		  _A
;		F| |B
;		  -G
;		E| |C
;		  -D .dp
;
; 	The map from bits to segments is:
; 
; 	__7___6___5___4____3___2___1___0__
; 	| A | E | F | dp | G | C | B | D | 
; 	+---+---+---+----+---+---+---+---+
; 
; 	Arguments: >= 1 bytes
;		byte 0 - Bitmask of which LED's to set:

;		__7___6___5___4____3______2______1______0___
; 		| X | X | X | X | LED3 | LED2 | LED1 | LED0 | 
; 		----+---+---+---+------+------+------+------+
;
;	The number of bytes which should follow should be equal to the
;	number of bits set in byte 0. The bytes should be sent in order of 
;	increasing LED number. (e.g LED0, LED2, LED3 for a bitmask of 0x0D)
;
; 	Response: 1 byte
;		byte 0 - MTCP_ACK
;
; Opcode MTCP_LED_CLK
;	Put the LED display into clock mode. In this mode, the value of the
;	clock is displayed, rather than the value set with MTCP_LED_SET.
;
; Opcode MTCP_LED_USR
;	Put the LED display into user-mode. In this mode, the value specified
;	by the MTCP_LED_SET command is displayed.
; 	
; Opcode MTCP_CLK_RESET
;	Reset the clock. The clock value is set to zero, the direction is
;	set to down, and it is stopped.
;
; Opcode MTCP_CLK_SET
;	Set the value of the clock. There are two argument bytes, the first
;	is the number of minues, the second is the number of seconds. If
;	either argument is out of the range which makes sense, then the
;	result is undefined.
;
; 	Arguments: 2 bytes
;		byte 0 - Number of minutes to set (up to a maximum of 99).
;		byte 1 - number of seconds to set (up do a maximum of 59).
;
; Opcode MTCP_CLK_MAX
;	Set the maximum value of the clock. When the clock's direction
;	is set to up and it counts to this value, the MTC will generate
;	an MTCP_CLK_EVENT event.
; 	Arguments: 2 bytes
;		byte 0 - Number of minutes to set (up to a maximum of 99).
;	
; Opcode MTCP_CLK_POLL
; Command:
;	Read the state of the clock. The result specifies whether the
;	clock is running, its direction, and the current value in 
; 	minutes and seconds.
; 	Arguments: 0 bytes
; 	Response packet:
;		byte 0 : MTCP_POLL_OK
;		byte 1 :
;			+-7-+-6-5-4-3-2-1-0-+
;			| 1 |    Minutes    |
;			+---+---------------+
;		byte 2 :
;			+-7-+----6----+-5-4-3-2-1-0--+		
;			| 1 | Running |    Seconds   |
;			+---+---------+--------------+
;
; Opcode MTCP_CLK_RUN
;	Start the clock. If it was already running, then it will still
;	be running after this command.
;
; Opcode MTCP_CLK_STOP
;	Stop the clock. If it was already stopped, then it will still be
;	stopped after this command.
;
; Opcode MTCP_CLK_UP
;	Set the clock's direction to up. When running, It will count up
;	until reaching the value set by MTCP_CLK_MAX, then generate a 
;	CLK_EVENT
;
; Opcode MTCP_CLK_DOWN
;	Set the clock"s direction to Down. When running, it will count down
;	until reaching zero, then generate a CLK_EVENT.
;
;*/	

;/*
; ********************* Responses *************************
;
; MTCP_ERROR
;	Response when the MTC has debug-lockup disabled and receives an
;	unrecognized request from the PC.
;
; MTCP_ACK
;	Response when the MTC successfully completes a command.
;
; MTCP_POLL_OK	
;	First byte of a response to a poll of either the buttons or the
;	clock.
;
; MTCP_RESET
; 	Generated when the devide re-initializes itself after a power-up, 
;	a RESET button press, or an MTCP_RESET_DEV command.
;
;	Packet Format:
;		Byte 0 - MTCP_RESET
;		Byte 1 - reserved
;		Byte 2 - reserved
;
; MTCP_OFF
;	Generated when the device is turning itself off after either an
;	MTCP_OFF command or its internal 2-hour timer has expired. This
;	timer is reset anytime there is serial communication activity,
;	either to or from the device.
;
;	Packet Format:
;		Byte 0 - MTCP_OFF_EVENT
;		Byte 1 - reserved
;		Byte 2 - reserved
;
; MTCP_BIOC_EVT	
;	Generated when the Button Interrupt-on-change mode is enabled and 
;	a button is either pressed or released.
;
; 	Packet format:
;		Byte 0 - MTCP_BIOC_EVENT
;		byte 1  +-7-----4-+-3-+-2-+-1-+---0---+
;			| 1 X X X | C | B | A | START |
;			+---------+---+---+---+-------+
;		byte 2  +-7-----4-+---3---+--2---+--1---+-0--+
;			| 1 X X X | right | down | left | up |
;			+---------+-------+------+------+----+
; MTCP_CLK_EVENT
;	The clock has reached zero (when counting down) or its maximum value
;	(when counting up).
;
; 	Packet format:
;		Byte 0 - MTCP_CLK_EVENT
;		Byte 1 - reserved
;		Byte 2 - reserved
;
; MTCP_LEDS_POLL
;	Since there are 32 bits of data to return with this response, the 
;	data format is a bit screwy. It will be 2 3-byte packets of the
; 	form described above, with a slight modification: the opcode will
;	be used to encode some of the data. There is a different opcode 
;	for the first and second packets (1 bit of difference) to allow for
;	detection of a dropped packet.
;	The data from LED's 0 and 1 will be in the first packet, and the 
;	data from LED's 2 and 3 will be in the second packet. The data will
;	be of the same format as the LED_SET command, except that the segA
;	bits will be packed into the opcode, as shown in the diagram below
;	
;	In picto-gram form:
;	Packet 0 will be:
; 	  +-7-+-6-+-5-+--4-+-3-+-2-+-1--+-0--+
; 	0 | 0 | 1 | 0 |  1 | 0 | 0 | A1 | A0 | 
; 	  +---+---+---+----+---+---+----+----+
; 	1 | 1 | E | F | dp | G | C | B  | D  | (data for LED0)
; 	  +---+---+---+----+---+---+----+----+
; 	2 | 1 | E | F | dp | G | C | B  | D  | (data for LED1)
; 	  +---+---+---+----+---+---+----+----+
;
;	Packet 1 will be:
; 	  +-7-+-6-+-5-+--4-+-3-+-2-+-1--+-0--+
; 	0 | 0 | 1 | 0 |  1 | 0 | 1 | A1 | A0 | 
; 	  +---+---+---+----+---+---+----+----+
; 	1 | 1 | E | F | dp | G | C | B  | D  | (data for LED2)
; 	  +---+---+---+----+---+---+----+----+
; 	2 | 1 | E | F | dp | G | C | B  | D  | (data for LED3)
; 	  +---+---+---+----+---+---+----+----+
;	
;*/
