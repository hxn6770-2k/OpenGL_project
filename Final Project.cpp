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

#include <learnOpengl/camera.h> // Camera class

using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "Module 7: Final Project"; // Macro for window title

    // Variables for window width and height
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    // Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint vao;         // Handle for the vertex array object
        GLuint vbo;         // Handle for the vertex buffer object
        GLuint nVertices;    // Number of indices of the mesh
    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;

    // Plane mesh data
    GLMesh gMesh;
    // Cylinder mesh data
    GLMesh gCylinderMesh;
    // Sphere mesh data
    GLMesh gSphereMesh;
    // Cube mesh data
    GLMesh gCubeMesh;
    // Prism mesh data
    GLMesh gPrismMesh;
    // Torus mesh data
    GLMesh gTorusMesh;
    // Cup mesh data
    GLMesh gCupMesh;

    // Texture
    GLuint gCylinderTextureId;
    GLuint gSphereTextureId;
    GLuint gPlaneTextureId;
    GLuint gCylinderTextureId2;
    GLuint gTorusTextureId;
    GLuint gCubeTextureId;
    GLuint gPrismTextureId;
    GLuint gCupTextureId;

    glm::vec2 gUVScale(1.0f, 1.0f);
    GLint gTexWrapMode = GL_REPEAT;

    // Shader program
    GLuint gProgramId;
    GLuint gKeyLightProgramId;
    GLuint gFillLightProgramId;

    // camera
    Camera gCamera(glm::vec3(0.5f, -3.0f, 8.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // timing
    float gDeltaTime = 0.0f; // time between current frame and last frame
    float gLastFrame = 0.0f;

    // Subject position and scale
    glm::vec3 gPlanePosition(0.0f, 0.0f, 0.0f);
    glm::vec3 gPlaneScale(10.0f);


    // Cube and light color
    //m::vec3 gObjectColor(0.6f, 0.5f, 0.75f);
    glm::vec3 gObjectColor(1.f, 0.2f, 0.0f);
    glm::vec3 gKeyLightColor(1.0f, 0.5f, 0.0f);
    glm::vec3 gFillLightColor(1.0f, 1.0f, 1.0f);

    // Light position
    glm::vec3 gKeyLightPosition(-2.5f, -0.5f, 0.0f);
    glm::vec3 gFillLightPosition(3.0f, -2.0f, 0.0f);

    // Light scale
    glm::vec3 gLightScale(0.6f);

    // Lamp animation
    bool gIsLampOrbiting = true;

    // Switch Projection
    bool isPerspective = true;
}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void UCreateMesh(GLMesh& mesh, const char* type);
void UDestroyMesh(GLMesh& mesh);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);

/* Vertex Shader for Object*/
const GLchar* vertexShaderSource = GLSL(440,
    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data
layout(location = 1) in vec3 normal; // VAP position 1 for normals
layout(location = 2) in vec2 textureCoordinate;

out vec3 vertexNormal; // For outgoing normals to fragment shader
out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
out vec2 vertexTextureCoordinate;

//Global variables for the transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates

    vertexFragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)

    vertexNormal = mat3(transpose(inverse(model))) * normal; // Get normal vectors in world space only and exclude normal translation properties

    vertexTextureCoordinate = textureCoordinate;
}
);


