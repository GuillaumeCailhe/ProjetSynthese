#version 330

in  vec3 normalView;
//in  vec3 tangentView;
in  vec3 eyeView;
in  vec2 uvcoord;

out vec4 bufferColor;

uniform vec3      light;
uniform sampler2D colormap;
uniform sampler2D specularmap;
uniform sampler2D normalmap;
uniform sampler2D occmap;

void main() {
  float et      = 10.0;
  vec4 texColor = texture(colormap,uvcoord);
  vec4 texAO = texture(occmap, uvcoord);
  vec4 texSpecular = texture(specularmap, uvcoord);

  vec3 n = normalize(normalView);
  vec3 e = normalize(eyeView);
  vec3 l = normalize(light);

  float diff = max(dot(l,n),0.);
  float spec = pow(max(dot(reflect(l,n),e),0.0),et);

  bufferColor = texAO*texColor*(diff + texSpecular)*2.0;
}