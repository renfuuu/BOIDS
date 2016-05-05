#include <fstream>
#include <iostream>
#include <istream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>

// OpenGL library includes

#if defined(__APPLE__) || defined(MACOSX)
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#else
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#endif


#include <SDL.h> 
#include <SOIL.h>


bool drag_state = false;
int current_button = -1;



int screenWidth = 800, screenHeight = 600;
const std::string window_title = "OBJ Loader";



// VBO and VAO descriptors.
enum { 
	kVertexBuffer, // Buffer of vertex positions
	kIndexBuffer,  // Buffer of triangle indices
	kNumVbos };

GLuint vao = 0;                   // This will store the VAO descriptor.
enum {
  kBoid,
  kVelocity,
  kAcceleration,
  kAlign,
  kSeparate,
  kCohesion,
  kNumVaos
};




GLuint array_objects[kNumVaos];
GLuint buffer_objects[kNumVbos];  // These will store VBO descriptors.

const char* vertex_shader =
    "#version 330 core\n"
    "in vec3 vertex_position;" // A vector (x,y,z) representing the vertex's position
    "uniform vec3 light_position;" // Global variable representing the light's position
    "out vec3 vs_light_direction;" // Used for shading by the fragment shader
    "void main() {"
       "gl_Position = vec4(vertex_position, 1.0);" // Don't transform the vertices at all
       "vs_light_direction = light_position - vertex_position;" // Calculate vector to the light (used for shading in fragment shader)
    "}";

const char* geometry_shader =
    "#version 330 core\n"
    "layout (triangles) in;" // Reads in triangles
    "layout (triangle_strip, max_vertices = 3) out;" // And outputs triangles
    "uniform mat4 view_projection;" // The matrix encoding the camera position and settings. Don't worry about this for now
    "in vec3 vs_light_direction[];" // The light direction computed in the vertex shader
    "out vec3 normal;" // The normal of the triangle. Needs to be computed inside this shader
    "out vec3 light_direction;" // Light direction again (this is just passed straight through to the fragment shader)
    "void main() {"
    	//Took the cross product of (p2 - p1) and (p3 - p1) which returns a vector
       "vec3 norm = cross( gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz, gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz);"
       // normalized the resulting vector to obtain the unit vector
       "normal = normalize(norm);"
       "int n = 0;"
       "for (n = 0; n < gl_in.length(); n++) {" // Loop over three vertices of the triangle
          "light_direction = vs_light_direction[n];" // Pass the light direction to the fragment shader
          "gl_Position = view_projection * gl_in[n].gl_Position;" // Project the vertex into screen coordinates
          "EmitVertex();"
       "}"
       "EndPrimitive();"
    "}";

const char* fragment_shader =
    "#version 330 core\n"
    "in vec3 normal;" // Normal computed in the geometry shader
    "in vec3 light_direction;" // Light direction computed in the vertex shader
    "out vec4 fragment_color;" // This shader will compute the pixel color
    "void main() {"
       "vec4 color = vec4(1.0, 0.0, 0.0, 1.0);" // Red
       "float dot_nl = dot(normalize(light_direction), normalize(normal));" // Compute brightness based on angle between normal and light
       "dot_nl = clamp(dot_nl, 0.0, 1.0);" // Ignore back-facing triangles
       "fragment_color = clamp(dot_nl * color, 0.0, 1.0);"
    "}";
//---------------------------------------------

const char* sprite_vertex_shader =
  "#version 330 core"
  "layout (location = 0) in vec4 vertex;" // <vec2 position, vec2 texCoords>
  "out vec2 TexCoords;"
  "uniform mat4 model;"
  "uniform mat4 projection;"
  "void main()"
  "{"
  "    TexCoords = vertex.zw;"
  "    gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);"
  "}";

const char* sprite_fragment_shader =
  "#version 330 core"
  "in vec2 TexCoords;"
  "out vec4 color;"
  "uniform sampler2D image;"
  "uniform vec3 spriteColor;"
  "void main()"
  "{"
  "    color = vec4(spriteColor, 1.0)*texture(image, TexCoords);"
  "}";

const char* mesh_vertex_shader =
"#version 330 core"
"in vec2 TexCoords;"
"out vec4 color;"
"uniform vec3 meshColor;"
"void main()"
"{   " 
"    color = vec4(meshColor, 1.0);"
"} ";
const char* mesh_frag_shader =
  "#version 330 core"
  "layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>"
  "uniform mat4 model;"
  "uniform mat4 view;"
  "uniform mat4 projection;"
  ""
  "void main()"
  "{"
  "    gl_Position = projection* view * model * vec4(vertex.xyz, 1.0);"
  "}";

// // Functions and macros to help debug GL errors

const char* OpenGlErrorToString(GLenum error) {
  switch (error) {
    case GL_NO_ERROR:
      return "GL_NO_ERROR";
      break;
    case GL_INVALID_ENUM:
      return "GL_INVALID_ENUM";
      break;
    case GL_INVALID_VALUE:
      return "GL_INVALID_VALUE";
      break;
    case GL_INVALID_OPERATION:
      return "GL_INVALID_OPERATION";
      break;
    case GL_OUT_OF_MEMORY:
      return "GL_OUT_OF_MEMORY";
      break;
    default:
      return "Unknown Error";
      break;
  }
  return "Unicorns Exist";
}

