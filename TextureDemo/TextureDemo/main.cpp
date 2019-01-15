#include <iostream>
#include <stdexcept>
#include <string>
#define GLEW_STATIC
#include <GL/glew.h> // window management library
#include <GL/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> //
#include <SOIL/SOIL.h> // read image file

// Macro for printing exceptions
#define PrintException(exception_object)\
	std::cerr << exception_object.what() << std::endl

// Globals that define the OpenGL window and viewport
const std::string window_title_g = "Single Sprite Demo";
const unsigned int window_width_g = 800;
const unsigned int window_height_g = 600;
const glm::vec3 viewport_background_color_g(0.0, 0.6, 0.6);


// Source code of vertex shader
const char *source_vp = "#version 130\n\
\n\
// Vertex buffer\n\
in vec2 vertex;\n\
in vec3 color;\n\
in vec2 uv;\n\
out vec2 uv_interp;\n\
\n\
// Uniform (global) buffer\n\
uniform float time;\n\
uniform vec2 x ;\n\
//uniform float scale ;\n\
// nothing here for now\n\
// Attributes forwarded to the fragment shader\n\
out vec4 color_interp;\n\
\n\
\n\
void main()\n\
{\n\
   \n\
    gl_Position =  vec4(vertex + x , 0.0, 1.0);\n\
    color_interp = vec4(color, 1.0);\n\
	uv_interp = uv ;\n\
 }";


// Source code of fragment shader
const char *source_fp = "\n\
#version 130\n\
\n\
// Attributes passed from the vertex shader\n\
in vec4 color_interp;\n\
in vec2 uv_interp;\n\
\n\
uniform sampler2D texBird;\n\
uniform float time;\n\
\n\
\n\
void main()\n\
{\n\
	vec4 color = texture2D(texBird, uv_interp) ;\n\
	gl_FragColor = vec4(color.r,color.g,color.b,color.a);\n\
    if((gl_FragColor.r + gl_FragColor.g + gl_FragColor.b)/3 > 0.9)\n\
	{\n\
		//discard; \n\
		gl_FragColor = vec4(color.r,color.g,color.b,0);\n\
	} \n\
 //gl_FragColor = color_interp;\n\
}";


// Callback for when a key is pressed
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){

    // Quit the program when pressing 'q'
    if (key == GLFW_KEY_Q && action == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }
}


// Callback for when the window is resized
void ResizeCallback(GLFWwindow* window, int width, int height){

    // Set OpenGL viewport based on framebuffer width and height
    glViewport(0, 0, width, height);

}

// Create the geometry for a square (with two triangles)
// Return the number of array elements that form the square
int CreateSquare(void) {

	// The face of the square is defined by four vertices and two triangles

	// Number of attributes for vertices and faces
//	const int vertex_att = 7;  // 7 attributes per vertex: 2D (or 3D) position (2), RGB color (3), 2D texture coordinates (2)
//	const int face_att = 3; // Vertex indices (3)

	GLfloat vertex[]  = {
		//  square (two triangles)
		   //  Position      Color             Texcoords
		-0.5f, 0.5f,	 1.0f, 0.0f, 0.0f,		0.0f, 0.0f, // Top-left
		0.5f, 0.5f,		 1.0f, 1.0f, 0.0f,		1.0f, 0.0f, // Top-right
		0.5f, -0.5f,	 1.0f, 0.0f, 1.0f,		1.0f, 1.0f, // Bottom-right
		-0.5f, -0.5f,	 0.0f, 0.0f, 1.0f,		0.0f, 1.0f  // Bottom-left
	};


	GLuint face[] = {
		0, 1, 2, // t1
		2, 3, 0  //t2
	};

	GLuint vbo, ebo;

	// Create buffer for vertices
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);

	// Create buffer for faces (index buffer)
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(face), face, GL_STATIC_DRAW);

	// Return number of elements in array buffer
	return sizeof(face);

}


