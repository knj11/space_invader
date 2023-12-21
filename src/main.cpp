#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <iostream>

#define GL_ERROR_CASE(glerror)\
    case glerror: snprintf(error, sizeof(error), "%s", #glerror)

inline void gl_debug(const char *file, int line) {
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR){
        char error[128];

        switch(err) {
            GL_ERROR_CASE(GL_INVALID_ENUM); break;
            GL_ERROR_CASE(GL_INVALID_VALUE); break;
            GL_ERROR_CASE(GL_INVALID_OPERATION); break;
            GL_ERROR_CASE(GL_INVALID_FRAMEBUFFER_OPERATION); break;
            GL_ERROR_CASE(GL_OUT_OF_MEMORY); break;
            default: snprintf(error, sizeof(error), "%s", "UNKNOWN_ERROR"); break;
        }

        fprintf(stderr, "%s - %s: %d\n", error, file, line);
    }
}

#undef GL_ERROR_CASE

void validate_shader(GLuint shader, const char *file = 0){
    static const unsigned int BUFFER_SIZE = 512;
    char buffer[BUFFER_SIZE];
    GLsizei length = 0;

    glGetShaderInfoLog(shader, BUFFER_SIZE, &length, buffer);

    if(length>0){
        printf("Shader %d(%s) compile error: %s\n", shader, (file? file: ""), buffer);
    }
}

bool validate_program(GLuint program){
    static const GLsizei BUFFER_SIZE = 512;
    GLchar buffer[BUFFER_SIZE];
    GLsizei length = 0;

    glGetProgramInfoLog(program, BUFFER_SIZE, &length, buffer);

    if(length>0){
        printf("Program %d link error: %s\n", program, buffer);
        return false;
    }

    return true;
}

struct Buffer
{
    size_t width, height;
    uint32_t* data;
};

struct Sprite
{
    size_t width, height;
    uint8_t* data;
};

struct Alien
{
    size_t x, y;
    uint8_t type;
};

struct Player
{
    size_t x, y;
    size_t life;
};

struct Game
{
    size_t width, height;
    size_t num_aliens;
    Alien* aliens;
    Player player;
};

struct SpriteAnimation
{
    bool loop;
    size_t num_frames;
    size_t frame_duration;
    size_t time;
    Sprite** frames;
};

void buffer_clear(Buffer* buffer, uint32_t color)
{
    for(size_t i = 0; i < buffer->width * buffer->height; ++i)
    {
        buffer->data[i] = color;
    }
}

void buffer_draw_sprite(Buffer* buffer, const Sprite& sprite, size_t x, size_t y, uint32_t color)
{
    for(size_t xi = 0; xi < sprite.width; ++xi)
    {
        for(size_t yi = 0; yi < sprite.height; ++yi)
        {
            if(sprite.data[yi * sprite.width + xi] &&
               (sprite.height - 1 + y - yi) < buffer->height &&
               (x + xi) < buffer->width)
            {
                buffer->data[(sprite.height - 1 + y - yi) * buffer->width + (x + xi)] = color;
            }
        }
    }
}

uint32_t rgb_to_uint32(uint8_t r, uint8_t g, uint8_t b)
{
    return (r << 24) | (g << 16) | (b << 8) | 255;
}
// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

