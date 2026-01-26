#version 330 core
out vec4 FragColor;

// material 
in vec3 outColor;

void main()
{
  FragColor = vec4(outColor.xyz, 1);
}