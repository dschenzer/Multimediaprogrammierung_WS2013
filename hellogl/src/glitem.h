#ifndef GLITEM_H
#define GLITEM_H

#include <QQuickItem>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <QVector>
#include <QTimer>

class GlItem : public QQuickItem
{
    Q_OBJECT
    typedef enum{
        VERTEX_LOCATION,
        NORMAL_LOCATION,
        COLOR_LOCATION,
        TEXCOORD_LOCATION
    }AttributeLocation;

public:
    explicit GlItem(QQuickItem *parent = 0);
   // QSGNode * updatePaintNode(QSGNode *node, UpdatePaintNodeData *);

signals:
    
public slots:
    void paint();
    void cleanup();
    void synchronizeThreads();
    void toggleMove();
private slots:
    void handleWindowChanged(QQuickWindow *win);
    void onTimerTimeout();
private:
    /**
     * @brief createShaderProgram
     * creates and links the shader program using vshader.
     */
    void createShaderProgram();
    /**
     * @brief setupGeometry puts the geometric data into the arrays (m_Vertices etc.) and sets geometryIsValid flag.
     */
    void setupGeometry();
    /**
     * @brief setupView Setup matrices, lighting and basic GL rendering settings
     */
    void setupView();
    /**
     * @brief activateShaderProgram Binds program enables arrays and transfers uniform values
     */
    void activateShaderProgram();
    /**
     * @brief releaseShaderprogram Deactivates Arrays and release program.
     */
    void releaseShaderProgram();
    /**
     * @brief geometryIsValid
     *  true when geometry has been defined in m_Vertices etc.
     *  Set by setupGeometry.
     */
    bool m_geometryIsValid;

    QOpenGLShaderProgram * m_program;
    QMatrix4x4 m_mvMatrix;
    QMatrix4x4 m_pMatrix;
    QMatrix4x4 m_mvpMatrix;
    QMatrix3x3 m_normalMatrix;
    QVector3D m_lightDirection;
    GLfloat m_ambientLightBrightness;
    bool m_colorArrayEnabled;
    bool m_texturesEnabled;
    bool m_lightingEnabled;
    QVector<QVector3D> m_vertices;
    QVector<QVector3D> m_normals;
    QVector<QVector3D> m_texCoords;
    QVector<QVector4D> m_colors;
    QTimer * m_timer;
    double m_guiThreadRotation;
    double m_renderThreadRotation;
};

#endif // GLITEM_H
