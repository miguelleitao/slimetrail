CFLAGS=-O3 -Wall  `mysql_config --cflags` 
LDFLAGS=`mysql_config --libs`
TARGET=slimetrail
OBJ=${TARGET}.o
SRC=${TARGET}.c

all: ${TARGET}

commit: all
	sftp magnocom@ftp.magnocomp.com <<END_SCRIPT \
	cd www/miguelleitao.com/slimetrail \
	put index.php \
	quit \
	END_SCRIPT

push: ${TARGET}.c ${TARGET}.php ${TARGET}.css index.html Makefile README.md
	git add $^
	git commit -m "update"
	git push

${TARGET}: ${OBJ}

