#include "gl_widget.h"
#include "libvector.h"
#include <QDebug>
#include <QMouseEvent>
#include <iostream>

GLWidget3D::GLWidget3D(QWidget *parent)
    : QOpenGLWidget(parent), m_program(nullptr)
{
    setAutoFillBackground(false);
    setMouseTracking(true);
}

void GLWidget3D::initializeGL()
{
    initializeOpenGLFunctions();

    // Создаем шейдерную программу
    m_program = new QOpenGLShaderProgram(this);

    // Вершинный шейдер (GLSL)
    const char *vertexShaderSource = R"_(
        #version 330
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        uniform mat4 u_transform;
        out vec3 normal;

        void main()
        {
            gl_Position = vec4(aPos, 1.0f) * u_transform;
            normal = normalize(aNormal);
        };
    )_";
        

    // Фрагментный шейдер (GLSL)
    const char *fragmentShaderSource = R"_(
        #version 330
        out vec4 FragColor;
        in vec3 normal;

        void main()
        {
            vec3 lightDir = normalize(vec3(0.8, 1.0, 1.2));
            vec3 color = vec3(0.8, 0.3, 0.2);
            FragColor = vec4(color * max(dot(abs(normal), lightDir), 0.0), 1.0f);
        };
    )_";
        

    // Грузим шейдеры
    if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, QString(vertexShaderSource)))
    {
        qDebug() << "Vertex shader error:" << m_program->log();
    }
    if (!m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, QString(fragmentShaderSource)))
    {
        qDebug() << "Fragment shader error:" << m_program->log();
    }

    // Связываем программу
    if (!m_program->link())
    {
        qDebug() << "Program link error:" << m_program->log();
    }

    gl = new QOpenGLExtraFunctions;
    gl->initializeOpenGLFunctions();
    
}

void GLWidget3D::paintGL()
{
    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float scale_factor = (float)height() / (float)width();
    auto scale = mat4<float>::scale(scale_factor, 1, 0.001);
    auto _transform = scale * transform;

    glEnable(GL_DEPTH_TEST);

    if (model_loaded) {
        m_program->bind();
        gl->glProgramUniformMatrix4fv(m_program->programId(), m_program->uniformLocation("u_transform"), 1, false, (GLfloat*)&_transform);
        gl->glBindVertexArray(vao);
        gl->glDrawArrays(GL_TRIANGLES, 0, vertex->size() * 3);
        gl->glBindVertexArray(0);
        m_program->release();
    }
}

void GLWidget3D::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void GLWidget3D::loadModel(std::vector<std::array<vec3<float>, 3>> *_vertex) {
    vertex = _vertex;
    // Создаем VBO и VAO
    makeCurrent();
    gl->glGenVertexArrays(1, &vao);
    gl->glGenBuffers(1, &vbo);
    gl->glGenBuffers(1, &vbo_normal);

    gl->glBindVertexArray(vao);

    gl->glBindBuffer(GL_ARRAY_BUFFER, vbo);
    gl->glBufferData(GL_ARRAY_BUFFER, vertex->size() * sizeof(std::array<vec3<float>, 3>), vertex->data(), GL_STATIC_DRAW);
    gl->glEnableVertexAttribArray(0);
    gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    gl->glBindBuffer(GL_ARRAY_BUFFER, vbo_normal);

    auto dot = [](vec3<float> a, vec3<float> b) -> float { return a.x * b.x + a.y * b.y + a.z * b.z; };
    auto cross = [](vec3<float> a, vec3<float> b){ 
        return vec3(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
    };
    for (int i = 0; i < vertex->size(); i++) {
        auto vert = (*vertex)[i];
        auto norm = cross(vert[1] - vert[0], vert[2] - vert[0]);
        normals.push_back(norm);
        normals.push_back(norm);
        normals.push_back(norm);
    }

    gl->glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(vec3<float>), normals.data(), GL_STATIC_DRAW);
    gl->glEnableVertexAttribArray(1);
    gl->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
    gl->glBindVertexArray(0);
    
    model_loaded = true;
    update();
}

void GLWidget3D::mouseMoveEvent(QMouseEvent *event) {
    if (isPressed) {
        vec2<float> delta = vec2<float>(
            (event->position().x() - old_mousepos.x()) / height(),
            (event->position().y() - old_mousepos.y()) / height()
        );
        if (event->buttons() == Qt::RightButton) {
            // transform = transform * mat4<float>::rotate(0, delta.x, delta.y);
            transform *= mat4<float>::rotateY(-delta.x);
            transform *= mat4<float>::rotateX(-delta.y);
            transform *= mat4<float>::rotateZ(delta.y * -start_mousepos.x() * 3);
            transform *= mat4<float>::rotateZ(delta.x * start_mousepos.y() * 3);
        } else if (event->buttons() == Qt::MiddleButton) {
            transform *= mat4<float>::translate(delta.x * 2., -delta.y * 2., 0);
        }
        old_mousepos = event->position();
        update();
    }
}

void GLWidget3D::mousePressEvent(QMouseEvent *event) {
    isPressed = true;
    old_mousepos = event->position();
    start_mousepos = QPointF{
      event->position().x() / height() - 0.5,
      event->position().y() / height() - 0.5
    };
}

void GLWidget3D::mouseReleaseEvent(QMouseEvent *event) {
    isPressed = false;
}

void GLWidget3D::wheelEvent(QWheelEvent *event) {
    float new_scale = pow(2, event->angleDelta().y() * 0.001);

    transform *= mat4<float>::scale(new_scale);
    transform *= mat4<float>::translate(
        -(event->position().x() / width() * 2. - 1.) / ((float)height() / (float)width()) * (new_scale - 1.0),
        (event->position().y() / height() * 2. - 1.) * (new_scale - 1.0),
        0
    );
    update();
}
