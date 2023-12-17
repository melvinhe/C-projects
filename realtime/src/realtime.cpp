#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include "settings.h"
#include "utils/shaderloader.h"
#include "glm/gtx/transform.hpp"
#include "utils/sceneparser.h"
#include "shapes/sphere.h"
#include "shapes/cone.h"
#include "shapes/cylinder.h"
#include "shapes/cube.h"


// ================== Project 5: Lights, Camera

Realtime::Realtime(QWidget *parent)
    : QOpenGLWidget(parent)
{
    m_prev_mouse_pos = glm::vec2(size().width()/2, size().height()/2);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_keyMap[Qt::Key_W]       = false;
    m_keyMap[Qt::Key_A]       = false;
    m_keyMap[Qt::Key_S]       = false;
    m_keyMap[Qt::Key_D]       = false;
    m_keyMap[Qt::Key_Control] = false;
    m_keyMap[Qt::Key_Space]   = false;

    // If you must use this function, do not edit anything above this
}

void Realtime::finish() {
    killTimer(m_timer);
    this->makeCurrent();

    // Students: anything requiring OpenGL calls when the program exits should be done here
    // Delete OpenGL resources (VBOs, VAOs, Shader Program, etc.)
    for (auto const &[type, vbo] : m_vbos) {
        glDeleteBuffers(1, &vbo);
    }
    m_vbos.clear();

    for (auto const &[type, vao] : m_vaos) {
        glDeleteVertexArrays(1, &vao);
    }
    m_vaos.clear();

    glDeleteProgram(m_shader);
    glDeleteProgram(m_texture_shader);
    glDeleteVertexArrays(1, &m_fullscreen_vao);
    glDeleteBuffers(1, &m_fullscreen_vbo);

    // Delete OpenGL memory here
    glDeleteTextures(1, &m_fbo_texture);
    glDeleteRenderbuffers(1, &m_fbo_renderbuffer);
    glDeleteFramebuffers(1, &m_fbo);
    this->doneCurrent();
}

void Realtime::initializeGL() {
    m_devicePixelRatio = this->devicePixelRatio();

    m_timer = startTimer(1000/60);
    m_elapsedTimer.start();

    // Initialize GL extension wrangler
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error while initializing GL: " << glewGetErrorString(err) << std::endl;
    }
    std::cout << "Initialized GL: Version " << glewGetString(GLEW_VERSION) << std::endl;
    // Clear values
    m_glmInit = true;
    glClearColor(0,0,0,1);
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    m_shader = ShaderLoader::createShaderProgram(":/resources/shaders/default.vert", ":/resources/shaders/default.frag");
    m_defaultFBO = 2;
    m_screen_width = size().width() * m_devicePixelRatio;
    m_screen_height = size().height() * m_devicePixelRatio;
    m_fbo_width = m_screen_width;
    m_fbo_height = m_screen_height;
    m_texture_shader = ShaderLoader::createShaderProgram(":/resources/shaders/texture.vert", ":/resources/shaders/texture.frag");

    updateShapes();
    glUseProgram(m_texture_shader);
    glUniform1i(glGetUniformLocation(m_texture_shader, "ourTexture"), 0);
    glUseProgram(0);
    std::vector<GLfloat> fullscreen_quad_data =
        { //     POSITIONS      //     UV COORDINATES  //
            -1.0f,  1.0f, 0.0f,   0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f,   0.0f, 0.0f,
            1.0f, -1.0f, 0.0f,   1.0f, 0.0f,
            1.0f,  1.0f, 0.0f,   1.0f, 1.0f,
            -1.0f,  1.0f, 0.0f,   0.0f, 1.0f,
            1.0f, -1.0f, 0.0f,   1.0f, 0.0f
        };

    // Generate and bind a VBO and a VAO for a fullscreen quad
    glGenBuffers(1, &m_fullscreen_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_fullscreen_vbo);
    glBufferData(GL_ARRAY_BUFFER, fullscreen_quad_data.size()*sizeof(GLfloat), fullscreen_quad_data.data(), GL_STATIC_DRAW);
    glGenVertexArrays(1, &m_fullscreen_vao);
    glBindVertexArray(m_fullscreen_vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);
    // Add the second attribute (UV coordinates)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    // Unbind the fullscreen quad's VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    makeFBO();
}

