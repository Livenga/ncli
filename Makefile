CC     = gcc
LINK   = -lssl -lcrypto
FLAG   = -g -Wall -O2
PRJC   = ncli
DEFINE = -D __DEBUG

OBJDIR = objs
BINDIR = bin

SRC     = $(shell find src/ -name \*.c)
OBJS    = $(addprefix $(OBJDIR)/, $(patsubst %.c, %.o, $(SRC)))
OBJSDIR = $(dir $(OBJS))


default:
	[ -d  $(OBJDIR)   ] || mkdir -pv $(OBJDIR)
	[ -d "$(OBJSDIR)" ] || mkdir -pv $(OBJSDIR)
	make $(PRJC)

$(PRJC):$(OBJS)
	$(CC) -o $(PRJC) $(OBJS) $(LINK) $(DEFINE) $(FLAG)

$(OBJDIR)/%.o:%.c
	$(CC) -c -o $@ $< $(DEFINE) $(FLAG)

clean:
	[ ! -d $(OBJDIR) ] || rm -r $(OBJDIR)
	[ ! -d $(PRJC)   ] || rm $(PRJC)

all:
	make clean
	make