/* Fragment Shader for Object */
const GLchar* fragmentShaderSource = GLSL(440,
    in vec3 vertexNormal;
in vec3 vertexFragmentPos;
in vec2 vertexTextureCoordinate;

out vec4 fragmentColor;

uniform vec3 objectColor;

uniform vec3 lightColor1;
uniform vec3 lightPos1;

uniform vec3 lightColor2;
uniform vec3 lightPos2;

uniform vec3 viewPosition;
uniform sampler2D uTexture;
uniform vec2 uvScale;

void main()
{
    float ambientStrength1 = 0.3f;
    vec3 ambient1 = ambientStrength1 * lightColor1;

    float ambientStrength2 = 0.1f;
    vec3 ambient2 = ambientStrength2 * lightColor2;

    // Calculate Diffuse lighting
    vec3 norm = normalize(vertexNormal);
    vec3 lightDirection1 = normalize(lightPos1 - vertexFragmentPos);
    vec3 lightDirection2 = normalize(lightPos2 - vertexFragmentPos);

    float impact1 = max(dot(norm, lightDirection1), 0.0);
    vec3 diffuse1 = impact1 * lightColor1;

    float impact2 = max(dot(norm, lightDirection2), 0.0);
    vec3 diffuse2 = impact2 * lightColor2;

    // Calculate Specular lighting
    float specularIntensity1 = 0.1f;
    float specularIntensity2 = 0.1f;

    float highlightSize = 16.0f;
    vec3 viewDir = normalize(viewPosition - vertexFragmentPos);
    vec3 reflectDir1 = reflect(-lightDirection1, norm);
    vec3 reflectDir2 = reflect(-lightDirection2, norm);

    float specularComponent1 = pow(max(dot(viewDir, reflectDir1), 0.0), highlightSize);
    vec3 specular1 = specularIntensity1 * specularComponent1 * lightColor1;

    float specularComponent2 = pow(max(dot(viewDir, reflectDir2), 0.0), highlightSize);
    vec3 specular2 = specularIntensity2 * specularComponent2 * lightColor2;

    // Texture holds the color to be used for all three components
    vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);

    // Calculate phong result for both lights and sum them up
    vec3 phong = (ambient1 + diffuse1 + specular1 + ambient2 + diffuse2 + specular2) * textureColor.xyz;

    // Send lighting results to GPU
    //fragmentColor = vec4(phong, 1.0);
    fragmentColor = vec4(phong * textureColor.xyz, 1.0);
    //vec4 reflectiveColor = vec4(phong, 1.0) * texture(uTexture, vertexTextureCoordinate);
    //fragmentColor = reflectiveColor;
}
);

/* Lamp Shader Source Code*/
const GLchar* lampVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data

//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Uniform color for each shape
uniform vec4 shapeColor;

out vec4 vertexColor; // Variable to transfer color data to the fragment shader

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
    vertexColor = shapeColor; // Use the uniform color
}
);

