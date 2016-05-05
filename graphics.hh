// //------------------------------

// Flock flock;

// void setup() {
//   size(640, 360);
//   flock = new Flock();
//   // Add an initial set of boids into the system
//   for (int i = 0; i < 150; i++) {
//     flock.addBoid(new Boid(width/2,height/2));
//   }
// }

// void draw() {
//   background(50);
//   flock.run();
// }

// // Add a new boid into the System
// void mousePressed() {
//   flock.addBoid(new Boid(mouseX,mouseY));
// }



// // The Flock (a list of Boid objects)

// class Flock {
//   ArrayList<Boid> boids; // An ArrayList for all the boids

//   Flock() {
//     boids = new ArrayList<Boid>(); // Initialize the ArrayList
//   }

//   void run() {
//     for (Boid b : boids) {
//       b.run(boids);  // Passing the entire list of boids to each boid individually
//     }
//   }

//   void addBoid(Boid b) {
//     boids.add(b);
//   }

// };




// // The Boid class

// class Boid {

//   PVector location;
//   PVector velocity;
//   PVector acceleration;
//   float r;
//   float maxforce;    // Maximum steering force
//   float maxspeed;    // Maximum speed

//     Boid(float x, float y) {
//     acceleration = new PVector(0, 0);

//     // This is a new PVector method not yet implemented in JS
//     // velocity = PVector.random2D();

//     // Leaving the code temporarily this way so that this example runs in JS
//     float angle = random(TWO_PI);
//     velocity = new PVector(cos(angle), sin(angle));

//     location = new PVector(x, y);
//     r = 2.0;
//     maxspeed = 2;
//     maxforce = 0.03;
//   }

//   void run(ArrayList<Boid> boids) {
//     flock(boids);
//     update();
//     borders();
//     render();
//   }

//   void applyForce(PVector force) {
//     // We could add mass here if we want A = F / M
//     acceleration.add(force);
//   }

//   // We accumulate a new acceleration each time based on three rules
//   void flock(ArrayList<Boid> boids) {
//     PVector sep = separate(boids);   // Separation
//     PVector ali = align(boids);      // Alignment
//     PVector coh = cohesion(boids);   // Cohesion
//     // Arbitrarily weight these forces
//     sep.mult(1.5);
//     ali.mult(1.0);
//     coh.mult(1.0);
//     // Add the force vectors to acceleration
//     applyForce(sep);
//     applyForce(ali);
//     applyForce(coh);
//   }

//   // Method to update location
//   void update() {
//     // Update velocity
//     velocity.add(acceleration);
//     // Limit speed
//     velocity.limit(maxspeed);
//     location.add(velocity);
//     // Reset accelertion to 0 each cycle
//     acceleration.mult(0);
//   }

//   // A method that calculates and applies a steering force towards a target
//   // STEER = DESIRED MINUS VELOCITY
//   PVector seek(PVector target) {
//     PVector desired = PVector.sub(target, location);  // A vector pointing from the location to the target
//     // Scale to maximum speed
//     desired.normalize();
//     desired.mult(maxspeed);

//     // Above two lines of code below could be condensed with new PVector setMag() method
//     // Not using this method until Processing.js catches up
//     // desired.setMag(maxspeed);

//     // Steering = Desired minus Velocity
//     PVector steer = PVector.sub(desired, velocity);
//     steer.limit(maxforce);  // Limit to maximum steering force
//     return steer;
//   }

//   void render() {
//     // Draw a triangle rotated in the direction of velocity
//     float theta = velocity.heading2D() + radians(90);
//     // heading2D() above is now heading() but leaving old syntax until Processing.js catches up
    
//     fill(200, 100);
//     stroke(255);
//     pushMatrix();
//     translate(location.x, location.y);
//     rotate(theta);
//     beginShape(TRIANGLES);
//     vertex(0, -r*2);
//     vertex(-r, r*2);
//     vertex(r, r*2);
//     endShape();
//     popMatrix();
//   }

//   // Wraparound
//   void borders() {
//     if (location.x < -r) location.x = width+r;
//     if (location.y < -r) location.y = height+r;
//     if (location.x > width+r) location.x = -r;
//     if (location.y > height+r) location.y = -r;
//   }

