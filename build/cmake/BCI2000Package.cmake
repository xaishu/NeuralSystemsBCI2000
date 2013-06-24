###########################################################################
## $Id$
## Author: juergen.mellinger@uni-tuebingen.de
## Description: CPack setup for packaging BCI2000

IF( BUILD_DISTRIBUTION )

SET(CPACK_GENERATOR "ZIP")
SET(CPACK_IGNORE_FILES 
  "/CVS/;/\\\\.svn/;\\\\.swp$;\\\\.#;/#;\\\\.exp;\\\\.idb;\\\\.ilk;\\\\.tds;\\\\.lib;\\\\.pdb"
)
SET(CPACK_NSIS_DISPLAY_NAME "${PROJECT_NAME} ${PROJECT_VERSION}")
#SET(CPACK_PACKAGE_DESCRIPTION_FILE "")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "BCI2000 is a software for Brain-Computer Interface research")
SET(CPACK_PACKAGE_EXECUTABLES "")
SET(CPACK_PACKAGE_FILE_NAME "BCI2000")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "BCI2000")
SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "BCI2000")
SET(CPACK_PACKAGE_NAME "BCI2000 3.0")
SET(CPACK_PACKAGE_VENDOR "The BCI2000 Project")
SET(CPACK_PACKAGE_VERSION "3.0.0")
SET(CPACK_PACKAGE_VERSION_MAJOR "3")
SET(CPACK_PACKAGE_VERSION_MINOR "0")
SET(CPACK_PACKAGE_VERSION_PATCH "0")
#SET(CPACK_RESOURCE_FILE_LICENSE "")
#SET(CPACK_RESOURCE_FILE_README "")
#SET(CPACK_RESOURCE_FILE_WELCOME "")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-src")
SET(CPACK_SOURCE_STRIP_FILES "")
SET(CPACK_STRIP_FILES TRUE)
SET(CPACK_SYSTEM_NAME "Win32")
SET(CPACK_TOPLEVEL_TAG "Win32")
SET(CPACK_INSTALL_PREFIX "")

SET(CPACK_SOURCE_INSTALLED_DIRECTORIES
  "${BCI2000_ROOT_DIR}/src;./src"
)
SET(CPACK_INSTALLED_DIRECTORIES 
  "${BCI2000_ROOT_DIR}/batch;./batch"
  "${BCI2000_ROOT_DIR}/prog;./prog"
  "${BCI2000_ROOT_DIR}/tools;./tools"
  "${BCI2000_ROOT_DIR}/parms;./parms"
  "${BCI2000_ROOT_DIR}/data;./data"
  "${BCI2000_ROOT_DIR}/doc;./doc"  
)
SET(INSTALLED_FILES 
  GettingStarted.html 
  BCI2000_Help.html
)
FOREACH( _currentItem ${INSTALLED_FILES} )
  SET(CPACK_INSTALL_COMMANDS
    ${CPACK_INSTALL_COMMANDS}; "${CMAKE_COMMAND} -E copy \\\"${BCI2000_ROOT_DIR}/${_currentItem}\\\" _CPack_Packages/${CPACK_TOPLEVEL_TAG}/${CPACK_GENERATOR}/${CPACK_PACKAGE_FILE_NAME}"
  )
ENDFOREACH( _currentItem )

INCLUDE(CPack)

ENDIF( BUILD_DISTRIBUTION )