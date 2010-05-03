# Locate PTHREAD library and include paths

IF(UNIX)
  IF(APPLE)
    FIND_PATH(PTHREAD_INCLUDE_DIR pthread.h
      /usr/local/include
      ${MPTK_USER_INCLUDE_PATH}
       /usr/include
      )
    FIND_LIBRARY(PTHREAD_LIBRARY_FILE pthread
      /usr/local/lib
      /usr/lib
      $ENV{LD_LIBRARY_PATH}
      )
  ELSE(APPLE)
    FIND_PATH(PTHREAD_INCLUDE_DIR pthread.h
      /usr/local/include
      ${MPTK_USER_INCLUDE_PATH}
      /usr/include
      )
    FIND_LIBRARY(PTHREAD_LIBRARY_FILE pthread
      /usr/local/lib
       $ENV{LD_LIBRARY_PATH}
      /usr/lib
      )
  ENDIF(APPLE)
ELSE(UNIX)
  
  IF(WIN32)
    SET(RELATIVE_INCLUDE_PATH extras/windows/include)
    SET(RELATIVE_LIBRARY_PATH extras/windows/lib)
    FIND_PATH(PTHREAD_INCLUDE_DIR pthread.h ${MPTK_SOURCE_DIR}/${RELATIVE_INCLUDE_PATH})
    FIND_LIBRARY(PTHREAD_LIBRARY_FILE pthreadVC2 ${MPTK_SOURCE_DIR}/${RELATIVE_LIBRARY_PATH})
  ENDIF(WIN32)
ENDIF(UNIX)

GET_FILENAME_COMPONENT(PTHREAD_LIBRARY_PATH ${PTHREAD_LIBRARY_FILE} PATH CACHE)

SET (PTHREAD_FOUND 0)
IF(PTHREAD_INCLUDE_DIR)
  SET(PTHREAD_FOUND 1 )
   SET(HAVE_PTHREAD_H 1)
  MESSAGE(STATUS "pthread library found !!")
ELSE(PTHREAD_INCLUDE_DIR)
  MESSAGE(STATUS "pthread library not found !!")
ENDIF(PTHREAD_INCLUDE_DIR)
