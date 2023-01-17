#version 410 core

out vec4 fColor;

uniform int color;

void main() 
{    
    if(color == 0) fColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    else fColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
