#version 330 core

out vec4 FragColor;

in vec4 vertex_color;
in vec2 texture_uv;

// the current sprite sheet we are pulling sprites from
uniform sampler2D sprite_sheet;

void main()
{
    vec4 texture_color = texture(sprite_sheet, texture_uv);
    FragColor = vertex_color * texture_color;
}