#define CHECK_SUCCESS(x) \
  if (!(x)) {            \
    glfwTerminate();     \
    exit(EXIT_FAILURE);  \
  }

#define CHECK_GL_SHADER_ERROR(id)                                           \
  {                                                                          \
    GLint status = 0;                                                       \
    GLint length = 0;                                                       \
    glGetShaderiv(id, GL_COMPILE_STATUS, &status);                          \
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);                         \
    if (!status) {                                                          \
      std::string log(length, 0);                                           \
      glGetShaderInfoLog(id, length, nullptr, &log[0]);                     \
      std::cerr << "Line :" << __LINE__ << " OpenGL Shader Error: Log = \n" \
                << &log[0];                                                 \
      glfwTerminate();                                                      \
      exit(EXIT_FAILURE);                                                   \
    }                                                                       \
  }

#define CHECK_GL_PROGRAM_ERROR(id)                                           \
  {                                                                          \
    GLint status = 0;                                                        \
    GLint length = 0;                                                        \
    glGetProgramiv(id, GL_LINK_STATUS, &status);                             \
    glGetProgramiv(id, GL_INFO_LOG_LENGTH, &length);                         \
    if (!status) {                                                           \
      std::string log(length, 0);                                            \
      glGetProgramInfoLog(id, length, nullptr, &log[0]);                     \
      std::cerr << "Line :" << __LINE__ << " OpenGL Program Error: Log = \n" \
                << &log[0];                                                  \
      glfwTerminate();                                                       \
      exit(EXIT_FAILURE);                                                    \
    }                                                                        \
  }

#define CHECK_GL_ERROR(statement)                                             \
  {                                                                           \
    { statement; }                                                            \
    GLenum error = GL_NO_ERROR;                                               \
    if ((error = glGetError()) != GL_NO_ERROR) {                              \
      std::cerr << "Line :" << __LINE__ << " OpenGL Error: code  = " << error \
                << " description =  " << OpenGlErrorToString(error);          \
      glfwTerminate();                                                        \
      exit(EXIT_FAILURE);                                                     \
    }                                                                         \
  }


void LoadObj(const std::string& file, std::vector<glm::vec3>& vertices,
             std::vector<glm::uvec3>& indices) 
{
 
  std::cout << "LOADOBJ INVOKED" << std::endl;
  std::ifstream myfile (file);
  std::string line;
  glm::vec3 minC(0.0f);
  glm::vec3 maxC(0.0f);


  //Open the obj file containing indicies and vertices
  if(myfile.is_open())
  {
    while (getline(myfile, line))
    {
      // read the file
      std::istringstream is (line);
      char flag;
      is >> flag;
      switch(flag)
      {
      	//in the case that the line begins with v then add the point to the vertices vector
        case 'v':
        {
          float v1,v2,v3;
          is >> v1;
          is >> v2;
          is >> v3;

          vertices.push_back(glm::vec3(v1,v2,v3));
        }break;
        case 'f':
        {
          //in the case that the line begins with f then add the 3 points to the indices vector
          int v1,v2,v3;
          is >> v1;
          is >> v2;
          is >> v3;

          // Zero indexing because obj indices were 1 indexing
          indices.push_back(glm::uvec3(v1-1,v2-1,v3-1));
        
        }break;
        default:
        break;
      }
    }


    

    myfile.close();
  }
  else {
    std::cout << "Unable to open file" << std::endl;
  } 
}

void LoadObj(const std::string& file, std::vector<glm::vec4>& vertices,
             std::vector<glm::uvec3>& indices) 
{
 
  std::cout << "LOADOBJ INVOKED" << std::endl;
  std::ifstream myfile (file);
  std::string line;
  glm::vec3 minC(0.0f);
  glm::vec3 maxC(0.0f);


  //Open the obj file containing indicies and vertices
  if(myfile.is_open())
  {
    while (getline(myfile, line))
    {
      // read the file
      std::istringstream is (line);
      char flag;
      is >> flag;
      switch(flag)
      {
        //in the case that the line begins with v then add the point to the vertices vector
        case 'v':
        {
          float v1,v2,v3;
          is >> v1;
          is >> v2;
          is >> v3;

        
          vertices.push_back(glm::vec4(v1,v2,v3, 1.0f));
        }break;
        case 'f':
        {
          //in the case that the line begins with f then add the 3 points to the indices vector
          int v1,v2,v3;
          is >> v1;
          is >> v2;
          is >> v3;

          // Zero indexing because obj indices were 1 indexing
          indices.push_back(glm::uvec3(v1-1,v2-1,v3-1));
        
        }break;
        default:
        break;
      }
    }


    // auto normalizer = [](glm::vec3 p, glm::vec3 min, glm::vec3 max)
    // {
    //   glm::vec3 q;
    //   float x,y,z;
    //   if(min[0] != max[0])
    //     x = 2.0f*(p[0]-min[0])/(max[0]-min[0])-1.0f;
    //   else
    //     x = min[0];

    //   if(min[1] != max[1])
    //     y = 2.0f*(p[1]-min[1])/(max[1]-min[1])-1.0f;
    //   else
    //     y = min[1];

    //   if(min[2] != max[2])
    //     z = 2.0f*(p[2]-min[2])/(max[2]-min[2])-1.0f;
    //   else
    //     z = min[2];

    //   return glm::vec3(x,y,z);
    // };

    // bool a = (minC[0] >=-1.0f && minC[1] >=-1.0f && minC[2] >=-1.0f);
    // bool b = (maxC[0] <=1.0f && maxC[1] <=1.0f && maxC[2] <=1.0f);

    // if(!(a&&b))
    // {
    //   for (std::vector<glm::vec3>::iterator i = vertices.begin(); i != vertices.end(); ++i)
    //   {
    //     (*i) = normalizer((*i),minC,maxC);
    //   }
    // }

    myfile.close();
  }
  else {
    std::cout << "Unable to open file" << std::endl;
  } 
}

