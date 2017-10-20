# ---------------------------------------------------------------------------
# Имя проекта

NAME	= lab1

# Настройки компилятора и линкера

CC      = clang
CFLAGS  = -std=c99 -Wall -pedantic
#LFLAGS  = --code-loc 0x2100 --xram-loc 0x6000 --stack-auto --stack-loc 0x80 

# Настройки системы автоинкремента версии сборки

PROJECT  = $(cmd type PROJECT)
VERSION  = $(cmd type VERSION)
BUILD    = $(cmd type BUILD)
TYPE     = $(cmd type TYPE)

PROJNAME = ${PROJECT}-${VERSION}-${BUILD}-${TYPE}
TARBALL  = ${PROJNAME}.tar

# Каталоги с исходными текстами

SRC_DIR = SRC

DEL = rm -f --
# ---------------------------------------------------------------------------

clang -std=c99 -Wall -pedantic *.c.

all: lab1

clean:
	$(DEL) $(NAME).hex
	$(DEL) $(NAME).bin
	$(DEL) $(NAME).map
	
	$(DEL) $(NAME).lnk
	$(DEL) $(NAME).lk
	
	
	$(DEL) $(TARBALL).gz
	$(DEL) $(SRC_DIR)/*.asm
	$(DEL) $(SRC_DIR)/*.rel
	$(DEL) $(SRC_DIR)/*.rst
	$(DEL) $(SRC_DIR)/*.sym
	$(DEL) $(SRC_DIR)/*.lst 

dist:
	tar -cvf $(TARBALL) --exclude=*.tar .
	gzip $(TARBALL)

LIST_SRC = \
$(SRC_DIR)/lab1.c \
$(SRC_DIR)/ipc.c \
$(SRC_DIR)/io.c



lab1 : $(LIST_OBJ) makefile
	$(CC) $(LFLAGS) *.c