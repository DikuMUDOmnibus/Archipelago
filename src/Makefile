#compiler name
CC = gcc
#compiler flags you want to use (other than profiling, such as -Wall)
MYFLAGS = -g3 -fno-builtin-log -pedantic -DDEBUG 
#MYFLAGS = -O4 -fno-strength-reduce
#flags for profiling (see hacker.doc for more information)
PROFILE = 

LIBS = -lcrypt

#############################################################################

CFLAGS = $(MYFLAGS) $(PROFILE) $(OSFLAGS)

TAGFLAGS = -d -T -C 

SRCS = comm.c act.comm.c act.inform1.c act.inform2.c act.movement.c act.obj1.c\
	act.obj2.c act.offensive.c act.other.c act.social.c act.wizard1.c \
	act.wizard2.c handler.c db1.c db2.c guilds.c interpreter.c utility.c \
        weather.c constants.c limits.c fight.c modify.c  \
        shop.c  rmact.c mobact.c objact.c spec_procs.c spec_assign.c \
	signals.c objsave.c boards.c mail.c ban.c graph.c config.c \
	mobedit.c objedit.c roomedit.c shopedit.c zoneedit.c \
	magic.c spell_parser1.c spell_parser2.c spells.c 

INCLUDES = boards.h handler.h mail.h structs.h comm.h interpreter.h\
	screen.h utils.h db.h limits.h spells.h mobedit.h shop.h objedit.h\
	roomedit.h shopedit.h zoneedit.h

OBJFILES = comm.o act.comm.o act.inform1.o act.inform2.o act.movement.o act.obj1.o\
	act.obj2.o act.offensive.o act.other.o act.social.o act.wizard1.o \
	act.wizard2.o handler.o db1.o db2.o guilds.o interpreter.o utility.o \
        weather.o constants.o limits.o fight.o modify.o  \
        shop.o  rmact.o mobact.o objact.o spec_procs.o spec_assign.o \
	signals.o objsave.o boards.o mail.o ban.o graph.o config.o \
	mobedit.o objedit.o roomedit.o shopedit.o zoneedit.o \
	magic.o spell_parser1.o spell_parser2.o spells.o 

arch   : $(OBJFILES)

../bin/archipelago : $(OBJFILES)


utils:
	$(MAKE) ../bin/autowiz
	$(MAKE) ../bin/delobjs
	$(MAKE) ../bin/hasmail
	$(MAKE) ../bin/listrent
	$(MAKE) ../bin/mailindex
	$(MAKE) ../bin/mudpasswd
	$(MAKE) ../bin/purgeplay
	$(MAKE) ../bin/readmail
	$(MAKE) ../bin/showplay
	$(MAKE) ../bin/sign
	$(MAKE) ../bin/split
	$(MAKE) ../bin/syntax_check

all:
	$(MAKE) ../bin/archipelago
	$(MAKE) utils
	$(MAKE) tags

tags: TAGS
	/usr/local/bin/etags $(TAGFLAGS)  $(INCLUDES) $(SRCS)
ctags:
	/usr/local/bin/ctags $(TAGFLAGS)  $(INCLUDES) $(SRCS)
TAGS:
	/usr/local/bin/etags $(TAGFLAGS)  $(INCLUDES) $(SRCS)
autowiz:
	$(MAKE) ../bin/autowiz
delobjs:
	$(MAKE) ../bin/delobjs
hasmail:
	$(MAKE) ../bin/hasmail
listrent:
	$(MAKE) ../bin/listrent
mailindex:
	$(MAKE) ../bin/mailindex
mudpasswd:
	$(MAKE) ../bin/mudpasswd
purgeplay:
	$(MAKE) ../bin/purgeplay
readmail:
	$(MAKE) ../bin/readmail
showplay:
	$(MAKE) ../bin/showplay
sign:
	$(MAKE) ../bin/sign
split:
	$(MAKE) ../bin/split
syntax_check:
	$(MAKE) ../bin/syntax_check

../bin/autowiz : util/autowiz.c structs.h db.h utils.h
	$(CC) $(CFLAGS) -o ../bin/autowiz util/autowiz.c
../bin/delobjs : util/delobjs.c structs.h
	$(CC) $(CFLAGS) -o ../bin/delobjs util/delobjs.c
../bin/hasmail : util/hasmail.c mail.h
	$(CC) $(CFLAGS) -o ../bin/hasmail util/hasmail.c
../bin/listrent : util/listrent.c structs.h
	$(CC) $(CFLAGS) -o ../bin/listrent util/listrent.c
../bin/mailindex : util/mailindex.c mail.h
	$(CC) $(CFLAGS) -o ../bin/mailindex util/mailindex.c