//----------------------------------------------------------------------------
class Shader
{
public:
    // State
    GLuint ID; 
    // Constructor
    Shader()  {}
    ~Shader() {}
    // Sets the current shader as active
    Shader &Use()
    {
      glUseProgram(this->ID);
      return *this;
    }
    // Compiles the shader from given source code
    void Compile(const GLchar *vertexSource, const GLchar *fragmentSource, const GLchar *geometrySource = nullptr) // Note: geometry source code is optional 
    {
      GLuint sVertex, sFragment, gShader;
      // Vertex Shader
      sVertex = glCreateShader(GL_VERTEX_SHADER);
      glShaderSource(sVertex, 1, &vertexSource, NULL);
      glCompileShader(sVertex);
      checkCompileErrors(sVertex, "VERTEX");
      // Fragment Shader
      sFragment = glCreateShader(GL_FRAGMENT_SHADER);
      glShaderSource(sFragment, 1, &fragmentSource, NULL);
      glCompileShader(sFragment);
      checkCompileErrors(sFragment, "FRAGMENT");
      // If geometry shader source code is given, also compile geometry shader
      if (geometrySource != nullptr)
      {
          gShader = glCreateShader(GL_GEOMETRY_SHADER);
          glShaderSource(gShader, 1, &geometrySource, NULL);
          glCompileShader(gShader);
          checkCompileErrors(gShader, "GEOMETRY");
      }
      // Shader Program
      this->ID = glCreateProgram();
      glAttachShader(this->ID, sVertex);
      glAttachShader(this->ID, sFragment);
      if (geometrySource != nullptr)
          glAttachShader(this->ID, gShader);
      glLinkProgram(this->ID);
      checkCompileErrors(this->ID, "PROGRAM");
      // Delete the shaders as they're linked into our program now and no longer necessery
      glDeleteShader(sVertex);
      glDeleteShader(sFragment);
      if (geometrySource != nullptr)
          glDeleteShader(gShader);
    }
    // Utility functions
    
    void SetFloat (const GLchar *name, GLfloat value, GLboolean useShader = false)
    {
      if (useShader)
        this->Use();
      glUniform1f(glGetUniformLocation(this->ID, name), value);
    }
    void SetInteger  (const GLchar *name, GLint value, GLboolean useShader = false)
    {
      if (useShader)
        this->Use();
      glUniform1i(glGetUniformLocation(this->ID, name), value);
    }

    void SetVector2f (const GLchar *name, GLfloat x, GLfloat y, GLboolean useShader = false)
    {
      if (useShader)
        this->Use();
      glUniform2f(glGetUniformLocation(this->ID, name), x, y);
    }

    void SetVector2f (const GLchar *name, const glm::vec2 &value, GLboolean useShader = false)
    {
      if (useShader)
        this->Use();
      glUniform2f(glGetUniformLocation(this->ID, name), value.x, value.y);
    }

    void SetVector3f(const GLchar *name, GLfloat x, GLfloat y, GLfloat z, GLboolean useShader = false)
    {
        if (useShader)
            this->Use();
        glUniform3f(glGetUniformLocation(this->ID, name), x, y, z);
    }

    void SetVector3f(const GLchar *name, const glm::vec3 &value, GLboolean useShader = false)
    {
        if (useShader)
            this->Use();
        glUniform3f(glGetUniformLocation(this->ID, name), value.x, value.y, value.z);
    }

    void SetVector4f(const GLchar *name, GLfloat x, GLfloat y, GLfloat z, GLfloat w, GLboolean useShader = false)
    {
        if (useShader)
            this->Use();
        glUniform4f(glGetUniformLocation(this->ID, name), x, y, z, w);
    }

    void SetVector4f(const GLchar *name, const glm::vec4 &value, GLboolean useShader = false)
    {
        if (useShader)
            this->Use();
        glUniform4f(glGetUniformLocation(this->ID, name), value.x, value.y, value.z, value.w);
    }

    void SetMatrix4(const GLchar *name, const glm::mat4 &matrix, GLboolean useShader = false)
    {
        if (useShader)
            this->Use();
        glUniformMatrix4fv(glGetUniformLocation(this->ID, name), 1, GL_FALSE, &matrix[0][0]);
    }

