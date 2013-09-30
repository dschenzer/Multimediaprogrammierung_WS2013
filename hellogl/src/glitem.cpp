#include "glitem.h"

#include <QDebug>
#include <QQuickWindow>
#include <QtQuick/QQuickItem>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLContext>

#include <QSGSimpleRectNode>

GlItem::GlItem(QQuickItem *parent) :
    QQuickItem(parent)
{
    setFlag(ItemHasContents);
    m_program = NULL;
    m_geometryIsValid = false;
    m_colorArrayEnabled = false;
    m_texturesEnabled = false;
    m_lightingEnabled = true;
    //The windowChanged signal is emitted by QQuickItem when it is added to the scenegraph.
    //This is the first time when a valid window exists.
    connect(this, SIGNAL(windowChanged(QQuickWindow*)),
            this, SLOT(handleWindowChanged(QQuickWindow*)));
    m_geometryIsValid = false; //invalidate geometry, we may need to set it up for the new window
    m_timer = new QTimer(this);
    m_timer->setInterval(50);
    connect(m_timer, SIGNAL(timeout()),
            this, SLOT(onTimerTimeout()), Qt::DirectConnection);
    m_guiThreadRotation = 0.0;
    m_renderThreadRotation = 0.0;
}

/*
QSGNode * GlItem::updatePaintNode(QSGNode *node, UpdatePaintNodeData *)
{
    QSGSimpleRectNode *rect = static_cast<QSGSimpleRectNode *>(node);
    if (!rect) {
        rect = new QSGSimpleRectNode();
    }
    double w = 100;
    if(window())
        w = window()->width();
    rect->setRect(0, 0, w, 100);
    rect->setColor(Qt::white);
    return rect;
}
*/
void GlItem::paint()
{
    //qDebug() << "GlItem::paint() called";
    if (!m_program)
       createShaderProgram();
    if(!m_geometryIsValid)
        setupGeometry();

    setupView();

    activateShaderProgram();

    //We are ready to draw now
    glDrawArrays(GL_TRIANGLES, 0, m_vertices.size());

    //turn over and draw a second triangle
    m_mvMatrix.rotate(90.0, QVector3D(1.0,0.0,0.0));
    m_mvpMatrix = m_pMatrix * m_mvMatrix;
    m_normalMatrix= m_mvMatrix.normalMatrix();

    //copy new matrices to GPU
    m_program->setUniformValue("u_MvpMatrix", m_mvpMatrix);
    m_program->setUniformValue("u_NormalMatrix", m_normalMatrix);

    //draw the second triangle
    glDrawArrays(GL_TRIANGLES, 0, m_vertices.size());

    //we are done
    releaseShaderProgram();
}

void GlItem::cleanup()
{
    qDebug() <<"GlItem::cleanup() called.";
    if (m_program) {
        delete m_program;
        m_program = 0;
    }
}

void GlItem::synchronizeThreads()
{
    m_renderThreadRotation = m_guiThreadRotation;
}

void GlItem::toggleMove()
{
    qDebug() << "GlItem::move() called";
    if(m_timer->isActive())
        m_timer->stop();
    else m_timer->start();
}

void GlItem::createShaderProgram()
{
    qDebug() <<"GlItem::createShaderProgram called.";
    if(m_program)
        return;
    m_program = new QOpenGLShaderProgram();
    m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/src/vshader1.vsh");
    m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/src/fshader1.fsh");

    m_program->bindAttributeLocation("a_Vertex", VERTEX_LOCATION);
    m_program->bindAttributeLocation("a_Normal", NORMAL_LOCATION);
    m_program->bindAttributeLocation("a_Color", 2);
    m_program->bindAttributeLocation("a_TexCoord", 3);

    if(!m_program->link())
    {
        qDebug() <<"GlItem::createShaderProgram: Critical error: Program linking failed";
    }

    if(!window())
    {
        qDebug() <<"GlItem::createShaderProgram: Critical error: No window available";
    }
    else connect(window()->openglContext(), SIGNAL(aboutToBeDestroyed()), //cleanup context before destroying window
                 this, SLOT(cleanup()), Qt::DirectConnection);
}