../bin/mudpasswd   : util/mudpasswd.c structs.h
	$(CC) $(CFLAGS) -o ../bin/mudpasswd util/mudpasswd.c
../bin/purgeplay : util/purgeplay.c structs.h
	$(CC) $(CFLAGS) -o ../bin/purgeplay util/purgeplay.c
../bin/readmail : util/readmail.c mail.h
	$(CC) $(CFLAGS) -o ../bin/readmail util/readmail.c
../bin/showplay : util/showplay.c structs.h
	$(CC) $(CFLAGS) -o ../bin/showplay util/showplay.c
../bin/sign: util/sign.c
	$(CC) $(CFLAGS) -o ../bin/sign $(COMMFLAGS) util/sign.c
../bin/split: util/split.c
	$(CC) $(CFLAGS) -o ../bin/split util/split.c
../bin/syntax_check: util/syntax_check.c structs.h utils.h db.h comm.h \
	handler.h limits.h
	$(CC) $(CFLAGS) -o ../bin/syntax_check util/syntax_check.c

clean:
	rm -f *.o ../bin/archipelago ../bin/test1 ../bin/debug

# Dependencies for the main mud

comm.o : comm.c structs.h utils.h comm.h interpreter.h handler.h db.h \
	limits.h
	$(CC) -c $(CFLAGS) $(COMMFLAGS) comm.c
act.comm.o : act.comm.c structs.h utils.h comm.h interpreter.h handler.h \
	db.h screen.h
	$(CC) -c $(CFLAGS) act.comm.c
act.inform1.o : act.inform1.c structs.h utils.h comm.h interpreter.h \
	handler.h db.h spells.h limits.h screen.h
	$(CC) -c $(CFLAGS) act.inform1.c
act.inform2.o : act.inform2.c structs.h utils.h comm.h interpreter.h \
	handler.h db.h spells.h limits.h screen.h
	$(CC) -c $(CFLAGS) act.inform2.c
act.movement.o : act.movement.c structs.h utils.h comm.h interpreter.h \
	handler.h db.h spells.h
	$(CC) -c $(CFLAGS) act.movement.c
act.obj1.o : act.obj1.c structs.h utils.h comm.h interpreter.h handler.h \
	db.h spells.h
	$(CC) -c $(CFLAGS) act.obj1.c
act.obj2.o : act.obj2.c structs.h utils.h comm.h interpreter.h handler.h \
	db.h spells.h limits.h
	$(CC) -c $(CFLAGS) act.obj2.c
act.offensive.o : act.offensive.c structs.h utils.h comm.h interpreter.h \
	handler.h db.h spells.h limits.h
	$(CC) -c $(CFLAGS) act.offensive.c
act.other.o : act.other.c structs.h utils.h comm.h interpreter.h handler.h \
	db.h spells.h screen.h limits.h
	$(CC) -c $(CFLAGS) act.other.c
act.social.o : act.social.c structs.h utils.h comm.h interpreter.h \
	handler.h db.h spells.h
	$(CC) -c $(CFLAGS) act.social.c
act.wizard1.o : act.wizard1.c structs.h utils.h comm.h interpreter.h \
	handler.h db.h spells.h limits.h screen.h shop.h
	$(CC) -c $(CFLAGS) act.wizard1.c
act.wizard2.o : act.wizard2.c structs.h utils.h comm.h interpreter.h \
	handler.h db.h spells.h limits.h screen.h shop.h
	$(CC) -c $(CFLAGS) act.wizard2.c
handler.o : handler.c structs.h utils.h comm.h db.h handler.h interpreter.h \
	spells.h
	$(CC) -c $(CFLAGS) handler.c 
db1.o : db1.c structs.h utils.h db.h comm.h handler.h limits.h spells.h \
	mail.h interpreter.h shop.h
	$(CC) -c $(CFLAGS) db1.c
db2.o : db2.c structs.h utils.h db.h comm.h handler.h limits.h spells.h \
	mail.h interpreter.h shop.h
	$(CC) -c $(CFLAGS) db2.c
ban.o : ban.c structs.h utils.h comm.h interpreter.h handler.h db.h
	$(CC) -c $(CFLAGS) ban.c
interpreter.o : interpreter.c structs.h comm.h interpreter.h db.h utils.h \
	limits.h spells.h handler.h mail.h screen.h
	$(CC) -c $(CFLAGS) interpreter.c
guilds.o : guilds.c structs.h comm.h interpreter.h db.h utils.h \
	limits.h spells.h handler.h mail.h screen.h
	$(CC) -c $(CFLAGS) guilds.c
