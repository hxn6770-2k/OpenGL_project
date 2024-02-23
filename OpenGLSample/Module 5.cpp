#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>      // Image loading Utility functions

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include <learnOpengl/camera.h> // Camera class

using namespace std;  // Standard namespaace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "5-5 Milestone: Texturing Objects";  // Macro for window title


    // Variables for window width and height
    const int WINDOW_WIDTH = 600;
    const int WINDOW_HEIGHT = 600;

    // Stores the GL data relation to a given mesh
    struct GLMesh
    {
        GLuint vao; // Handle for the vertex array object
        GLuint vbos[3]; // Handles for the vertex buffer objects 
        GLuint nIndices; // Number of indices of the mesh
        //GLuint nVertices;    // Number of indices of the mesh for the table
    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;

    // Cylinder mesh data
    GLMesh gCylinderMesh;

    // Sphere mesh data
    GLMesh gSphereMesh;

    // Plan mesh data
    GLMesh gPlaneMesh;

    // Texture
    GLuint gTextureIdDesk;
    GLuint gTextureIdGlass;
    GLuint gTextureIdWood;
    glm::vec2 gUVScale(5.0f, 5.0f);
    GLint gTexWrapMode = GL_REPEAT;    

    // Shader program
    GLuint gProgramId;

    // camera
    Camera gCamera(glm::vec3(0.0f, 0.0f, 3.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // timing
    float gDeltaTime = 0.0f; // time between current frame and last frame
    float gLastFrame = 0.0f;
}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
void UResizeWindow(GLFWwindow* window, int width, int height);
bool UInitialize(int argc, char* argv[], GLFWwindow** window);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void URender();
void UCreateMesh(GLMesh& mesh, const char* type);
void UDestroyMesh(GLMesh& mesh);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);

// Vertex Shader Source Code
const GLchar* vertexShaderSource = GLSL(440,
    layout(location = 0) in vec3 position; // Vertex data from Vertex Attrib Pointer 0
    layout(location = 1) in vec2 textureCoordinate;

    out vec2 vertexTextureCoordinate;

    // Global variables for the transform matrices
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {
        gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates
        vertexTextureCoordinate = textureCoordinate;
    }
);

// Fragment Shader Source Code
const GLchar* fragmentShaderSource = GLSL(440,
    in vec2 vertexTextureCoordinate;

    out vec4 fragmentColor;

    uniform sampler2D uTextureDesk;
    uniform sampler2D uTextureGlass;
    uniform sampler2D uTextureWood;
    uniform vec2 uvScale;

    void main()
    {
        fragmentColor = texture(uTextureDesk, vertexTextureCoordinate * uvScale);
    }
);

// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
    for (int j = 0; j < height / 2; ++j)
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i)
        {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}

int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;
    
    // Create meshes
    UCreateMesh(gCylinderMesh, "cylinder"); // Calls the function to create the Vertex Buffer Object
    UCreateMesh(gSphereMesh, "sphere");     // Calls the function to create the Vertex Buffer Object
    UCreateMesh(gPlaneMesh, "plane");       // Calls the function to create the Vertex Buffer Object

    // Create the shader program
    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;
    
    // Set wireframe mode
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    // Load texture
    const char* texFilename = "../../textures/desk-texture.png";
    if (!UCreateTexture(texFilename, gTextureIdDesk))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "../../resources/textures/glass-texture.png";
    if (!UCreateTexture(texFilename, gTextureIdGlass))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "../../resources/textures/wood-texture.png";
    if (!UCreateTexture(texFilename, gTextureIdWood))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gProgramId);
    // We set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gProgramId, "uTextureDesk"), 0);
    // We set the texture as texture unit 1
    glUniform1i(glGetUniformLocation(gProgramId, "uTextureGlass"), 1);
    // We set the texture as texture unit 3
    glUniform1i(glGetUniformLocation(gProgramId, "uTextureWood"), 2);

    // Sets the background color fo the window to black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glfwSetWindowSizeCallback(gWindow, UResizeWindow);

    // Render Loop
    while (!glfwWindowShouldClose(gWindow))
    {
        // per-frame timing
        // --------------------
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;
        
        // Input
        UProcessInput(gWindow);

        // Render this fram
        URender();

        glfwSwapBuffers(gWindow);

        glfwPollEvents();
    }

    // Release mesh data
    UDestroyMesh(gCylinderMesh);
    UDestroyMesh(gSphereMesh);
    UDestroyMesh(gPlaneMesh);
    
    // Destroy texture shader
    UDestroyTexture(gTextureIdDesk);
    UDestroyTexture(gTextureIdGlass);
    UDestroyTexture(gTextureIdWood);

    // Release shader program
    UDestroyShaderProgram(gProgramId);

    exit(EXIT_SUCCESS);  // Terminates the program successfully
}

// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: Initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window create
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, nullptr, nullptr);
    if (*window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLEW: Initialize
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
    static const float cameraSpeed = 2.5f;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && gTexWrapMode != GL_REPEAT)
    {
        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_REPEAT;

        cout << "Current Texture Wrapping Mode: REPEAT" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && gTexWrapMode != GL_MIRRORED_REPEAT)
    {
        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_MIRRORED_REPEAT;

        cout << "Current Texture Wrapping Mode: MIRRORED REPEAT" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && gTexWrapMode != GL_CLAMP_TO_EDGE)
    {
        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_CLAMP_TO_EDGE;

        cout << "Current Texture Wrapping Mode: CLAMP TO EDGE" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS && gTexWrapMode != GL_CLAMP_TO_BORDER)
    {
        float color[] = { 1.0f, 0.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_CLAMP_TO_BORDER;

        cout << "Current Texture Wrapping Mode: CLAMP TO BORDER" << endl;
    }

    if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS)
    {
        gUVScale += 0.1f;
        cout << "Current scale (" << gUVScale[0] << ", " << gUVScale[1] << ")" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS)
    {
        gUVScale -= 0.1f;
        cout << "Current scale (" << gUVScale[0] << ", " << gUVScale[1] << ")" << endl;
    }
}

// GLFW: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (gFirstMouse)
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

    gLastX = xpos;
    gLastY = ypos;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    gCamera.ProcessMouseScroll(yoffset);
}

// glfw: handle mouse button events
// --------------------------------
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
        gCamera.SetMouseControl(true);
    else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
        gCamera.SetMouseControl(false);
}

