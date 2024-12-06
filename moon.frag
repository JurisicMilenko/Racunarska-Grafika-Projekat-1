#version 330 core //Koju verziju GLSL jezika da koristim (za programabilni pipeline: GL verzija * 100) i koji profil (core - samo programabilni pajplajn, compatibility - core + zastarjele stvari)

//Kanali (in, out)
in vec4 chCol; //Kanal iz Verteks sejdera - mora biti ISTOG IMENA I TIPA kao u vertex sejderu
out vec4 outCol; //Izlazni kanal koji ce biti zavrsna boja tjemena

out vec4 FragColor;

in vec2 FragPos;
uniform vec2 center;

void main() {
    float dist = length(FragPos - center);
    float t = smoothstep(0.0, 0.5, dist);

    // Interpolate from orange (center) to yellow (edge)
    vec4 centerColor = vec4(1.0, 1.0, 0.9, 1.0);
    vec4 edgeColor = vec4(0.9, 0.9, 0.85, 1.0);
    vec4 gradientColor = mix(centerColor, edgeColor, t);

    FragColor = gradientColor;
}