utility.o : utility.c structs.h utils.h comm.h screen.h
	$(CC) -c $(CFLAGS) utility.c
spec_assign.o : spec_assign.c structs.h db.h interpreter.h utils.h
	$(CC) -c $(CFLAGS) spec_assign.c
spec_procs.o : spec_procs.c structs.h utils.h comm.h interpreter.h \
	handler.h db.h spells.h limits.h
	$(CC) -c $(CFLAGS) spec_procs.c
limits.o : limits.c structs.h limits.h utils.h spells.h comm.h db.h handler.h
	$(CC) -c $(CFLAGS) limits.c
fight.o : fight.c structs.h utils.h comm.h handler.h interpreter.h db.h \
	spells.h limits.h screen.h
	$(CC) -c $(CFLAGS) fight.c
weather.o : weather.c structs.h utils.h comm.h handler.h interpreter.h db.h
	$(CC) -c $(CFLAGS) weather.c
shop.o : shop.c structs.h comm.h handler.h db.h interpreter.h utils.h
	$(CC) -c $(CFLAGS) shop.c
spells.o : spells.c structs.h utils.h comm.h db.h interpreter.h spells.h \
	handler.h
	$(CC) -c $(CFLAGS) spells.c 
magic.o : magic.c structs.h utils.h comm.h spells.h handler.h limits.h \
	interpreter.h db.h
	$(CC) -c $(CFLAGS) magic.c 
spell_parser1.o : spell_parser1.c structs.h utils.h comm.h db.h interpreter.h \
	spells.h handler.h
	$(CC) -c $(CFLAGS) spell_parser1.c
spell_parser2.o : spell_parser2.c structs.h utils.h comm.h db.h interpreter.h \
	spells.h handler.h
	$(CC) -c $(CFLAGS) spell_parser2.c 
mobact.o : mobact.c utils.h structs.h db.h comm.h interpreter.h handler.h
	$(CC) -c $(CFLAGS) mobact.c
rmact.o : rmact.c utils.h structs.h db.h comm.h interpreter.h handler.h
	$(CC) -c $(CFLAGS) rmact.c
objact.o : objact.c utils.h structs.h db.h comm.h interpreter.h handler.h
	$(CC) -c $(CFLAGS) objact.c
modify.o : modify.c structs.h utils.h interpreter.h handler.h db.h comm.h \
	mail.h
	$(CC) -c $(CFLAGS) modify.c
constants.o : constants.c structs.h limits.h
	$(CC) -c $(CFLAGS) constants.c
objsave.o : objsave.c structs.h comm.h handler.h db.h interpreter.h \
	utils.h spells.h
	$(CC) -c $(CFLAGS) objsave.c
boards.o : boards.c structs.h utils.h comm.h db.h boards.h interpreter.h \
	handler.h
	$(CC) -c $(CFLAGS) boards.c
mail.o : mail.c mail.h structs.h utils.h comm.h db.h interpreter.h handler.h
	$(CC) -c $(CFLAGS) mail.c
signals.o : signals.c utils.h structs.h
	$(CC) -c $(CFLAGS) signals.c
graph.o : graph.c structs.h utils.h comm.h interpreter.h handler.h db.h \
	spells.h
	$(CC) -c $(CFLAGS) graph.c
config.o : config.c structs.h
	$(CC) -c $(CFLAGS) config.c
mobedit.o : mobedit.c mobedit.h structs.h utils.h comm.h interpreter.h \
	handler.h db.h limits.h screen.h
	$(CC) -c $(CFLAGS) mobedit.c
objedit.o : objedit.c objedit.h structs.h utils.h comm.h interpreter.h \
	handler.h db.h limits.h screen.h
	$(CC) -c $(CFLAGS) objedit.c
roomedit.o : roomedit.c roomedit.h structs.h utils.h comm.h  \
	interpreter.h handler.h db.h limits.h screen.h
	$(CC) -c $(CFLAGS) roomedit.c
shopedit.o : shopedit.c shopedit.h structs.h utils.h comm.h  \
	interpreter.h handler.h db.h limits.h shop.h screen.h
	$(CC) -c $(CFLAGS) shopedit.c
zoneedit.o : zoneedit.c zoneedit.h structs.h utils.h comm.h  \
	interpreter.h handler.h db.h limits.h screen.h 
	$(CC) -c $(CFLAGS) shopedit.c
arch  :
	$(CC) -o ../bin/test1 $(PROFILE) $(OBJFILES) $(LIBS)
../bin/archipelago  :
	$(CC) -o ../bin/archipelago $(PROFILE) $(OBJFILES) $(LIBS)
