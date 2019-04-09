GLM_PATH  = ../ext/glm-0.9.4.1

TEMPLATE  = app
TARGET    = projet

LIBS     += -lGLEW -lGL -lGLU -lm
INCLUDEPATH  += $${GLM_PATH}

SOURCES   = shader.cpp meshLoader.cpp trackball.cpp camera.cpp main.cpp viewer.cpp grid.cpp
HEADERS   = shader.h meshLoader.h trackball.h camera.h viewer.h grid.h

CONFIG   += qt opengl warn_on thread uic4 release
QT       *= xml opengl core
