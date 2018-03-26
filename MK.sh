# /bin/sh
echo "start compile...."
cd $HOME/core
make Clean
make
if [ $? == 0 ]
then
	cp taskPool ../bin
	echo "compile succ...."
else
	echo "compile fail...."
	exit 1
fi
