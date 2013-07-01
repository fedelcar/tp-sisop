cd
cd git/tp-20131c-tp-so-1c2013/commons/Debug
make clean
make
cd
cd git/tp-20131c-tp-so-1c2013/uncommons/Debug
make clean
make
cd
cd git/tp-20131c-tp-so-1c2013/personaje/Debug
make clean
make
cd
cd git/tp-20131c-tp-so-1c2013/personaje/Debug
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/git/tp-20131c-tp-so-1c2013/uncommons/Debug
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/git/tp-20131c-tp-so-1c2013/commons/Debug
ldd personaje
./personaje