  private:
    // Checks if compilation or linking failed and if so, print the error logs
    void checkCompileErrors(GLuint object, std::string type)
    {
      GLint success;
      GLchar infoLog[1024];
      if (type != "PROGRAM")
      {
          glGetShaderiv(object, GL_COMPILE_STATUS, &success);
          if (!success)
          {
              glGetShaderInfoLog(object, 1024, NULL, infoLog);
              std::cout << "| ERROR::SHADER: Compile-time error: Type: " << type << "\n"
                  << infoLog << "\n -- --------------------------------------------------- -- "
                  << std::endl;
          }
      }
      else
      {
          glGetProgramiv(object, GL_LINK_STATUS, &success);
          if (!success)
          {
              glGetProgramInfoLog(object, 1024, NULL, infoLog);
              std::cout << "| ERROR::Shader: Link-time error: Type: " << type << "\n"
                  << infoLog << "\n -- --------------------------------------------------- -- "
                  << std::endl;
          }
      }
    }
};

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
const GLfloat YAW        = -90.0f;
const GLfloat PITCH      =  0.0f;
const GLfloat SPEED      =  3.0f;
const GLfloat SENSITIVTY =  0.25f;
const GLfloat ZOOM       =  45.0f;


// An abstract camera class that processes input and calculates the corresponding Eular Angles, Vectors and Matrices for use in OpenGL
class Camera
{
  public:
    // Camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // Eular Angles
    GLfloat Yaw;
    GLfloat Pitch;
    // Camera options
    GLfloat MovementSpeed;
    GLfloat MouseSensitivity;
    GLfloat Zoom;

    // Constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), GLfloat yaw = YAW, GLfloat pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), Zoom(ZOOM)
    {
        this->Position = position;
        this->WorldUp = up;
        this->Yaw = yaw;
        this->Pitch = pitch;
        this->updateCameraVectors();
    }
    // Constructor with scalar values
    Camera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat upX, GLfloat upY, GLfloat upZ, GLfloat yaw, GLfloat pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), Zoom(ZOOM)
    {
        this->Position = glm::vec3(posX, posY, posZ);
        this->WorldUp = glm::vec3(upX, upY, upZ);
        this->Yaw = yaw;
        this->Pitch = pitch;
        this->updateCameraVectors();
    }

    ~Camera()
    {}

    // Returns the view matrix calculated using Eular Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(this->Position, this->Position + this->Front, this->Up);
    }

    // Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime)
    {
        GLfloat velocity = this->MovementSpeed * deltaTime;
        if (direction == FORWARD)
        {
          this->Position += this->Front * velocity;
        }
        if (direction == BACKWARD)
        {
          this->Position -= this->Front * velocity;
        }
        if (direction == LEFT)
        {
          this->Position -= this->Right * velocity;
        }
        if (direction == RIGHT)
        {
          this->Position += this->Right * velocity;
        }

        std::cout << "pos:: ";
        std::cout << "x: " << this->Position[0] << "y: " << this->Position[1] << "z: " << this->Position[2] <<std::endl;
    }

    // Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= this->MouseSensitivity;
        yoffset *= this->MouseSensitivity;

        this->Yaw   += xoffset;
        this->Pitch += yoffset;

        // Make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (this->Pitch > 89.0f)
                this->Pitch = 89.0f;
            if (this->Pitch < -89.0f)
                this->Pitch = -89.0f;
        }

        // Update Front, Right and Up Vectors using the updated Eular angles
        this->updateCameraVectors();
    }

    // Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(GLfloat yoffset)
    {
        if (this->Zoom >= 1.0f && this->Zoom <= 45.0f)
            this->Zoom -= yoffset;
        if (this->Zoom <= 1.0f)
            this->Zoom = 1.0f;
        if (this->Zoom >= 45.0f)
            this->Zoom = 45.0f;
    }

  private:
    // Calculates the front vector from the Camera's (updated) Eular Angles
    void updateCameraVectors()
    {
        // Calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
        front.y = sin(glm::radians(this->Pitch));
        front.z = sin(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
        this->Front = glm::normalize(front);
        // Also re-calculate the Right and Up vector
        this->Right = glm::normalize(glm::cross(this->Front, this->WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        this->Up    = glm::normalize(glm::cross(this->Right, this->Front));
    }
};

class Texture2D
{
  public:
    // Holds the ID of the texture object, used for all texture operations to reference to this particlar texture
    GLuint ID;
    // Texture image dimensions
    GLuint Width, Height; // Width and height of loaded image in pixels
    // Texture Format
    GLuint Internal_Format; // Format of texture object
    GLuint Image_Format; // Format of loaded image
    // Texture configuration
    GLuint Wrap_S; // Wrapping mode on S axis
    GLuint Wrap_T; // Wrapping mode on T axis
    GLuint Filter_Min; // Filtering mode if texture pixels < screen pixels
    GLuint Filter_Max; // Filtering mode if texture pixels > screen pixels
    // Constructor (sets default texture modes)
    Texture2D()
        : Width(0), Height(0), Internal_Format(GL_RGB), Image_Format(GL_RGB), Wrap_S(GL_REPEAT), Wrap_T(GL_REPEAT), Filter_Min(GL_LINEAR), Filter_Max(GL_LINEAR)
    {
        glGenTextures(1, &this->ID);
    }    
    // Generates texture from image data
    void Generate(GLuint width, GLuint height, unsigned char* data)
    {
        this->Width = width;
        this->Height = height;
        // Create Texture
        glBindTexture(GL_TEXTURE_2D, this->ID);
        glTexImage2D(GL_TEXTURE_2D, 0, this->Internal_Format, width, height, 0, this->Image_Format, GL_UNSIGNED_BYTE, data);
        // Set Texture wrap and filter modes
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->Wrap_S);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->Wrap_T);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, this->Filter_Min);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, this->Filter_Max);
        // Unbind texture
        glBindTexture(GL_TEXTURE_2D, 0);
    }    
    // Binds the texture as the current active GL_TEXTURE_2D texture object

    void Bind() const
    {
        glBindTexture(GL_TEXTURE_2D, this->ID);
    }
};

