
all:
	./createProject.sh
	make -f Makefile.qtpaint
	if test -e libqtpaint.so; then cp libqtpaint.so qtpaint.so; fi
	## for macx:
	if test -e libqtpaint.dylib; then cp libqtpaint.dylib qtpaint.so; fi


clean:
	rm -f *.so *.o *.a libqt* qtpaint.pro Makefile.qtpaint moc*.cpp


