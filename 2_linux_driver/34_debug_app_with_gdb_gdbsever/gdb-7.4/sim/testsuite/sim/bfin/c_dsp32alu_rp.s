//Original:/testcases/core/c_dsp32alu_rp/c_dsp32alu_rp.dsp
// Spec Reference: dsp32alu
# mach: bfin

.include "testutils.inc"
	start




imm32 r0, 0xa5678911;
imm32 r1, 0x2a89ab1d;
imm32 r2, 0x34a45515;
imm32 r3, 0x466a7717;
imm32 r4, 0x5567891b;
imm32 r5, 0x6789ab1d;
imm32 r6, 0x74445a15;
imm32 r7, 0x866677a7;
R0 = R0 + R0 (NS);
R1 = R0 + R1 (NS);
R2 = R0 + R2 (NS);
R3 = R0 + R3 (NS);
R4 = R0 + R4 (NS);
R5 = R0 + R5 (NS);
R6 = R0 + R6 (NS);
R7 = R0 + R7 (NS);
CHECKREG r0, 0x4ACF1222;
CHECKREG r1, 0x7558BD3F;
CHECKREG r2, 0x7F736737;
CHECKREG r3, 0x91398939;
CHECKREG r4, 0xA0369B3D;
CHECKREG r5, 0xB258BD3F;
CHECKREG r6, 0xBF136C37;
CHECKREG r7, 0xD13589C9;

imm32 r0, 0xabc78911;
imm32 r1, 0x27c9ab1d;
imm32 r2, 0x344c5515;
imm32 r3, 0x4666c717;
imm32 r4, 0x5567c91b;
imm32 r5, 0x6789ab1d;
imm32 r6, 0x74445c15;
imm32 r7, 0x866677c7;
R0 = R1 + R0 (NS);
R1 = R1 + R1 (NS);
R2 = R1 + R2 (NS);
R3 = R1 + R3 (NS);
R4 = R1 + R4 (NS);
R5 = R1 + R5 (NS);
R6 = R1 + R6 (NS);
R7 = R1 + R7 (NS);
CHECKREG r0, 0xD391342E;
CHECKREG r1, 0x4F93563A;
CHECKREG r2, 0x83DFAB4F;
CHECKREG r3, 0x95FA1D51;
CHECKREG r4, 0xA4FB1F55;
CHECKREG r5, 0xB71D0157;
CHECKREG r6, 0xC3D7B24F;
CHECKREG r7, 0xD5F9CE01;

imm32 r0, 0xdd678911;
imm32 r1, 0x2789ab1d;
imm32 r2, 0x34445515;
imm32 r3, 0x46d67717;
imm32 r4, 0x5567891b;
imm32 r5, 0x678dab1d;
imm32 r6, 0x7444d515;
imm32 r7, 0x86667d77;
R0 = R2 + R0 (NS);
R1 = R2 + R1 (NS);
R2 = R2 + R2 (NS);
R3 = R2 + R3 (NS);
R4 = R2 + R4 (NS);
R5 = R2 + R5 (NS);
R6 = R2 + R6 (NS);
R7 = R2 + R7 (NS);
CHECKREG r0, 0x11ABDE26;
CHECKREG r1, 0x5BCE0032;
CHECKREG r2, 0x6888AA2A;
CHECKREG r3, 0xAF5F2141;
CHECKREG r4, 0xBDF03345;
CHECKREG r5, 0xD0165547;
CHECKREG r6, 0xDCCD7F3F;
CHECKREG r7, 0xEEEF27A1;

imm32 r0, 0x15678911;
imm32 r1, 0x2789ab1d;
imm32 r2, 0x34445515;
imm32 r3, 0x46667717;
imm32 r4, 0x5567891b;
imm32 r5, 0x6789ab1d;
imm32 r6, 0x74445515;
imm32 r7, 0x86667777;
R0 = R3 + R0 (NS);
R1 = R3 + R1 (NS);
R2 = R3 + R2 (NS);
R3 = R3 + R3 (NS);
R4 = R3 + R4 (NS);
R5 = R3 + R5 (NS);
R6 = R3 + R6 (NS);
R7 = R3 + R7 (NS);
CHECKREG r0, 0x5BCE0028;
CHECKREG r1, 0x6DF02234;
CHECKREG r2, 0x7AAACC2C;
CHECKREG r3, 0x8CCCEE2E;
CHECKREG r4, 0xE2347749;
CHECKREG r5, 0xF456994B;
CHECKREG r6, 0x01114343;
CHECKREG r7, 0x133365A5;

imm32 r0, 0xee678911;
imm32 r1, 0x2789ab1d;
imm32 r2, 0x34e45515;
imm32 r3, 0x46667717;
imm32 r4, 0x556e891b;
imm32 r5, 0x6789eb1d;
imm32 r6, 0x74445515;
imm32 r7, 0x86667e77;
R0 = R4 + R0 (NS);
R1 = R4 + R1 (NS);
R2 = R4 + R2 (NS);
R3 = R4 + R3 (NS);
R4 = R4 + R4 (NS);
R5 = R4 + R5 (NS);
R6 = R4 + R6 (NS);
R7 = R4 + R7 (NS);
CHECKREG r0, 0x43D6122C;
CHECKREG r1, 0x7CF83438;
CHECKREG r2, 0x8A52DE30;
CHECKREG r3, 0x9BD50032;
CHECKREG r4, 0xAADD1236;
CHECKREG r5, 0x1266FD53;
CHECKREG r6, 0x1F21674B;
CHECKREG r7, 0x314390AD;