void GlItem::setupView()
{
    m_pMatrix.setToIdentity();
    m_pMatrix.perspective(45.0,                               //fovy
                            (double)width() / (double)height(), //aspect
                            1.0,                                //near
                            100.0);                             //far
    m_mvMatrix.setToIdentity();
    m_mvMatrix.lookAt(QVector3D(0.0,0.0, 5.0), //eye
                       QVector3D(0.0,0.0, 0.0), //center
                       QVector3D(0.0,1.0, 0.0));//up
    m_mvMatrix.rotate(m_renderThreadRotation, QVector3D(0.0,1.0,1.0));
    m_normalMatrix = m_mvMatrix.normalMatrix();

    m_mvpMatrix = m_pMatrix * m_mvMatrix;

    m_lightDirection = QVector3D(0.0,0.0,1.0);
    m_ambientLightBrightness = 0.5;

    QMatrix4x4 nMatrix(m_normalMatrix); //we need a 4x4 matrix for multiplication
    m_lightDirection = nMatrix * m_lightDirection; //transform light direction into eye space before passing it to the vertex shader

    glViewport(0, 0, window()->width(), window()->height());
    glEnable(GL_DEPTH_TEST);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
   // glEnable(GL_BLEND);
   // glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void GlItem::activateShaderProgram()
{
    m_program->bind();

    m_program->enableAttributeArray(VERTEX_LOCATION);
    m_program->setAttributeArray(VERTEX_LOCATION, GL_FLOAT, m_vertices.data(), 3);
    m_program->enableAttributeArray(NORMAL_LOCATION);
    m_program->setAttributeArray(NORMAL_LOCATION, GL_FLOAT, m_normals.data(), 3);
    m_program->enableAttributeArray(COLOR_LOCATION);
    m_program->setAttributeArray(COLOR_LOCATION, GL_FLOAT, m_colors.data(), 4);
    m_program->enableAttributeArray(TEXCOORD_LOCATION);
    m_program->setAttributeArray(TEXCOORD_LOCATION, GL_FLOAT, m_texCoords.data(), 3);

    //matrices
    m_program->setUniformValue("u_MvpMatrix", m_mvpMatrix);
    m_program->setUniformValue("u_NormalMatrix", m_normalMatrix);

    //flags
    m_program->setUniformValue("u_ColorArrayEnabled", m_colorArrayEnabled);
    m_program->setUniformValue("u_TexturesEnabled", m_texturesEnabled);
    m_program->setUniformValue("u_LightingEnabled", m_lightingEnabled);
    m_program->setUniformValue("u_LightDirection", m_lightDirection);
    m_program->setUniformValue("u_AmbientAndDiffuseColor", QVector4D(1.0,0.0,0.0,1.0));
    m_program->setUniformValue("u_AmbientLightBrightness", m_ambientLightBrightness);
}

void GlItem::releaseShaderProgram()
{
    m_program->disableAttributeArray(VERTEX_LOCATION);
    m_program->disableAttributeArray(NORMAL_LOCATION);
    m_program->disableAttributeArray(COLOR_LOCATION);
    m_program->disableAttributeArray(TEXCOORD_LOCATION);
    m_program->release();
}

/**
 * @brief GlItem::handleWindowChanged
 *
 * Connect to the window's signals. This can not be done in the constructor, because at that time
 * there is no valid window yet.
 * @param win The window in which this QQuickItem will be painted.
 */
void GlItem::handleWindowChanged(QQuickWindow *win)
{
    qDebug() << "GlItem::handleWindowChanged() called.";
    if (win) {
        // Connect the beforeRendering signal to our paint function.
        // Since this call is executed on the rendering thread it must be
        // a Qt::DirectConnection
        connect(win, SIGNAL(beforeRendering()), this, SLOT(paint()), Qt::DirectConnection);
        connect(win, SIGNAL(beforeSynchronizing()), this, SLOT(synchronizeThreads()), Qt::DirectConnection);
        // If we allow QML to do the clearing, they would clear what we paint
        // and nothing would show.
        win->setClearBeforeRendering(false);
    }
}

void GlItem::onTimerTimeout()
{
    //qDebug() << "GlItem::onTimerTimeout() called";
    m_guiThreadRotation += 1.0;
    if (window())
        window()->update();
}

void GlItem::setupGeometry()
{
    if(!m_program)
        createShaderProgram();
    //Now we create the geometry. Every corner needs a vertex, a normal, a color and a texture
    //vertices
    QVector3D v0 = QVector3D(0.0, 0.0, 0.0);
    QVector3D v1 = QVector3D(1.0, 0.0, 0.0);
    QVector3D v2 = QVector3D(0.0, 1.0, 0.0);

    //append the vertices for the triangle
    m_vertices.append(v0);
    m_vertices.append(v1);
    m_vertices.append(v2);

    //Normals are all equal, because we have only one direction
    QVector3D n0 = QVector3D(0.0, 0.0, 1.0);
    m_normals.append(n0);
    m_normals.append(n0);
    m_normals.append(n0);

    //Colors
    QVector4D  cRed = QVector4D(1.0, 0.0, 0.0, 1.0);
    QVector4D  cGreen = QVector4D(0.0, 1.0, 0.0, 1.0);
    QVector4D  cBlue = QVector4D(0.0, 0.0, 1.0, 1.0);

    m_colors.append(cRed);
    m_colors.append(cGreen);
    m_colors.append(cBlue);
    m_colorArrayEnabled = true;

    //Textures. We use a dummy here, because some GPUs want a texture array
    QVector3D t0 = QVector3D(0.0,0.0,0.0);
    m_texCoords.append(t0);
    m_texCoords.append(t0);
    m_texCoords.append(t0);

    //final check
    int size = m_vertices.size();
    if(m_normals.size() != size || m_colors.size() != size || m_texCoords.size()!=size)
    {
        qDebug() << "GlItem::setupGeometry() Critical error: Sizes of arrays are not equal. Aborting.";
        exit(1);
    }
    m_geometryIsValid = true;
 }
