.global legacyInt_noLock
.global legacyInt_unreal_entry
.global legacyInt_16bit_entry

.data
#include <inc/x86/real.h>
#include <inc/x86/desc.h>

.text
.code32

legacyInt_noLock:

    cli

    pushl %ebp
    movl  %esp, %ebp

    pushl %ebp
    pushl %ebx
    pushl %esi
    pushl %edi

    movl  %cr0, %eax
    pushl %eax

    movl  %cr4, %eax
    pushl %eax

    movl  %cr3, %eax
    pushl %eax


    # Currently the kernel stack looks like:
    # [ leave, ebp (ours), ebx, esi, edi, cr0, cr4, cr3 -> 
    # backup current Stack ESP
    movl  %esp, (cpu0_real_context)

    # ALL CONTEXTs are saved by here.

    # Turn OFF paging
    movl  %cr0, %eax
    andl  $~REAL_MODE_PG_BIT, %eax
    movl  %eax, %cr0
    movl  $0, %eax
    movl  %eax, %cr3

    ljmp  $CS_16BIT_SEL, $REAL_MODE_CODE_BASE * 16

back_32bit_entry:
    movl  $KERNEL_DS_SEL, %esp
    movl  %esp, %ds
    movl  %esp, %ss
    movl  %esp, %fs
    movl  %esp, %gs
    movl  %esp, %es

    # return to correct stack
    movl  cpu0_real_context, %esp
    pushw %dx
    pushw %cx
    pushw %bx
    pushw %ax
    pushw %si
    pushw %di
    pushw $0                    # this will save context to cpu0_real_context
    call save_real_context
    addl  $7*2, %esp

    popl  %eax
    popl  %ecx
    popl  %edx
    movl  %eax, %cr3
    movl  %ecx, %cr4
    movl  %edx, %cr0
    popl  %edi
    popl  %esi
    popl  %ebx
    popl  %ebp

    # restore idt
    lidt  idtr_val

    # Up till now, we only have stack frame (for "leave") left
    leave
    ret

.code16

legacyInt_16bit_entry:
    movw  $DS_16BIT_SEL, %ax
    movw  %ax, %ds
    movw  %ax, %es
    movw  %ax, %fs
    movw  %ax, %gs
    movw  %ax, %ss
    movl  $0 , %esp

    # enter real mode!
    movl  %cr0, %eax
    andl  $~REAL_MODE_BIT, %eax
    movl  %eax, %cr0
    ljmp  $REAL_MODE_CODE_BASE, $real_mode_entry - legacyInt_16bit_entry

real_mode_entry:
    movw  $REAL_MODE_DATA_BASE, %ax
    movw  %ax, %ss
    movw  %ax, %ds

    movw  $REAL_MODE_CODE_BASE , %ax
    movw  %ax, %es
    movw  %ax, %fs
    movw  %ax, %gs

    // 0x700 should be enough to skip the saved input (8 16-bit regs)
    STACK_FROM = 0x700
    movw  $STACK_FROM, %sp

    pushw $0        # base_high
    pushw $0        # base_low
    pushw $0x3ff    # limit

    # initializae IDT / IVT
    lidt  STACK_FROM - 4 - 2

    # we use bp to store interrupt number
    movw  0 , %bp

    // Calculate: *(%bp + %bp + interrupt_callers - legacyInt_16bit_entry)
    add  %bp, %bp
    add  $interrupt_callers - legacyInt_16bit_entry, %bp
    mov  %es:(%bp), %bp

    movw  4+2 , %ax
    movw  4+4 , %bx
    movw  4+6 , %cx
    movw  4+8 , %dx

    movw  4+10, %di
    movw  4+12, %si

    movw  4+16, %es

    # calls the jump table to call the correct interrupt NUMBER
    jmp %bp

start_return_to_protected:
    # ----- From now on: Working on returning to protected mode -----
    cli

    # No need to lgdt gdtr_val, because we've loaded it once.

    movl $REAL_MODE_BIT, %esp
    movl %esp, %cr0
    ljmpl $KERNEL_CS_SEL, $back_32bit_entry

#define DEF_INT(ID) call_int_##ID: int $##ID; jmp start_return_to_protected;
#define USE_INT(ID) call_int_##ID - legacyInt_16bit_entry