int main()
{
        const size_t buffer_width = 224;
        const size_t buffer_height = 256;
        
        glfwSetErrorCallback(error_callback);

        if (!glfwInit()) return -1;
        
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

        GLFWwindow* window = glfwCreateWindow(buffer_width, buffer_height, "Space Invaders", NULL, NULL);
        if (window == NULL)
        {
                std::cout << "Failed to create GLFW window" << std::endl;
                glfwTerminate();
                return -1;
        }
        glfwMakeContextCurrent(window);

        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

        // glad: load all OpenGL function pointers
        // ---------------------------------------
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
                std::cout << "Failed to initialize GLAD" << std::endl;
                glfwTerminate();
                return -1;
        }
        int glVersion[2] = {-1,1};
        glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]);
        glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]);

        gl_debug(__FILE__, __LINE__);

        printf("Using OpenGL: %d.%d\n", glVersion[0], glVersion[1]);
        printf("Renderer used: %s\n", glGetString(GL_RENDERER));
        printf("Shading Language: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

        // args: red, green, blue, alpha
        glClearColor(1.0, 0.0, 0.0, 1.0);

        // Create graphics buffer
        Buffer buffer;
        buffer.width  = buffer_width;
        buffer.height = buffer_height;
        buffer.data   = new uint32_t[buffer.width * buffer.height];

        buffer_clear(&buffer, 0);

        // Create texture for presenting buffer to OpenGL
        GLuint buffer_texture;
        /** buffer_texture will store the names/ints of the generated
         *  textures in opengl
         */
        glGenTextures(1, &buffer_texture);
        /* The generated number is then associated with a 2d texture */
        glBindTexture(GL_TEXTURE_2D, buffer_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, buffer.width, buffer.height, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, buffer.data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Create vao for generating fullscreen triangle
        GLuint fullscreen_triangle_vao;
        glGenVertexArrays(1, &fullscreen_triangle_vao);

        // Create shader for displaying buffer
        static const char* fragment_shader =
                "\n"
                "#version 330\n"
                "\n"
                "uniform sampler2D buffer;\n"
                "noperspective in vec2 TexCoord;\n"
                "\n"
                "out vec3 outColor;\n"
                "\n"
                "void main(void){\n"
                "    outColor = texture(buffer, TexCoord).rgb;\n"
                "}\n";

        static const char* vertex_shader =
                "\n"
                "#version 330\n"
                "\n"
                "noperspective out vec2 TexCoord;\n"
                "\n"
                "void main(void){\n"
                "\n"
                "    TexCoord.x = (gl_VertexID == 2)? 2.0: 0.0;\n"
                "    TexCoord.y = (gl_VertexID == 1)? 2.0: 0.0;\n"
                "    \n"
                "    gl_Position = vec4(2.0 * TexCoord - 1.0, 0.0, 1.0);\n"
                "}\n";

        GLuint shader_id = glCreateProgram();

        {
            //Create vertex shader
            GLuint shader_vp = glCreateShader(GL_VERTEX_SHADER);

            glShaderSource(shader_vp, 1, &vertex_shader, 0);
            glCompileShader(shader_vp);
            validate_shader(shader_vp, vertex_shader);
            glAttachShader(shader_id, shader_vp);

            glDeleteShader(shader_vp);
        }

        {
            //Create fragment shader
            GLuint shader_fp = glCreateShader(GL_FRAGMENT_SHADER);

            glShaderSource(shader_fp, 1, &fragment_shader, 0);
            glCompileShader(shader_fp);
            validate_shader(shader_fp, fragment_shader);
            glAttachShader(shader_id, shader_fp);

            glDeleteShader(shader_fp);
        }

        glLinkProgram(shader_id);

        if(!validate_program(shader_id)){
            fprintf(stderr, "Error while validating shader.\n");
            glfwTerminate();
            glDeleteVertexArrays(1, &fullscreen_triangle_vao);
            delete[] buffer.data;
            return -1;
        }

        glUseProgram(shader_id);

        /* attach texture to uniform sampler2D var in the fragment shader */
        GLint location = glGetUniformLocation(shader_id, "buffer");
        glUniform1i(location, 0);

        //OpenGL setup
        glDisable(GL_DEPTH_TEST);
        glActiveTexture(GL_TEXTURE0);

        glBindVertexArray(fullscreen_triangle_vao);

        // Prepare game
        Sprite alien_sprite0;
        alien_sprite0.width = 11;
        alien_sprite0.height = 8;
        alien_sprite0.data = new uint8_t[88]
        {
            0,0,1,0,0,0,0,0,1,0,0, // ..@.....@..
            0,0,0,1,0,0,0,1,0,0,0, // ...@...@...
            0,0,1,1,1,1,1,1,1,0,0, // ..@@@@@@@..
            0,1,1,0,1,1,1,0,1,1,0, // .@@.@@@.@@.
            1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
            1,0,1,1,1,1,1,1,1,0,1, // @.@@@@@@@.@
            1,0,1,0,0,0,0,0,1,0,1, // @.@.....@.@
            0,0,0,1,1,0,1,1,0,0,0  // ...@@.@@...
        };
        
        Sprite alien_sprite1;
        alien_sprite1.width = 11;
        alien_sprite1.height = 8;
        alien_sprite1.data = new uint8_t[88]
        {
                0,0,1,0,0,0,0,0,1,0,0, // ..@.....@..
                1,0,0,1,0,0,0,1,0,0,1, // @..@...@..@
                1,0,1,1,1,1,1,1,1,0,1, // @.@@@@@@@.@
                1,1,1,0,1,1,1,0,1,1,1, // @@@.@@@.@@@
                1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
                0,1,1,1,1,1,1,1,1,1,0, // .@@@@@@@@@.
                0,0,1,0,0,0,0,0,1,0,0, // ..@.....@..
                0,1,0,0,0,0,0,0,0,1,0  // .@.......@.
        };

        Sprite player_sprite;
        player_sprite.width = 11;
        player_sprite.height = 7;
        player_sprite.data = new uint8_t[77]
        {
                0,0,0,0,0,1,0,0,0,0,0, // .....@.....
                0,0,0,0,1,1,1,0,0,0,0, // ....@@@....
                0,0,0,0,1,1,1,0,0,0,0, // ....@@@....
                0,1,1,1,1,1,1,1,1,1,0, // .@@@@@@@@@.
                1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
                1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
                1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
        };

        uint32_t clear_color = rgb_to_uint32(0, 128, 0);
        // render loop
        // -----------
        while (!glfwWindowShouldClose(window))
        {
                buffer_clear(&buffer, clear_color);

                buffer_draw_sprite(&buffer, alien_sprite0, 112, 128, rgb_to_uint32(128, 0, 0));

                glTexSubImage2D(
                    GL_TEXTURE_2D, 0, 0, 0,
                    buffer.width, buffer.height,
                    GL_RGBA, GL_UNSIGNED_INT_8_8_8_8,
                    buffer.data
                );
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

                // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
                // -------------------------------------------------------------------------------
                glfwSwapBuffers(window);
                glfwPollEvents();
        }

        // glfw: terminate, clearing all previously allocated GLFW resources.
        // ------------------------------------------------------------------
        glfwDestroyWindow(window);
        glfwTerminate();
    
        glDeleteVertexArrays(1, &fullscreen_triangle_vao);

        delete[] alien_sprite0.data;
        delete[] buffer.data;

        return 0;
}