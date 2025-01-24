#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <vector>


const char* vertexGridShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

const char* fragmentGridShaderSource = R"(
#version 330 core
out vec4 FragColor;

void main() {
    FragColor = vec4(0, 0, 0.2, 1.0); // Dark blue color
}
)";


const char* fragmentFieldShaderSource = R"(
#version 330 core

in vec2 TexCoord;

out vec4 color;

uniform sampler2D texture1;

vec4 computeColor(float normal_value)
{
    vec3 color;
    if(normal_value<0.0) normal_value = 0.0;
    if(normal_value>1.0) normal_value = 1.0;
    float v1 = 1.0/7.0;
    float v2 = 2.0/7.0;
    float v3 = 3.0/7.0;
    float v4 = 4.0/7.0;
    float v5 = 5.0/7.0;
    float v6 = 6.0/7.0;
    //compute color
    if(normal_value<v1)
    {
      float c = normal_value/v1;
      color.x = 70.*(1.-c);
      color.y = 70.*(1.-c);
      color.z = 219.*(1.-c) + 91.*c;
    }
    else if(normal_value<v2)
    {
      float c = (normal_value-v1)/(v2-v1);
      color.x = 0.;
      color.y = 255.*c;
      color.z = 91.*(1.-c) + 255.*c;
    }
    else if(normal_value<v3)
    {
      float c = (normal_value-v2)/(v3-v2);
      color.x =  0.*c;
      color.y = 255.*(1.-c) + 128.*c;
      color.z = 255.*(1.-c) + 0.*c;
    }
    else if(normal_value<v4)
    {
      float c = (normal_value-v3)/(v4-v3);
      color.x = 255.*c;
      color.y = 128.*(1.-c) + 255.*c;
      color.z = 0.;
    }
    else if(normal_value<v5)
    {
      float c = (normal_value-v4)/(v5-v4);
      color.x = 255.*(1.-c) + 255.*c;
      color.y = 255.*(1.-c) + 96.*c;
      color.z = 0.;
    }
    else if(normal_value<v6)
    {
      float c = (normal_value-v5)/(v6-v5);
      color.x = 255.*(1.-c) + 107.*c;
      color.y = 96.*(1.-c);
      color.z = 0.;
    }
    else
    {
      float c = (normal_value-v6)/(1.-v6);
      color.x = 107.*(1.-c) + 223.*c;
      color.y = 77.*c;
      color.z = 77.*c;
    }
    return vec4(color.r/255.0,color.g/255.0,color.b/255.0,1.0);
}

void main() {
    float value = texture(texture1, TexCoord).r; // Access red channel for GL_R32F.
    color = computeColor(value); // Set color to red (RGBA)
}
)";

const char* vertexFieldShaderSource = R"(
#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texCoord;
out vec2 TexCoord;

void main() {
	gl_Position = position;
	TexCoord = texCoord;
}
)";


static unsigned int compileShader(unsigned int type, const char* src) {
    unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)_malloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        std::cout << "Failed to compile" << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);
        return 0;
    }

    return id;
};

static unsigned int createShader(const char* vertexShader, const char* fragmentShader) {
    unsigned int program = glCreateProgram();

    unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteProgram(vs);
    glDeleteProgram(fs);

    return program;
}

static std::string parseFile(const std::string filePath) {
    std::ifstream file(filePath);
    std::string str;
    std::string content;
    while (std::getline(file, str)) {
        content.append(str + "\n");
    }
    return content;
}

static unsigned int loadShader(const std::string& vs_filePath, const std::string& fs_filePath) {
    std::string vs_source = parseFile(vs_filePath);
    std::string fs_source = parseFile(fs_filePath);

    return createShader(vs_source.c_str(), fs_source.c_str());
}


static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height); // Update the viewport
}


