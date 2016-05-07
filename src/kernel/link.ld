/* Link.ld -- Linker script for the kernel - ensure everything goes in the */
/*            Correct place.  */
/*            Original file taken from Bran's Kernel Development */
/*            tutorials: http://www.osdever.net/bkerndev/index.php. */

ENTRY(start)
SECTIONS
{
  .text 0x101000 :
  {
    code = .; _code = .; __code = .;
    *(.text)
    *(.gnu.linkonce.t.*)
    . = ALIGN(4096);
  }
  
  .data :
  {
    data = .; _data = .; __data = .;
    *(.data)
    *(.rodata)
    *(.gnu.linkonce.d.*)
    *(.gnu.linkonce.r.*)
    . = ALIGN(4096);
  }

  _bss_start = .;
  .bss :
  {
    bss = .; _bss = .; __bss = .;
    *(.bss)
    *(.gnu.linkonce.b.*)
    . = ALIGN(4096);
  }
  _bss_end = .;
  
   end = .; _end = .; __end = .;
}
