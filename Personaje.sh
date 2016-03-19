cd /home/federico/git/tp-20131c-tp-so-1c2013/personaje/Debug
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/git/tp-20131c-tp-so-1c2013/uncommons/Debug
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/git/tp-20131c-tp-so-1c2013/so-commons-library/Debug
ldd personaje
./personaje $1
