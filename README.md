<snippet>
## Sha1 of the commit for Kernel 4
95362e45126d99c7e820e51d22d2f4cd1d6f0433

## Loading the program

Full pathname to executable: /u/cs452/tftp/ARM/z283chen/kernel.elf 

Redboot must be restarted before loading the program.

To load and run the program in the Redboot terminal, run the following command:
> load -b 0x00218000 -h 10.15.167.5 "ARM/z283chen/kernel.elf" ; go

The executable can also be created by going into the directory “kernel” and calling make:
> cd kernel

> make

And then copy the elf to wherever you want to run it.

</snippet>