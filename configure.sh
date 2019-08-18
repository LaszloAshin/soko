#!/bin/sh
# Makefile generator script by Laszlo Ashin <laszlo@ashin.hu>

# default settings for make
CC="gcc"
CFLAGS="-Wall"
LDFLAGS="-s -O2 -lSDL"
BDIR="build"
#CCPREFIX="ccache"

# programs to build, separated by one space
#PROGS="soko armsoko"
PROGS="soko"

# settings for each program listed in PROGS (above)
# FILES is required for each PROG (filenames must be separated by one space)
# CC, CFLAGS, LDFLAGS and BDIR is optional
# if not defined, the default setting is used for that variable
FILES_soko="main gr map field list player menu inval"

FILES_armsoko=${FILES_soko}
BDIR_armsoko="release"
CC_armsoko="arm-linux-gnu-gcc"
CFLAGS_armsoko="-Wall -I/home/laci/arm/include"
LDFLAGS_armsoko="-s -O2 -Wl,-rpath-link,/home/laci/arm/lib -L/home/laci/arm/lib -lSDL"

# don't edit below this line unless you know what you're doing.

# function to simplify the output process (output to Makefile)
put() {
	echo "${1}" >> Makefile
}

# check the default compiler
${CC} &> /dev/null
if [ "x${?}" == "x127" ]; then
	echo "ERROR: Default compiler is not available: ${CC}"
	exit 1
fi

# check the compiler prefix if defined
if [ ${CCPREFIX} ]; then
	${CCPREFIX} &> /dev/null
	if [ "x${?}" == "x127" ]; then
		echo "WARNING: compiler prefix is not available: ${CCPREFIX}"
		CCPREFIX=
	fi
fi

echo "Creating Makefile"
rm -f Makefile
put '.PHONY: all clean'
put
DESTS=""
SKIPTARGETS=" "
for PROG in ${PROGS}; do
	_CC=$(eval echo \$$(echo "CC_${PROG}"))
	[ "${_CC}" ] || _CC=${CC}
	_BDIR=$(eval echo \$$(echo "BDIR_${PROG}"))
	[ "${_BDIR}" ] || _BDIR=${BDIR}
	# check the compiler
	${_CC} &> /dev/null
	if [ "x${?}" == "x127" ]; then
		echo "WARNING: Compiler is not available: ${_CC}"
		SKIPTARGETS=" ${PROG}${SKIPTARGETS}"
	else
		DESTS="${_BDIR}/${PROG} ${DESTS}"
	fi
done
put "all: ${DESTS}"
put
BDIRS=" "
echo "Generating dependencies for:"
for PROG in ${PROGS}; do
	if [ "$(echo "${SKIPTARGETS}" | grep " ${PROG} ")"  ]; then
		echo "  ${PROG}: skipping"
		continue
	fi
	echo "  ${PROG} "

	# try to load the actual PROG-specific settings
	# fall back to default settings if the specific one is empty
	_CC=$(eval echo \$$(echo "CC_${PROG}"))
	[ "${_CC}" ] || _CC=${CC}
	_CFLAGS=$(eval echo \$$(echo "CFLAGS_${PROG}"))
	[ "${_CFLAGS}" ] || _CFLAGS=${CFLAGS}
	_LDFLAGS=$(eval echo \$$(echo "LDFLAGS_${PROG}"))
	[ "${_LDFLAGS}" ] || _LDFLAGS=${LDFLAGS}
	_BDIR=$(eval echo \$$(echo "BDIR_${PROG}"))
	[ "${_BDIR}" ] || _BDIR=${BDIR}
	_FILES=$(eval echo \$$(echo "FILES_${PROG}"))
	# FILES variable is required as i said in the config section
	if [ -z "${_FILES}" ]; then
		echo "No FILES defined for ${PROG}."
		exit 1
	fi
	# use prefix if available
	[ ${CCPREFIX} ] && _CC="${CCPREFIX} ${_CC}"

	OBJS=""
	for FILE in ${_FILES}; do
		OBJS="${_BDIR}/${FILE}.o ${OBJS}"
	done
	put "${_BDIR}/${PROG}: ${_BDIR} ${OBJS}"
	put "	@echo \"	LD	${PROG}\""
	put "	@${_CC} ${OBJS} ${_LDFLAGS} -o ${_BDIR}/${PROG}"
	put ""
	for FILE in ${_FILES}; do
		echo "    ${FILE}.c"
		DEP="$(${_CC} -MM ${FILE}.c -MT ${_BDIR}/${FILE}.o)"
		[ "x${?}" = "x0" ] || exit 1
		put "${DEP}"
		put "	@echo \"	CC	${FILE}.o\""
		put "	@${_CC} ${_CFLAGS} -c ${FILE}.c -o ${_BDIR}/${FILE}.o"
		put
	done
	if [ -z "$(echo "${BDIRS}" | grep " ${_BDIR} ")"  ]; then
		BDIRS=" ${_BDIR}${BDIRS}"
		put "${_BDIR}:"
		put "	@echo \"	MKD	${_BDIR}\""
		put "	@mkdir -p ${_BDIR}"
		put
	fi
done
put "clean:"
put '	@echo "	CLEAN"'
put "	@rm -rf${BDIRS}"
echo "Makefile is ready."
