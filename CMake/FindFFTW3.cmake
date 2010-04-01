# Locate FFTW3 library and include paths

IF(UNIX)
	IF(APPLE)
		FIND_PATH(FFTW3_INCLUDE_DIR fftw3.h
		/usr/local/include
		${MPTK_USER_INCLUDE_PATH}
		/usr/include)
		FIND_LIBRARY(FFTW3_LIBRARY_FILE NAMES libfftw3.a
		/usr/local/lib
		/usr/lib
		$ENV{LD_LIBRARY_PATH}
		${MPTK_USER_LIB_PATH})
	ELSE(APPLE)
		FIND_PATH(FFTW3_INCLUDE_DIR fftw3.h
		/usr/local/include
		${MPTK_USER_INCLUDE_PATH}
		/usr/include)
		FIND_LIBRARY(FFTW3_LIBRARY_FILE NAMES libfftw3.a
		/usr/local/lib
		$ENV{LD_LIBRARY_PATH}
		${MPTK_USER_LIB_PATH}  
		/usr/lib)
	ENDIF(APPLE)
ELSE(UNIX)
	IF(WIN32)
		SET(RELATIVE_INCLUDE_PATH extras/windows/include)
		SET(RELATIVE_LIBRARY_PATH extras/windows/lib)
		FIND_PATH(FFTW3_INCLUDE_DIR fftw3.h ${MPTK_SOURCE_DIR}/${RELATIVE_INCLUDE_PATH})
		FIND_LIBRARY(FFTW3_LIBRARY_FILE fftw3-3 ${MPTK_SOURCE_DIR}/${RELATIVE_LIBRARY_PATH})
	ENDIF(WIN32)
ENDIF(UNIX)

SET (FFTW3_INCLUDE_FOUND 0)
IF(FFTW3_INCLUDE_DIR)
  SET(FFTW3_INCLUDE_FOUND 1 )
 SET(HAVE_FFTW3 1)
  MESSAGE(STATUS "fftw3.h found !!")
ELSE(FFTW3_INCLUDE_DIR)
  MESSAGE(STATUS "fftw3.h not found !!")
ENDIF(FFTW3_INCLUDE_DIR)
SET (FFTW3_LIB_FOUND 0)
IF(FFTW3_LIBRARY_FILE)
  SET(FFTW3_LIB_FOUND 1 )
  MESSAGE(STATUS "fftw3 lib found !!")
ELSE(FFTW3_LIBRARY_FILE)
  MESSAGE(STATUS "fftw3 lib not found !!")
ENDIF(FFTW3_LIBRARY_FILE)