class ResourceManager
{
  public:
      // Resource storage
      static std::map<std::string, Shader>    Shaders;
      static std::map<std::string, Texture2D> Textures;
      // Loads (and generates) a shader program from file loading vertex, fragment (and geometry) shader's source code. If gShaderFile is not nullptr, it also loads a geometry shader
      static Shader   LoadShader(const GLchar *vShaderFile, const GLchar *fShaderFile, const GLchar *gShaderFile, std::string name);
      // Retrieves a stored sader
      static Shader   GetShader(std::string name);
      // Loads (and generates) a texture from file
      static Texture2D LoadTexture(const GLchar *file, GLboolean alpha, std::string name);
      // Retrieves a stored texture
      static Texture2D GetTexture(std::string name);
      // Properly de-allocates all loaded resources
      static void      Clear();
  private:
      // Private constructor, that is we do not want any actual resource manager objects. Its members and functions should be publicly available (static).
      ResourceManager() { }
      // Loads and generates a shader from file
      static Shader    loadShaderFromFile(const GLchar *vShaderFile, const GLchar *fShaderFile, const GLchar *gShaderFile = nullptr);
      // Loads a single texture from file
      static Texture2D loadTextureFromFile(const GLchar *file, GLboolean alpha);
};


std::map<std::string, Texture2D>    ResourceManager::Textures;
std::map<std::string, Shader>       ResourceManager::Shaders;

Shader ResourceManager::LoadShader(const GLchar *vShaderFile, const GLchar *fShaderFile, const GLchar *gShaderFile, std::string name)
{
    Shaders[name] = loadShaderFromFile(vShaderFile, fShaderFile, gShaderFile);
    return Shaders[name];
}

Shader ResourceManager::GetShader(std::string name)
{
    return Shaders[name];
}

Texture2D ResourceManager::LoadTexture(const GLchar *file, GLboolean alpha, std::string name)
{
    Textures[name] = loadTextureFromFile(file, alpha);
    return Textures[name];
}

Texture2D ResourceManager::GetTexture(std::string name)
{
    return Textures[name];
}

void ResourceManager::Clear()
{
    // (Properly) delete all shaders  
    for (auto iter : Shaders)
        glDeleteProgram(iter.second.ID);
    // (Properly) delete all textures
    for (auto iter : Textures)
        glDeleteTextures(1, &iter.second.ID);
}

Shader ResourceManager::loadShaderFromFile(const GLchar *vShaderFile, const GLchar *fShaderFile, const GLchar *gShaderFile)
{
    // 1. Retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::string geometryCode;
    try
    {
        // Open files
        std::ifstream vertexShaderFile(vShaderFile);
        std::ifstream fragmentShaderFile(fShaderFile);
        std::stringstream vShaderStream, fShaderStream;
        // Read file's buffer contents into streams
        vShaderStream << vertexShaderFile.rdbuf();
        fShaderStream << fragmentShaderFile.rdbuf();
        // close file handlers
        vertexShaderFile.close();
        fragmentShaderFile.close();
        // Convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
        // If geometry shader path is present, also load a geometry shader
        if (gShaderFile != nullptr)
        {
            std::ifstream geometryShaderFile(gShaderFile);
            std::stringstream gShaderStream;
            gShaderStream << geometryShaderFile.rdbuf();
            geometryShaderFile.close();
            geometryCode = gShaderStream.str();
        }
    }
    catch (std::exception e)
    {
        std::cout << "ERROR::SHADER: Failed to read shader files" << std::endl;
    }
    const GLchar *vShaderCode = vertexCode.c_str();
    const GLchar *fShaderCode = fragmentCode.c_str();
    const GLchar *gShaderCode = geometryCode.c_str();
    // 2. Now create shader object from source code
    Shader shader;
    shader.Compile(vShaderCode, fShaderCode, gShaderFile != nullptr ? gShaderCode : nullptr);
    std::cout << "SUCCESS::SHADER: Completed Shader Compilation" << std::endl;
    return shader;
}