void Realtime::makeFBO(){
    // Generate and bind an empty texture, set its min/mag filter interpolation, then unbind
    glGenTextures(1, &m_fbo_texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_fbo_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_fbo_width, m_fbo_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    // Generate and bind a renderbuffer of the right size, set its format, then unbind
    glGenRenderbuffers(1, &m_fbo_renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_fbo_renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_fbo_width, m_fbo_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    // Generate and bind an FBO
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    // Add our texture as a color attachment, and our renderbuffer as a depth+stencil attachment, to our FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fbo_texture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_fbo_renderbuffer);
    // Unbind the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void Realtime::paintGL() {
    // Students: anything requiring OpenGL calls every frame should be done here
    if (!m_glmInit) {
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_fbo_width, m_fbo_height);
    // Clear screen color and depth before painting
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::mat4 viewMatrix = m_camera.getViewMatrix();
    glm::mat4 projMatrix = m_camera.getProjMatrix(settings.nearPlane, settings.farPlane);
    for (int i = 0; i < m_renderData.shapes.size(); i++) {
        auto &shape = m_renderData.shapes[i];
        glBindVertexArray(m_vaos[shape.primitive.type]);
        glUseProgram(m_shader);
        glUniformMatrix4fv(glGetUniformLocation(m_shader, "model"), 1, GL_FALSE, &shape.ctm[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(m_shader, "view"), 1, GL_FALSE, &viewMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(m_shader, "projection"), 1, GL_FALSE, &projMatrix[0][0]);

        glUniform4fv(glGetUniformLocation(m_shader, "cAmbient"), 1, &shape.primitive.material.cAmbient[0]);
        glUniform4fv(glGetUniformLocation(m_shader, "cDiffuse"), 1, &shape.primitive.material.cDiffuse[0]);
        glUniform4fv(glGetUniformLocation(m_shader, "cSpecular"), 1, &shape.primitive.material.cSpecular[0]);

        glUniform1f(glGetUniformLocation(m_shader, "k_a"), m_renderData.globalData.ka);
        glUniform1f(glGetUniformLocation(m_shader, "k_d"), m_renderData.globalData.kd);
        glUniform1f(glGetUniformLocation(m_shader, "k_s"), m_renderData.globalData.ks);

        glUniform1f(glGetUniformLocation(m_shader, "shininess"), shape.primitive.material.shininess);
        glUniform4fv(glGetUniformLocation(m_shader, "cameraPos"), 1, &glm::inverse(viewMatrix)[3][0]);

        int numLights = static_cast<int>(m_renderData.lights.size());
        glUniform1i(glGetUniformLocation(m_shader, "numLights"), numLights);
        for (int j = 0; j < numLights; j++) {
            glUniform1i(glGetUniformLocation(m_shader, ("lights[" + std::to_string(j) + "].type").c_str()), static_cast<int>(m_renderData.lights[j].type));
            glUniform4fv(glGetUniformLocation(m_shader, ("lights[" + std::to_string(j) + "].position").c_str()), 1, &m_renderData.lights[j].pos[0]);
            glUniform4fv(glGetUniformLocation(m_shader, ("lights[" + std::to_string(j) + "].direction").c_str()), 1, &m_renderData.lights[j].dir[0]);
            glUniform4fv(glGetUniformLocation(m_shader, ("lights[" + std::to_string(j) + "].color").c_str()), 1, &m_renderData.lights[j].color[0]);
            glUniform3fv(glGetUniformLocation(m_shader, ("lights[" + std::to_string(j) + "].attenuation").c_str()), 1, &m_renderData.lights[j].function[0]);
            glUniform1f(glGetUniformLocation(m_shader, ("lights[" + std::to_string(j) + "].angle").c_str()), m_renderData.lights[j].angle);
            glUniform1f(glGetUniformLocation(m_shader, ("lights[" + std::to_string(j) + "].penumbra").c_str()), m_renderData.lights[j].penumbra);

        }

        glDrawArrays(GL_TRIANGLES, 0, m_shapes_map[shape.primitive.type].size() / 6);
        glBindVertexArray(0);
    }
    glUseProgram(0);

    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
    glViewport(0, 0, m_screen_width, m_screen_height);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    paintTexture(m_fbo_texture, settings.perPixelFilter, settings.kernelBasedFilter, settings.extraCredit1, settings.extraCredit2, settings.extraCredit3, settings.extraCredit4);
}

void Realtime::paintTexture(GLuint texture, bool pixel_filter, bool kernel_filter, bool extra_credit1, bool extra_credit2, bool extra_credit3, bool extra_credit4){
    glUseProgram(m_texture_shader);
    glUniform1i(glGetUniformLocation(m_texture_shader, "invertFilter"), pixel_filter ? 1 : 0);
    glUniform1i(glGetUniformLocation(m_texture_shader, "sharpenFilter"), kernel_filter ? 1 : 0);
    glUniform1i(glGetUniformLocation(m_texture_shader, "grayscaleFilter"), extra_credit1 ? 1 : 0);
    glUniform1i(glGetUniformLocation(m_texture_shader, "boxFilter"), extra_credit2 ? 1 : 0);
    glUniform1i(glGetUniformLocation(m_texture_shader, "sepiaHueFilter"), extra_credit3 ? 1 : 0);
    glUniform1i(glGetUniformLocation(m_texture_shader, "edgeFilter"), extra_credit4 ? 1 : 0);
    glUniform1f(glGetUniformLocation(m_texture_shader, "sceneWidth"), static_cast<float>(size().width()));
    glUniform1f(glGetUniformLocation(m_texture_shader, "sceneHeight"), static_cast<float>(size().height()));

    glBindVertexArray(m_fullscreen_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}


void Realtime::resizeGL(int w, int h) {
    // Tells OpenGL how big the screen is
    glViewport(0, 0, w * m_devicePixelRatio, h * m_devicePixelRatio);
    // Notify the camera about the size change
    m_camera.resize(w, h);

    glDeleteTextures(1, &m_fbo_texture);
    glDeleteRenderbuffers(1, &m_fbo_renderbuffer);
    glDeleteFramebuffers(1, &m_fbo);
    m_screen_width = size().width() * m_devicePixelRatio;
    m_screen_height = size().height() * m_devicePixelRatio;
    m_fbo_width = m_screen_width;
    m_fbo_height = m_screen_height;
    makeFBO();

}

void Realtime::updateShapes() {
    if (!m_glmInit) {
        return;
    }
    Sphere mySphere;
    mySphere.updateParams(settings.shapeParameter1, settings.shapeParameter2);
    m_shapes_map[PrimitiveType::PRIMITIVE_SPHERE] = mySphere.generateShape();
    Cylinder myCylinder;
    myCylinder.updateParams(settings.shapeParameter1, settings.shapeParameter2);
    m_shapes_map[PrimitiveType::PRIMITIVE_CYLINDER] = myCylinder.generateShape();
    Cube myCube;
    myCube.updateParams(settings.shapeParameter1);
    m_shapes_map[PrimitiveType::PRIMITIVE_CUBE] = myCube.generateShape();
    Cone myCone;
    myCone.updateParams(settings.shapeParameter1, settings.shapeParameter2);
    m_shapes_map[PrimitiveType::PRIMITIVE_CONE] = myCone.generateShape();
    // Generate VBOs and VAOs outside the loop
    for (auto const &[type, data] : m_shapes_map) {
        // Set up VBO
        glGenBuffers(1, &m_vbos[type]);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbos[type]);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(GLfloat), data.data(), GL_STATIC_DRAW);

        // Set up VAO
        glGenVertexArrays(1, &m_vaos[type]);
        glBindVertexArray(m_vaos[type]);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));

        // Unbind VAO
        glBindVertexArray(0);

        // Unbind VBO
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

}

void Realtime::sceneChanged() {
    bool res = SceneParser::parse(settings.sceneFilePath, m_renderData);
    if (res) {
        std::cout << "Parsed scene file " << settings.sceneFilePath;
    } else {
        std::cout << "Erro parse scenefile";
    }
    SceneCameraData& camData = m_renderData.cameraData;
    m_camera = Camera(camData, size().width(), size().height());
    m_camera.updatePlane(settings.nearPlane, settings.farPlane);
    update(); // asks for a PaintGL() call to occur
}

void Realtime::settingsChanged() {
    updateShapes();
    m_camera.updatePlane(settings.nearPlane, settings.farPlane);
    update(); // asks for a PaintGL() call to occur
}

// ================== Project 6: Action!

void Realtime::keyPressEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = true;
}

