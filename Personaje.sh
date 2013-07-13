cd
cd /home/utnso/git/tp-20131c-tp-so-1c2013/so-commons-library/Debug
make clean
make
cd
cd /home/utnso/git/tp-20131c-tp-so-1c2013/uncommons/Debug
make clean
make
cd
cd /home/utnso/git/tp-20131c-tp-so-1c2013/personaje/Debug
make clean
make
cd
cd /home/utnso/git/tp-20131c-tp-so-1c2013/personaje/Debug
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/git/tp-20131c-tp-so-1c2013/uncommons/Debug
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/git/tp-20131c-tp-so-1c2013/so-commons-library/Debug
ldd personaje
./personaje $1