Texture2D ResourceManager::loadTextureFromFile(const GLchar *file, GLboolean alpha)
{
    // Create Texture object
    Texture2D texture;
    if (alpha)
    {
        texture.Internal_Format = GL_RGBA;
        texture.Image_Format = GL_RGBA;
    }
    // Load image
    int width, height;
    unsigned char* image = SOIL_load_image(file, &width, &height, 0, texture.Image_Format == GL_RGBA ? SOIL_LOAD_RGBA : SOIL_LOAD_RGB);
    // Now generate texture
    std::cout << image << std::endl;
    texture.Generate(width, height, image);
    // And finally free image data
    SOIL_free_image_data(image);
    std::cout << "SUCCESS::Texture2D: Completed Texture Loading" << std::endl;
    return texture;
}

Camera* camera;
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// The MAIN function, from here we start our application and run our Game loop



//-------------------------------------------------------------------------------------
//BOIDS

enum SimulationState { RUN, PAUSE, kSimStates};
enum SteerState {NATURAL, RETURN, CHAOS, kSteerStates};

int sim_state = SimulationState::RUN;
int steer_state = SteerState::NATURAL;


struct Boid
{
  glm::vec3 mLocation;
  glm::vec3 mVelocity;
  glm::vec3 mAcceleration;
  float r;
  float maxforce;
  float maxspeed;

  glm::vec3 mColor;

  glm::vec3 mWeights;

  glm::vec3 cohIncentive;
  glm::vec3 aliIncentive;
  glm::vec3 sepIncentive;

  Boid(glm::vec3 pos)
  {
    mAcceleration = glm::vec3(0.0f);
    mLocation = pos;
    r = 2.0;
    maxspeed = 2.0f;
    maxforce = 0.03;

    float a = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    float c = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    mColor = glm::vec3(a,b,c);
    mWeights = glm::vec3(1.0f);
  }

  void update()
  {
    mVelocity +=mAcceleration;
    if(glm::length(mVelocity) > maxspeed)
    {
      glm::normalize(mVelocity);
      mVelocity = mVelocity*maxspeed;
    }
    mLocation += mVelocity;
    mAcceleration  = 0.0f*mAcceleration;
  }

  void applyForce(glm::vec3 force)
  {
    mAcceleration += force;
    if(glm::length(mAcceleration) > maxforce)
      {
        glm::normalize(mAcceleration);
        mAcceleration = mAcceleration*maxforce;
      }
  }

  glm::vec3 seek(glm::vec3 target)
  {
    glm::vec3 desired = target - mLocation;
    glm::normalize(desired);
    desired = desired*maxspeed;

    glm::vec3 steer = desired - mVelocity;
    if(glm::length(steer) > maxforce)
    {
      glm::normalize(steer);
      steer = maxforce*steer;
    }
    return steer;
  }

  glm::vec3 separate(std::vector<Boid*> boids)
  {
    float desiredSeparation = 25.0f;
    glm::vec3 steer(0.0f);
    int count = 0;

    for(Boid* other: boids)
    { 
      glm::vec3 diff = mLocation - other->mLocation;
      float d = glm::length(diff);
      if((d>0) && (d < desiredSeparation))
      {
        glm::normalize(diff);
        diff = (1.0f/d)*diff;
        steer += diff;
        count++; 
      }
    }

    if(count > 0)
    {
      steer = (1.0f/count)*steer;
    }

    if(glm::length(steer) > 0)
    {
      glm::normalize(steer);
      steer = maxspeed*steer;
      steer -= mVelocity;
      if(glm::length(steer) > maxforce)
      {
        glm::normalize(steer);
        steer = steer*maxforce;
      }
    }
    return steer;
  }

  glm::vec3 align(std::vector<Boid*> boids)
  {
    float neighbordist = 50;
    glm::vec3 sum(0.0f,0.0f,0.0f);
    int count = 0;
    for (Boid* other : boids) {
      float d = glm::length(mLocation - other->mLocation);
      if ((d > 0) && (d < neighbordist)) 
      {
        sum += other->mVelocity;
        count++;
      }
    }
    if (count > 0) {
      sum *= (1.0f/(float)count);
      // First two lines of code below could be condensed with new PVector setMag() method
      // Not using this method until Processing.js catches up
      // sum.setMag(maxspeed);

      // Implement Reynolds: Steering = Desired - Velocity
      glm::normalize(sum);
      sum = sum*maxspeed;
      glm::vec3 steer = sum - mVelocity;
      if(glm::length(steer) > maxforce)
      {
        glm::normalize(steer);
        steer = steer*maxforce;
      }
      return steer;
    } 
    else {
      return glm::vec3(0.0f);
    }
  }

  glm::vec3 cohesion(std::vector<Boid*> boids)
  {
    float neighbordist = 100;
    glm::vec3 sum(0.0f);   // Start with empty vector to accumulate all locations
    int count = 0;
    for (Boid* other : boids) {
      float d = glm::length(mLocation - other->mLocation);
      if ((d > 0) && (d < neighbordist)) {
        sum += other->mLocation; // Add location
        count++;
      }
    }
    if (count > 0) {
      sum *= (1.0f/(float)count);
      return seek(sum);  // Steer towards the location
    } 
    else {
      return glm::vec3(0.0f);
    }
  }

