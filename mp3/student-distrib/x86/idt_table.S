
.global raw_interrupt_handlers

.text
// Note: PUSHAD pushes 32 bytes on to stack
#define HASERR(ID)        ; \
handler##ID:              ; \
    pushl %eax            ; \
    movl 4(%esp), %eax    ; \
    movl %eax, -32+4(%esp); \
    popl %eax             ; \
    addl $4, %esp         ; \
    pushal                ; \
    subl $4, %esp         ; \
    pushl $##ID           ; \
    call interrupt_handler_with_number  ; \
    addl $8, %esp         ; \
    popal                 ; \
    iret

#define  NOERR(ID)        ; \
handler##ID:              ; \
    pushal                ; \
    pushl $0              ; \
    pushl $##ID           ; \
    call interrupt_handler_with_number  ; \
    addl $8, %esp         ; \
    popal                 ; \
    iret

NOERR(0)
NOERR(1)
NOERR(2)
NOERR(3)
NOERR(4)
NOERR(5)
NOERR(6)
NOERR(7)
HASERR(8)
NOERR(9)
HASERR(10)
HASERR(11)
HASERR(12)
HASERR(13)
HASERR(14)
NOERR(15)
NOERR(16)
HASERR(17)
NOERR(18)
NOERR(19)
NOERR(20)
NOERR(21)
NOERR(22)
NOERR(23)
NOERR(24)
NOERR(25)
NOERR(26)
NOERR(27)
NOERR(28)
NOERR(29)
NOERR(30)
NOERR(31)
NOERR(32)
NOERR(33)
NOERR(34)
NOERR(35)
NOERR(36)
NOERR(37)
NOERR(38)
NOERR(39)
NOERR(40)
NOERR(41)
NOERR(42)
NOERR(43)
NOERR(44)
NOERR(45)
NOERR(46)
NOERR(47)
NOERR(128)

#define ITEM(ID) handler##ID
#define NOITEM(ID) 0

.data
raw_interrupt_handlers:
.long _raw_interrupt_handlers
_raw_interrupt_handlers:

.long ITEM(0),ITEM(1),ITEM(2),ITEM(3),ITEM(4),ITEM(5),ITEM(6),ITEM(7),ITEM(8),ITEM(9),ITEM(10),ITEM(11),ITEM(12),ITEM(13),ITEM(14),ITEM(15),ITEM(16),ITEM(17),ITEM(18),ITEM(19),ITEM(20),ITEM(21),ITEM(22),ITEM(23),ITEM(24),ITEM(25),ITEM(26),ITEM(27),ITEM(28),ITEM(29),ITEM(30),ITEM(31),ITEM(32),ITEM(33),ITEM(34),ITEM(35),ITEM(36),ITEM(37),ITEM(38),ITEM(39),ITEM(40),ITEM(41),ITEM(42),ITEM(43),ITEM(44),ITEM(45),ITEM(46),ITEM(47),NOITEM(48),NOITEM(49),NOITEM(50),NOITEM(51),NOITEM(52),NOITEM(53),NOITEM(54),NOITEM(55),NOITEM(56),NOITEM(57),NOITEM(58),NOITEM(59),NOITEM(60),NOITEM(61),NOITEM(62),NOITEM(63),NOITEM(64),NOITEM(65),NOITEM(66),NOITEM(67),NOITEM(68),NOITEM(69),NOITEM(70),NOITEM(71),NOITEM(72),NOITEM(73),NOITEM(74),NOITEM(75),NOITEM(76),NOITEM(77),NOITEM(78),NOITEM(79),NOITEM(80),NOITEM(81),NOITEM(82),NOITEM(83),NOITEM(84),NOITEM(85),NOITEM(86),NOITEM(87),NOITEM(88),NOITEM(89),NOITEM(90),NOITEM(91),NOITEM(92),NOITEM(93),NOITEM(94),NOITEM(95),NOITEM(96),NOITEM(97),NOITEM(98),NOITEM(99),NOITEM(100),NOITEM(101),NOITEM(102),NOITEM(103),NOITEM(104),NOITEM(105),NOITEM(106),NOITEM(107),NOITEM(108),NOITEM(109),NOITEM(110),NOITEM(111),NOITEM(112),NOITEM(113),NOITEM(114),NOITEM(115),NOITEM(116),NOITEM(117),NOITEM(118),NOITEM(119),NOITEM(120),NOITEM(121),NOITEM(122),NOITEM(123),NOITEM(124),NOITEM(125),NOITEM(126),NOITEM(127),ITEM(128)
