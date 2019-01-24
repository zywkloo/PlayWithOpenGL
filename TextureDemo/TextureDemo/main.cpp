//Yiwei Zhang 101071022
//	Basic: add the uniform value "offset" to let the obj move
//         collision detectoin between obj and walls
//   	   colour changing respectively
//	Bonus:press S to stop all move  , press d to continue
//=======================================
//All changes are between those quation lines like this
//=======================================

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

//============================================================================ 
const GLint maxObjs = 128;
//def a struct named gameObj ,including attributes listed below : vec2 position, vec2 velocity, rgba
typedef  struct {
	GLfloat x;  //x coordinate
	GLfloat y;	//y coordinate
	GLfloat dx;	//velocity x coordinate
	GLfloat dy;	//velocity y coordinate 
	GLfloat r;  //rgb
	GLfloat g;
	GLfloat b; 
	GLfloat a;  //alpha value
} gameObj;

gameObj objArr[maxObjs];  //creat a gameObj array
GLint objArrSize=0;   //the initial size of the array is 0;

GLboolean allMoveStop = false;
//================================================================================

// Source code of vertex shader
const char *source_vp = "#version 130\n\
\n\
// Vertex buffer\n\
in vec2 vertex;\n\
in vec3 color;\n\
in vec2 uv;\n\
out vec2 uv_interp;\n\
// Uniform (global) buffer\n\
uniform float time;\n\
//                      \n\
//set a new uniform value offset\n\
//===================================================================;\n\
//                      \n\
uniform vec2 offset ;\n\
//                      \n\
//===================================================================;\n\
// Attributes forwarded to the fragment shader\n\
out vec4 color_interp;\n\
\n\
\n\
void main()\n\
{\n\
   \n\
    gl_Position =  vec4(vertex + offset, 0.0, 1.0);\n\
	color_interp =  vec4(color,1.0);\n\
	//color_interp =  colorIn;\n\
    //color_interp =  mix(vec4(color,1.0),colorIn,0.3);\n\
	uv_interp = uv ;\n\
 }";


// Source code of fragment shader
const char *source_fp = "\n\
#version 130\n\
\n\
// Attributes passed from the vertex shader\n\
in vec4 color_interp;\n\
in vec2 uv_interp;\n\
uniform sampler2D texBird;\n\
uniform float time;\n\
//                      \n\
//set a new uniform vec4 colorIn\n\
//===================================================================;\n\
//                      \n\
uniform vec4 colorIn;\n\
//                      \n\
//===================================================================;\n\
\n\
\n\
void main()\n\
{\n\
	vec4 color = texture2D(texBird, 2*uv_interp*(time+0.5));\n\
	vec4 side = colorIn;\n\
	//gl_FragColor =  color;\n\
	gl_FragColor =  mix(color,colorIn,0.3);\n\
    if(((gl_FragColor.r + gl_FragColor.g + gl_FragColor.b)/3 > 0.7) ||\n\
	(uv_interp.t > 0.95) || (uv_interp.t < 0.05))\n\
	{\n\
		discard; \n\
		//gl_FragColor = vec4(color.r,color.g,color.b,0);\n\
	} \n\
 //gl_FragColor = color_interp;\n\
}";


// Callback for when a key is pressed
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){

	//===================================================
	// press S to stop all move  , press d to continue
	if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		allMoveStop = true;
	}
	if (key == GLFW_KEY_D && action == GLFW_PRESS) {
		allMoveStop = false;
	}
	//===================================================
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
		-0.2f, 0.2f,	 1.0f, 0.0f, 0.0f,		0.0f, 0.0f, // Top-left
		0.2f, 0.2f,		 1.0f, 1.0f, 0.0f,		1.0f, 0.0f, // Top-right
		0.2f, -0.2f,	 1.0f, 0.0f, 1.0f,		1.0f, 1.0f, // Bottom-right
		-0.2f, -0.2f,	 0.0f, 0.0f, 1.0f,		0.0f, 1.0f  // Bottom-left
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
		unsigned char* image = SOIL_load_image("Illidan.png", &width, &height, 0, SOIL_LOAD_RGBA);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		SOIL_free_image_data(image);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// set time uniform.
		// first, get location:


		GLint timeloc = glGetUniformLocation(program, "time");
		GLint offsetLoc = glGetUniformLocation(program, "offset");  //get the coordinates relative to the original coordinates 
		GLint colorLoc = glGetUniformLocation(program, "colorIn");




		//now, you can set it:
		glUniform1f(timeloc, 0.1f);
		glUniform2f(offsetLoc, 0.2f, 0.2f);



        // Set event callbacks
        glfwSetKeyCallback(window, KeyCallback);
        glfwSetFramebufferSizeCallback(window, ResizeCallback);

		//CHANGES: struct init
		//========================================================================================
		for (int i = 0; i < 5; ++i) {
			GLfloat x = GLfloat(0.5)*GLfloat(rand()) / GLfloat(RAND_MAX);
			GLfloat y = GLfloat(0.5)*GLfloat(rand()) / GLfloat(RAND_MAX);
			GLfloat dx = GLfloat(0.008)*GLfloat(rand()) / GLfloat(RAND_MAX);
			GLfloat dy = GLfloat(0.008)*GLfloat(rand()) / GLfloat(RAND_MAX);
			GLfloat r = GLfloat(rand()) / GLfloat(RAND_MAX);
			GLfloat g = GLfloat(rand()) / GLfloat(RAND_MAX);
			GLfloat b = GLfloat(i) / 5.0f;
			GLfloat a = 1;
			objArr[i] = { x,y,dx,dy,r,g,b,a };
			printf("x,y: %f,%f  dx,dy:%f,%f\n", x, y, dx, dy);
			objArrSize++;
		}
		//=========================================================================================


        // Run the main loop
        bool animating = 1;
		GLfloat simpletime = 0.0;
        while (!glfwWindowShouldClose(window)){
            // Clear background
            glClearColor(viewport_background_color_g[0], 
                         viewport_background_color_g[1],
                         viewport_background_color_g[2], 0.0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// update "simple time"
			simpletime += GLfloat(0.0001);
			if (simpletime > 1.0) simpletime = 0.0; // reset

            // Draw the square

            // Select proper shader program to use
            glUseProgram(program);
			glUniform1f(timeloc, simpletime);
			GLfloat realTime = glfwGetTime();
	
			
			for (int j = 0; j < objArrSize; ++j) {
				if (allMoveStop == false) {
					objArr[j].x += objArr[j].dx;
					objArr[j].y += objArr[j].dy;
					if (objArr[j].x - 0.2 < -1 || objArr[j].x + 0.2 > 1) {
						objArr[j].dx = -objArr[j].dx;
					}
					else if (objArr[j].y - 0.2 < -1 || objArr[j].y + 0.2 > 1) {
						objArr[j].dy = -objArr[j].dy;
					}
					printf("/n %d /n  x,y: %f,%f  dx,dy:%f,%f\n", j, objArr[j].x, objArr[j].y, objArr[j].dx, objArr[j].dy);
				}
				glUniform2f(offsetLoc, GLfloat(objArr[j].x), GLfloat(objArr[j].y));
				glUniform4f(colorLoc, objArr[j].r, objArr[j].g, objArr[j].b, 1);
				glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, 0);
			}
			
			
			//glDrawArrays(GL_TRIANGLES, 0, 6); // if glDrawArrays is used, glDrawElements will be ignored 

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


 