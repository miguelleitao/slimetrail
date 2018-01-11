CFLAGS=-O3 -Wall  `mysql_config --cflags` 
LDFLAGS=`mysql_config --libs`
TARGET=slimetrail
OBJ=${TARGET}.o
SRC=${TARGET}.c

all: ${TARGET}

${TARGET}: ${OBJ}

push: ${TARGET}.c ${TARGET}.php Makefile
	git add $^
	git commit -m "update"
	git push