/* Fragment Shader Source Code*/
const GLchar* lampFragmentShaderSource = GLSL(440,
    in vec4 vertexColor; // Variable to hold incoming color data from vertex shader

out vec4 fragmentColor; // For outgoing lamp color (smaller cube) to the GPU

void main()
{
    fragmentColor = vertexColor; // Use the incoming color directly
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

    // Create the mesh
    UCreateMesh(gMesh, "plane");
    UCreateMesh(gCylinderMesh, "cylinder");
    UCreateMesh(gSphereMesh, "sphere");
    UCreateMesh(gCubeMesh, "cube");
    UCreateMesh(gPrismMesh, "prism");
    UCreateMesh(gTorusMesh, "torus");
    UCreateMesh(gCupMesh, "cup");

    // Create the shader program
    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;
    if (!UCreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, gKeyLightProgramId))
        return EXIT_FAILURE;
    if (!UCreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, gFillLightProgramId))
        return EXIT_FAILURE;

    // Load textures for each shape
    const char* cylinderTextureFile = "../../resources/textures/wood-texture.png";
    const char* sphereTextureFile = "../../resources/textures/snow-texture.png";
    const char* planeTextureFile = "../../resources/textures/desk-texture.png";
    const char* cylinder2TextureFile = "../../resources/textures/speaker-texture.png";
    const char* torusTextureFile = "../../resources/textures/silver-texture.png";
    const char* cubeTextureFile = "../../resources/textures/rubik-texture.png";
    const char* prismTextureFile = "../../resources/textures/laptop-texture.png";
    const char* cupTextureFile = "../../resources/textures/cup-texture.png";

    if (!UCreateTexture(cylinderTextureFile, gCylinderTextureId))
    {
        cout << "Failed to load texture " << cylinderTextureFile << endl;
        return EXIT_FAILURE;
    }

    if (!UCreateTexture(sphereTextureFile, gSphereTextureId))
    {
        cout << "Failed to load texture " << sphereTextureFile << endl;
        return EXIT_FAILURE;
    }

    if (!UCreateTexture(planeTextureFile, gPlaneTextureId))
    {
        cout << "Failed to load texture " << planeTextureFile << endl;
        return EXIT_FAILURE;
    }

    if (!UCreateTexture(cylinder2TextureFile, gCylinderTextureId2))
    {
        cout << "Failed to load texture " << cylinder2TextureFile << endl;
        return EXIT_FAILURE;
    }

    if (!UCreateTexture(torusTextureFile, gTorusTextureId))
    {
        cout << "Failed to load texture " << torusTextureFile << endl;
        return EXIT_FAILURE;
    }

    if (!UCreateTexture(cubeTextureFile, gCubeTextureId))
    {
        cout << "Failed to load texture " << cubeTextureFile << endl;
        return EXIT_FAILURE;
    }

    if (!UCreateTexture(prismTextureFile, gPrismTextureId))
    {
        cout << "Failed to load texture " << prismTextureFile << endl;
        return EXIT_FAILURE;
    }

    if (!UCreateTexture(cupTextureFile, gCupTextureId))
    {
        cout << "Failed to load texture " << cupTextureFile << endl;
        return EXIT_FAILURE;
    }

    // Tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gProgramId);
    // We set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);

    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Render loop
    // -----------
    while (!glfwWindowShouldClose(gWindow))
    {
        // per-frame timing
        // --------------------
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // input
        // -----
        UProcessInput(gWindow);

        // Render this frame
        URender();

        glfwPollEvents();
    }

    // Release mesh data
    UDestroyMesh(gMesh);
    UDestroyMesh(gCylinderMesh);
    UDestroyMesh(gSphereMesh);
    UDestroyMesh(gCubeMesh);
    UDestroyMesh(gPrismMesh);
    UDestroyMesh(gTorusMesh);
    UDestroyMesh(gCupMesh);

    // Release textures
    UDestroyTexture(gCylinderTextureId);
    UDestroyTexture(gSphereTextureId);
    UDestroyTexture(gPlaneTextureId);
    UDestroyTexture(gCylinderTextureId2);
    UDestroyTexture(gTorusTextureId);
    UDestroyTexture(gPrismTextureId);
    UDestroyTexture(gCupTextureId);

    // Release shader program
    UDestroyShaderProgram(gProgramId);
    UDestroyShaderProgram(gKeyLightProgramId);
    UDestroyShaderProgram(gFillLightProgramId);

    exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window creation
    // ---------------------
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
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

    // GLEW: initialize
    // ----------------
    // Note: if using GLEW version 1.13 or earlier
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
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        gCamera.Position += glm::vec3(0.0f, cameraSpeed * gDeltaTime, 0.0f);  // Move upward
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gCamera.Position -= glm::vec3(0.0f, cameraSpeed * gDeltaTime, 0.0f);  // Move downward

    // Pause and resume lamp orbiting
    static bool isLKeyDown = false;
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && !gIsLampOrbiting)
        gIsLampOrbiting = true;
    else if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS && gIsLampOrbiting)
        gIsLampOrbiting = false;

    // Switch between perspective and orthographic projections when "P" or "O" is pressed
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !isPerspective)
    {
        isPerspective = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS && isPerspective)
    {
        isPerspective = false;
    }


}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
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
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
    {
        if (action == GLFW_PRESS)
            cout << "Left mouse button pressed" << endl;
        else
            cout << "Left mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
    {
        if (action == GLFW_PRESS)
            cout << "Middle mouse button pressed" << endl;
        else
            cout << "Middle mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        if (action == GLFW_PRESS)
            cout << "Right mouse button pressed" << endl;
        else
            cout << "Right mouse button released" << endl;
    }
    break;

    default:
        cout << "Unhandled mouse button event" << endl;
        break;
    }
}


