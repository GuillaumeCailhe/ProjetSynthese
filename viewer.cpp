#include "viewer.h"

#include <math.h>
#include <iostream>
#include "meshLoader.h"
#include <QTime>

using namespace std;


Viewer::Viewer(const QGLFormat &format)
  : QGLWidget(format),
    _timer(new QTimer(this)),
    _currentshader(0),
    _light(glm::vec3(0,0,1)),
    _mode(false) {

  setlocale(LC_ALL,"C");

  _cam  = new Camera(1,glm::vec3(0,0,0));
  _grid = new Grid();

  _timer->setInterval(10);
  connect(_timer,SIGNAL(timeout()),this,SLOT(updateGL()));
}

Viewer::~Viewer() {
  delete _timer;
  delete _cam;

  for(unsigned int i=0;i<_shaders.size();++i) {
    delete _shaders[i];
  }

  deleteVAO();
  deleteTextures();
}

void Viewer::createTextures() {
  QImage image;
  
  // enable the use of 2D textures 
  glEnable(GL_TEXTURE_2D);

  // create one texture on the GPU
  //glGenTextures(1,_texIds);
}

void Viewer::deleteTextures() {
  //lDeleteTextures(1,_texIds);
}

void Viewer::createVAO() {
  //the variable _grid should be an instance of Grid
  //the .h file should contain the following VAO/buffer ids
  //GLuint _vaoTerrain;
  //GLuint _vaoQuad;
  //GLuint _terrain[2];
  //GLuint _quad;

  const GLfloat quadData[] = {
    -1.0f,-1.0f,0.0f, 1.0f,-1.0f,0.0f, -1.0f,1.0f,0.0f, -1.0f,1.0f,0.0f, 1.0f,-1.0f,0.0f, 1.0f,1.0f,0.0f };

  glGenBuffers(2,_terrain);
  glGenBuffers(1,&_quad);
  glGenVertexArrays(1,&_vaoTerrain);
  glGenVertexArrays(1,&_vaoQuad);

  // create the VBO associated with the grid (the terrain)
  glBindVertexArray(_vaoTerrain);
  glBindBuffer(GL_ARRAY_BUFFER,_terrain[0]); // vertices
  glBufferData(GL_ARRAY_BUFFER,_grid->nbVertices()*3*sizeof(float),_grid->vertices(),GL_STATIC_DRAW);
  glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void *)0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_terrain[1]); // indices
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,_grid->nbFaces()*3*sizeof(int),_grid->faces(),GL_STATIC_DRAW);

  // create the VBO associated with the screen quad
  glBindVertexArray(_vaoQuad);
  glBindBuffer(GL_ARRAY_BUFFER,_quad); // vertices
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadData),quadData,GL_STATIC_DRAW);
  glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void *)0);
  glEnableVertexAttribArray(0);
}

void Viewer::deleteVAO() {
  glDeleteBuffers(2,_terrain);
  glDeleteBuffers(1,&_quad);
  glDeleteVertexArrays(1,&_vaoTerrain);
  glDeleteVertexArrays(1,&_vaoQuad);
}

/**
 * Create the Frame Buffer Object
 * Used for Perlin Noise
 */
void Viewer::createFBO() {
  glGenFramebuffers(1,&_fboPerlin);
  glGenTextures(1,&_noiseTextureID_D);
  glGenTextures(1,&_noiseTextureID_N);
}

/**
 * Initialize the Frame Buffer Object
 */
void Viewer::initFBO() {
  // Diffuse map texture
  glBindTexture(GL_TEXTURE_2D,_noiseTextureID_D);
  glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA32F,_resolutionX,_resolutionY,0,GL_RGBA,GL_FLOAT,NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 

  // Normal map texture
  glBindTexture(GL_TEXTURE_2D,_noiseTextureID_N);
  glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA32F,_resolutionX,_resolutionY,0,GL_RGBA,GL_FLOAT,NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // Attach textures
  glBindFramebuffer(GL_FRAMEBUFFER,_fboPerlin);

  glBindTexture(GL_TEXTURE_2D,_noiseTextureID_N);
  glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,_noiseTextureID_N,0);
  

  glBindTexture(GL_TEXTURE_2D,_noiseTextureID_D);
  glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,_noiseTextureID_D,0);

  glBindFramebuffer(GL_FRAMEBUFFER,0);
}

/**
 * Delete the created FBO
 */
void Viewer::deleteFBO() {
  glDeleteFramebuffers(1,&_fboPerlin);
  glDeleteTextures(1,&_noiseTextureID_D);
  glDeleteTextures(1,&_noiseTextureID_N);
}

/**
 * Draw a simple quad.
 * This quad is used to generate the perlin noise.
 */
void Viewer::drawVAOQuad() {
  glBindVertexArray(_vaoQuad);
  glDrawArrays(GL_TRIANGLES,0,6);
  glBindVertexArray(0);
}

/**
 * Draw a grid.
 * Is used to render the terrain.
 */
void Viewer::drawVAOGrid() {
  glBindVertexArray(_vaoTerrain);
  glDrawElements(GL_TRIANGLES,3*_grid->nbFaces(),GL_UNSIGNED_INT,(void *)0);
  glBindVertexArray(0);
}

void Viewer::createShaders() {
  // perlin (pinpin issou) shader
  _vertexFilenames.push_back("shaders/noise.vert");
  _fragmentFilenames.push_back("shaders/noise.frag");

    // Normal Shader
  _vertexFilenames.push_back("shaders/normal.vert");
  _fragmentFilenames.push_back("shaders/normal.frag");
}

