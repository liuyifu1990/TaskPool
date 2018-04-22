# /bin/sh
echo "start compile...."
cd $HOME/core
make -f makefile_test Clean
make -f makefile_moni Clean
make -f makefile_test
make -f makefile_moni
if [ $? == 0 ]
then
	cp taskPool ../bin
	cp taskMoni ../bin
	echo "compile succ...."
else
	echo "compile fail...."
	exit 1
fi