// Functioned called to render a frame
void URender()
{
    // Enable z-depth
    glEnable(GL_DEPTH_TEST);

    // Clear the frame and z buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1. Scales the object by 2
    glm::mat4 scale = glm::scale(glm::vec3(2.0f, 2.0f, 2.0f));
    // 2. Rotates shape by 45 degrees in the x axis
    glm::mat4 rotation = glm::rotate(45.0f, glm::vec3(-2.0, -1.0f, -2.0f));
    // 3. Place object at the origin
    glm::mat4 translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));
    
    // Model matrix: transformations are applied right-to-left order
    glm::mat4 model = translation * rotation * scale;

    // camera/view transformation
    glm::mat4 view = gCamera.GetViewMatrix();

    // Creates a perspective projection
    glm::mat4 projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);

    // Set the shader to be used
    glUseProgram(gProgramId);
 
    // Retrieves and passes transform matrices to the Shader program
    GLint viewLoc = glGetUniformLocation(gProgramId, "view");
    GLint projLoc = glGetUniformLocation(gProgramId, "projection");
    GLint modelLoc = glGetUniformLocation(gProgramId, "model");
        
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    GLint UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // Render the Cylinder
    glm::mat4 cylinderModel = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f)) * glm::rotate(45.0f, glm::vec3(0.5, -0.5f, 0.0f)) * glm::scale(glm::vec3(2.0f, 2.0f, 1.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(cylinderModel));

    // Activate the VBOs contained with the cylinder mesh's VAO
    glBindVertexArray(gCylinderMesh.vao);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdWood);

    // Draw the cylinder
    glDrawElements(GL_TRIANGLES, gCylinderMesh.nIndices, GL_UNSIGNED_SHORT, nullptr);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

    // Render the sphere
    glm::mat4 sphereModel = cylinderModel * glm::translate(glm::vec3(0.0f, 0.75f, 0.0f)) * glm::scale(glm::vec3(1.5f, 1.5f, 1.5f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(sphereModel));

    // Activate the VBOs contained with the sphere mesh's VAO
    glBindVertexArray(gSphereMesh.vao);

    // Set texture for the sphere
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gTextureIdGlass);

    // Draws the sphere
    glDrawElements(GL_TRIANGLES, gSphereMesh.nIndices, GL_UNSIGNED_SHORT, nullptr);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);
   
    // Render the Plane (Table)
    glm::mat4 planeModel = glm::translate(glm::vec3(0.0f, -1.0f, 0.0f)) * glm::rotate(45.0f, glm::vec3(0.5, -0.5f, 0.0f)) * glm::scale(glm::vec3(5.0f, 5.0f, 5.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(planeModel));

    // Set texture for the table
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gTextureIdDesk);

    // Activate the VBOs contained within the plane mesh's VAO
    glBindVertexArray(gPlaneMesh.vao);

    // Draw the plane
    glDrawArrays(GL_TRIANGLES, 0, 6); // Assuming the plane has 6 vertices

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow); // Flips the the back buffer with the front buffer every frame.
    */
}

