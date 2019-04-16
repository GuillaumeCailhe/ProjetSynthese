#ifndef VIEWER_H
#define VIEWER_H

// GLEW lib: needs to be included first!!
#include <GL/glew.h> 

// OpenGL library 
#include <GL/gl.h>

// OpenGL Utility library
#include <GL/glu.h>

// OpenGL Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <QGLFormat>
#include <QGLWidget>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QTimer>
#include <stack>

#include "camera.h"
#include "meshLoader.h"
#include "shader.h"
#include "grid.h"

class Viewer : public QGLWidget {
 public:
  Viewer(const QGLFormat &format=QGLFormat::defaultFormat());
  ~Viewer();
  
 protected :
  virtual void paintGL();
  virtual void initializeGL();
  virtual void resizeGL(int width,int height);
  virtual void keyPressEvent(QKeyEvent *ke);
  virtual void mousePressEvent(QMouseEvent *me);
  virtual void mouseMoveEvent(QMouseEvent *me);

 private:
  void createVAO();
  void deleteVAO();

  void drawVAOQuad();
  void drawVAOGrid();

  void createFBO();
  void deleteFBO();
  void initFBO();

  void pass1();
  void pass2();
  void pass3();
  void pass4();
  void pass5();

  void createShaders();
  void enableShader(unsigned int shader=0);
  void disableShader();

  void createTextures();
  void deleteTextures();

  QTimer        *_timer;    // timer that controls the animation
  unsigned int   _currentshader; // current shader index

  Camera *_cam;    // the camera
  Grid *_grid;    // the grid to render the terrain

  glm::vec3 _light; // light direction
  bool      _mode;  // camera motion or light motion

  std::vector<std::string> _vertexFilenames;   // all vertex filenames
  std::vector<std::string> _fragmentFilenames; // all fragment filenames
  std::vector<Shader *>    _shaders;           // all the shaders 

  // vao/vbo ids 
  GLuint _vao;
  GLuint _buffers[5];

  // texture ids 
  GLuint _texIds[10];

  GLuint _vaoTerrain;
  GLuint _vaoQuad;
  GLuint _terrain[2];
  GLuint _quad;

  GLuint _resolutionX = 1024;
  GLuint _resolutionY = 1024;
  GLuint _noiseTextureID_D;
  GLuint _noiseTextureID_N;


  GLuint _fboPerlin;
};

#endif // VIEWER_H