//   // Separation
//   // Method checks for nearby boids and steers away
//   PVector separate (ArrayList<Boid> boids) {
//     float desiredseparation = 25.0f;
//     PVector steer = new PVector(0, 0, 0);
//     int count = 0;
//     // For every boid in the system, check if it's too close
//     for (Boid other : boids) {
//       float d = PVector.dist(location, other.location);
//       // If the distance is greater than 0 and less than an arbitrary amount (0 when you are yourself)
//       if ((d > 0) && (d < desiredseparation)) {
//         // Calculate vector pointing away from neighbor
//         PVector diff = PVector.sub(location, other.location);
//         diff.normalize();
//         diff.div(d);        // Weight by distance
//         steer.add(diff);
//         count++;            // Keep track of how many
//       }
//     }
//     // Average -- divide by how many
//     if (count > 0) {
//       steer.div((float)count);
//     }

//     // As long as the vector is greater than 0
//     if (steer.mag() > 0) {
//       // First two lines of code below could be condensed with new PVector setMag() method
//       // Not using this method until Processing.js catches up
//       // steer.setMag(maxspeed);

//       // Implement Reynolds: Steering = Desired - Velocity
//       steer.normalize();
//       steer.mult(maxspeed);
//       steer.sub(velocity);
//       steer.limit(maxforce);
//     }
//     return steer;
//   }

//   // Alignment
//   // For every nearby boid in the system, calculate the average velocity
//   PVector align (ArrayList<Boid> boids) {
//     float neighbordist = 50;
//     PVector sum = new PVector(0, 0);
//     int count = 0;
//     for (Boid other : boids) {
//       float d = PVector.dist(location, other.location);
//       if ((d > 0) && (d < neighbordist)) {
//         sum.add(other.velocity);
//         count++;
//       }
//     }
//     if (count > 0) {
//       sum.div((float)count);
//       // First two lines of code below could be condensed with new PVector setMag() method
//       // Not using this method until Processing.js catches up
//       // sum.setMag(maxspeed);

//       // Implement Reynolds: Steering = Desired - Velocity
//       sum.normalize();
//       sum.mult(maxspeed);
//       PVector steer = PVector.sub(sum, velocity);
//       steer.limit(maxforce);
//       return steer;
//     } 
//     else {
//       return new PVector(0, 0);
//     }
//   }

//   // Cohesion
//   // For the average location (i.e. center) of all nearby boids, calculate steering vector towards that location
//   PVector cohesion (ArrayList<Boid> boids) {
//     float neighbordist = 50;
//     PVector sum = new PVector(0, 0);   // Start with empty vector to accumulate all locations
//     int count = 0;
//     for (Boid other : boids) {
//       float d = PVector.dist(location, other.location);
//       if ((d > 0) && (d < neighbordist)) {
//         sum.add(other.location); // Add location
//         count++;
//       }
//     }
//     if (count > 0) {
//       sum.div(count);
//       return seek(sum);  // Steer towards the location
//     } 
//     else {
//       return new PVector(0, 0);
//     }
//   }
// };
// //------------------------------

class SpriteRenderer
{
    private:
      Shader shader; 
      GLuint quadVAO;

    public:
      SpriteRenderer(Shader sh)
      {
        shader = sh;
        this->initRenderData();
      }

      ~SpriteRenderer()
      {}

      void DrawSprite(Texture2D texture, glm::vec2 position, glm::vec2 size = glm::vec2(10, 10), GLfloat rotate = 0.0f, glm::vec3 color = glm::vec3(1.0f))
      {
        // Prepare transformations
        this->shader.Use();
        glm::mat4 model;
        model = glm::translate(model, glm::vec3(position, 0.0f));  

        model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f)); 
        model = glm::rotate(model, rotate, glm::vec3(0.0f, 0.0f, 1.0f)); 
        model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));

        model = glm::scale(model, glm::vec3(size, 1.0f)); 
      
        this->shader.SetMatrix4("model", model);
        this->shader.SetVector3f("spriteColor", color);
      
        glActiveTexture(GL_TEXTURE0);
        texture.Bind();

        glBindVertexArray(this->quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
      }

      void initRenderData()
      {
        // Configure VAO/VBO
        GLuint VBO;
        GLfloat vertices[] = { 
          0.0f, 1.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 
        
            0.0f, 1.0f, 0.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 0.0f, 1.0f, 0.0f
        };

        glGenVertexArrays(1, &this->quadVAO);
        glGenBuffers(1, &VBO);
        
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindVertexArray(this->quadVAO);
        glEnableVertexAttribArray(0);
        // glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);  
        glBindVertexArray(0);
      }
};