DEF_INT(0) DEF_INT(1) DEF_INT(2) DEF_INT(3) DEF_INT(4) DEF_INT(5) DEF_INT(6) DEF_INT(7) DEF_INT(8) DEF_INT(9) DEF_INT(10) DEF_INT(11) DEF_INT(12) DEF_INT(13) DEF_INT(14) DEF_INT(15) DEF_INT(16) DEF_INT(17) DEF_INT(18) DEF_INT(19) DEF_INT(20) DEF_INT(21) DEF_INT(22) DEF_INT(23) DEF_INT(24) DEF_INT(25) DEF_INT(26) DEF_INT(27) DEF_INT(28) DEF_INT(29) DEF_INT(30) DEF_INT(31) DEF_INT(32) DEF_INT(33) DEF_INT(34) DEF_INT(35) DEF_INT(36) DEF_INT(37) DEF_INT(38) DEF_INT(39) DEF_INT(40) DEF_INT(41) DEF_INT(42) DEF_INT(43) DEF_INT(44) DEF_INT(45) DEF_INT(46) DEF_INT(47) DEF_INT(48) DEF_INT(49) DEF_INT(50) DEF_INT(51) DEF_INT(52) DEF_INT(53) DEF_INT(54) DEF_INT(55) DEF_INT(56) DEF_INT(57) DEF_INT(58) DEF_INT(59) DEF_INT(60) DEF_INT(61) DEF_INT(62) DEF_INT(63) DEF_INT(64) DEF_INT(65) DEF_INT(66) DEF_INT(67) DEF_INT(68) DEF_INT(69) DEF_INT(70) DEF_INT(71) DEF_INT(72) DEF_INT(73) DEF_INT(74) DEF_INT(75) DEF_INT(76) DEF_INT(77) DEF_INT(78) DEF_INT(79) DEF_INT(80) DEF_INT(81) DEF_INT(82) DEF_INT(83) DEF_INT(84) DEF_INT(85) DEF_INT(86) DEF_INT(87) DEF_INT(88) DEF_INT(89) DEF_INT(90) DEF_INT(91) DEF_INT(92) DEF_INT(93) DEF_INT(94) DEF_INT(95) DEF_INT(96) DEF_INT(97) DEF_INT(98) DEF_INT(99) DEF_INT(100) DEF_INT(101) DEF_INT(102) DEF_INT(103) DEF_INT(104) DEF_INT(105) DEF_INT(106) DEF_INT(107) DEF_INT(108) DEF_INT(109) DEF_INT(110) DEF_INT(111) DEF_INT(112) DEF_INT(113) DEF_INT(114) DEF_INT(115) DEF_INT(116) DEF_INT(117) DEF_INT(118) DEF_INT(119) DEF_INT(120) DEF_INT(121) DEF_INT(122) DEF_INT(123) DEF_INT(124) DEF_INT(125) DEF_INT(126) DEF_INT(127) DEF_INT(128) DEF_INT(129) DEF_INT(130) DEF_INT(131) DEF_INT(132) DEF_INT(133) DEF_INT(134) DEF_INT(135) DEF_INT(136) DEF_INT(137) DEF_INT(138) DEF_INT(139) DEF_INT(140) DEF_INT(141) DEF_INT(142) DEF_INT(143) DEF_INT(144) DEF_INT(145) DEF_INT(146) DEF_INT(147) DEF_INT(148) DEF_INT(149) DEF_INT(150) DEF_INT(151) DEF_INT(152) DEF_INT(153) DEF_INT(154) DEF_INT(155) DEF_INT(156) DEF_INT(157) DEF_INT(158) DEF_INT(159) DEF_INT(160) DEF_INT(161) DEF_INT(162) DEF_INT(163) DEF_INT(164) DEF_INT(165) DEF_INT(166) DEF_INT(167) DEF_INT(168) DEF_INT(169) DEF_INT(170) DEF_INT(171) DEF_INT(172) DEF_INT(173) DEF_INT(174) DEF_INT(175) DEF_INT(176) DEF_INT(177) DEF_INT(178) DEF_INT(179) DEF_INT(180) DEF_INT(181) DEF_INT(182) DEF_INT(183) DEF_INT(184) DEF_INT(185) DEF_INT(186) DEF_INT(187) DEF_INT(188) DEF_INT(189) DEF_INT(190) DEF_INT(191) DEF_INT(192) DEF_INT(193) DEF_INT(194) DEF_INT(195) DEF_INT(196) DEF_INT(197) DEF_INT(198) DEF_INT(199) DEF_INT(200) DEF_INT(201) DEF_INT(202) DEF_INT(203) DEF_INT(204) DEF_INT(205) DEF_INT(206) DEF_INT(207) DEF_INT(208) DEF_INT(209) DEF_INT(210) DEF_INT(211) DEF_INT(212) DEF_INT(213) DEF_INT(214) DEF_INT(215) DEF_INT(216) DEF_INT(217) DEF_INT(218) DEF_INT(219) DEF_INT(220) DEF_INT(221) DEF_INT(222) DEF_INT(223) DEF_INT(224) DEF_INT(225) DEF_INT(226) DEF_INT(227) DEF_INT(228) DEF_INT(229) DEF_INT(230) DEF_INT(231) DEF_INT(232) DEF_INT(233) DEF_INT(234) DEF_INT(235) DEF_INT(236) DEF_INT(237) DEF_INT(238) DEF_INT(239) DEF_INT(240) DEF_INT(241) DEF_INT(242) DEF_INT(243) DEF_INT(244) DEF_INT(245) DEF_INT(246) DEF_INT(247) DEF_INT(248) DEF_INT(249) DEF_INT(250) DEF_INT(251) DEF_INT(252) DEF_INT(253) DEF_INT(254) DEF_INT(255)