// Functioned called to render a frame
void URender()
{

    // Animation: Lamp orbits around the origin
    const float angularVelocity = glm::radians(45.0f);
    if (gIsLampOrbiting)
    {
        glm::vec4 newPosition = glm::rotate(angularVelocity * gDeltaTime, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(gKeyLightPosition, 1.0f);
        gKeyLightPosition.x = newPosition.x;
        gKeyLightPosition.y = newPosition.y;
        gKeyLightPosition.z = newPosition.z;

        glm::vec4 newPosition2 = glm::rotate(angularVelocity * gDeltaTime, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(gFillLightPosition, 1.0f);
        gFillLightPosition.x = newPosition2.x;
        gFillLightPosition.y = newPosition2.y;
        gFillLightPosition.z = newPosition2.z;
    }


    // Enable z-depth
    glEnable(GL_DEPTH_TEST);

    // Clear the frame and z buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMesh.vao);

    // PLANE: Draw plane
    //----------------
    // Set the shader to be used
    glUseProgram(gProgramId);

    // Model matrix: transformations are applied right-to-left order
    glm::mat4 model = glm::translate(gPlanePosition) * glm::scale(gPlaneScale);

    // Camera/view transformation
    glm::mat4 view = gCamera.GetViewMatrix();

    // Choose the projection matrix based on the current mode
    glm::mat4 projection;
    if (isPerspective)
    {
        // Creates a perspective projection
        projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
    }
    else
    {
        // Creates an orthographic projection
        projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);
    }

    // Retrieves and passes transform matrices to the Shader program
    GLint modelLoc = glGetUniformLocation(gProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gProgramId, "view");
    GLint projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference matrix uniforms from the Cube Shader program for the cube color, light1 color, light1 position, light2 color, light2 position, and camera position
    GLint objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
    GLint light1ColorLoc = glGetUniformLocation(gProgramId, "lightColor1");
    GLint light1PositionLoc = glGetUniformLocation(gProgramId, "lightPos1");
    GLint light2ColorLoc = glGetUniformLocation(gProgramId, "lightColor2");
    GLint light2PositionLoc = glGetUniformLocation(gProgramId, "lightPos2");
    GLint viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");

    // Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(light1ColorLoc, gKeyLightColor.r, gKeyLightColor.g, gKeyLightColor.b);
    glUniform3f(light1PositionLoc, gKeyLightPosition.x, gKeyLightPosition.y, gKeyLightPosition.z);
    glUniform3f(light2ColorLoc, gFillLightColor.r, gFillLightColor.g, gFillLightColor.b);
    glUniform3f(light2PositionLoc, gFillLightPosition.x, gFillLightPosition.y, gFillLightPosition.z);

    const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    GLint UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // Bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPlaneTextureId);

    // Draws the pyramid
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);


    // CYLINDER: Draw Cylinder
    //----------------
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gCylinderMesh.vao);

    // Render the Cylinder
    glm::mat4 cylinderModel = glm::translate(glm::vec3(-1.0f, -5.25f, 2.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(cylinderModel));

    // Bind textures on corresponding texture units
    glBindTexture(GL_TEXTURE_2D, gCylinderTextureId);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLE_STRIP, 0, gCylinderMesh.nVertices);


    // CYLINDER 2: Draw Second Cylinder
    //----------------
    // Render the Second Cylinder
    glm::mat4 cylinderModel2 = glm::scale(glm::vec3(1.5f, 1.5f, 1.5f)) * glm::rotate(90.0f, glm::vec3(1.0f, -1.0f, 1.0f)) * glm::translate(glm::vec3(0.0f, 0.0f, 3.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(cylinderModel2));

    // Bind textures on corresponding texture units
    glBindTexture(GL_TEXTURE_2D, gCylinderTextureId2);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLE_STRIP, 0, gCylinderMesh.nVertices);


    // SPHERE: Draw Sphere
    //----------------
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gSphereMesh.vao);

    // Render the sphere
    glm::mat4 sphereModel = cylinderModel * glm::translate(glm::vec3(0.0f, 0.75f, 0.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(sphereModel));

    // bind textures on corresponding texture units
    glBindTexture(GL_TEXTURE_2D, gSphereTextureId);

    // Draws the sphere
    glDrawArrays(GL_TRIANGLE_FAN, 0, gSphereMesh.nVertices);


    // PRISM: Draw Prism
    //----------------
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gPrismMesh.vao);

    // Render the prism
    glm::mat4 prismModel = glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::translate(glm::vec3(0.75f, 1.0f, 5.0f)) * glm::scale(glm::vec3(2.0f, 2.0f, 1.0f));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(prismModel));

    // bind textures on corresponding texture units
    glBindTexture(GL_TEXTURE_2D, gPrismTextureId);

    // Draws the prism
    glDrawArrays(GL_TRIANGLES, 0, gPrismMesh.nVertices);


    // CUBE: Draw Cube
    //----------------
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gCubeMesh.vao);

    // Render the cube
    glm::mat4 cubeModel = glm::translate(glm::vec3(0.25f, -4.6f, 0.75f)) * glm::scale(glm::vec3(0.5f, 0.5f, 0.5f)) * glm::rotate(90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(cubeModel));

    // bind textures on corresponding texture units
    glBindTexture(GL_TEXTURE_2D, gCubeTextureId);

    // Draws the cube
    glDrawArrays(GL_TRIANGLES, 0, gCubeMesh.nVertices);

    // CUP: Draw Cup 
    //----------------
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gCupMesh.vao);

    // Render the cup
    glm::mat4 cupModel = glm::translate(glm::vec3(1.5f, -5.0f, -0.5f)) * glm::rotate(90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(cupModel));

    // bind textures on corresponding texture units
    glBindTexture(GL_TEXTURE_2D, gCupTextureId);

    // Draws the cup
    glDrawArrays(GL_TRIANGLE_STRIP, 0, gCupMesh.nVertices);


    // KEY LIGHT: Draw Cube 1
    //----------------
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gCubeMesh.vao);

    // Set the shader to be used
    glUseProgram(gKeyLightProgramId);

    //Transform the smaller cube used as a visual que for the light source
    model = glm::translate(gKeyLightPosition) * glm::scale(gLightScale);

    // Reference matrix uniforms from the Light Shader program
    modelLoc = glGetUniformLocation(gKeyLightProgramId, "model");
    viewLoc = glGetUniformLocation(gKeyLightProgramId, "view");
    projLoc = glGetUniformLocation(gKeyLightProgramId, "projection");
    GLint shapeColorLoc1 = glGetUniformLocation(gKeyLightProgramId, "shapeColor"); // Declare shapeColorLoc


    // Pass matrix data to the Light Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Set color for the cube
    glUniform4f(shapeColorLoc1, 1.0f, 0.5f, 0.0f, 1.0f);

    // Draws the cube
    glDrawArrays(GL_TRIANGLES, 0, gCubeMesh.nVertices);


    // FILL LIGHT: Draw Cube 2
    //----------------
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gCubeMesh.vao);

    // Set the shader to be used
    glUseProgram(gFillLightProgramId);

    //Transform the smaller cube used as a visual que for the light source
    model = glm::translate(gFillLightPosition) * glm::scale(gLightScale);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gFillLightProgramId, "model");
    viewLoc = glGetUniformLocation(gFillLightProgramId, "view");
    projLoc = glGetUniformLocation(gFillLightProgramId, "projection");
    GLint shapeColorLoc2 = glGetUniformLocation(gFillLightProgramId, "shapeColor"); // Declare shapeColorLoc

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Set color for the cube
    glUniform4f(shapeColorLoc2, 1.0f, 1.0f, 1.0f, 1.0f);

    // Draws the cube
    glDrawArrays(GL_TRIANGLES, 0, gCubeMesh.nVertices);

    // Deactivate the Vertex Array Object and shader program
    glBindVertexArray(0);
    glUseProgram(0);

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.

}


