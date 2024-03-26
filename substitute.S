
.section .text

substitutionA:
bl	packet_init
or	4,27,27
b	init2__10MarioActorFRCQ29JGeometry8TVec3fRCQ29JGeometry8TVec3fl + 0x5C

substitutionB:
stwu 1,-0x20 (1)
mflr 0
stw 0, 0x1C (1)
stw 31, 0x18 (1)
mr 31,3
li 3,0
bl getWPad__2MRFl
lwz 3, 8 (3)
bl testButton1__10WPadButtonCFv
cmpi 0,0,3,0
bne proceed
bl packet_mainloop
proceed:
mr 3,31
bl movement__10MarioActorFv
lwz 31,0x18 (1)
lwz 0,0x1C (1)
mtlr 0
addi 1,1,0x20
blr
