#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;

uniform vec3 albedo;
uniform float metallic;

uniform vec3 cameraPos;
uniform samplerCube skybox;

void main()
{   
    vec3 I = normalize(Position - cameraPos);
    vec3 R = reflect(I, normalize(Normal));
    vec3 color = albedo * texture(skybox, R).rgb;
    FragColor = vec4(color.rgb, 1.0);
}