// Implements the UCreateMesh function
void UCreateMesh(GLMesh& mesh, const char* type)
{
    // For the cylinder
    if (type == "cylinder")
    {
        const float PI = 3.14159265359f;
        int sectorCount = 30;
        float radius = 0.2f;
        float height = 1.0f;

        // Vertex data
        std::vector<float> verts;
        float sectorStep = 2 * PI / sectorCount;

        for (int i = 0; i <= sectorCount; ++i)
        {
            float sectorAngle = i * sectorStep;
            float x = radius * cos(sectorAngle);
            float y = radius * sin(sectorAngle);

            // Bottom vertex
            verts.push_back(x);
            verts.push_back(0.0f);
            verts.push_back(y);

            // Calculate normal for the bottom vertex (pointing radially outward)
            float normalX = x / radius;
            float normalY = 0.0f;
            float normalZ = y / radius;

            verts.push_back(normalX);
            verts.push_back(normalY);
            verts.push_back(normalZ);

            verts.push_back(float(i) / sectorCount);
            verts.push_back(0.0f);

            // Top vertex
            verts.push_back(x);
            verts.push_back(height);
            verts.push_back(y);

            // Calculate normal for the top vertex (pointing radially outward)
            normalX = x / radius;
            normalY = 0.0f;
            normalZ = y / radius;

            verts.push_back(normalX);
            verts.push_back(normalY);
            verts.push_back(normalZ);

            verts.push_back(float(i) / sectorCount);
            verts.push_back(1.0f);
        }

        mesh.nVertices = verts.size() / 8;  // Updated for the additional normal components

        glGenVertexArrays(1, &mesh.vao);
        glBindVertexArray(mesh.vao);

        glGenBuffers(1, &mesh.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * verts.size(), verts.data(), GL_STATIC_DRAW);

        GLint stride = sizeof(float) * 8;  // Updated for the additional normal components

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
        glEnableVertexAttribArray(0);

        // Normal attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 3));
        glEnableVertexAttribArray(1);

        // Texture coordinate attribute
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 6));
        glEnableVertexAttribArray(2);
    }
    // For the sphere
    else if (type == "sphere")
    {
        const float PI = glm::pi<float>();
        int sectorCount = 30;
        int stackCount = 30;
        float radius = 0.35f;

        std::vector<float> vertices;

        for (int i = 0; i <= stackCount; ++i)
        {
            float stackAngle = PI / 2 - i * PI / stackCount;
            float xy = radius * cos(stackAngle);
            float z = radius * sin(stackAngle);

            for (int j = 0; j <= sectorCount; ++j)
            {
                float sectorAngle = j * 2 * PI / sectorCount;

                float x = xy * cos(sectorAngle);
                float y = xy * sin(sectorAngle);

                float normalX = x / radius;  // Calculate normal vectors
                float normalY = y / radius;
                float normalZ = z / radius;

                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);

                vertices.push_back(normalX);
                vertices.push_back(normalY);
                vertices.push_back(normalZ);

                float s = (float)j / sectorCount;
                float t = (float)i / stackCount;

                vertices.push_back(s);
                vertices.push_back(t);
            }
        }

        mesh.nVertices = vertices.size() / 8;  // Updated for the additional normal components

        glGenVertexArrays(1, &mesh.vao);
        glBindVertexArray(mesh.vao);

        glGenBuffers(1, &mesh.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

        GLint stride = sizeof(float) * 8;  // Updated for the additional normal components

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
        glEnableVertexAttribArray(0);

        // Normal attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 3));
        glEnableVertexAttribArray(1);

        // Texture coordinate attribute
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 6));
        glEnableVertexAttribArray(2);
    }
    // For the plane
    else if (type == "plane")
    {
        // Vertex data
        GLfloat verts[] = {
            // Vertex Positions    // Texture coordiantion (u,v)
              -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  0.0f,  0.0f, 1.0f,
               0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  0.0f,  1.0f, 1.0f,
               0.5f, -0.5f,  0.5f,  0.0f, 1.0f,  0.0f,  1.0f, 0.0f,
               0.5f, -0.5f,  0.5f,  0.0f, 1.0f,  0.0f,  1.0f, 0.0f,
              -0.5f, -0.5f,  0.5f,  0.0f, 1.0f,  0.0f,  0.0f, 0.0f,
              -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  0.0f,  0.0f, 1.0f,
        };

        const GLuint floatsPerVertex = 3;
        const GLuint floatsPerNormal = 3;
        const GLuint floatsPerUV = 2;

        mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

        glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
        glBindVertexArray(mesh.vao);

        // Create 2 buffers: first one for the vertex data; second one for the indices
        glGenBuffers(1, &mesh.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

        // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
        GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);// The number of floats before each

        // Create Vertex Attribute Pointers
        glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
        glEnableVertexAttribArray(2);
    }
    // For the cube
    else if (type == "cube")
    {

        GLfloat verts[] = {
            //Positions          //Normals
            // ------------------------------------------------------
            //Back Face          //Negative Z Normal  Texture Coords.
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
             0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

            //Front Face         //Positive Z Normal
             -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
              0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
              0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
              0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
             -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
             -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

             //Left Face          //Negative X Normal
             -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
             -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
             -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
             -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
             -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
             -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

             //Right Face         //Positive X Normal
              0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
              0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
              0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
              0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
              0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
              0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

              //Bottom Face        //Negative Y Normal
              -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
               0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
               0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
               0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
              -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
              -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

              //Top Face           //Positive Y Normal
              -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
               0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
               0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
               0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
              -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
              -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
        };

        const GLuint floatsPerVertex = 3;
        const GLuint floatsPerNormal = 3;
        const GLuint floatsPerUV = 2;

        mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

        glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
        glBindVertexArray(mesh.vao);

        // Create 2 buffers: first one for the vertex data; second one for the indices
        glGenBuffers(1, &mesh.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

        // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
        GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);// The number of floats before each

        // Create Vertex Attribute Pointers
        glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
        glEnableVertexAttribArray(2);
    }
    else if (type == "torus")
    {
        const float PI = 3.14159265359f;
        int ringCount = 30;  // Number of rings
        int sideCount = 30;  // Number of sides per ring
        float majorRadius = 1.0f;  // Radius from the center of the torus to the center of the tube
        float minorRadius = 0.3f;  // Radius of the tube

        // Vertex data
        std::vector<float> verts;
        float ringStep = 2 * PI / ringCount;
        float sideStep = 2 * PI / sideCount;

        for (int i = 0; i <= ringCount; ++i)
        {
            float ringAngle = i * ringStep;
            float cosRing = cos(ringAngle);
            float sinRing = sin(ringAngle);

            for (int j = 0; j <= sideCount; ++j)
            {
                float sideAngle = j * sideStep;
                float cosSide = cos(sideAngle);
                float sinSide = sin(sideAngle);

                // Calculate position
                float x = (majorRadius + minorRadius * cosSide) * cosRing;
                float y = (majorRadius + minorRadius * cosSide) * sinRing;
                float z = minorRadius * sinSide;

                verts.push_back(x);
                verts.push_back(y);
                verts.push_back(z);

                // Calculate normal (pointing radially outward)
                float normalX = cosSide * cosRing;
                float normalY = cosSide * sinRing;
                float normalZ = sinSide;

                verts.push_back(normalX);
                verts.push_back(normalY);
                verts.push_back(normalZ);

                verts.push_back(float(j) / sideCount);
                verts.push_back(float(i) / ringCount);
            }
        }

        mesh.nVertices = verts.size() / 8;  // Updated for the additional normal components

        glGenVertexArrays(1, &mesh.vao);
        glBindVertexArray(mesh.vao);

        glGenBuffers(1, &mesh.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * verts.size(), verts.data(), GL_STATIC_DRAW);

        GLint stride = sizeof(float) * 8;  // Updated for the additional normal components

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
        glEnableVertexAttribArray(0);

        // Normal attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 3));
        glEnableVertexAttribArray(1);

        // Texture coordinate attribute
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 6));
        glEnableVertexAttribArray(2);
    }
    else if (type == "cup")
    {
        const float PI = 3.14159265359f;
        int sectorCount = 30;
        float topRadius = 0.5f;  // Radius of the top of the cup
        float bottomRadius = 0.3f;  // Radius of the bottom of the cup
        float height = 1.5f;

        // Vertex data
        std::vector<float> verts;
        float sectorStep = 2 * PI / sectorCount;

        for (int i = 0; i <= sectorCount; ++i)
        {
            float sectorAngle = i * sectorStep;
            float xTop = topRadius * cos(sectorAngle);
            float yTop = topRadius * sin(sectorAngle);

            float xBottom = bottomRadius * cos(sectorAngle);
            float yBottom = bottomRadius * sin(sectorAngle);

            // Bottom vertex
            verts.push_back(xBottom);
            verts.push_back(0.0f);
            verts.push_back(yBottom);

            // Calculate normal for the bottom vertex (pointing radially outward)
            float normalX = xBottom / bottomRadius;
            float normalY = 0.0f;
            float normalZ = yBottom / bottomRadius;

            verts.push_back(normalX);
            verts.push_back(normalY);
            verts.push_back(normalZ);

            verts.push_back(float(i) / sectorCount);
            verts.push_back(0.0f);

            // Top vertex
            verts.push_back(xTop);
            verts.push_back(height);
            verts.push_back(yTop);

            // Calculate normal for the top vertex (pointing radially outward)
            normalX = xTop / topRadius;
            normalY = 0.0f;
            normalZ = yTop / topRadius;

            verts.push_back(normalX);
            verts.push_back(normalY);
            verts.push_back(normalZ);

            verts.push_back(float(i) / sectorCount);
            verts.push_back(1.0f);
        }

        mesh.nVertices = verts.size() / 8;  // Updated for the additional normal components

        glGenVertexArrays(1, &mesh.vao);
        glBindVertexArray(mesh.vao);

        glGenBuffers(1, &mesh.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * verts.size(), verts.data(), GL_STATIC_DRAW);

        GLint stride = sizeof(float) * 8;  // Updated for the additional normal components

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
        glEnableVertexAttribArray(0);

        // Normal attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 3));
        glEnableVertexAttribArray(1);

        // Texture coordinate attribute
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 6));
        glEnableVertexAttribArray(2);
    }
    else if (type == "prism")
    {
        GLfloat verts[] = {
            //Positions          //Normals           // Texture Coords
            // ------------------------------------------------------
            //Back Face          //Negative Z Normal  Texture Coords.
            -0.75f, -0.5f, -0.1f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
             0.75f, -0.5f, -0.1f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
             0.75f,  0.5f, -0.1f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
             0.75f,  0.5f, -0.1f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
            -0.75f,  0.5f, -0.1f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
            -0.75f, -0.5f, -0.1f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,

            //Front Face         //Positive Z Normal
            -0.75f, -0.5f,  0.1f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
             0.75f, -0.5f,  0.1f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
             0.75f,  0.5f,  0.1f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
             0.75f,  0.5f,  0.1f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
            -0.75f,  0.5f,  0.1f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
            -0.75f, -0.5f,  0.1f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,

            //Left Face          //Negative X Normal
            -0.75f,  0.5f,  0.1f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
            -0.75f,  0.5f, -0.1f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
            -0.75f, -0.5f, -0.1f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
            -0.75f, -0.5f, -0.1f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
            -0.75f, -0.5f,  0.1f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
            -0.75f,  0.5f,  0.1f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,

            //Right Face         //Positive X Normal
             0.75f,  0.5f,  0.1f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
             0.75f,  0.5f, -0.1f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
             0.75f, -0.5f, -0.1f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
             0.75f, -0.5f, -0.1f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
             0.75f, -0.5f,  0.1f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
             0.75f,  0.5f,  0.1f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,

             //Bottom Face        //Negative Y Normal
             -0.75f, -0.5f, -0.1f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
              0.75f, -0.5f, -0.1f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
              0.75f, -0.5f,  0.1f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
              0.75f, -0.5f,  0.1f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
             -0.75f, -0.5f,  0.1f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
             -0.75f, -0.5f, -0.1f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

             //Top Face           //Positive Y Normal
             -0.75f,  0.5f, -0.1f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
              0.75f,  0.5f, -0.1f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
              0.75f,  0.5f,  0.1f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
              0.75f,  0.5f,  0.1f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
             -0.75f,  0.5f,  0.1f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
             -0.75f,  0.5f, -0.1f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
        };



        const GLuint floatsPerVertex = 3;
        const GLuint floatsPerNormal = 3;
        const GLuint floatsPerUV = 2;

        mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

        glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
        glBindVertexArray(mesh.vao);

        // Create 2 buffers: first one for the vertex data; second one for the indices
        glGenBuffers(1, &mesh.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

        // Strides between vertex coordinates is 8 (x, y, z, r, g, b, a, s, t). A tightly packed stride is 0.
        GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);

        // Create Vertex Attribute Pointers
        glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
        glEnableVertexAttribArray(2);
    }
}

void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(1, &mesh.vbo);
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

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the vertex shader, and print compilation errors (if any)
    glCompileShader(vertexShaderId); // compile the vertex shader
    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId); // compile the fragment shader
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   // links the shader program
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);    // Uses the shader program

    return true;
}


void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}
