#ifndef TEXTRENDERER_H
#define TEXTRENDERER_H

#include <unordered_map>

#include "glad.h"
#include <glm/glm.hpp>

#include "Texture.h"
#include "Shader.h"
#include "ResourceManager.h"

struct Character
{
    unsigned int TextureID;
    glm::ivec2 Size;
    glm::ivec2 Bearing;
    long int Advance;
};

class TextRenderer
{
    public:
        // characters
        std::unordered_map<char, Character> Characters;
        Shader TextShader;

        TextRenderer(unsigned int width, unsigned int height);
        void Load(std::string font, unsigned int fontSize);
        void RenderText(std::string text, float x, float y, float scale, glm::vec4 color = glm::vec4(1.0f));

    private:
        unsigned int VAO, VBO;
};

#endif // TEXTRENDERER_H
