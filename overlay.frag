#version 330 core

out vec4 color;

uniform float alpha;

void main() {
    color = vec4(0.0f, 0.0f, 0.0f, alpha); 
}