void Realtime::keyReleaseEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = false;
}

void Realtime::mousePressEvent(QMouseEvent *event) {
    if (event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = true;
        m_prev_mouse_pos = glm::vec2(event->position().x(), event->position().y());
    }
}

void Realtime::mouseReleaseEvent(QMouseEvent *event) {
    if (!event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = false;
    }
}


void Realtime::mouseMoveEvent(QMouseEvent *event) {
    if (m_mouseDown) {
        int posX = event->position().x();
        int posY = event->position().y();
        int deltaX = posX - m_prev_mouse_pos.x;
        int deltaY = posY - m_prev_mouse_pos.y;
        m_prev_mouse_pos = glm::vec2(posX, posY);

        // Calculate rotation speed based on mouse movement
        float rotationSpeed = 0.001f; // Adjust this value as needed for sensitivity
        float horizontalRotation = -rotationSpeed * deltaX;
        float verticalRotation = -rotationSpeed * deltaY;

        // Rotate the camera based on mouse movement
        m_camera.rotateHorizontal(horizontalRotation);
        m_camera.rotateVertical(verticalRotation);
        update(); // asks for a PaintGL() call to occur
    }
}



void Realtime::timerEvent(QTimerEvent *event) {
    int elapsedms   = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();

    // Use deltaTime and m_keyMap here to move around
    float translationSpeed = 5.0f * deltaTime;
    // Translate the camera based on key presses
    if (m_keyMap[Qt::Key_W]) {
        m_camera.translate(m_camera.getForwardVector() * translationSpeed);
    }
    if (m_keyMap[Qt::Key_S]) {
        m_camera.translate(-m_camera.getForwardVector() * translationSpeed);
    }
    if (m_keyMap[Qt::Key_A]) {
        m_camera.translate(-m_camera.getRightVector() * translationSpeed);
    }
    if (m_keyMap[Qt::Key_D]) {
        m_camera.translate(m_camera.getRightVector() * translationSpeed);
    }
    if (m_keyMap[Qt::Key_Space]) {
        m_camera.translate(glm::vec3(0.0f, 1.0f, 0.0f) * translationSpeed);
    }
    if (m_keyMap[Qt::Key_Control]) {
        m_camera.translate(glm::vec3(0.0f, -1.0f, 0.0f) * translationSpeed);
    }

    update(); // asks for a PaintGL() call to occur
}

// DO NOT EDIT
void Realtime::saveViewportImage(std::string filePath) {
    // Make sure we have the right context and everything has been drawn
    makeCurrent();

    int fixedWidth = 1024;
    int fixedHeight = 768;

    // Create Frame Buffer
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create a color attachment texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fixedWidth, fixedHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    // Optional: Create a depth buffer if your rendering uses depth testing
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fixedWidth, fixedHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    // Render to the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, fixedWidth, fixedHeight);

    // Clear and render your scene here
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    paintGL();

    // Read pixels from framebuffer
    std::vector<unsigned char> pixels(fixedWidth * fixedHeight * 3);
    glReadPixels(0, 0, fixedWidth, fixedHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // Unbind the framebuffer to return to default rendering to the screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Convert to QImage
    QImage image(pixels.data(), fixedWidth, fixedHeight, QImage::Format_RGB888);
    QImage flippedImage = image.mirrored(); // Flip the image vertically

    // Save to file using Qt
    QString qFilePath = QString::fromStdString(filePath);
    if (!flippedImage.save(qFilePath)) {
        std::cerr << "Failed to save image to " << filePath << std::endl;
    }

    // Clean up
    glDeleteTextures(1, &texture);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteFramebuffers(1, &fbo);
}
