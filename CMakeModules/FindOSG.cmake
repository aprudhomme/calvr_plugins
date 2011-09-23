FIND_PATH(OSG_HOME include/osg/Viewport
  PATHS
  $ENV{OSG_HOME}
  NO_DEFAULT_PATH
)

IF(NOT OSG_HOME)
FIND_PATH(OSG_HOME include/osg/Viewport
  PATHS
  /usr/local
  /usr
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt
)
ENDIF(NOT OSG_HOME)

FIND_PATH(OSG_INCLUDE_DIR osg/Viewport
  PATHS ${OSG_HOME}
  NO_DEFAULT_PATH
    PATH_SUFFIXES include
)

FIND_LIBRARY(OSG_LIBRARY 
  NAMES osg
  PATHS ${OSG_HOME}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)

IF(OSG_LIBRARY)
  SET(OSG_LIBRARIES ${OSG_LIBRARY})
ENDIF(OSG_LIBRARY)

FIND_LIBRARY(OPENTHREADS_LIBRARY 
  NAMES OpenThreads
  PATHS ${OSG_HOME}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)

IF(OPENTHREADS_LIBRARY)
  SET(OSG_LIBRARIES ${OPENTHREADS_LIBRARY} ${OSG_LIBRARIES})
ENDIF(OPENTHREADS_LIBRARY)

FIND_LIBRARY(OSGANIMATION_LIBRARY 
  NAMES osgAnimation
  PATHS ${OSG_HOME}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)

IF(OSGANIMATION_LIBRARY)
  SET(OSG_LIBRARIES ${OSGANIMATION_LIBRARY} ${OSG_LIBRARIES})
ENDIF(OSGANIMATION_LIBRARY)

FIND_LIBRARY(OSGDB_LIBRARY 
  NAMES osgDB
  PATHS ${OSG_HOME}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)

IF(OSGDB_LIBRARY)
  SET(OSG_LIBRARIES ${OSGDB_LIBRARY} ${OSG_LIBRARIES})
ENDIF(OSGDB_LIBRARY)

FIND_LIBRARY(OSGFX_LIBRARY 
  NAMES osgFX
  PATHS ${OSG_HOME}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)

IF(OSGFX_LIBRARY)
  SET(OSG_LIBRARIES ${OSGFX_LIBRARY} ${OSG_LIBRARIES})
ENDIF(OSGFX_LIBRARY)

FIND_LIBRARY(OSGGA_LIBRARY 
  NAMES osgGA
  PATHS ${OSG_HOME}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)

IF(OSGGA_LIBRARY)
  SET(OSG_LIBRARIES ${OSGGA_LIBRARY} ${OSG_LIBRARIES})
ENDIF(OSGGA_LIBRARY)

FIND_LIBRARY(OSGMANIPULATOR_LIBRARY 
  NAMES osgManipulator
  PATHS ${OSG_HOME}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)

IF(OSGMANIPULATOR_LIBRARY)
  SET(OSG_LIBRARIES ${OSGMANIPULATOR_LIBRARY} ${OSG_LIBRARIES})
ENDIF(OSGMANIPULATOR_LIBRARY)

FIND_LIBRARY(OSGPARTICLE_LIBRARY 
  NAMES osgParticle
  PATHS ${OSG_HOME}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)

IF(OSGPARTICLE_LIBRARY)
  SET(OSG_LIBRARIES ${OSGPARTICLE_LIBRARY} ${OSG_LIBRARIES})
ENDIF(OSGPARTICLE_LIBRARY)

FIND_LIBRARY(OSGPRESENTATION_LIBRARY 
  NAMES osgPresentation
  PATHS ${OSG_HOME}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)

IF(OSGPRESENTATION_LIBRARY)
  SET(OSG_LIBRARIES ${OSGPRESENTATION_LIBRARY} ${OSG_LIBRARIES})
ENDIF(OSGPRESENTATION_LIBRARY)

