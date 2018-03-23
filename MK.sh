# /bin/sh
echo "start compile...."
cd $HOME/core
make Clean
make
mv taskPool ../bin
echo "end compile...."