// struct Mesh
// {
//   std::vector<glm::vec4> vertices;
//   std::vector<glm::uvec3> faces;
//   GLuint vao;
//   GLuint vbos[kNumVbos]; 

//   Mesh()
//   {
//   }

//   ~Mesh()
//   {}

//   void load(std::string filename)
//   {
//     LoadObj(filename, vertices, faces);
//   }

//   void setup()
//   {
//      // Setup our VAO.
//     CHECK_GL_ERROR(glGenVertexArrays(1, &vao));

//     // Switch to the VAO.
//     CHECK_GL_ERROR(glBindVertexArray(vao));

//     // Generate buffer objects
//     CHECK_GL_ERROR(glGenBuffers(2, vbos));

//     // Setup vertex data in a VBO.
//     CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vbos[kVertexBuffer]));
//     // NOTE: We do not send anything right now, we just describe it to OpenGL.
//     CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
//                                 sizeof(float) * vertices.size() * 4, nullptr,
//                                 GL_STATIC_DRAW));
//     CHECK_GL_ERROR(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0));
//     CHECK_GL_ERROR(glEnableVertexAttribArray(0));

//     // Setup element array buffer.
//     CHECK_GL_ERROR(
//         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos[kIndexBuffer]));
//     CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
//                                 sizeof(uint32_t) * faces.size() * 3,
//                                 &faces[0], GL_STATIC_DRAW));
//   }

//   void bindVao()
//   {
//     CHECK_GL_ERROR(glBindVertexArray(vao));
//   }

//   void unbind()
//   {
//     CHECK_GL_ERROR(glBindVertexArray(0));
//   }

//   void draw(Shader shader, glm::mat4 projection, glm::mat4 view, glm::vec3 color)
//   {   bindVao();
//       shader.SetMatrix4("projection", projection);
//       shader.SetMatrix4("view", view);
//       shader.SetVector3f("meshColor", color);
//       CHECK_GL_ERROR(
//         glDrawElements(GL_TRIANGLES, faces.size() * 3, GL_UNSIGNED_INT, 0));
//       unbind();
//   }

//   void bindBuffers()
//   {
//     CHECK_GL_ERROR(
//         glBindBuffer(GL_ARRAY_BUFFER, vbos[kVertexBuffer]));
//     CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
//                                 sizeof(float) * vertices.size() * 4,
//                                 &vertices[0], GL_STATIC_DRAW));
//   }
// };



// class Mesh2d
// {
//   private:
//     Shader shader;
//     GLuint buffer_objects[kNumVbos];
//     std::vector<glm::vec3> vertices;
//     std::vector<glm::uvec3> faces;
//     GLuint VAO;
//   public:
//     Mesh2d(std::string filename, Shader s)
//     {
//       this->shader = s;

//       LoadObj(filename, this->vertices, this->faces);
//       // Setup our VAOs.
//       std::cout << "vertices size:: " <<vertices.size()<< " indicies size:: "<<faces.size()<<std::endl;

//       CHECK_GL_ERROR(glGenVertexArrays(1, &VAO));

//       // Setup the object array object.

//       CHECK_GL_ERROR(glBindVertexArray(VAO));

//       CHECK_GL_ERROR(glGenBuffers(kNumVbos, &buffer_objects[0]));

//       // Setup vertex data in a VBO.
//       CHECK_GL_ERROR(
//           glBindBuffer(GL_ARRAY_BUFFER, buffer_objects[kVertexBuffer]));
//       CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
//                                   sizeof(float) * vertices.size() * 3,
//                                   &vertices[0], GL_STATIC_DRAW));
//       CHECK_GL_ERROR(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0));
//       CHECK_GL_ERROR(glEnableVertexAttribArray(0));

