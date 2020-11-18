#! /bin/sh
export QT_DIR=/usr/local/Qt5.12.1
export PATH=$QT_DIR/bin:$PATH

if [[ $1 = "clean" ]]
then
	echo "make clean..."
	make clean
elif [[ $1 = "qmake" ]]
then
	echo "qmake..."
	${QT_DIR}/bin/qmake
elif [[ $1 = "project" ]]
then
	echo "qmake project..."
	${QT_DIR}/bin/qmake -project
else
	echo "make..."
	make
fi


