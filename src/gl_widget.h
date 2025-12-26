#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLExtraFunctions>
#include"libvector.h"

class GLWidget3D : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit GLWidget3D(QWidget *parent = nullptr);

    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void loadModel(std::vector<std::array<vec3<float>, 3>> *vertex);

    QOpenGLShaderProgram *m_program{};
    QOpenGLExtraFunctions *gl{};
    GLuint vbo{}, vao{}, vbo_normal{};
    
    std::vector<std::array<vec3<float>, 3>> *vertex{};
    std::vector<vec3<float>> normals{};
    
    bool model_loaded = false;
    mat4<float> transform{};

    QPointF old_mousepos{};
    QPointF start_mousepos{};
    bool isPressed = false;
    
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
};