//       // Setup element array buffer.
//       CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
//                                   buffer_objects[kIndexBuffer]));
//       CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
//                                   sizeof(uint32_t) * faces.size() * 3,
//                                   &faces[0], GL_STATIC_DRAW));
//     }

//     void DrawMesh(glm::vec2 position, glm::vec2 size = glm::vec2(10, 10), GLfloat rotate = 0.0f, glm::vec3 color = glm::vec3(1.0f), Texture2D* texture = NULL)
//     {
//       this->shader.Use();
//       glm::mat4 model;
//       model = glm::translate(model, glm::vec3(position, 0.0f));  

//       model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f)); 
//       model = glm::rotate(model, rotate, glm::vec3(0.0f, 0.0f, 1.0f)); 
//       model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));

//       model = glm::scale(model, glm::vec3(size, 1.0f)); 
    
//       this->shader.SetMatrix4("model", model);
//       this->shader.SetVector3f("meshColor", color);
//       // glm::mat4 projection = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 5.0f);
//       // glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, 0.1f, 100.0f);
//       glm::mat4 projection = glm::mat4(1.0f);

//       this->shader.SetMatrix4("projection", projection);


//       if(texture)
//       {
//         glActiveTexture(GL_TEXTURE0);
//         texture->Bind();
//       }

//       glBindVertexArray(this->VAO);
//       CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, this->faces.size() * 3, GL_UNSIGNED_INT, 0)); 
//       // glDrawArrays(GL_TRIANGLES, 0, vertices.size()*3);
//       glBindVertexArray(0);
//     }
// };
//------------------------------------------------------------------------------------------------

// // GLFW function declerations
// void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);



// SpriteRenderer* Renderer;
// TriangleRenderer* TriRender;


// Mesh2d* mesh1;
// Mesh2d* mesh2;
// Mesh2d* mesh3;


void init()
{


  // glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(SCREEN_WIDTH), 
  //     static_cast<GLfloat>(SCREEN_HEIGHT), 0.0f, -100.0f, 100.0f);
  

  // // Load shaders
  // ResourceManager::LoadShader("shaders/triangle.vs", "shaders/triangle.frag", nullptr, "tri");
  // ResourceManager::LoadShader("shaders/sprite.vs", "shaders/sprite.frag", nullptr, "sprite");
  
  // // Configure shaders
  // ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
  // ResourceManager::GetShader("tri").Use().SetInteger("image", 0);

  // ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
  // ResourceManager::GetShader("tri").SetMatrix4("projection", projection);
  // ResourceManager::LoadTexture("textures/awesomeface.png", GL_TRUE, "face");

  // Shader triangleShader;
  // triangleShader = ResourceManager::GetShader("tri");
  // TriRender = new TriangleRenderer(triangleShader);

  // // Set render-specific controls
  // Shader spriteShader;
  // spriteShader = ResourceManager::GetShader("sprite");
  // Renderer = new SpriteRenderer(spriteShader);  // Load textures




 

  // ResourceManager::LoadShader("shaders/mesh2d.vs", "shaders/mesh2d.frag", nullptr, "mesh2d");
  // ResourceManager::GetShader("mesh2d").SetMatrix4("projection", projection);
  // Shader meshShader;
  // meshShader = ResourceManager::GetShader("mesh2d");

  // std::string objfilename = "obj/alligator.obj";
  // mesh1 = new Mesh2d(objfilename, meshShader);

  // objfilename = "obj/bunny.obj";
  // mesh2 = new Mesh2d(objfilename, meshShader);

  // objfilename = "obj/triangle.obj";
  // mesh3 = new Mesh2d(objfilename, meshShader);




}

// void render()
// {
//   Renderer->DrawSprite(ResourceManager::GetTexture("face"), 
//         glm::vec2(200, 200), glm::vec2(100, 100), 0.45f, glm::vec3(0.0f, 1.0f, 0.0f));

//   // mesh1->DrawMesh(glm::vec2(0.0f, 0.0f), glm::vec2(0.5f, 0.25f), 0.0f, glm::vec3(0.0f,0.6f,0.4f));
//   // mesh2->DrawMesh(glm::vec2(0.2f, 0.5f), glm::vec2(1.0f, 1.0f), 0.0f, glm::vec3(0.0f,0.6f,0.0f));
//   // mesh3->DrawMesh(glm::vec2(0.7f, 0.2f), glm::vec2(1.0f, 1.0f), 0.0f, glm::vec3(1.0f,0.0f,0.0f));
//   TriRender->DrawTriangle(ResourceManager::GetTexture("face"), glm::vec2(200, 200), glm::vec2(50, 50), 0.0f, glm::vec3(1.0f,1.0f,1.0f));
// }

