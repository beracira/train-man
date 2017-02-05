void cache_on() {
  asm("MRC p15, 0, r0, c1, c0, 0"); // read c1
  asm("AND r0, r0, #0xffffefff"); // clear bits to be cleared
  asm("ORR r0, r0, #0x00001000"); // set bits to be set
  asm("MCR p15, 0, r0, c1, c0, 0"); // write c1

  asm("MRC p15, 0, r0, c1, c0, 0"); // read c1
  asm("AND r0, r0, #0xfffffffb"); // clear bits to be cleared
  asm("ORR r0, r0, #0x00000004"); // set bits to be set
  asm("MCR p15, 0, r0, c1, c0, 0"); // write c1
}
