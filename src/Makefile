QTBASE_DIR := $(shell Rscript -e 'cat(system.file(package="qtbase"))')

all:
	cmake ../inst/kdebindings/CMakeLists.txt \
	      -DCMAKE_INSTALL_PREFIX=../local -DQTBASE_DIR=$(QTBASE_DIR) \
	      -DR_INCLUDE_DIR=$(R_INCLUDE_DIR)
	LD_LIBRARY_PATH=${QTBASE_DIR}/kdebindings/generator/bin \
		make -C ../inst/kdebindings install
	QTBASE_DIR=${QTBASE_DIR} bash createProject.sh
	make -f Makefile.qtpaint
	if test -e libqtpaint.so; then cp libqtpaint.so qtpaint.so; fi
## for macx:
	if test -e libqtpaint.dylib; then cp libqtpaint.dylib qtpaint.so; fi


clean:
	rm -f *.so *.o *.a libqt* qtpaint.pro Makefile.qtpaint moc*.cpp
	make -C ../inst/kdebindings clean
	rm -f smokedata.cpp x_1.cpp