// void cleanup()
// {
//   delete Renderer;
//   delete TriRender;
//   delete mesh1;
//   delete mesh2;
//   delete mesh3;

// }
// 
// 

// int main(int argc, char* argv[]) {
//   std::string file;
//   if(argc > 1)
//   {
//      file = std::string(argv[1]);
//      std::cout << "file = " << file << "\n";
//   }

//   // Set up OpenGL context
//   if (!glfwInit()) 
//     exit(EXIT_FAILURE);
  
//   glfwSetErrorCallback(error_callback);
//   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
//   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//   GLFWwindow* window = glfwCreateWindow(window_width, window_height,
//                                         &window_title[0], nullptr, nullptr);
//   CHECK_SUCCESS(window != nullptr);
//   glfwMakeContextCurrent(window);
//   glewExperimental = GL_TRUE;
//   CHECK_SUCCESS(glewInit() == GLEW_OK);
//   glGetError();  // clear GLEW's error for it
//   glfwSetKeyCallback(window, key_callback);
//   glfwSwapInterval(1);
//   //-------------------------------------------------------------------------
//   const GLubyte* renderer = glGetString(GL_RENDERER);  // get renderer string
//   const GLubyte* version = glGetString(GL_VERSION);    // version as a string
//   const GLubyte* glsl_version =
//       glGetString(GL_SHADING_LANGUAGE_VERSION);  // version as a
//                                                  // string
//   std::cout << "Renderer: " << renderer << "\n";
//   std::cout << "OpenGL version supported:" << version << "\n";
//   std::cout << "GLSL version supported:" << glsl_version << "\n";
//   //-------------------------------------------------------------------------
  

//   // Load geometry to render
//   std::vector<glm::vec3> obj_vertices;
//   std::vector<glm::uvec3> obj_faces; 
//   LoadObj(file, obj_vertices, obj_faces);
//   std::cout << "Found " << obj_vertices.size() << " vertices and "
//             << obj_faces.size() << " faces.\n";

//   // Create Vertex Array Object
//   CHECK_GL_ERROR(glGenVertexArrays(1, &vao));
//   CHECK_GL_ERROR(glBindVertexArray(vao));

//   // Create Vertex Buffer Objects
//   CHECK_GL_ERROR(glGenBuffers(2, buffer_objects));

//   // Vertex positions
//   CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, buffer_objects[kVertexBuffer]));
//   // NOTE: We do not send anything right now, we just describe it to OpenGL.
//   CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
//                               sizeof(float) * obj_vertices.size() * 3, // total size of the position buffer
//            nullptr, // don't provide data yet, we will pass it in during the rendering loop
//                               GL_STATIC_DRAW));
//   CHECK_GL_ERROR(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0)); // Assign position buffer to vertex attribute 0
//   CHECK_GL_ERROR(glEnableVertexAttribArray(0)); 

//   // Triangle indices
//   CHECK_GL_ERROR(
//       glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_objects[kIndexBuffer]));
//   CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
//                               sizeof(uint32_t) * obj_faces.size() * 3, // total size of the triangle index buffer
//                               &obj_faces[0], // pointer to the data to pass to the GPU
//            GL_STATIC_DRAW));


//   // Create shader program
//   GLuint program_id = 0;
//   CHECK_GL_ERROR(program_id = glCreateProgram());
    
//   // Compile shaders and attach to shader program
//   // One vertex shader
//   GLuint vertex_shader_id = 0;
//   const char* vertex_source_pointer = vertex_shader;
//   CHECK_GL_ERROR(vertex_shader_id = glCreateShader(GL_VERTEX_SHADER));
//   CHECK_GL_ERROR(
//       glShaderSource(vertex_shader_id, 1, &vertex_source_pointer, nullptr));
//   glCompileShader(vertex_shader_id);
//   CHECK_GL_SHADER_ERROR(vertex_shader_id);