class Renderer {
public:
    Renderer(unsigned int data_width, unsigned int data_height, std::vector<float*> data_table, std::vector<float> lines): m_width(data_width), m_height(data_height), m_data_table(data_table), m_gridVertices(lines) {
        if (m_data_table.size() > 8) {
            throw std::runtime_error("Can only accomodate up to 8 different fields.");
        }
        if (m_data_table.size() == 0) {
            throw std::runtime_error("Data table must have at least one entry.");
        }

        /* Initialize the library */
        if (!glfwInit())
            return; // TODO ERROR HANDLING

        /* Create a windowed mode window and its OpenGL context */
        m_window = glfwCreateWindow(1920, 1080, "2d Live Fields Renderer", NULL, NULL);
        if (!m_window)
        {
            glfwTerminate();
            return; // TODO ERROR HANDLING
        }

        /* Make the window's context current */
        glfwMakeContextCurrent(m_window);

        if (glewInit() != GLEW_OK)
            std::cout << "Couldn't init GLEW" << std::endl;

        std::cout << glGetString(GL_VERSION) << std::endl;

        glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);

        // Load shaders
        m_field_shader_program = createShader(vertexFieldShaderSource, fragmentFieldShaderSource);
        m_grid_shader_program = createShader(vertexGridShaderSource, fragmentGridShaderSource);

        float vertexData[16] = {
            -1.0f, -1.0f, 0.0f, 0.0f,
             1.0f, -1.0f, 1.0f, 0.0f,
             1.0f,  1.0f, 1.0f, 1.0f,
             -1.0f, 1.0f, 0.0f, 1.0f,
        };

        unsigned int indices[6] = {
            0, 1, 2,
            2, 3, 0
        };

        // Vertex Buffer
        GLuint buffer;
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(float), vertexData, GL_STATIC_DRAW);

        // Index Buffer
        GLuint indexBuffer;
        glGenBuffers(1, &indexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW);

        // Position Vertex Attribute
        unsigned int vStride = 4 * sizeof(float);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, vStride, (void*)0);
        glEnableVertexAttribArray(0);

        // Texture Vertex Attribute
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, vStride, (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // Texture Setup
        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, m_width, m_height, 0, GL_RED, GL_FLOAT, NULL);


        // Grid
        GLuint gridVBO;

        glGenVertexArrays(1, &m_gridVAO);
        glGenBuffers(1, &gridVBO);

        glBindVertexArray(m_gridVAO);

        glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
        glBufferData(GL_ARRAY_BUFFER, m_gridVertices.size() * sizeof(float), m_gridVertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
        
        // ImGUI
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.FontGlobalScale = 3.0f;

        ImGui_ImplGlfw_InitForOpenGL(m_window, true);
        ImGui_ImplOpenGL3_Init("#version 130");

        ImGui::StyleColorsDark();
    }

    void update() {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RED, GL_FLOAT, m_data_table[m_active_data_idx]);

    }

    void draw() {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        // Draw Field
        glUseProgram(m_field_shader_program);
        glDrawElements(
            GL_TRIANGLES,      // mode
            6,    // count
            GL_UNSIGNED_INT,   // type
           (void*)0           // element array buffer offset
        );

        // Draw Grid on top
        glUseProgram(m_grid_shader_program);
        glBindVertexArray(m_gridVAO);
        glDrawArrays(GL_LINES, 0, m_gridVertices.size() / 2);
        glBindVertexArray(0);

        {
            ImGui::Begin("Live 2D Field Renderer");                          // Create a window called "Hello, world!" and append into it.
            for (int i = 0; i < m_data_table.size(); i++) {
                std::string buttonLabel = "Texture " + std::to_string(i + 1);
                if (ImGui::Button(buttonLabel.c_str())) {
                    m_active_data_idx = i; // Update the selected texture
                }
            }
            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        /* Swap front and back buffers */
        glfwSwapBuffers(m_window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    bool alive() {
        return !glfwWindowShouldClose(m_window);
    }

    void close() {
        glfwTerminate();
    }

private:
    unsigned int m_width;
    unsigned int m_height;
    unsigned int m_field_shader_program;
    unsigned int m_grid_shader_program;

    unsigned int m_gridVAO;
    std::vector<float> m_gridVertices;
    std::vector<float*> m_data_table;
    std::vector<GLuint> m_textureIDs;
    unsigned int m_active_data_idx = 0;
    GLFWwindow* m_window = nullptr;

};