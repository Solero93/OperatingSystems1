## Bash script to run the minikernel 

set -e # Exits on error

make

boot/boot minikernel/kernel > out.out

##Removing generated binary and .o files

rm -f minikernel/kernel.o
rm -f minikernel/kernel

rm -f usuario/*.o
rm -f usuario/dormilon
rm -f usuario/excep_arit
rm -f usuario/excep_mem
rm -f usuario/get_pid
rm -f usuario/init
rm -f usuario/simplon
rm -f usuario/yosoy
rm -f usuario/get_ppid

rm -f usuario/lib/serv.o

##Opening the output file

# You can change kate for your favourite editor, let it be sublime-text, vim or whatever
kate out.out || nano out.out ## If kate is not installed, use nano instead (should be on Linux by default)
