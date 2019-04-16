#version 330

// input attributes 
layout(location = 0) in vec3 position;// position of the vertex in world space

uniform mat4 mdvMat; // modelview matrix (constant for all the vertices)
uniform mat4 projMat; // projection matrix (constant for all the vertices)
uniform mat3 normalMat; // normal matrix (constant for all the vertices)
uniform sampler2D heightmap; // generated with perlin noise
uniform sampler2D normalmap; 

// output variables that will be interpolated during rasterization (equivalent to varying)
out vec3 normalView;
out vec3 eyeView;
//out vec3 tangentView;
out vec2 uvcoord;

void main() {
  uvcoord  = (vec2(position.x,position.y)*0.5+0.5);	
  vec3 g = texture(heightmap,uvcoord).xyz;
  float h = g.x;
  
  vec3 adjustedPosition = vec3(position.x,position.y,h);
  gl_Position = projMat*mdvMat*vec4(adjustedPosition,1.0);
  
  eyeView = normalize((mdvMat*vec4(adjustedPosition,1.0)).xyz);
  //tangentView = normalize(normalMat*tangent);
  normalView = normalize(texture(normalmap, uvcoord).xyz);
}
