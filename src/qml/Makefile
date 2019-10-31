all:tesra-qml

ifeq ($(shell uname),Linux)
MTime:=$(shell stat -c %Y qml.pro)
endif
ifeq ($(shell uname),Darwin)
MTime:=$(shell stat -f %m qml.pro)
endif

LastTime:=$(shell cat qml.pro.time||echo "")

tesra-qml:
	@echo "Building tesra-qml ...depends_prefix: " ${depends_prefix} && \
	${depends_prefix}/native/bin/protoc --cpp_out=./ ./qt_native/paymentrequest.proto && \
	mkdir -p build  && \
	${depends_prefix}/qml/bin/lrelease qml.pro && \
	cd build && \
	if [ "$(LastTime)" = "" ]; then \
		${depends_prefix}/qml/bin/qmake ../qml.pro  "depends_prefix=$${depends_prefix}"; echo $(MTime) > ../qml.pro.time;  \
	else \
		if [ "$(MTime)" -gt "$(LastTime)" ]; then ${depends_prefix}/qml/bin/qmake ../qml.pro  "depends_prefix=$${depends_prefix}" ; echo $(MTime) > ../qml.pro.time;  fi; \
	fi && \
	$(MAKE) && \
	cd ../
clean:
	rm -rf build
	rm -rf qt_native/paymentrequest.pb.cc
	rm -rf qt_native/paymentrequest.pb.h

install:
	@echo "Please Manual Copy Tesra-QML To Output Dir"
	
install-strip:
	@echo "Please Manual Copy Tesra-QML To Output Dir"

.PHPNY: all clean install install-strip  uninstall