void Viewer::enableShader(unsigned int shader) {
  glUseProgram(_shaders[shader]->id());
}


void Viewer::disableShader() {
  // desactivate all shaders 
  glUseProgram(0);
}

void Viewer::paintGL() {
  /*
    First pass : generate a heightfield (the terrain) using a procedural perlin noise.
  */
  // Viewport initialization 
  glBindFramebuffer(GL_FRAMEBUFFER,_fboPerlin); // fbo init
  glDrawBuffer(GL_COLOR_ATTACHMENT0);

  // Shader of the perlin noise
  enableShader(0);

  //clearing buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0,0,_resolutionX,_resolutionY);

  // Drawing the quad
  drawVAOQuad();

  // Deactivate
  glBindFramebuffer(GL_FRAMEBUFFER,0); // deactivate fbo
  disableShader();

  /*
    Second pass : generate the normal map associated with the heightfield
  */
  glBindFramebuffer(GL_FRAMEBUFFER,_fboPerlin);
  // Drawing diffuse
  glDrawBuffer(GL_COLOR_ATTACHMENT1); 
  enableShader(1);

  // perlin noise texture creation
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D,_noiseTextureID_N);
  glUniform1i(glGetUniformLocation(_shaders[1]->id(),"colormap"),0);

  // clearing buffers
  //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  //glViewport(0,0,_resolutionX,_resolutionY);

  // draw
  drawVAOQuad();
  glBindFramebuffer(GL_FRAMEBUFFER,0);
  //disableShader();
  /*
    Drawing the scene
  */ 
  //drawVAOGrid();
  //disableShader();
}

void Viewer::resizeGL(int width,int height) {
  _cam->initialize(width,height,false);
  glViewport(0,0,width,height);
  updateGL();
}

void Viewer::mousePressEvent(QMouseEvent *me) {
  const glm::vec2 p((float)me->x(),(float)(height()-me->y()));

  if(me->button()==Qt::LeftButton) {
    _cam->initRotation(p);
    _mode = false;
  } else if(me->button()==Qt::MidButton) {
    _cam->initMoveZ(p);
    _mode = false;
  } else if(me->button()==Qt::RightButton) {
    _light[0] = (p[0]-(float)(width()/2))/((float)(width()/2));
    _light[1] = (p[1]-(float)(height()/2))/((float)(height()/2));
    _light[2] = 1.0f-std::max(fabs(_light[0]),fabs(_light[1]));
    _light = glm::normalize(_light);
    _mode = true;
  } 

  updateGL();
}

void Viewer::mouseMoveEvent(QMouseEvent *me) {
  const glm::vec2 p((float)me->x(),(float)(height()-me->y()));
 
  if(_mode) {
    // light mode
    _light[0] = (p[0]-(float)(width()/2))/((float)(width()/2));
    _light[1] = (p[1]-(float)(height()/2))/((float)(height()/2));
    _light[2] = 1.0f-std::max(fabs(_light[0]),fabs(_light[1]));
    _light = glm::normalize(_light);
  } else {
    // camera mode
    _cam->move(p);
  }

  updateGL();
}

void Viewer::keyPressEvent(QKeyEvent *ke) {
  
  // key a: play/stop animation
  if(ke->key()==Qt::Key_A) {
    if(_timer->isActive()) 
      _timer->stop();
    else 
      _timer->start();
  }

  // key i: init camera
  if(ke->key()==Qt::Key_I) {
    _cam->initialize(width(),height(),true);
  }
  
  // key f: compute FPS
  if(ke->key()==Qt::Key_F) {
    int elapsed;
    QTime timer;
    timer.start();
    unsigned int nb = 500;
    for(unsigned int i=0;i<nb;++i) {
      paintGL();
    }
    elapsed = timer.elapsed();
    double t = (double)nb/((double)elapsed);
    cout << "FPS : " << t*1000.0 << endl;
  }

  // key r: reload shaders 
  if(ke->key()==Qt::Key_R) {
    for(unsigned int i=0;i<_vertexFilenames.size();++i) {
      _shaders[i]->reload(_vertexFilenames[i].c_str(),_fragmentFilenames[i].c_str());
    }
  }

  // space: next shader
  if(ke->key()==Qt::Key_Space) {
    _currentshader = (_currentshader+1)%_shaders.size();
  }

  updateGL();
}

void Viewer::initializeGL() {
  // make this window the current one
  makeCurrent();

  // init and chack glew
  glewExperimental = GL_TRUE;

  if(glewInit()!=GLEW_OK) {
    cerr << "Warning: glewInit failed!" << endl;
  }

  // init OpenGL settings
  glClearColor(0.0,0.0,0.0,1.0);
  glEnable(GL_DEPTH_TEST);
  glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
  glViewport(0,0,width(),height());

  // initialize camera
  _cam->initialize(width(),height(),true);

  // load shader files
  createShaders();

  // create textures
  createTextures();

  // init and load all shader files
  for(unsigned int i=0;i<_vertexFilenames.size();++i) {
    _shaders.push_back(new Shader());
    _shaders[i]->load(_vertexFilenames[i].c_str(),_fragmentFilenames[i].c_str());
  }

  // init the first shader 
  createVAO();

  //createFBO();
  //initFBO();

  // starts the timer 
  _timer->start();
}