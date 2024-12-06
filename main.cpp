// Autor: Nedeljko Tesanovic
// Opis: Zestoko iskomentarisan program koji crta sareni trougao u OpenGL-u

#define _CRT_SECURE_NO_WARNINGS
 //Biblioteke za stvari iz C++-a (unos, ispis, fajlovi itd) 
#include <iostream>
#include <fstream>
#include <sstream>

//Biblioteke OpenGL-a
#include <GL/glew.h>   //Omogucava upotrebu OpenGL naredbi
#include <GLFW/glfw3.h>//Olaksava pravljenje i otvaranje prozora (konteksta) sa OpenGL sadrzajem
#define STB_IMAGE_IMPLEMENTATION    
#include "stb_image.h"
unsigned int compileShader(GLenum type, const char* source); //Uzima kod u fajlu na putanji "source", kompajlira ga i vraca sejder tipa "type"
unsigned int createShader(const char* vsSource, const char* fsSource); //Pravi objedinjeni sejder program koji se sastoji od Vertex sejdera ciji je kod na putanji vsSource i Fragment sejdera na putanji fsSource
static unsigned loadImageToTexture(const char* filePath); //Ucitavanje teksture, izdvojeno u funkciju



int main(void)
{
    //variables for stuff idk
    float middayColor[4] = { 0.15f, 0.55f, 1.0f, 1.0f };
    float sunCenter[] = {-0.75,0.75};
    float moonCenter[] = { -0.75, -3.25 };
    bool changeTime = false;
    float nightColor[4] = { 0.0, 0.0, 0.1, 1.0 };
    bool transitioning = false;
    float transitionProgress = 0.0f;
    float currentColor[4] = { 0.15f, 0.55f, 1.0f, 1.0f };
    float overlay = 0.0;
    float sunAngle = 0.0;
    float moonAngle = 3.141592;
    bool sunsTurn = false;
    bool doneTurning = true;
    bool dark = true;
    bool seeThroughWindow = false;
    float windowAlpha = 1.0f;
    float dogCenterX = 0.1f;
    float dogCenterY = -0.5f;
    float dogCenterXOld = 0.1f;
    float dogCenterYOld = -0.5f;
    bool facingRight = true;
    float eepX = 0.0f;
    float eepY = 0.1f;
    float eepAlpha = 0.0f;
    bool eepUp = true;
    float lime = 0.0f;
    float foodLocX = 0;
    float foodLocY = 0;
    int eating = 0;
    bool goingForFood = false;
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ INICIJALIZACIJA ++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // Pokretanje GLFW biblioteke
    // Nju koristimo za stvaranje okvira prozora / konteksta
    if (!glfwInit()) // !0 == 1  | glfwInit inicijalizuje GLFW i vrati 1 ako je inicijalizovana uspjesno, a 0 ako nije
    {
        std::cout<<"GLFW Biblioteka se nije ucitala! :(\n";
        return 1;
    }

    

    //Odredjivanje OpenGL verzije i profila (3.3, programabilni pajplajn)
    //bez ovoga, koristi se najnoviji moguci OpenGL koji hardver podrzava
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //Stvaranje prozora
    GLFWwindow* window; //Mjesto u memoriji za prozor
    unsigned int wWidth = 1000;
    unsigned int wHeight = 1000;
    const char wTitle[] = "[Generic Title]";
    window = glfwCreateWindow(wWidth, wHeight, wTitle, NULL, NULL); // Napravi novi prozor
    // glfwCreateWindow( sirina, visina, naslov, monitor na koji ovaj prozor ide preko citavog ekrana (u tom slucaju umjesto NULL ide glfwGetPrimaryMonitor() ), i prozori sa kojima ce dijeliti resurse )
    if (window == NULL) //Ako prozor nije napravljen
    {
        std::cout << "Prozor nije napravljen! :(\n";
        glfwTerminate(); //Gasi GLFW
        return 2; //Vrati kod za gresku
    }
    // Postavljanje novopecenog prozora kao aktivni (sa kojim cemo da radimo)
    glfwMakeContextCurrent(window);

   


    // Inicijalizacija GLEW biblioteke
    if (glewInit() != GLEW_OK) //Slicno kao glfwInit. GLEW_OK je predefinisani kod za uspjesnu inicijalizaciju sadrzan unutar biblioteke
    {
        std::cout << "GLEW nije mogao da se ucita! :'(\n";
        return 3;
    }

    int width, height, channels;
    unsigned char* pixels = stbi_load("bone.png", &width, &height, &channels, 4); // Force RGBA
    GLFWimage image;
    image.width = width;
    image.height = height;
    image.pixels = pixels;

    // Create the custom cursor
    GLFWcursor* cursor = glfwCreateCursor(&image, width / 2, height / 2);
    glfwSetCursor(window, cursor);


    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ PROMJENLJIVE I BAFERI +++++++++++++++++++++++++++++++++++++++++++++++++

    unsigned int sunShader = createShader("sun.vert", "sun.frag");
    unsigned int overlayShader = createShader("overlay.vert", "overlay.frag");
    unsigned int moonShader = createShader("moon.vert", "moon.frag");
    unsigned int basicShader = createShader("basic.vert", "basic.frag"); 
    unsigned int dogShader = createShader("dog.vert", "dog.frag");
    unsigned int insideShader = createShader("inside.vert", "inside.frag");
    unsigned int eepShader = createShader("eep.vert", "eep.frag");
    unsigned int textShader = createShader("text.vert", "text.frag");
    unsigned int lightShader = createShader("light.vert", "light.frag");

    //Nightfall
    float overlayVertices[] = {
    -1.0f, -1.0f, // Bottom-left
     1.0f, -1.0f, // Bottom-right
     1.0f,  1.0f, // Top-right
    -1.0f,  1.0f  // Top-left
    };
    
    glDisable(GL_DEPTH_TEST);

    glUseProgram(overlayShader);

    unsigned int overlayVAO, overlayVBO;
    glGenVertexArrays(1, &overlayVAO);
    glGenBuffers(1, &overlayVBO);

    glBindVertexArray(overlayVAO);
    glBindBuffer(GL_ARRAY_BUFFER, overlayVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(overlayVertices), overlayVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    glEnable(GL_DEPTH_TEST);



    //dawg
    //Tekstura
    unsigned texture = loadImageToTexture("dawg.png"); //Ucitavamo teksturu
    glBindTexture(GL_TEXTURE_2D, texture); //Podesavamo teksturu
    glGenerateMipmap(GL_TEXTURE_2D); //Generisemo mipmape 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);//S = U = X    GL_REPEAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);// T = V = Y
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);   //GL_NEAREST, GL_LINEAR
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(dogShader);
    unsigned uTexLoc = glGetUniformLocation(dogShader, "uTex");
    glUniform1i(uTexLoc, 0); // Indeks teksturne jedinice (sa koje teksture ce se citati boje)
    glUseProgram(0);

    float dogVertices[] = {
        // Positions          // Texture Coords
        dogCenterX - 0.1f, dogCenterY - 0.1f,0.0f,   0.0f, 0.0f, // Bottom-left
        dogCenterX + 0.1f, dogCenterY - 0.1f, 0.0f,   1.0f, 0.0f, // Bottom-right
        dogCenterX + 0.1f,  dogCenterY + 0.1f, 0.0f,   1.0f, 1.0f, // Top-right
        dogCenterX - 0.1f, dogCenterY + 0.1f, 0.0f,   0.0f, 1.0f  // Top-left
    };

    GLuint dogVBO, dogVAO;
    glGenVertexArrays(1, &dogVAO);
    glGenBuffers(1, &dogVBO);

    glBindVertexArray(dogVAO);

    glBindBuffer(GL_ARRAY_BUFFER, dogVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(dogVertices), dogVertices, GL_STATIC_DRAW);



    // Vertex position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinates
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //Eeeep
    unsigned textureEeep = loadImageToTexture("Eeep.png"); //Ucitavamo teksturu
    glBindTexture(GL_TEXTURE_2D, textureEeep); //Podesavamo teksturu
    glGenerateMipmap(GL_TEXTURE_2D); //Generisemo mipmape 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);//S = U = X    GL_REPEAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);// T = V = Y
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);   //GL_NEAREST, GL_LINEAR
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(eepShader);
    uTexLoc = glGetUniformLocation(eepShader, "uTex");
    glUniform1i(uTexLoc, 0); // Indeks teksturne jedinice (sa koje teksture ce se citati boje)
    glUseProgram(0);

    float eepVertices[] = {
        // Positions          // Texture Coords
        dogCenterX - 0.1f + eepX, dogCenterY - 0.1f + eepY - 0.2f ,0.0f,   0.0f, 0.0f, // Bottom-left
        dogCenterX + 0.1f + eepX, dogCenterY - 0.1f + eepY - 0.2f, 0.0f,   1.0f, 0.0f, // Bottom-right
        dogCenterX + 0.1f + eepX,  dogCenterY + 0.1f + eepY - 0.2f, 0.0f,   1.0f, 1.0f, // Top-right
        dogCenterX - 0.1f + eepX, dogCenterY + 0.1f + eepY - 0.2f, 0.0f,   0.0f, 1.0f  // Top-left
    };

    GLuint eepVBO, eepVAO;
    glGenVertexArrays(1, &eepVAO);
    glGenBuffers(1, &eepVBO);

    glBindVertexArray(eepVAO);

    glBindBuffer(GL_ARRAY_BUFFER, eepVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(eepVertices), eepVertices, GL_STATIC_DRAW);

    //text
    unsigned textureText = loadImageToTexture("Index.png"); //Ucitavamo teksturu
    glBindTexture(GL_TEXTURE_2D, textureText); //Podesavamo teksturu
    glGenerateMipmap(GL_TEXTURE_2D); //Generisemo mipmape 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);//S = U = X    GL_REPEAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);// T = V = Y
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);   //GL_NEAREST, GL_LINEAR
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(textShader);
    uTexLoc = glGetUniformLocation(textShader, "uTex");
    glUniform1i(uTexLoc, 0); // Indeks teksturne jedinice (sa koje teksture ce se citati boje)
    glUseProgram(0);

    float textVertices[] = {
        // Positions          // Texture Coords
        0.6, 0.95f ,0.0f,   0.0f, 0.0f, // Bottom-left
        1.0f, 0.95f, 0.0f,   1.0f, 0.0f, // Bottom-right
        1.0f,  1.0f, 0.0f,   1.0f, 1.0f, // Top-right
        0.6, 1.0f, 0.0f,   0.0f, 1.0f  // Top-left
    };

    GLuint textVBO, textVAO;
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);

    glBindVertexArray(textVAO);

    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(textVertices), textVertices, GL_STATIC_DRAW);

    // Vertex position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinates
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //sljiva
    float sljivaVertices[] = {
     0.89f - 0.2f, -0.6f,
     0.9f - 0.2f, -0.6f,
     0.89f - 0.2f, -0.2f,

     0.9f - 0.2f, -0.6f,
     0.89f - 0.2f, -0.2f,
     0.9f - 0.2, -0.2f,

    };
    glUseProgram(basicShader);

    GLint colLoc = glGetUniformLocation(basicShader, "col");
    glUniform4f(colLoc, 0.6f, 0.3f, 0.0f,1.0f);



    glBufferData(GL_ARRAY_BUFFER, sizeof(sljivaVertices), sljivaVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //lime
    float limeVertices[] = {
     0.89f - 0.2f, -0.6f,
     0.9f - 0.2f, -0.6f,
     0.89f - 0.2f, -0.2f,

     0.9f - 0.2f, -0.6f,
     0.89f - 0.2f, -0.2f,
     0.9f - 0.2, -0.2f,

    };
    glUseProgram(basicShader);

    colLoc = glGetUniformLocation(basicShader, "col");
    glUniform4f(colLoc, 1.6f, 1.3f, 1.0f, 1.0f);



    glBufferData(GL_ARRAY_BUFFER, sizeof(limeVertices), limeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //Ground
    float groundVertices[] = {
    -1.0f, -1.0f, // Bottom-left
     1.0f, -1.0f, // Bottom-right
     1.0f,  -0.6f, // Top-right
    -1.0f,  -0.6f  // Top-left
    };

    glUseProgram(basicShader); 

    colLoc = glGetUniformLocation(basicShader, "col");
    glUniform3f(colLoc, 0.0f, 1.0f, 0.0f); 



    unsigned int basicVAO, basicVBO;
    glGenVertexArrays(1, &basicVAO);
    glGenBuffers(1, &basicVBO);

    glBindVertexArray(basicVAO);
    glBindBuffer(GL_ARRAY_BUFFER, basicVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    //House
    float houseVertices[] = {
    -0.9f, -0.6f,
     -0.2f, -0.6f,
     
    -0.2f,  0.2f,
    -0.9f,  0.2f
    };

    glUseProgram(basicShader);

    colLoc = glGetUniformLocation(basicShader, "col");
    glUniform3f(colLoc, 1.0f, 1.0f, 0.0f);


    glBindVertexArray(basicVAO);
    glBindBuffer(GL_ARRAY_BUFFER, basicVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(houseVertices), houseVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);



    //Roof
    float roofVertices[] = {
     -1.0f, 0.2f,
     -0.1f, 0.2f,
     -0.55f, 0.45f,

     

    };

    glUseProgram(basicShader);

    colLoc = glGetUniformLocation(basicShader, "col");
    glUniform3f(colLoc, 1.0f, 1.0f, 1.0f);

    glBufferData(GL_ARRAY_BUFFER, sizeof(roofVertices), roofVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //Light overlay
    float lightOverlayVertices[] = {
    -0.68f, -0.335f,
    -0.8f, -0.4f,
    -0.8f, -0.5f,

    -0.63f,  -0.5f,
    -0.63f,  -0.4f
    };

    glDisable(GL_DEPTH_TEST);

    glUseProgram(lightShader);

    unsigned int lightVAO, lightVBO;
    glGenVertexArrays(1, &lightVAO);
    glGenBuffers(1, &lightVBO);

    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lightOverlayVertices), lightOverlayVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    glEnable(GL_DEPTH_TEST);

    //Window 1f
    float window1fVertices[] = {
    -0.8f, -0.5f,
     -0.63f, -0.5f,

    -0.63f,  -0.25f,
    -0.8f,  -0.25f
    };

    glUseProgram(basicShader);

    colLoc = glGetUniformLocation(basicShader, "col");
    glUniform3f(colLoc, 0.15, 0.55, 1.0);


    glBindVertexArray(basicVAO);
    glBindBuffer(GL_ARRAY_BUFFER, basicVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(window1fVertices), window1fVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //Window 1f inside
    unsigned textureInside = loadImageToTexture("Inside.png"); //Ucitavamo teksturu
    glBindTexture(GL_TEXTURE_2D, textureInside); //Podesavamo teksturu
    glGenerateMipmap(GL_TEXTURE_2D); //Generisemo mipmape 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);//S = U = X    GL_REPEAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);// T = V = Y
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);   //GL_NEAREST, GL_LINEAR
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(insideShader);
    uTexLoc = glGetUniformLocation(insideShader, "uTex");
    glUniform1i(uTexLoc, 0); // Indeks teksturne jedinice (sa koje teksture ce se citati boje)
    glUseProgram(0);
    float window1fInsideVertices[] = {
        // Positions          // Texture Coords
         -0.8, -0.5f,0.0f,   0.0f, 0.0f, // Bottom-left
         -0.63f, -0.5f, 0.0f,   1.0f, 0.0f, // Bottom-right
        -0.63f,  -0.25f, 0.0f,   1.0f, 1.0f, // Top-right
        -0.8f,  -0.25f, 0.0f,   0.0f, 1.0f  // Top-left
    };

    GLuint insideVBO, insideVAO;
    glGenVertexArrays(1, &insideVAO);
    glGenBuffers(1, &insideVBO);

    glBindVertexArray(insideVAO);

    glBindBuffer(GL_ARRAY_BUFFER, insideVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(window1fInsideVertices), window1fInsideVertices, GL_STATIC_DRAW);



    // Vertex position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinates
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //frame 1f
    float frame1fVertices[] = {
     -0.725f, -0.5f,
     -0.725f, -0.25f,
     -0.705f, -0.5f,

     -0.725f, -0.25f,
     -0.705f, -0.5f,
     -0.705f, -0.25f,

     -0.8f, -0.365f,
     -0.63f, -0.365f,
     -0.8f, -0.385f,
     
     -0.63f, -0.385f,
     -0.63f, -0.365f,
     -0.8f, -0.385f


    };

    glUseProgram(basicShader);

    colLoc = glGetUniformLocation(basicShader, "col");
    glUniform3f(colLoc, 1.0f, 1.0f, 1.0f);

    glBufferData(GL_ARRAY_BUFFER, sizeof(frame1fVertices), frame1fVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //Front fence
    float frontFenceVertices[] = {
     0.89f, -0.6f,
     0.9f, -0.6f,
     0.89f, -0.4f,

     0.9f, -0.6f,
     0.89f, -0.4f,
     0.9f, -0.4f,

    };

    glUseProgram(basicShader);

    colLoc = glGetUniformLocation(basicShader, "col");
    glUniform3f(colLoc, 1.0f, 1.0f, 1.0f);

    glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //Side fence
    float sideFenceVertices[] = {
     0.84f, -0.6f,
     0.88f, -0.6f,
     0.84f, -0.43f,

     0.88f, -0.6f,
     0.84f, -0.43f,
     0.88f, -0.43f,

     0.84f, -0.43f,
     0.88f, -0.43f,
     0.86f, -0.4f,
    };

    glUseProgram(basicShader);

    colLoc = glGetUniformLocation(basicShader, "col");
    glUniform3f(colLoc, 1.0f, 1.0f, 1.0f);

    glBufferData(GL_ARRAY_BUFFER, sizeof(sideFenceVertices), sideFenceVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //fence connector
    float connectorVertices[] = {
    -1.0f, -0.50f, // Bottom-left
     0.89f, -0.50f, // Bottom-right
     0.89f,  -0.48f, // Top-right
    -1.0f,  -0.48f  // Top-left
    };
    glUseProgram(basicShader);

    colLoc = glGetUniformLocation(basicShader, "col");
    glUniform3f(colLoc, 1.0f, 1.0f, 1.0f);

    glBufferData(GL_ARRAY_BUFFER, sizeof(connectorVertices), connectorVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    //Moon
    float moon[(30 + 2) * 2];
    float r = 0.2; //poluprecnik

    moon[0] = moonCenter[0]; //Centar X0
    moon[1] = moonCenter[1]; //Centar Y0
    int i;
    for (i = 0; i <= 30; i++)
    {

        moon[2 + 2 * i] = moon[0] + r * cos((3.141592 / 180) * (i * 360 / 30)); //Xi (Matematicke funkcije rade sa radijanima)
        moon[2 + 2 * i + 1] = moon[1] + r * sin((3.141592 / 180) * (i * 360 / 30)); //Yi
    }

    glUseProgram(moonShader);

    float moonCenterS[2] = { -0.75, 0.75 };
    int centerMoonLocation = glGetUniformLocation(moonShader, "center");
    glUniform2fv(centerMoonLocation, 1, moonCenterS);



    unsigned int moonVAO, moonVBO;
    glGenVertexArrays(1, &moonVAO);
    glGenBuffers(1, &moonVBO);

    glBindVertexArray(moonVAO);
    glBindBuffer(GL_ARRAY_BUFFER, moonVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(moon), moon, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //Sun
    float circle[(30 + 2) * 2];
    r = 0.2; //poluprecnik

    circle[0] = sunCenter[0]; //Centar X0
    circle[1] = sunCenter[1]; //Centar Y0

    for (i = 0; i <= 30; i++)
    {

        circle[2 + 2 * i] = circle[0] + r * cos((3.141592 / 180) * (i * 360 / 30)); //Xi (Matematicke funkcije rade sa radijanima)
        circle[2 + 2 * i + 1] = circle[1] + r * sin((3.141592 / 180) * (i * 360 / 30)); //Yi
    }

    glUseProgram(sunShader); // Activate your shader program

    float circleCenter[2] = { -0.75, 0.75 };
    int centerLocation = glGetUniformLocation(sunShader, "center");
    glUniform2fv(centerLocation, 1, circleCenter);

    unsigned int stride = 2 * sizeof(float); //Korak pri kretanju po podacima o tjemenima = Koliko mjesta u memoriji izmedju istih komponenti susjednih tjemena
    //Za svrhe vjezbi ovo je efektivno velicina jednog tjemena

    //Vertex Array Object - Sadrzi bar 16 pokazivaca na atribute koji opusuju sadrzaje bafer objekata
    unsigned int VAO;
    glGenVertexArrays(1, &VAO); //Generisi 1 VAO na adresi datoj adresi
    glBindVertexArray(VAO); //Povezi VAO za aktivni kontekst - sva naknadna podesavanja ce se odnositi na taj VAO

    //Vertex Buffer Object - Bafer objekat, u ovom slucaju za tjemena trougla koji crtamo
    unsigned int VBO;
    glGenBuffers(1, &VBO); //Generisi 1 bafer na datoj adresi 
    glBindBuffer(GL_ARRAY_BUFFER, VBO); //Povezi dati bafer za aktivni kontekst. Array buffer se koristi za tjemena figura.
    glBufferData(GL_ARRAY_BUFFER, sizeof(circle), circle, GL_STATIC_DRAW); //Slanje podataka na memoriju graficke karte
    //glBufferData(koji bafer, koliko podataka ima, adresa podataka, nacin upotrebe podataka (GL_STATIC_DRAW, GL_DYNAMIC_DRAW, GL_STREAM_DRAW;)

    //Opisivanje pokazivaca na atribute: Pokazivac 0 ceo opisati poziciju (koordinate x i y), a pokazivac 1 boju (komponente r, g, b i a).
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
    //glVertexAttribPointer(indeks pokazivaca, broj komponenti atributa, tip komponenti atributa, da li je potrebno normalizovati podatke (nama uvijek GL_FALSE), korak/velicina tjemena, pomjeraj sa pocetka jednog tjemena do komponente za ovaj atribut - mora biti (void*))  
    glEnableVertexAttribArray(0); //Aktiviraj taj pokazivac i tako intepretiraj podatke
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(2 * sizeof(float))); //Objasni da su preostala cetiri broja boja (preskacemo preko XY od pozicije, pa je pomjeraj 2 * sizeof(float))
    glEnableVertexAttribArray(1);

    //Postavili smo sta treba, pa te stvari iskljucujemo, da se naknadna podesavanja ne bi odnosila na njih i nesto poremetila
    //To radimo tako sto bindujemo 0, pa kada treba da nacrtamo nase stvari, samo ih ponovo bindujemo
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(circle), circle, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ RENDER LOOP - PETLJA ZA CRTANJE +++++++++++++++++++++++++++++++++++++++++++++++++

    glClearColor(0.15, 0.55, 1.0, 1.0); //Podesavanje boje pozadine (RGBA format);

    while (!glfwWindowShouldClose(window)) //Beskonacna petlja iz koje izlazimo tek kada prozor treba da se zatvori
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //Unos od korisnika bez callback funckcije. GLFW_PRESS = Dugme je trenutno pritisnuto. GLFW_RELEASE = Dugme trenutno nije pritisnuto
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            if (xpos > 420 && xpos < 900 && ypos > 800 && dark == true) {
                if (foodLocX == 0 && foodLocY == 0) {
                    foodLocX = (2.0 * xpos / 1000) - 1.0;
                    std::cout << foodLocX << " " << foodLocY << std::endl;
                    dogCenterXOld = dogCenterX;
                    eating = 0;
                    goingForFood = true;
                }
            }
        }
        eating++;
        if (foodLocX  != 0) {
            if (foodLocX - dogCenterX < 0) {
                dogCenterX -= 0.01f;
                facingRight = false;
            }
            if (foodLocX - dogCenterX > 0) {
                dogCenterX += 0.01f;
                facingRight = true;
            }
            if (abs(foodLocX - dogCenterX) < 0.01f && eating > 360) {
                foodLocX = 0;
            }
        }
        if (foodLocX == 0 && dogCenterX != dogCenterXOld && goingForFood == true) {
            if (dogCenterXOld - dogCenterX < 0) {
                dogCenterX -= 0.01f;
                facingRight = false;
            }
            if (dogCenterXOld - dogCenterX > 0) {
                dogCenterX += 0.01f;
                facingRight = true;
            }
            if (abs(dogCenterXOld - dogCenterX) < 0.01f) {
                dogCenterX = dogCenterXOld;
                goingForFood = false;
                
            }
        }
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }
        if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
        {
            eepAlpha = 1.0f;
            if (doneTurning == true) {
            if (sunsTurn == false) {
                            sunsTurn = true;
                        }
                        else {
                            sunsTurn = false;
                        }
            if (dark == true) {
                dark = false;
            }
            else {
                dark = true;
            }
                        changeTime = true;
                        transitioning = true;
                        doneTurning = false;
            }
        }
        if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
        {
            if (seeThroughWindow == true) {
                seeThroughWindow = false;
            }
            else {
                seeThroughWindow = true;
            }
            
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            lime += 0.01f;
            if (lime >= 0.4f) {
                lime = 0.4f;
            }

        }
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            lime -= 0.01f;
            if (lime <= 0.0f) {
                lime = 0.0f;
            }

        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && dark == true)
        {
            dogCenterX += 0.01;
            if (dogCenterX + 0.1f >= 0.89) {
                dogCenterX = 0.79;
            }
            facingRight = true;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && dark == true)
        {
            dogCenterX -= 0.01;
            if (dogCenterX - 0.1f <= -0.2) {
                dogCenterX = -0.1;
            }
            facingRight = false;
        }

        if (changeTime == true) {
            //move sun
            sunAngle += 0.1;
            sunCenter[0] += 0.2 * cos(sunAngle);
            sunCenter[1] -= 0.2 * sin(sunAngle);
                    float circleCenter[2] = { sunCenter[0], sunCenter[1] };
                    circle[0] = sunCenter[0]; //Centar X0
                    circle[1] = sunCenter[1]; //Centar Y0
                    int i;
                    for (i = 0; i <= 30; i++)
                    {

                        circle[2 + 2 * i] = circle[0] + r * cos((3.141592 / 180) * (i * 360 / 30)); //Xi (Matematicke funkcije rade sa radijanima)
                        circle[2 + 2 * i + 1] = circle[1] + r * sin((3.141592 / 180) * (i * 360 / 30)); //Yi
                    }
            if (sunAngle >= 3.141592f && sunsTurn == true) {
                doneTurning = true;
                changeTime = false;
            }
            //move moon
            moonAngle += 0.1;
            moonCenter[0] += 0.2 * cos(moonAngle);
            moonCenter[1] -= 0.2 * sin(moonAngle);
            float moonCenterS[2] = { moonCenter[0], moonCenter[1] };
            moon[0] = moonCenter[0]; //Centar X0
            moon[1] = moonCenter[1]; //Centar Y0
            for (i = 0; i <= 30; i++)
            {

                moon[2 + 2 * i] = moon[0] + r * cos((3.141592 / 180) * (i * 360 / 30)); //Xi (Matematicke funkcije rade sa radijanima)
                moon[2 + 2 * i + 1] = moon[1] + r * sin((3.141592 / 180) * (i * 360 / 30)); //Yi
            }
            if (moonAngle >= 3.141592f && sunsTurn == false) {
                doneTurning = true;
                changeTime = false;
            }
            if (sunAngle >= 3.141592f * 2) {
                sunAngle = 0;
                sunCenter[0] = -0.75;
                sunCenter[1] = 0.75;
            }
            if (moonAngle >= 3.141592f * 2) {
                moonAngle = 0;
                moonCenter[0] = -0.75; 
                moonCenter[1] = 0.75;
            }
        }
        
        //window die
        if (seeThroughWindow == true) {
            windowAlpha -= 0.006f;
            if (windowAlpha < 0) {
                windowAlpha = 0;
            }
        }
        if (seeThroughWindow == false) {
            windowAlpha += 0.006f;
            if (windowAlpha > 1.0f) {
                windowAlpha = 1.0f;
            }
        }

        //darken
        if (transitioning) {
            if (dark == false) {
                overlay += 0.006f;
            }
            else {
                overlay -= 0.006f;
            }
            
            if (overlay > 0.8f) {
                overlay = 0.8f;
                transitioning = false;
            }
            if (overlay < 0) {
                overlay = 0;
                transitioning = false;
            }
        }

        if (dark == true) {
            eepAlpha = 0.0f;
        }
        if (eepAlpha <= 0.0f) {
            eepX = 0.0f;
            eepY = 0.0f;
            if (dark == false) {
                eepAlpha = 1.0f;
            }
        }
        if (dark == false) {
            eepX += 0.001;
            eepAlpha -= 0.003;
            if (eepUp == true) {
                eepY += 0.001;
            }
            else {
                eepY -= 0.001;
            }
            if (eepY >= 0.05 && eepUp == true) {
                eepUp = false;
            }
            if (eepY <= 0 && eepUp == false) {
                eepUp = true;
            }

        }
        glClearColor(currentColor[0], currentColor[1], currentColor[2], currentColor[3]);
        circleCenter[0] = sunCenter[0];
        circleCenter[1] = sunCenter[1];
        
        //Brisanje ekrana
        glClear(GL_COLOR_BUFFER_BIT);


        

        // [KOD ZA CRTANJE]
        //ground
        glUseProgram(basicShader);
        GLint colLoc = glGetUniformLocation(basicShader, "col");
        glUniform4f(colLoc, 0.0f, 1.0f, 0.0f, 1.0f); // Green color
        glBindVertexArray(basicVAO);
        glBindBuffer(GL_ARRAY_BUFFER, basicVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glBindVertexArray(basicVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindVertexArray(0);


        //house
        glUseProgram(basicShader);
        colLoc = glGetUniformLocation(basicShader, "col");
        glUniform4f(colLoc, 1.0f, 1.0f, 0.0f, 1.0f);
        glBindVertexArray(basicVAO);
        glBindBuffer(GL_ARRAY_BUFFER, basicVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(houseVertices), houseVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glBindVertexArray(basicVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindVertexArray(0);

        


        
        glDisable(GL_DEPTH_TEST);
        //window1finside
        glBindVertexArray(insideVAO);

        glBindBuffer(GL_ARRAY_BUFFER, insideVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(window1fInsideVertices), window1fInsideVertices, GL_STATIC_DRAW);



        // Vertex position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Texture coordinates
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glUseProgram(insideShader);
        glBindVertexArray(insideVAO);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureInside);

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindVertexArray(0);


        glUseProgram(0);
        glEnable(GL_DEPTH_TEST);
        
        glDisable(GL_DEPTH_TEST);
        //window1f
        glUseProgram(basicShader);
        colLoc = glGetUniformLocation(basicShader, "col");
        glUniform4f(colLoc, 0.15f, 0.7f, 1.0f, windowAlpha);
        glBindVertexArray(basicVAO);
        glBindBuffer(GL_ARRAY_BUFFER, basicVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(window1fVertices), window1fVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glBindVertexArray(basicVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindVertexArray(0);

        //frame1f
        glUseProgram(basicShader);
        colLoc = glGetUniformLocation(basicShader, "col");
        glUniform4f(colLoc, 1.0f, 1.0f, 1.0f, windowAlpha);
        glBindVertexArray(basicVAO);
        glBindBuffer(GL_ARRAY_BUFFER, basicVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(frame1fVertices), frame1fVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glBindVertexArray(basicVAO);
        glDrawArrays(GL_TRIANGLES, 0, 12);
        glBindVertexArray(0);

#

        glEnable(GL_DEPTH_TEST);

        //roof
        glUseProgram(basicShader);
        colLoc = glGetUniformLocation(basicShader, "col");
        glUniform4f(colLoc, 1.0f, 0.0f, 0.0f, 1.0f);
        glBindVertexArray(basicVAO);
        glBindBuffer(GL_ARRAY_BUFFER, basicVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(roofVertices), roofVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glBindVertexArray(basicVAO);
        glDrawArrays(GL_TRIANGLES, 0, 4);
        glBindVertexArray(0);

        //lime
        float limeVertices[] = {
         0.89f - 0.2f, -0.6f-lime,
         0.9f - 0.2f, -0.6f - lime,
         0.89f - 0.2f, -0.2f - lime,

         0.9f - 0.2f, -0.6f - lime,
         0.89f - 0.2f, -0.2f - lime,
         0.9f - 0.2, -0.2f - lime,

        };


        glUseProgram(basicShader);
        colLoc = glGetUniformLocation(basicShader, "col");
        glUniform4f(colLoc, 1.0f, 1.0f, 1.0f, 1.0f);
        glBindVertexArray(basicVAO);
        glBindBuffer(GL_ARRAY_BUFFER, basicVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(limeVertices), limeVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glBindVertexArray(basicVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        //sljiva
        glUseProgram(basicShader);
        colLoc = glGetUniformLocation(basicShader, "col");
        glUniform4f(colLoc, 0.6f, 0.3f, 0.0f, 1.0f);
        glBindVertexArray(basicVAO);
        glBindBuffer(GL_ARRAY_BUFFER, basicVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(sljivaVertices), sljivaVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glBindVertexArray(basicVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);



        //front fence
        glUseProgram(basicShader);
        colLoc = glGetUniformLocation(basicShader, "col");
        glUniform4f(colLoc, 1.0f, 1.0f, 1.0f, 1.0f);
        glBindVertexArray(basicVAO);
        glBindBuffer(GL_ARRAY_BUFFER, basicVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(frontFenceVertices), frontFenceVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glBindVertexArray(basicVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        //side fence
        for (int j = 0; j < 40; j++) {
            float sideFenceContVertices[] = {
         0.84f - j * 0.05, -0.6f,
         0.88f - j * 0.05, -0.6f,
         0.84f - j * 0.05, -0.43f,

         0.88f - j * 0.05, -0.6f,
         0.84f - j * 0.05, -0.43f,
         0.88f - j * 0.05, -0.43f,

         0.84f - j * 0.05, -0.43f,
         0.88f - j * 0.05, -0.43f,
         0.86f - j * 0.05, -0.4f,
            };
        glUseProgram(basicShader);
        colLoc = glGetUniformLocation(basicShader, "col");
        glUniform4f(colLoc, 1.0f, 1.0f, 1.0f, 1.0f);
        glBindVertexArray(basicVAO);
        glBindBuffer(GL_ARRAY_BUFFER, basicVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(sideFenceContVertices), sideFenceContVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glBindVertexArray(basicVAO);
        glDrawArrays(GL_TRIANGLES, 0, 9);
        glBindVertexArray(0);
        }

        //fence connector
        glUseProgram(basicShader);
        colLoc = glGetUniformLocation(basicShader, "col");
        glUniform4f(colLoc, 1.0f, 1.0f, 1.0f, 1.0f);
        glBindVertexArray(basicVAO);
        glBindBuffer(GL_ARRAY_BUFFER, basicVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(connectorVertices), connectorVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glBindVertexArray(basicVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindVertexArray(0);

        //sun
        glUseProgram(sunShader); //Izaberi nas sejder program za crtanje i koristi ga za svo naknadno crtanje (Ukoliko ne aktiviramo neke druge sejder programe)
        unsigned int uA = glGetUniformLocation(sunShader, "uA");
        glUniform1f(uA, abs(sin(10 * glfwGetTime())));
        glBindVertexArray(VAO); //Izaberemo sta zelimo da crtamo
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(circle), circle, GL_STATIC_DRAW);
        glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(circle) / (2 * sizeof(float))); //To i nacrtamo
        //glDrawArrays(tip primitive, indeks pocetnog tjemena, koliko narednih tjemena crtamo);
        // 
        // 
        // 
        // 
        
        glDisable(GL_DEPTH_TEST);

        //dawg

        if (facingRight == true) {
            float dogVertices[] = {
                // Positions          // Texture Coords
                dogCenterX - 0.1f, dogCenterY - 0.1f,0.0f,   0.0f, 0.0f, // Bottom-left
                dogCenterX + 0.1f, dogCenterY - 0.1f, 0.0f,   1.0f, 0.0f, // Bottom-right
                dogCenterX + 0.1f,  dogCenterY + 0.1f, 0.0f,   1.0f, 1.0f, // Top-right
                dogCenterX - 0.1f, dogCenterY + 0.1f, 0.0f,   0.0f, 1.0f  // Top-left
            };
            glBindVertexArray(dogVAO);

            glBindBuffer(GL_ARRAY_BUFFER, dogVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(dogVertices), dogVertices, GL_STATIC_DRAW);



            // Vertex position
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            // Texture coordinates
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
            glUseProgram(dogShader);
            glBindVertexArray(dogVAO);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture);

            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

            glBindTexture(GL_TEXTURE_2D, 0);
            glBindVertexArray(0);


            glUseProgram(0);
        }
        else {
            float dogVertices[] = {
                // Positions          // Texture Coords
                dogCenterX - 0.1f, dogCenterY - 0.1f,0.0f,   1.0f, 0.0f, // Bottom-left
                dogCenterX + 0.1f, dogCenterY - 0.1f, 0.0f,   0.0f, 0.0f, // Bottom-right
                dogCenterX + 0.1f,  dogCenterY + 0.1f, 0.0f,   0.0f, 1.0f, // Top-right
                dogCenterX - 0.1f, dogCenterY + 0.1f, 0.0f,   1.0f, 1.0f  // Top-left
            };
            glBindVertexArray(dogVAO);

            glBindBuffer(GL_ARRAY_BUFFER, dogVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(dogVertices), dogVertices, GL_STATIC_DRAW);



            // Vertex position
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            // Texture coordinates
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
            glUseProgram(dogShader);
            glBindVertexArray(dogVAO);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture);

            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

            glBindTexture(GL_TEXTURE_2D, 0);
            glBindVertexArray(0);


            glUseProgram(0);
        }
        //Food
        if(foodLocX != 0){
        float foodVertices[] = {
        foodLocX + 0.1f, -0.6f,
         foodLocX - 0.1f, -0.6f,

        foodLocX - 0.1f,  -0.58f,
        foodLocX + 0.1f,  -0.58f
        };

        glUseProgram(basicShader);

        colLoc = glGetUniformLocation(basicShader, "col");
        glUniform4f(colLoc, 0.0f, 0.0f, 0.0f,1.0f);


        glBindVertexArray(basicVAO);
        glBindBuffer(GL_ARRAY_BUFFER, basicVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(foodVertices), foodVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindVertexArray(0);
}
        //Eeep
        float eepVertices[] = {
            // Positions          // Texture Coords
            dogCenterX - 0.1f + eepX, dogCenterY - 0.1f + eepY + 0.2f,0.0f,   0.0f, 0.0f, // Bottom-left
            dogCenterX + 0.1f + eepX, dogCenterY - 0.1f + eepY + 0.2f, 0.0f,   1.0f, 0.0f, // Bottom-right
            dogCenterX + 0.1f + eepX,  dogCenterY + 0.1f + eepY + 0.2f, 0.0f,   1.0f, 1.0f, // Top-right
            dogCenterX - 0.1f + eepX, dogCenterY + 0.1f + eepY + 0.2f, 0.0f,   0.0f, 1.0f  // Top-left
        };
        glBindVertexArray(eepVAO);

        glBindBuffer(GL_ARRAY_BUFFER, eepVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(eepVertices), eepVertices, GL_STATIC_DRAW);



        // Vertex position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Texture coordinates
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glUseProgram(eepShader);
        glBindVertexArray(eepVAO);

        glActiveTexture(GL_TEXTURE0);
        int alpha = glGetUniformLocation(eepShader, "alpha");
        glUniform1f(alpha, eepAlpha); // Set alpha to 50%
        glBindTexture(GL_TEXTURE_2D, textureEeep);

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindVertexArray(0);


        glUseProgram(0);

        
        glDisable(GL_DEPTH_TEST);
        //Text


        glBindVertexArray(textVAO);

        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(textVertices), textVertices, GL_STATIC_DRAW);



        // Vertex position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Texture coordinates
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glUseProgram(textShader);
        glBindVertexArray(textVAO);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureText);

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindVertexArray(0);


        glUseProgram(0);

        //Light overlay
        if (seeThroughWindow == true) {

            glDisable(GL_DEPTH_TEST);

            glEnable(GL_STENCIL_TEST);
            glClearStencil(0);
            glClear(GL_STENCIL_BUFFER_BIT);
            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

            
            glUseProgram(lightShader);
            if (dark != true) {
                glUniform1f(glGetUniformLocation(overlayShader, "alpha"), 0.3);
            }
            else {
                glUniform1f(glGetUniformLocation(overlayShader, "alpha"), 0.0);
            }


            glBindVertexArray(lightVAO);
            glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(lightOverlayVertices), lightOverlayVertices, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);

            glBindVertexArray(lightVAO);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 5);
            glBindVertexArray(0);

            glEnable(GL_DEPTH_TEST);
            
        }
glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        //Darkness overlay
        glDisable(GL_DEPTH_TEST);

        glUseProgram(overlayShader);
        glUniform1f(glGetUniformLocation(overlayShader, "alpha"), overlay);

        glBindVertexArray(overlayVAO);
        glBindBuffer(GL_ARRAY_BUFFER, overlayVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(overlayVertices), overlayVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glBindVertexArray(overlayVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindVertexArray(0);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);
        
        //Moon draw
        glUseProgram(moonShader); //Izaberi nas sejder program za crtanje i koristi ga za svo naknadno crtanje (Ukoliko ne aktiviramo neke druge sejder programe)
        glBindVertexArray(moonVAO); //Izaberemo sta zelimo da crtamo
        glBindBuffer(GL_ARRAY_BUFFER, moonVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(moon), moon, GL_STATIC_DRAW);
        glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(moon) / (2 * sizeof(float))); //To i nacrtamo


        int centerMoonLocation = glGetUniformLocation(moonShader, "center");
        glUniform2fv(centerMoonLocation, 1, moonCenterS);

        

        
        //end
        glUseProgram(sunShader); //Izaberi nas sejder program za crtanje i koristi ga za svo naknadno crtanje (Ukoliko ne aktiviramo neke druge sejder programe)
        uA = glGetUniformLocation(sunShader, "uA");
        glUniform1f(uA, abs(sin(10 * (0.01 + abs(sunCenter[0]) + abs(sunCenter[1])) * glfwGetTime())));

        glBindVertexArray(0);
        glUseProgram(0);

        

        //Zamjena vidljivog bafera sa pozadinskim
        glfwSwapBuffers(window);

        //Hvatanje dogadjaja koji se ticu okvira prozora (promjena velicine, pomjeranje itd)
        glfwPollEvents();
    }

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ POSPREMANJE +++++++++++++++++++++++++++++++++++++++++++++++++


    //Brisanje bafera i sejdera
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(sunShader);
    //Sve OK - batali program
    glfwTerminate();
    return 0;
}

unsigned int compileShader(GLenum type, const char* source)
{
    //Uzima kod u fajlu na putanji "source", kompajlira ga i vraca sejder tipa "type"
    //Citanje izvornog koda iz fajla
    std::string content = "";
    std::ifstream file(source);
    std::stringstream ss;
    if (file.is_open())
    {
        ss << file.rdbuf();
        file.close();
        std::cout << "Uspjesno procitao fajl sa putanje \"" << source << "\"!" << std::endl;
    }
    else {
        ss << "";
        std::cout << "Greska pri citanju fajla sa putanje \"" << source << "\"!" << std::endl;
    }
    std::string temp = ss.str();
    const char* sourceCode = temp.c_str(); //Izvorni kod sejdera koji citamo iz fajla na putanji "source"

    int shader = glCreateShader(type); //Napravimo prazan sejder odredjenog tipa (vertex ili fragment)

    int success; //Da li je kompajliranje bilo uspjesno (1 - da)
    char infoLog[512]; //Poruka o gresci (Objasnjava sta je puklo unutar sejdera)
    glShaderSource(shader, 1, &sourceCode, NULL); //Postavi izvorni kod sejdera
    glCompileShader(shader); //Kompajliraj sejder

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success); //Provjeri da li je sejder uspjesno kompajliran
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog); //Pribavi poruku o gresci
        if (type == GL_VERTEX_SHADER)
            printf("VERTEX");
        else if (type == GL_FRAGMENT_SHADER)
            printf("FRAGMENT");
        printf(" sejder ima gresku! Greska: \n");
        printf(infoLog);
    }
    return shader;
}
unsigned int createShader(const char* vsSource, const char* fsSource)
{
    //Pravi objedinjeni sejder program koji se sastoji od Vertex sejdera ciji je kod na putanji vsSource

    unsigned int program; //Objedinjeni sejder
    unsigned int vertexShader; //Verteks sejder (za prostorne podatke)
    unsigned int fragmentShader; //Fragment sejder (za boje, teksture itd)

    program = glCreateProgram(); //Napravi prazan objedinjeni sejder program

    vertexShader = compileShader(GL_VERTEX_SHADER, vsSource); //Napravi i kompajliraj vertex sejder
    fragmentShader = compileShader(GL_FRAGMENT_SHADER, fsSource); //Napravi i kompajliraj fragment sejder

    //Zakaci verteks i fragment sejdere za objedinjeni program
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program); //Povezi ih u jedan objedinjeni sejder program
    glValidateProgram(program); //Izvrsi provjeru novopecenog programa

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_VALIDATE_STATUS, &success); //Slicno kao za sejdere
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(program, 512, NULL, infoLog);
        std::cout << "Objedinjeni sejder ima gresku! Greska: \n";
        std::cout << infoLog << std::endl;
    }

    //Posto su kodovi sejdera u objedinjenom sejderu, oni pojedinacni programi nam ne trebaju, pa ih brisemo zarad ustede na memoriji
    glDetachShader(program, vertexShader);
    glDeleteShader(vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(fragmentShader);

    return program;
}
static unsigned loadImageToTexture(const char* filePath) {
    int TextureWidth;
    int TextureHeight;
    int TextureChannels;
    unsigned char* ImageData = stbi_load(filePath, &TextureWidth, &TextureHeight, &TextureChannels, 0);
    if (ImageData != NULL)
    {
        //Slike se osnovno ucitavaju naopako pa se moraju ispraviti da budu uspravne
        stbi__vertical_flip(ImageData, TextureWidth, TextureHeight, TextureChannels);

        // Provjerava koji je format boja ucitane slike
        GLint InternalFormat = -1;
        switch (TextureChannels) {
        case 1: InternalFormat = GL_RED; break;
        case 2: InternalFormat = GL_RG; break;
        case 3: InternalFormat = GL_RGB; break;
        case 4: InternalFormat = GL_RGBA; break;
        default: InternalFormat = GL_RGB; break;
        }

        unsigned int Texture;
        glGenTextures(1, &Texture);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, TextureWidth, TextureHeight, 0, InternalFormat, GL_UNSIGNED_BYTE, ImageData);
        glBindTexture(GL_TEXTURE_2D, 0);
        // oslobadjanje memorije zauzete sa stbi_load posto vise nije potrebna
        stbi_image_free(ImageData);
        return Texture;
    }
    else
    {
        std::cout << "Textura nije ucitana! Putanja texture: " << filePath << std::endl;
        stbi_image_free(ImageData);
        return 0;
    }
}