  glm::vec3 color()
  {
    return mColor;
  }

  void run(std::vector<Boid*> boids)
  {
    flock(boids, SteerState::NATURAL);
    update();
  }

  void setWeights(glm::vec3 biases)
  {
    mWeights = biases;
  }

  void setCohesionBias(float c)
  {
    mWeights[0] = c;
  }

  void setAlignmentBias(float a)
  {
    mWeights[1] = a;
  }

  void setSeparationBias(float s)
  {
    mWeights[2] = s;
  }

  void updateIncentiveState(glm::vec3 c, glm::vec3 a, glm::vec3 s)
  {
    cohIncentive = c;
    aliIncentive = a;
    sepIncentive = s;
  }

  void flock(std::vector<Boid*> boids, int steertype)
  {
    glm::vec3 sep = separate(boids);
    glm::vec3 ali = align(boids);
    glm::vec3 coh = cohesion(boids);

    if(steertype == SteerState::NATURAL)
    {

      sepIncentive = mWeights[0]*sep;
      aliIncentive = mWeights[1]*ali;
      cohIncentive = mWeights[2]*coh;


      applyForce(cohIncentive);
      applyForce(aliIncentive);
      applyForce(sepIncentive);
    }
    else if(steertype == SteerState::RETURN)
    {
      sepIncentive = (0.05f*sep);
      aliIncentive = (0.05f*ali);
      cohIncentive = (0.05f*coh);
      applyForce(1.0f*seek(camera->Position));
      applyForce(cohIncentive);
      applyForce(aliIncentive);
      applyForce(sepIncentive);
    }
    else if (steertype == SteerState::CHAOS)
    {
      float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
      sepIncentive = r*sep;
      r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
      aliIncentive = r*ali;
      r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
      cohIncentive = r*coh;
      r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
      float r1 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
      float r2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);


      applyForce(cohIncentive);
      applyForce(aliIncentive);
      applyForce(sepIncentive);
      applyForce(glm::vec3(r,r1,r2));
    }
    else
    {
      applyForce(sep);
      applyForce(ali);
      applyForce(coh);
    }
  }

  glm::mat4 model()
  {
    glm::mat4 model(1.0f);
    model = glm::translate(model, mLocation);
    return model;
  }
};

struct Flock
{
  std::vector<Boid*> boids;

  Flock()
  {}

  ~Flock()
  {
    for (std::vector<Boid*>::iterator i = boids.begin(); i != boids.end(); ++i)
    {
      delete (*i);
    }
  }

  void addBoid(Boid* b)
  {
    boids.push_back(b);
  }

  void addRandBoid()
  {
    float LO = -5.0f;
    float HI = 5.0f;
    float r1 = LO + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(HI-LO)));
    float r2 = LO + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(HI-LO)));
    float r3 = LO + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(HI-LO)));

    boids.push_back(new Boid(glm::vec3(r1,r2,-12.0)));
  }

};

// Function prototypes
void error_callback(int error, const char* description);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void Do_Movement();




Flock* flock = new Flock;
glm::mat4 projection; 