//   // one geometry shader
//   GLuint geometry_shader_id = 0;
//   const char* geometry_source_pointer = geometry_shader;
//   CHECK_GL_ERROR(geometry_shader_id = glCreateShader(GL_GEOMETRY_SHADER));
//   CHECK_GL_ERROR(
//       glShaderSource(geometry_shader_id, 1, &geometry_source_pointer, nullptr));
//   glCompileShader(geometry_shader_id);
//   CHECK_GL_SHADER_ERROR(geometry_shader_id);

//   // one fragment shader
//   GLuint fragment_shader_id = 0;
//   const char* fragment_source_pointer = fragment_shader;
//   CHECK_GL_ERROR(fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER));
//   CHECK_GL_ERROR(
//       glShaderSource(fragment_shader_id, 1, &fragment_source_pointer, nullptr));
//   glCompileShader(fragment_shader_id);
//   CHECK_GL_SHADER_ERROR(fragment_shader_id);

//   CHECK_GL_ERROR(glAttachShader(program_id, vertex_shader_id));
//   CHECK_GL_ERROR(glAttachShader(program_id, fragment_shader_id));
//   CHECK_GL_ERROR(glAttachShader(program_id, geometry_shader_id));

//   // Link shader program
//   CHECK_GL_ERROR(glBindAttribLocation(program_id, 0, "vertex_position"));
//   CHECK_GL_ERROR(glBindFragDataLocation(program_id, 0, "fragment_color"));
//   glLinkProgram(program_id);
//   CHECK_GL_PROGRAM_ERROR(program_id);
//   GLint view_projection_matrix_location = 0;
//   CHECK_GL_ERROR(view_projection_matrix_location =
//                      glGetUniformLocation(program_id, "view_projection"));
//   GLint light_position_location = 0;
//   CHECK_GL_ERROR(light_position_location =
//                      glGetUniformLocation(program_id, "light_position"));


//   camera = new Camera;


//   // Set up camera and light (ignore for now)
//   glm::vec3 min_bounds = glm::vec3(std::numeric_limits<float>::max());
//   glm::vec3 max_bounds = glm::vec3(-std::numeric_limits<float>::max());
//   for (int i = 0; i < obj_vertices.size(); ++i) {
//     min_bounds = glm::min(obj_vertices[i], min_bounds);
//     max_bounds = glm::max(obj_vertices[i], max_bounds);
//   }
//   std::cout << "min_bounds = " << glm::to_string(min_bounds) << "\n";
//   std::cout << "max_bounds = " << glm::to_string(max_bounds) << "\n";
//   std::cout << "center = " << glm::to_string(0.5f * (min_bounds + max_bounds))
//             << "\n";

//   glm::vec3 light_position = glm::vec3(10.0f, 0.0f, 10.0f);
//   glm::vec3 eye = glm::vec3(0.0f, 0.1f, 0.4f);
//   glm::vec3 look = glm::vec3(0.0f, 0.1f, 0.0f);
//   glm::vec3 up = glm::vec3(0.0f, 0.1f, 0.4f);
//   glm::mat4 view_matrix = glm::lookAt(eye, look, up);

//   float aspect = static_cast<float>(window_width) / window_height;
//   glm::mat4 projection_matrix = glm::perspective(glm::radians(45.0f), aspect, 0.0001f, 1000.0f);
//   // glm::mat4 projection_matrix = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f);
//   glm::mat4 view_projection_matrix = projection_matrix * view_matrix;

//   while (!glfwWindowShouldClose(window)) 
//   {
//     // Clear screen
//     glfwGetFramebufferSize(window, &window_width, &window_height);
//     glViewport(0, 0, window_width, window_height);
//     glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
//     glEnable(GL_DEPTH_TEST);
//     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//     glDepthFunc(GL_LESS);

//     // Tell OpenGL what shader program to use
//     CHECK_GL_ERROR(glUseProgram(program_id));

//     // Tell OpenGL what to render
//     CHECK_GL_ERROR(
//         glBindBuffer(GL_ARRAY_BUFFER, buffer_objects[kVertexBuffer]));
//     CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
//                                 sizeof(float) * obj_vertices.size() * 3, // same size as before
//                                 &obj_vertices[0], // this time we do pass the vertex position data
//        GL_STATIC_DRAW));