FIND_LIBRARY(OSGSHADOW_LIBRARY 
  NAMES osgShadow
  PATHS ${OSG_HOME}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)

IF(OSGSHADOW_LIBRARY)
  SET(OSG_LIBRARIES ${OSGSHADOW_LIBRARY} ${OSG_LIBRARIES})
ENDIF(OSGSHADOW_LIBRARY)

FIND_LIBRARY(OSGSIM_LIBRARY 
  NAMES osgSim
  PATHS ${OSG_HOME}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)

IF(OSGSIM_LIBRARY)
  SET(OSG_LIBRARIES ${OSGSIM_LIBRARY} ${OSG_LIBRARIES})
ENDIF(OSGSIM_LIBRARY)

FIND_LIBRARY(OSGTERRAIN_LIBRARY 
  NAMES osgTerrain
  PATHS ${OSG_HOME}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)

IF(OSGTERRAIN_LIBRARY)
  SET(OSG_LIBRARIES ${OSGTERRAIN_LIBRARY} ${OSG_LIBRARIES})
ENDIF(OSGTERRAIN_LIBRARY)

FIND_LIBRARY(OSGTEXT_LIBRARY 
  NAMES osgText
  PATHS ${OSG_HOME}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)

IF(OSGTEXT_LIBRARY)
  SET(OSG_LIBRARIES ${OSGTEXT_LIBRARY} ${OSG_LIBRARIES})
ENDIF(OSGTEXT_LIBRARY)

FIND_LIBRARY(OSGUTIL_LIBRARY 
  NAMES osgUtil
  PATHS ${OSG_HOME}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)

IF(OSGUTIL_LIBRARY)
  SET(OSG_LIBRARIES ${OSGUTIL_LIBRARY} ${OSG_LIBRARIES})
ENDIF(OSGUTIL_LIBRARY)

FIND_LIBRARY(OSGVIEWER_LIBRARY 
  NAMES osgViewer
  PATHS ${OSG_HOME}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)

IF(OSGVIEWER_LIBRARY)
  SET(OSG_LIBRARIES ${OSGVIEWER_LIBRARY} ${OSG_LIBRARIES})
ENDIF(OSGVIEWER_LIBRARY)

FIND_LIBRARY(OSGVOLUME_LIBRARY 
  NAMES osgVolume
  PATHS ${OSG_HOME}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)

IF(OSGVOLUME_LIBRARY)
  SET(OSG_LIBRARIES ${OSGVOLUME_LIBRARY} ${OSG_LIBRARIES})
ENDIF(OSGVOLUME_LIBRARY)

FIND_LIBRARY(OSGWIDGET_LIBRARY 
  NAMES osgWidget
  PATHS ${OSG_HOME}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)

IF(OSGWIDGET_LIBRARY)
  SET(OSG_LIBRARIES ${OSGWIDGET_LIBRARY} ${OSG_LIBRARIES})
ENDIF(OSGWIDGET_LIBRARY)

SET(OSG_FOUND "NO")
IF(OSG_LIBRARIES AND OSG_INCLUDE_DIR)
  SET(OSG_FOUND "YES")
ENDIF(OSG_LIBRARIES AND OSG_INCLUDE_DIR)

#message("${OSG_LIBRARIES}")

mark_as_advanced(
  OSG_LIBRARY
  OPENTHREADS_LIBRARY
  OSGANIMATION_LIBRARY
  OSGDB_LIBRARY
  OSGFX_LIBRARY
  OSGGA_LIBRARY
  OSGMANIPULATOR_LIBRARY
  OSGPARTICLE_LIBRARY
  OSGPRESENTATION_LIBRARY
  OSGSHADOW_LIBRARY
  OSGSIM_LIBRARY
  OSGTERRAIN_LIBRARY
  OSGTEXT_LIBRARY
  OSGUTIL_LIBRARY
  OSGVIEWER_LIBRARY
  OSGVOLUME_LIBRARY
  OSGWIDGET_LIBRARY
)