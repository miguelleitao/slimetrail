CFLAGS=-O3 -Wall  `mysql_config --cflags` 
LDFLAGS=`mysql_config --libs`
TARGET=slimetrail
OBJ=${TARGET}.o
SRC=${TARGET}.c

all: ${TARGET}

push: ${TARGET}.c ${TARGET}.php ${TARGET}.css index.html Makefile upload.sh README.md
	git add $^
	git commit -m "update"
	git push

${TARGET}: ${OBJ}

