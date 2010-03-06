QTBASE_DIR := $(shell Rscript -e 'cat(system.file(package="qtbase"))')

all:
	mkdir -p ../kdebindings-build;
	mkdir -p ../src-build;
	cd ../kdebindings-build; \
	cmake ../inst/kdebindings \
	      -DCMAKE_INSTALL_PREFIX=../inst/local -DQTBASE_DIR=$(QTBASE_DIR) \
	      -DR_INCLUDE_DIR=$(R_INCLUDE_DIR); \
	LD_LIBRARY_PATH=${QTBASE_DIR}/local/lib make install
	cd ../src-build; \
	cmake ../src \
	      -DCMAKE_INSTALL_PREFIX=../src \
	      -DR_LIBRARIES="${LIBR}" \
	      -DR_INCLUDES=${R_INCLUDE_DIR} \
	      -DQTBASE_DIR=${QTBASE_DIR}; \
	make install

clean:
	rm -rf ../kdebindings-build
	rm -rf ../src-build
	rm -rf ../inst/local