// Implements the UCreateMesh function
void UCreateMesh(GLMesh& mesh, const char* type)
{
    // For the cylinder
    if (type == "cylinder")
    {
        // Define parameters for the cylinder
        const int segments = 30;        // Number of segments forming the cylinder
        const float radius = 0.5f;      // Radius of the cylider
        const float height = 1.0f;      // Height of the cylinder

        // Containers to store vertex data and indices
        std::vector<GLfloat> verts; // Vertex data: x, y, z, color
        std::vector<GLushort> indices;  // Indices for drawing triangles

        // Generate vertices and indices for each segment of the cylinder
        for (int i = 0; i < segments; ++i)
        {
            // Calculate polar coodinates for the current segment
            float theta = static_cast<float>(i) / segments * glm::two_pi<float>();
            float x = radius * std::cos(theta);
            float z = radius * std::sin(theta);

            // Top vertex of the current segment
            verts.push_back(x);
            verts.push_back(height / 2.0f);
            verts.push_back(z);
            verts.push_back(0.0f); // Red color

            // Bottom vertex of the current segment
            verts.push_back(x);
            verts.push_back(-height / 2.0f);
            verts.push_back(z);
            verts.push_back(0.0f); // Red color

            // Define indices for the triangles formingthe current segment
            indices.push_back(i * 2);
            indices.push_back(i * 2 + 1);
            indices.push_back((i * 2 + 2) % (segments * 2));
            indices.push_back((i * 2 + 1) % (segments * 2));
            indices.push_back((i * 2 + 2) % (segments * 2));
            indices.push_back((i * 2 + 3) % (segments * 2));
        }

        // Specify the number of floates per vertex
        const GLuint floatsPerVertex = 4;

        glGenVertexArrays(1, &mesh.vao); // Generate multiple VAOs or buffers at the same time
        glBindVertexArray(mesh.vao);

        // Create 2 buffers: first one for the vertex data; second one for the indices
        glGenBuffers(2, mesh.vbos);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(GLfloat), verts.data(), GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

        mesh.nIndices = indices.size();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), indices.data(), GL_STATIC_DRAW);

        // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
        GLint stride = sizeof(float) * floatsPerVertex;  // The number of floats before each

        // Create Vertex Attribute Pointers
        glVertexAttribPointer(0, floatsPerVertex - 1, GL_FLOAT, GL_FALSE, stride, 0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * (floatsPerVertex - 1)));
        glEnableVertexAttribArray(1);
    }
    // For the sphere
    else if (type == "sphere")
    {
        // Define parameters for the sphere
        const int segments = 30;       // Number of vertical and horizontal segments 
        const float radius = 0.5f;     // Radius of the sphere

        // Containers to store vertex data and indices
        std::vector<GLfloat> verts;
        std::vector<GLushort> indices;

        // Generate vertices and indices for the sphere
        for (int i = 0; i <= segments; ++i)
        {
            // Calculate polar angle (theta) for the current vertical segment
            float theta = static_cast<float>(i) / segments * glm::pi<float>();

            for (int j = 0; j <= segments; ++j)
            {
                // Calculate azimuthal angle (phi) for the current horizontal segment
                float phi = static_cast<float>(j) / segments * glm::two_pi<float>();

                // Convert spherical coordinates to Cartesian coordinates
                float x = radius * std::sin(theta) * std::cos(phi);
                float y = radius * std::cos(theta);
                float z = radius * std::sin(theta) * std::sin(phi);

                // Add vertex data to the verts vector
                verts.push_back(x);
                verts.push_back(y);
                verts.push_back(z);
                verts.push_back(1.0f); // Red color

                // Define indices for the triangles forming the current segment
                if (i < segments && j < segments)
                {
                    indices.push_back((i + 1) * (segments + 1) + j);
                    indices.push_back(i * (segments + 1) + j + 1);
                    indices.push_back(i * (segments + 1) + j);

                    indices.push_back((i + 1) * (segments + 1) + j);
                    indices.push_back((i + 1) * (segments + 1) + j + 1);
                    indices.push_back(i * (segments + 1) + j + 1);
                }
            }
        }

        // Specify the number of floats per vertex
        const GLuint floatsPerVertex = 4;

        glGenVertexArrays(1, &mesh.vao); // Generate multiple VAOs or buffers at the same time
        glBindVertexArray(mesh.vao);

        // Create 2 buffers: first one for the vertex data; second one for the indices
        glGenBuffers(2, mesh.vbos);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(GLfloat), verts.data(), GL_STATIC_DRAW);

        mesh.nIndices = indices.size();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), indices.data(), GL_STATIC_DRAW);

        // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
        GLint stride = sizeof(float) * floatsPerVertex; // The number of floats before each

        // Create Vertex Attribute Pointers
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * 3));
        glEnableVertexAttribArray(1);
    }
    else if (type == "plane")
    {
        // Vertex data
        GLfloat verts[] = {
             -1.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
              1.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
              1.0f, 0.0f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
              1.0f, 0.0f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
             -1.0f, 0.0f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
             -1.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f
        };

        // Specify the number of floats per vertex
        const GLuint floatsPerVertex = 3;
        const GLuint floatsPerColor = 4;

        mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerColor));

        glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
        glBindVertexArray(mesh.vao);

        // Create VBO
        glGenBuffers(1, &mesh.vbos[0]);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

        // Strides between vertex coordinates
        GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor);

        // Create Vertex Attribute Pointers
        glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float)* floatsPerVertex));
        glEnableVertexAttribArray(1);
    }
}


void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(2, mesh.vbos);
}

/*Generate and load the texture*/
bool UCreateTexture(const char* filename, GLuint& textureId)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        flipImageVertically(image, width, height, channels);

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

        return true;
    }

    // Error loading the image
    return false;
}


void UDestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}

// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    // Create a Shader program object
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, nullptr);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, nullptr);

    // Compile the vertex shader, and print compilation errors (if any)
    glCompileShader(vertexShaderId); // Compile the vertex shader
    // Check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        return false;
    }

    glCompileShader(fragmentShaderId); // Compile the fragment shader

    // Check gor shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), nullptr, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        return false;
    }

    // Attached compiled shders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    // Linksthe shader program
    glLinkProgram(programId);
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), nullptr, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        return false;
    }

    // Use the shader program
    glUseProgram(programId);

    return true;
}

void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}