int main()
{
    // Init GLFW
 


  if (!glfwInit()) exit(EXIT_FAILURE);
  glfwSetErrorCallback(error_callback);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, 4);
  GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight,
                                        "Boids", nullptr, nullptr);
  CHECK_SUCCESS(window != nullptr);

  glfwMakeContextCurrent(window);
  
  glewExperimental = GL_TRUE;
  CHECK_SUCCESS(glewInit() == GLEW_OK);
  glGetError();  // clear GLEW's error for it

  glfwSetKeyCallback(window, key_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSwapInterval(1);
  const GLubyte* renderer = glGetString(GL_RENDERER);  // get renderer string
  const GLubyte* version = glGetString(GL_VERSION);    // version as a string
  std::cout << "Renderer: " << renderer << "\n";
  std::cout << "OpenGL version supported:" << version << "\n";

    // Setup and compile our shaders
    Shader ourShader = ResourceManager::LoadShader("shaders/mesh2d.vs", "shaders/mesh2d.frag", "shaders/mesh2d.geo", "mesh2d");
    ResourceManager::GetShader("mesh2d").SetVector3f("meshColor", glm::vec3(1.0f,1.0f,0.0f));
    // Set up our vertex data (and buffer(s)) and attribute pointers
    GLfloat vertices[] = {
         0.0f,  6.0f,  0.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         0.0f,  6.0f,  0.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,

         0.0f,  6.0f,  0.0f,
        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,

         0.0f,  6.0f,  0.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f, 

         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f
    };

    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // Bind our Vertex Array Object first, then bind and set our buffers and pointers.
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0); // Unbind VAO

    camera = new Camera(glm::vec3(0.0f, 0.0f, 3.0f));
    srand (static_cast <unsigned> (time(0)));
    

    for (int i = 0; i < 10; ++i)
    {
      flock->addRandBoid();
    }


    while(!glfwWindowShouldClose(window))
    {
        // Set frame time
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Check and call events
        glfwPollEvents();
        Do_Movement();

        // Clear the colorbuffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw our first triangle
        ourShader.Use();
        
        // Create camera transformation
        glm::vec4 light_position = glm::vec4(camera->Position, 1.0f);
        glm::mat4 view;
        view = camera->GetViewMatrix();
        projection = glm::perspective(camera->Zoom, (float)screenWidth/(float)screenHeight, 0.1f, 1000.0f);
        // projection = glm::ortho(0.0f, (float)screenWidth, (float)screenHeight, 0.0f);


        // Get the uniform locations
        // GLint modelLoc = glGetUniformLocation(ourShader.ID, "model");
        // GLint viewLoc = glGetUniformLocation(ourShader.ID, "view");
        // GLint projLoc = glGetUniformLocation(ourShader.ID, "projection");
        // Pass the matrices to the shader
        // glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
        // glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);
        ResourceManager::GetShader("mesh2d").SetMatrix4("projection", projection);
        ResourceManager::GetShader("mesh2d").SetMatrix4("view", view);


        ourShader.Use();
        glBindVertexArray(VAO);
        for (std::vector<Boid*>::iterator i = flock->boids.begin(); i != flock->boids.end(); ++i)
        {
            // Calculate the model matrix for each object and pass it to shader before drawing
            // glm::mat4 model;
            // model = glm::translate(model, cubePositions[i]);
            // GLfloat angle = 20.0f * i; 
            // model = glm::rotate(model, angle, glm::vec3(1.0f, 0.3f, 0.5f));
           

            // glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
            ResourceManager::GetShader("mesh2d").SetVector3f("meshColor", (*i)->color());
            ResourceManager::GetShader("mesh2d").SetMatrix4("model", (*i)->model());
            ResourceManager::GetShader("mesh2d").SetVector4f("light_position", light_position);

            glDrawArrays(GL_TRIANGLES, 0, 36);      
        }

        for (std::vector<Boid*>::iterator i = flock->boids.begin(); i != flock->boids.end(); ++i)
        {
          if(sim_state != SimulationState::PAUSE)
          {
            (*i)->flock(flock->boids, steer_state);
            (*i)->update(); 
          }
        }

        glBindVertexArray(0);
        // Swap the buffers
        glfwSwapBuffers(window);
    }
    // Properly de-allocate all resources once they've outlived their purpose
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return 0;
}

// Moves/alters the camera positions based on user input
void Do_Movement()
{
    // Camera controls
    if(keys[GLFW_KEY_W])
    {
      camera->ProcessKeyboard(FORWARD, deltaTime);
    }
    if(keys[GLFW_KEY_S])
    {
      camera->ProcessKeyboard(BACKWARD, deltaTime);
    }
    if(keys[GLFW_KEY_A])
    {
      camera->ProcessKeyboard(LEFT, deltaTime);
    }
    if(keys[GLFW_KEY_D])
    {
      camera->ProcessKeyboard(RIGHT, deltaTime);
    }
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    //cout << key << endl;
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
      glfwSetWindowShouldClose(window, GL_TRUE);
    else if(key == GLFW_KEY_P && action == GLFW_PRESS)
    {
      sim_state++;
      sim_state = sim_state%2;
    }
    else if(key == GLFW_KEY_N && action == GLFW_PRESS)
    {
      steer_state = SteerState::NATURAL;
    }
    else if(key == GLFW_KEY_R && action == GLFW_PRESS)
    {
      steer_state++;
      steer_state =  SteerState::RETURN;
    }
    else if(key == GLFW_KEY_C && action == GLFW_PRESS)
    {
      steer_state++;
      steer_state =  SteerState::CHAOS;
    }


    if (key >= 0 && key < 1024)
    {
        if(action == GLFW_PRESS)
            keys[key] = true;
        else if(action == GLFW_RELEASE)
            keys[key] = false;  
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if(firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos;  // Reversed since y-coordinates go from bottom to left
    
    lastX = xpos;
    lastY = ypos;

    camera->ProcessMouseMovement(xoffset, yoffset);
} 


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera->ProcessMouseScroll(yoffset);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) 
{
  drag_state = (action == GLFW_PRESS);
  current_button = button;

  if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
  {

      float x,y,z;
      x = (2.0f*lastX)/screenWidth-1.0f;
      y = 1.0f - (2.0f*lastY)/screenHeight;
      glm::vec3 ray_nds = glm::vec3(x,y,1.0f);
      glm::vec4 ray_clip = glm::vec4 (ray_nds.x,ray_nds.y, -1.0, 1.0);
      glm::vec4 ray_eye = glm::inverse (projection) * ray_clip;
      ray_eye = glm::vec4 (ray_eye.x, ray_eye.y, -1.0, 0.0);
      glm::vec4 ray_wor4 = glm::inverse(camera->GetViewMatrix()) * ray_eye;
      glm::vec3 ray_wor = glm::vec3(ray_wor4.x, ray_wor4.y, ray_wor4.z);
      // don't forget to normalise the vector at some point
      ray_wor = glm::normalize (ray_wor);


      flock->addBoid(new Boid((camera->Position + ray_wor) + camera->Front*10.0f));
  }
}

void error_callback(int error, const char* description) {
  std::cerr << "GLFW Error: " << description << "\n";
}