//     // Pass in global variables
//     CHECK_GL_ERROR(glUniformMatrix4fv(view_projection_matrix_location, 1,
//                                       GL_FALSE, &view_projection_matrix[0][0]));
//     CHECK_GL_ERROR(
//         glUniform3fv(light_position_location, 1, &light_position[0]));
//     CHECK_GL_ERROR(glBindVertexArray(vao));

//     // Render!
//     CHECK_GL_ERROR(
//         glDrawElements(GL_TRIANGLES, obj_faces.size() * 3, GL_UNSIGNED_INT, 0));


//     glfwPollEvents();
//     glfwSwapBuffers(window);
//   }

//   glfwDestroyWindow(window);
//   glfwTerminate();
//   exit(EXIT_SUCCESS);
// }

// void ErrorCallback(int error, const char* description) {
//   std::cerr << "GLFW Error: " << description << "\n";
// }

// void KeyCallback(GLFWwindow* window, int key, int scancode, int action,
//                  int mods) {
//   if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
//     glfwSetWindowShouldClose(window, GL_TRUE);
// }




// int main(int argc, char *argv[])
// {
//     glfwInit();
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//     glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//     glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

//     GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "2D Animation", nullptr, nullptr);
//     glfwMakeContextCurrent(window);

//     glewExperimental = GL_TRUE;
//     glewInit();
//     glGetError(); // Call it once to catch glewInit() bug, all other errors are now from our application.

//     glfwSetKeyCallback(window, key_callback);

//     // OpenGL configuration
//     glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
//     // glEnable(GL_CULL_FACE);
//     // glEnable(GL_BLEND);
//     // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


//     // DeltaTime variables
//     GLfloat deltaTime = 0.0f;
//     GLfloat lastFrame = 0.0f;

//     init();

//     while (!glfwWindowShouldClose(window))
//     {
//         // Calculate delta time
//         GLfloat currentFrame = glfwGetTime();
//         deltaTime = currentFrame - lastFrame;
//         lastFrame = currentFrame;
//         glfwPollEvents();



//         // Render
//         glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
//         glClear(GL_COLOR_BUFFER_BIT);

//         render();
//         glfwSwapBuffers(window);
//     }

//     // Delete all resources as loaded using the resource manager
//     ResourceManager::Clear();
//     cleanup();
//     glfwTerminate();
//     return 0;
// }

// void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
// {
//     // When a user presses the escape key, we set the WindowShouldClose property to true, closing the application
//     if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
//         glfwSetWindowShouldClose(window, GL_TRUE);
// }

//-------------------------------------------------------------------------------




class TriangleRenderer
{
    private:
      Shader shader; 
      GLuint quadVAO;

    public:
      TriangleRenderer(Shader sh)
      {
        shader = sh;
        this->initRenderData();
      }

      ~TriangleRenderer()
      {}

      void DrawTriangle(Texture2D texture, glm::vec2 position, glm::vec2 size = glm::vec2(10, 10), GLfloat rotate = 0.0f, glm::vec3 color = glm::vec3(1.0f))
      {
        // Prepare transformations
        this->shader.Use();
        glm::mat4 model;
        model = glm::translate(model, glm::vec3(position, 0.0f));  

        model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f)); 
        model = glm::rotate(model, rotate, glm::vec3(0.0f, 0.0f, 1.0f)); 
        model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));

        model = glm::scale(model, glm::vec3(size, 1.0f)); 
      
        this->shader.SetMatrix4("model", model);
        this->shader.SetVector3f("triColor", color);
      
        glActiveTexture(GL_TEXTURE0);
        texture.Bind();

        glBindVertexArray(this->quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
      }

      void initRenderData()
      {
        // Configure VAO/VBO
        GLuint VBO;
        GLfloat vertices[] = { 
            0.0f, 1.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 
        
            0.0f, 1.0f, 0.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 0.0f, 1.0f, 0.0f
        };

        glGenVertexArrays(1, &this->quadVAO);
        glGenBuffers(1, &VBO);
        
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindVertexArray(this->quadVAO);
        glEnableVertexAttribArray(0);
        // glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);  
        glBindVertexArray(0);
      }
};
