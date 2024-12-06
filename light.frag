#version 330 core

out vec4 color;

uniform float alpha;

void main() {
    // Combine overlay with background
    color = vec4(1.0f, 1.0f, 1.0f, alpha); 
}