.text
.align 4
interrupt_callers:
    .word USE_INT(0),USE_INT(1),USE_INT(2),USE_INT(3),USE_INT(4),USE_INT(5),USE_INT(6),USE_INT(7),USE_INT(8),USE_INT(9),USE_INT(10),USE_INT(11),USE_INT(12),USE_INT(13),USE_INT(14),USE_INT(15),USE_INT(16),USE_INT(17),USE_INT(18),USE_INT(19),USE_INT(20),USE_INT(21),USE_INT(22),USE_INT(23),USE_INT(24),USE_INT(25),USE_INT(26),USE_INT(27),USE_INT(28),USE_INT(29),USE_INT(30),USE_INT(31),USE_INT(32),USE_INT(33),USE_INT(34),USE_INT(35),USE_INT(36),USE_INT(37),USE_INT(38),USE_INT(39),USE_INT(40),USE_INT(41),USE_INT(42),USE_INT(43),USE_INT(44),USE_INT(45),USE_INT(46),USE_INT(47),USE_INT(48),USE_INT(49),USE_INT(50),USE_INT(51),USE_INT(52),USE_INT(53),USE_INT(54),USE_INT(55),USE_INT(56),USE_INT(57),USE_INT(58),USE_INT(59),USE_INT(60),USE_INT(61),USE_INT(62),USE_INT(63),USE_INT(64),USE_INT(65),USE_INT(66),USE_INT(67),USE_INT(68),USE_INT(69),USE_INT(70),USE_INT(71),USE_INT(72),USE_INT(73),USE_INT(74),USE_INT(75),USE_INT(76),USE_INT(77),USE_INT(78),USE_INT(79),USE_INT(80),USE_INT(81),USE_INT(82),USE_INT(83),USE_INT(84),USE_INT(85),USE_INT(86),USE_INT(87),USE_INT(88),USE_INT(89),USE_INT(90),USE_INT(91),USE_INT(92),USE_INT(93),USE_INT(94),USE_INT(95),USE_INT(96),USE_INT(97),USE_INT(98),USE_INT(99),USE_INT(100),USE_INT(101),USE_INT(102),USE_INT(103),USE_INT(104),USE_INT(105),USE_INT(106),USE_INT(107),USE_INT(108),USE_INT(109),USE_INT(110),USE_INT(111),USE_INT(112),USE_INT(113),USE_INT(114),USE_INT(115),USE_INT(116),USE_INT(117),USE_INT(118),USE_INT(119),USE_INT(120),USE_INT(121),USE_INT(122),USE_INT(123),USE_INT(124),USE_INT(125),USE_INT(126),USE_INT(127),USE_INT(128),USE_INT(129),USE_INT(130),USE_INT(131),USE_INT(132),USE_INT(133),USE_INT(134),USE_INT(135),USE_INT(136),USE_INT(137),USE_INT(138),USE_INT(139),USE_INT(140),USE_INT(141),USE_INT(142),USE_INT(143),USE_INT(144),USE_INT(145),USE_INT(146),USE_INT(147),USE_INT(148),USE_INT(149),USE_INT(150),USE_INT(151),USE_INT(152),USE_INT(153),USE_INT(154),USE_INT(155),USE_INT(156),USE_INT(157),USE_INT(158),USE_INT(159),USE_INT(160),USE_INT(161),USE_INT(162),USE_INT(163),USE_INT(164),USE_INT(165),USE_INT(166),USE_INT(167),USE_INT(168),USE_INT(169),USE_INT(170),USE_INT(171),USE_INT(172),USE_INT(173),USE_INT(174),USE_INT(175),USE_INT(176),USE_INT(177),USE_INT(178),USE_INT(179),USE_INT(180),USE_INT(181),USE_INT(182),USE_INT(183),USE_INT(184),USE_INT(185),USE_INT(186),USE_INT(187),USE_INT(188),USE_INT(189),USE_INT(190),USE_INT(191),USE_INT(192),USE_INT(193),USE_INT(194),USE_INT(195),USE_INT(196),USE_INT(197),USE_INT(198),USE_INT(199),USE_INT(200),USE_INT(201),USE_INT(202),USE_INT(203),USE_INT(204),USE_INT(205),USE_INT(206),USE_INT(207),USE_INT(208),USE_INT(209),USE_INT(210),USE_INT(211),USE_INT(212),USE_INT(213),USE_INT(214),USE_INT(215),USE_INT(216),USE_INT(217),USE_INT(218),USE_INT(219),USE_INT(220),USE_INT(221),USE_INT(222),USE_INT(223),USE_INT(224),USE_INT(225),USE_INT(226),USE_INT(227),USE_INT(228),USE_INT(229),USE_INT(230),USE_INT(231),USE_INT(232),USE_INT(233),USE_INT(234),USE_INT(235),USE_INT(236),USE_INT(237),USE_INT(238),USE_INT(239),USE_INT(240),USE_INT(241),USE_INT(242),USE_INT(243),USE_INT(244),USE_INT(245),USE_INT(246),USE_INT(247),USE_INT(248),USE_INT(249),USE_INT(250),USE_INT(251),USE_INT(252),USE_INT(253),USE_INT(254),USE_INT(255)
