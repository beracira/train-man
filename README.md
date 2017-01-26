<snippet>

## Kernel 1

Full pathname to executable: /u/cs452/tftp/ARM/z283chen/kernel.elf 

Redboot must be restarted before loading the program.

To load and run the program in the Redboot terminal, run the following command:
> load -b 0x00218000 -h 10.15.167.5 "ARM/z283chen/kernel.elf" ; go

The executable can also be created by going into the directory “kernel” and calling make:
> cd kernel

> make

And then copy the elf to wherever you want to run it.

</snippet>