imm32 r0, 0x15678911;
imm32 r1, 0x2789ab1d;
imm32 r2, 0x34445515;
imm32 r3, 0x46667717;
imm32 r4, 0x5567891b;
imm32 r5, 0x6789ab1d;
imm32 r6, 0x74445515;
imm32 r7, 0x86667777;
R0 = R5 + R0 (NS);
R1 = R5 + R1 (NS);
R2 = R5 + R2 (NS);
R3 = R5 + R3 (NS);
R4 = R5 + R4 (NS);
R5 = R5 + R5 (NS);
R6 = R5 + R6 (NS);
R7 = R5 + R7 (NS);
CHECKREG r0, 0x7CF1342E;
CHECKREG r1, 0x8F13563A;
CHECKREG r2, 0x9BCE0032;
CHECKREG r3, 0xADF02234;
CHECKREG r4, 0xBCF13438;
CHECKREG r5, 0xCF13563A;
CHECKREG r6, 0x4357AB4F;
CHECKREG r7, 0x5579CDB1;

imm32 r0, 0xff678911;
imm32 r1, 0x2789ab1d;
imm32 r2, 0x34f45515;
imm32 r3, 0x46667717;
imm32 r4, 0x556f891b;
imm32 r5, 0x6789ab1d;
imm32 r6, 0x7444f515;
imm32 r7, 0x86667f77;
R0 = R6 + R0 (NS);
R1 = R6 + R1 (NS);
R2 = R6 + R2 (NS);
R3 = R6 + R3 (NS);
R4 = R6 + R4 (NS);
R5 = R6 + R5 (NS);
R6 = R6 + R6 (NS);
R7 = R6 + R7 (NS);
CHECKREG r0, 0x73AC7E26;
CHECKREG r1, 0x9BCEA032;
CHECKREG r2, 0xA9394A2A;
CHECKREG r3, 0xBAAB6C2C;
CHECKREG r4, 0xC9B47E30;
CHECKREG r5, 0xDBCEA032;
CHECKREG r6, 0xE889EA2A;
CHECKREG r7, 0x6EF069A1;

imm32 r0, 0xed678911;
imm32 r1, 0x27d9ab1d;
imm32 r2, 0x344d5515;
imm32 r3, 0x46667717;
imm32 r4, 0x5567c91b;
imm32 r5, 0x6789ab1d;
imm32 r6, 0x74445c15;
imm32 r7, 0x866677c7;
R0 = R7 + R0 (NS);
R1 = R7 + R1 (NS);
R2 = R7 + R2 (NS);
R3 = R7 + R3 (NS);
R4 = R7 + R4 (NS);
R5 = R7 + R5 (NS);
R6 = R7 + R6 (NS);
R7 = R7 + R7 (NS);
CHECKREG r0, 0x73CE00D8;
CHECKREG r1, 0xAE4022E4;
CHECKREG r2, 0xBAB3CCDC;
CHECKREG r3, 0xCCCCEEDE;
CHECKREG r4, 0xDBCE40E2;
CHECKREG r5, 0xEDF022E4;
CHECKREG r6, 0xFAAAD3DC;
CHECKREG r7, 0x0CCCEF8E;

imm32 r0, 0x15678911;
imm32 r1, 0x2789ab1d;
imm32 r2, 0x34445515;
imm32 r3, 0x46667717;
imm32 r4, 0x5567891b;
imm32 r5, 0x6789ab1d;
imm32 r6, 0x74445515;
imm32 r7, 0x86667777;
R3 = R1 + R4 (S);
R7 = R4 + R6 (S);
R2 = R7 + R7 (S);
R4 = R5 + R0 (S);
R5 = R3 + R1 (S);
R6 = R2 + R3 (S);
R0 = R0 + R2 (S);
R1 = R6 + R5 (S);
CHECKREG r0, 0x7FFFFFFF;
CHECKREG r1, 0x7FFFFFFF;
CHECKREG r2, 0x7FFFFFFF;
CHECKREG r3, 0x7CF13438;
CHECKREG r4, 0x7CF1342E;
CHECKREG r5, 0x7FFFFFFF;
CHECKREG r6, 0x7FFFFFFF;
CHECKREG r7, 0x7FFFFFFF;

imm32 r0, 0x55678911;
imm32 r1, 0x6a89ab1d;
imm32 r2, 0x74d45515;
imm32 r3, 0x866f7717;
imm32 r4, 0x5567c91b;
imm32 r5, 0x6789ab1d;
imm32 r6, 0x74445515;
imm32 r7, 0x86667777;
R3 = R3 + R3 (S);
R1 = R7 + R6 (S);
R4 = R1 + R2 (S);
R7 = R4 + R0 (S);
R5 = R6 + R4 (S);
R2 = R5 + R5 (S);
R6 = R2 + R1 (S);
R0 = R0 + R7 (S);
CHECKREG r0, 0x7FFFFFFF;
CHECKREG r1, 0xFAAACC8C;
CHECKREG r2, 0x7FFFFFFF;
CHECKREG r3, 0x80000000;
CHECKREG r4, 0x6F7F21A1;
CHECKREG r5, 0x7FFFFFFF;
CHECKREG r6, 0x7AAACC8B;
CHECKREG r7, 0x7FFFFFFF;


pass
