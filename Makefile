#================================[[ beg-code ]]================================#



#===(manditory)============================================#
# basename of executable, header, and all code files
NAME_BASE  = yGOD
# additional standard and outside libraries
LIB_STD    = -lX11 -lGL -lm
# all heatherly libraries used, debug versions whenever available
LIB_MINE   = -lyFONT_debug -lyCOLOR_debug
# directory for production code, no trailing slash
INST_DIR   = 



#===(optional)=============================================#
# extra include directories required
INC_MINE   = 
# utilities generated, separate from main program
NAME_UTIL  = 
# libraries only for the utilities
LIB_UTIL   = 



#===(post-processing)======================================#
# create a rule for...
#install_prep       :
#remove_prep        :



#===(master template)======================================#
include /usr/local/sbin/make_program



#===(post-processing)======================================#
# create a rule for...
#install_post       :
#remove_post        :



#================================[[ end-code ]]================================#


##*============================---(source-start)---============================*#
#
#
#
##===[[ VARABLES ]]======================================================================================================================================================#
#
##===(current variables)=================================================================================================================================================#
#BASE    = yGOD
#DEBUG   = ${BASE}_debug
#UNIT    = ${BASE}_unit
#HDIR    = /home/system/yGOD.opengl_godview
#IDIR    = /usr/local/lib
#MDIR    = /usr/share/man/man3
#
##===(compilier variables)===============================================================================================================================================#
## must have "-x c" on gcc line so stripped files work with alternate extensions
#COMP    = gcc -c -std=gnu89 -x c -g -pg -Wall -Wextra
#INCS    = -I/usr/local/include 
#
##===(linker options)========================================================================================================================================================================#
##------   (0)-------------- (1)-------------- (2)-------------- (3)-------------- (4)-------------- (5)-------------- (6)-------------- (7)-------------- (8)-------------- (9)--------------
#LINK    = gcc
#LIBDIR  = -L/usr/local/lib
#LIBS    = ${LIBDIR}         -lX11             -lGL              -lm
#LIBD    = ${LIBDIR}         -lX11             -lGL              -lm               -lyLOG
#LIBU    = ${LIBDIR}         -lX11             -lGL              -lm               -lyLOG            -lyUNIT           -lyVAR
#
##===(file lists)============================================================================================================================================================================#
##------   (0)-------------- (1)-------------- (2)-------------- (3)-------------- (4)-------------- (5)-------------- (6)-------------- (7)-------------- (8)-------------- (9)--------------
#HEADS   = ${BASE}.h
#OBJS    = ${BASE}.os
#OBJD    = ${BASE}.o
#OBJU    = ${BASE}_unit.o    ${BASE}.o
#
##===(make variables)====================================================================================================================================================#
#COPY    = cp -f
#CLEAN   = rm -f
#PRINT   = @printf
#STRIP   = @grep -v -e " DEBUG_" -e " yLOG_" 
#
#
#
#
#
#
##*---(standard variables)-------------*#
#COMP    = gcc -c -std=c99 -g -pg -Wall -Wextra 
#INCS    = -I/usr/local/include
#LINK    = gcc 
#LIBS    = -L/usr/local/lib -L/lib64 -L/usr/lib64 -L/usr/lib64/opengl/nvidia/lib  -lX11 -lGL -lm
#COPY    = cp -f
#CLEAN   = rm -f
#ECHO    = @echo
#PRINT   = @printf
#
#
#
##*---(make libraries)-----------------*#
#all                : yGOD.so.1.0 yGOD.a
#
#
#
##*---(make components)----------------*#
#yGOD.so.1.0        : yGOD.o
#	${LINK}  -shared -Wl,-soname,libyGOD.so.1  ${LIBS}  -o libyGOD.so.1.0  yGOD.o
#
#yGOD.a             : yGOD.o
#	ar       rcs  libyGOD.a  yGOD.o
#
#yGOD.o             : yGOD.h yGOD.c
#	${COMP}  yGOD.c -fPIC ${INCS}
#
#
#
##*---(housecleaning)------------------*#
#bigclean           :
#	${PRINT}  "\n--------------------------------------\n"
#	${PRINT}  "clean swap files\n"
#	${CLEAN} .*.swp
#
#clean              :
#	${PRINT}  "\n--------------------------------------\n"
#	${PRINT}  "cleaning out object, backup, and temp files\n"
#	${CLEAN} *.o
#	${CLEAN} *~
#	${CLEAN} temp*
#
#remove             :
#	${PRINT}  "\n--------------------------------------\n"
#	${PRINT}  "remove previous version from production\n"
#	${CLEAN} /usr/local/lib/lib${BASE}*
#	${CLEAN} /usr/local/include/${BASE}*
#
#install            :
#	${PRINT}  "\n--------------------------------------\n"
#	${PRINT}  "install library\n"
#	_lib     -s yGOD
#	_lib     -a yGOD
#	ldconfig
#	rm -f     /usr/share/man/man3/${BASE}.3.bz2
#	cp -f     ${BASE}.3    /usr/share/man/man3/
#	bzip2     /usr/share/man/man3/${BASE}.3
#	chmod     0644  /usr/share/man/man3/${BASE}.3.bz2
#
#
#
#
##*============================----(source-end)----============================*#
