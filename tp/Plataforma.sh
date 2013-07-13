cd
cd /home/tp/git/tp-20131c-tp-so-1c2013/commons/Debug
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
cd /home/tp/git/tp-20131c-tp-so-1c2013/Server/Debug
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/tp/git/tp-20131c-tp-so-1c2013/uncommons/Debug
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/tp/git/tp-20131c-tp-so-1c2013/so-commons-library/Debug
ldd Server
./Server $1