// Main function that builds and runs the game
int main(void){

    try {
        // Initialize the window management library (GLFW)
        if (!glfwInit()){
            throw(std::runtime_error(std::string("Could not initialize the GLFW library")));
        }

        // Create a window and its OpenGL context
        GLFWwindow* window;
        window = glfwCreateWindow(window_width_g, window_height_g, window_title_g.c_str(), NULL, NULL);
        if (!window){
            glfwTerminate();
            throw(std::runtime_error(std::string("Could not create window")));
        }

        /* Make the window's OpenGL context the current one */
        glfwMakeContextCurrent(window);

        // Initialize the GLEW library to access OpenGL extensions
        // Need to do it after initializing an OpenGL context
        glewExperimental = GL_TRUE;
        GLenum err = glewInit();
        if (err != GLEW_OK){
            throw(std::runtime_error(std::string("Could not initialize the GLEW library: ")+std::string((const char *) glewGetErrorString(err))));
        }

        // Set up z-buffer for rendering
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

		// Enable Alpha blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Create geometry of the square
		int size = CreateSquare();

        // Set up shaders

        // Create a shader from vertex program source code
        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &source_vp, NULL);
        glCompileShader(vs);

        // Check if shader compiled successfully
        GLint status;
        glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
        if (status != GL_TRUE){
            char buffer[512];
            glGetShaderInfoLog(vs, 512, NULL, buffer);
            throw(std::ios_base::failure(std::string("Error compiling vertex shader: ")+std::string(buffer)));
        }

        // Create a shader from the fragment program source code
        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &source_fp, NULL);
        glCompileShader(fs);

        // Check if shader compiled successfully
        glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
        if (status != GL_TRUE){
            char buffer[512];
            glGetShaderInfoLog(fs, 512, NULL, buffer);
            throw(std::ios_base::failure(std::string("Error compiling fragment shader: ")+std::string(buffer)));
        }

        // Create a shader program linking both vertex and fragment shaders
        // together
        GLuint program = glCreateProgram();
        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glLinkProgram(program);

        // Check if shaders were linked successfully
        glGetProgramiv(program, GL_LINK_STATUS, &status);
        if (status != GL_TRUE){
            char buffer[512];
            glGetShaderInfoLog(program, 512, NULL, buffer);
            throw(std::ios_base::failure(std::string("Error linking shaders: ")+std::string(buffer)));
        }

        // Delete memory used by shaders, since they were already compiled
        // and linked
        glDeleteShader(vs);
        glDeleteShader(fs);

        // Set attributes for shaders
        // Should be consistent with how we created the buffers for the square
        GLint vertex_att = glGetAttribLocation(program, "vertex");
        glVertexAttribPointer(vertex_att, 2, GL_FLOAT, GL_FALSE, 7*sizeof(GLfloat), 0);
        glEnableVertexAttribArray(vertex_att);

        GLint color_att = glGetAttribLocation(program, "color");
        glVertexAttribPointer(color_att, 3, GL_FLOAT, GL_FALSE, 7*sizeof(GLfloat), (void *) (2 *sizeof(GLfloat)));
        glEnableVertexAttribArray(color_att);

        GLint tex_att = glGetAttribLocation(program, "uv");
        glVertexAttribPointer(tex_att, 2, GL_FLOAT, GL_FALSE, 7*sizeof(GLfloat), (void *) (5 *sizeof(GLfloat)));
        glEnableVertexAttribArray(tex_att);

		// Load texture
		GLuint tex;
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);

		int width, height;
		unsigned char* image = SOIL_load_image("parrot.png", &width, &height, 0, SOIL_LOAD_RGBA);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		SOIL_free_image_data(image);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// set time uniform.
		// first, get location:
		GLint timeloc = glGetUniformLocation(program, "time");
		GLint xLoc = glGetUniformLocation(program, "x");
		//now, you can set it:

		glUniform1f(timeloc, 0.1);
		glUniform2f(xLoc, 0.2f, 0.2f);

        // Set event callbacks
        glfwSetKeyCallback(window, KeyCallback);
        glfwSetFramebufferSizeCallback(window, ResizeCallback);

        // Run the main loop
        bool animating = 1;
		float simpletime = 0.0;
        while (!glfwWindowShouldClose(window)){
            // Clear background
            glClearColor(viewport_background_color_g[0], 
                         viewport_background_color_g[1],
                         viewport_background_color_g[2], 0.0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// update "simple time"
			simpletime += 0.0001;
			if (simpletime > 1.0) simpletime = 0.0; // reset

            // Draw the square

            // Select proper shader program to use
            glUseProgram(program);
			glUniform1f(timeloc, simpletime);
			GLfloat realTime = glfwGetTime();
			GLfloat vx = 0.8*simpletime;
			GLfloat vy = 0.8*simpletime;
			glUniform2f(xLoc, vx + 0.1 * sin(realTime), vy + 0.1 * cos(realTime));



			// Draw 
			glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, 0);


			glDrawArrays(GL_TRIANGLES, 0, 6); // if glDrawArrays is used, glDrawElements will be ignored 

            // Update other events like input handling
            glfwPollEvents();

            // Push buffer drawn in the background onto the display
            glfwSwapBuffers(window);
        }
    }
    catch (std::exception &e){
        PrintException(e);
    }

    return 0;
}


 