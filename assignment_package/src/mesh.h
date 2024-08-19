#pragma once

#include "drawable.h"
// #include <la.h>

#include <QOpenGLContext>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QListWidgetItem>

#include <memory>

class Vertex;
class Face;
class HalfEdge;

class Mesh : public Drawable
{
public:
    Mesh(OpenGLContext* mp_context);
    // Mesh(OpenGLContext* mp_context, const QString& filePath);
    virtual ~Mesh();
    // void initializeAndBufferGeometryData() override;
    void createVBOdata() override;
    void resetMesh();
    static int VERTEX_ID;
    static int FACE_ID;
    static int HALFEDGE_ID;

private:
    std::vector<std::unique_ptr<Vertex>> m_vertices;
    std::vector<std::unique_ptr<Face>> m_faces;
    std::vector<std::unique_ptr<HalfEdge>> m_halfEdges;
    void loadMeshFromObjFile(const QString &filePath);
    friend class MyGL;
    friend class MainWindow;
};

class Vertex : public QListWidgetItem
{
public:
    Vertex();
    int getId();
private:
    glm::vec3 m_pos;
    HalfEdge* m_halfEdge;
    int m_id;

    friend class Mesh;
    friend class Face;
    friend class HalfEdge;
    friend class VertexDisplay;
    friend class HalfEdgeDisplay;
    friend class FaceDisplay;
    friend class MyGL;
};

class Face : public QListWidgetItem
{
public:
    Face();
    int getId();

private:
    HalfEdge* m_halfEdge;
    glm::vec3 m_color;
    int m_id;

    friend class Mesh;
    friend class Vertex;
    friend class HalfEdge;
    friend class FaceDisplay;
    friend class MyGL;
};

class HalfEdge : public QListWidgetItem
{
public:
    HalfEdge();
    int getId();

private:
    HalfEdge* m_next;
    HalfEdge* m_sym;
    Face* m_face;
    Vertex* m_vertex;
    int m_id;
    void setVertex(Vertex* v);
    void setSym(HalfEdge* halfEdge);
    void setFace(Face* face);

    friend class Mesh;
    friend class Vertex;
    friend class Face;
    friend class HalfEdgeDisplay;
    friend class FaceDisplay;
    friend class MyGL;
};

glm::vec3 gen_rand_color();
