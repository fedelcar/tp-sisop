cd
cd /home/tp/git/tp-20131c-tp-so-1c2013/Libreria-nivel/Debug
make clean
make
cd
cd /home/tp/git/tp-20131c-tp-so-1c2013/so-commons-library/Debug
make clean
make
cd
cd /home/tp/git/tp-20131c-tp-so-1c2013/uncommons/Debug
make clean
make
cd
cd /home/tp/git/tp-20131c-tp-so-1c2013/Server/Debug
make clean
make
cd
cd /home/tp/git/tp-20131c-tp-so-1c2013/Nivel/Debug
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/tp/git/tp-20131c-tp-so-1c2013/uncommons/Debug
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/tp/git/tp-20131c-tp-so-1c2013/so-commons-library/Debug
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/tp/git/tp-20131c-tp-so-1c2013/Libreria-nivel/Debug
ldd Nivel
./Nivel $1
