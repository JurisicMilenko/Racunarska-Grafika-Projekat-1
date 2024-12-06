#version 330 core //Koju verziju GLSL jezika da koristim (za programabilni pipeline: GL verzija * 100) i koji profil (core - samo programabilni pipeline, compatibility - core + zastarjele stvari)

//Kanali (in, out)
layout(location = 0) in vec2 inPos; //Pozicija tjemena
layout(location = 1) in vec4 inCol; //Boja tjemena - ovo saljemo u fragment sejder
out vec4 chCol; //Izlazni kanal kroz koji saljemo boju do fragment sejdera

layout(location = 0) in vec2 aPos;

out vec2 FragPos;

void main() {
    FragPos = aPos; // Pass the position to the fragment shader
    gl_Position = vec4(aPos, 0.0, 1.0);
}