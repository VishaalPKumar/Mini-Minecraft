#include "mesh.h"
// #include<la.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <random>
#include <set>

int Mesh::VERTEX_ID = 0;
int Mesh::FACE_ID = 0;
int Mesh::HALFEDGE_ID = 0;

Mesh::Mesh(OpenGLContext *mp_context) : Drawable(mp_context) {
}

Mesh::~Mesh()
{}

void Mesh::createVBOdata()
{
    std::vector<glm::vec4> pos, nor, col;
    std::vector<GLuint> idx;

    glm::vec3 min = glm::vec3(std::numeric_limits<float>::max()), max = glm::vec3(-std::numeric_limits<float>::max());

    int maxIndex = 0;
    for (auto &f : this->m_faces) {
        HalfEdge *curr = f->m_halfEdge;

        int verticesOnFace = 0;
        do {
            // Player is creeper - 13 in x
            float x = curr->m_vertex->m_pos.x;// + 40;
            float y = curr->m_vertex->m_pos.y;// + 0;
            float z = curr->m_vertex->m_pos.z;// + 40;

            // std::cout << "Mesh X: " << x << std::endl;
            // std::cout << "Mesh Y: " << y << std::endl;
            // std::cout << "Mesh Z: " << z << std::endl;

            pos.push_back(glm::vec4(x, y, z, 1.f));
            min = glm::min(min, glm::vec3(x,y,z));
            max = glm::max(max, glm::vec3(x,y,z));

            // glm::vec3 v0 = curr->m_vertex->m_pos - curr->m_sym->m_vertex->m_pos;
            // glm::vec3 v1 = curr->m_next->m_vertex->m_pos - curr->m_vertex->m_pos;

            // nor.push_back(glm::normalize(glm::cross(v0, v1)));


            if (curr->m_sym != nullptr && curr->m_sym->m_vertex != nullptr) {
                glm::vec3 v0 = curr->m_vertex->m_pos - curr->m_sym->m_vertex->m_pos;
                glm::vec3 v1 = curr->m_next->m_vertex->m_pos - curr->m_vertex->m_pos;
                glm::vec3 crossed = glm::normalize(glm::cross(v0, v1));

                nor.push_back(glm::vec4(crossed.x, crossed.y, crossed.z, 0.f));
            } else {
                // handle the case where m_sym is nullptr
                glm::vec4 defaultNormal(0.0f, 0.0f, 0.0f, 1.0f);
                nor.push_back(defaultNormal);
            }

            col.push_back(glm::vec4(f->m_color.r, f->m_color.g, f->m_color.b, 1.f));
            ++verticesOnFace;

            curr = curr->m_next;
        } while (curr != f->m_halfEdge);

        for (int i = 0; i < verticesOnFace - 2; ++i) {
            idx.push_back(maxIndex);
            idx.push_back(maxIndex + i + 1);
            idx.push_back(maxIndex + i + 2);
        }

        maxIndex += verticesOnFace;
    }

    // this->indexBufferLength = idx.size();
    indexCounts[INDEX] = idx.size();

    generateBuffer(INDEX);
    bindBuffer(INDEX);
    // bufferData(INDEX, idx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generateBuffer(POSITION);
    bindBuffer(POSITION);
    // bufferData(POSITION, pos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(glm::vec4), pos.data(), GL_STATIC_DRAW);

    generateBuffer(NORMAL);
    bindBuffer(NORMAL);
    // bufferData(NORMAL, nor);
    mp_context->glBufferData(GL_ARRAY_BUFFER, nor.size() * sizeof(glm::vec4), nor.data(), GL_STATIC_DRAW);

    generateBuffer(COLOR);
    bindBuffer(COLOR);
    // bufferData(COLOR, col);
    mp_context->glBufferData(GL_ARRAY_BUFFER, col.size() * sizeof(glm::vec4), col.data(), GL_STATIC_DRAW);

    // std::cout << "BUFFERS:" << indexCounts[INDEX] << ", " << pos.size() << ", " << col.size() << std::endl;
}


void Mesh::resetMesh() {
    this->m_vertices.clear();
    this->m_faces.clear();
    this->m_halfEdges.clear();
    // this->indexCounts = 0;
    this->indexCounts[INDEX] = 0;
    this->VERTEX_ID = 0;
    this->FACE_ID = 0;
    this->HALFEDGE_ID = 0;
}

void Mesh::loadMeshFromObjFile(const QString &filePath) {
    std::ifstream objFile(filePath.toStdString());
    std::string line;

    // std::cout << "Entering while loop..." << std::endl;

    this->resetMesh();

    std::map<std::pair<Vertex*, Vertex*>, HalfEdge*> endpointsToHalfEdge;
    glm::vec3 min = glm::vec3(std::numeric_limits<float>::max()), max = glm::vec3(-std::numeric_limits<float>::max());

    while (std::getline(objFile, line)) {
        std::istringstream iss(line);
        std::string firstChar;
        iss >> firstChar;

        // std::cout << "Getting line " << counter++ << "..." << std::endl;

        if (firstChar == "v" && iss.peek() == ' ')  {
            this->m_vertices.push_back(std::make_unique<Vertex>(Vertex()));
            Vertex *vertex = m_vertices.back().get();
            iss >> vertex->m_pos.x >> vertex->m_pos.y >> vertex->m_pos.z;

            min = glm::min(min, glm::vec3(vertex->m_pos.x,vertex->m_pos.y,vertex->m_pos.z));
            max = glm::max(max, glm::vec3(vertex->m_pos.x,vertex->m_pos.y,vertex->m_pos.z));

        } else if (firstChar == "f" && iss.peek() == ' ') {
            this->m_faces.push_back(std::make_unique<Face>(Face()));
            Face *face = m_faces.back().get();
            face->m_color = gen_rand_color();//glm::vec3(r, g, b);

            // std::cout << "Face " << face->m_id << " has these vertices: " << std::endl;

            std::string token;
            std::vector<int> vertIndices;

            while (std::getline(iss, token, ' ')) {
                size_t pos = token.find('/');
                if (pos != std::string::npos) {
                    std::string number = token.substr(0, pos);
                    int index = std::stoi(number) - 1;
                    vertIndices.push_back(index);
                }
            }
            int numVertices = vertIndices.size();
            vertIndices.push_back(vertIndices[0]); // add first vertex to end again

            // Create all new half pointers and set all pointers EXCEPT next ptrs
            for (int i = 0; i < numVertices; i++) {
                this->m_halfEdges.push_back(std::make_unique<HalfEdge>(HalfEdge()));
                HalfEdge *halfEdge = m_halfEdges.back().get();

                Vertex *v1 = this->m_vertices[vertIndices[i]].get();
                Vertex *v2 = this->m_vertices[vertIndices[i + 1]].get();

                // Face ptr
                halfEdge->setFace(face);

                // Vertex ptr
                halfEdge->setVertex(v2);

                halfEdge->m_sym = nullptr;

                // Sym ptr
                std::pair<Vertex*, Vertex*> endpoints(std::min(v1, v2), std::max(v1, v2));
                // if (endpointsToHalfEdge.contains(endpoints)) {
                //     halfEdge->setSym(endpointsToHalfEdge.at(endpoints));
                // } else {
                //     endpointsToHalfEdge[endpoints] = halfEdge;
                // }
                if (endpointsToHalfEdge.find(endpoints) != endpointsToHalfEdge.end()) {
                    HalfEdge* symmetric = endpointsToHalfEdge[endpoints];
                    halfEdge->setSym(symmetric);
                    symmetric->setSym(halfEdge);
                } else {
                    endpointsToHalfEdge[endpoints] = halfEdge;
                }
            }

            // Assign next ptrs
            int startIndex = m_halfEdges.size() - numVertices;
            for (int i = startIndex; i < m_halfEdges.size() - 1; i++) {
                HalfEdge *halfEdge = this->m_halfEdges[i].get();
                HalfEdge  *nextHalfEdge = this->m_halfEdges[i + 1].get();
                halfEdge->m_next = nextHalfEdge;
                // std::cout << "SET: HE " << halfEdge->m_id << " next to " << nextHalfEdge->m_id << std::endl;
            }
            HalfEdge *firstHalfEdge = this->m_halfEdges[m_halfEdges.size() - numVertices].get();
            HalfEdge *lastHalfEdge = this->m_halfEdges[m_halfEdges.size() - 1].get();
            lastHalfEdge->m_next = firstHalfEdge;
            // std::cout << "SET: HE " << lastHalfEdge->m_id << " next to " << firstHalfEdge->m_id << std::endl;

        }
    }

    min;

    /*

    // Print vertices:
    std::cout << "Vertices: " << std::endl;
    for (auto& v : this->m_vertices) {
        std::cout << v.get()->m_id << ": (" << v.get()->m_pos.x << ", " << v.get()->m_pos.y << ", " << v.get()->m_pos.z << ")" << std::endl;
    }

    // Print Half Edges:
    std::cout << "Half Edges: " << std::endl;
    for (auto& v : this->m_halfEdges) {
        std::cout << v.get()->m_id << ": (" << v.get()->m_face << ", " << v.get()->m_next << ", " << v.get()->m_sym << ") | (" << v.get()->m_vertex->m_pos.x << ", " << v.get()->m_vertex->m_pos.y << ", " << v.get()->m_vertex->m_pos.z << ")" << std::endl;
    }

    // Print faces:
    std::cout << "Faces: " << std::endl;
    for (auto& f : this->m_faces) {
        std::cout << f.get()->m_id << ", " <<  f->m_halfEdge->m_id << std::endl;
    }

    */

}

Vertex::Vertex() {
    this->m_id = Mesh::VERTEX_ID++;
}

int Vertex::getId() {
    return this->m_id;
}

Face::Face() {
    this->m_id = Mesh::FACE_ID++;
}

int Face::getId() {
    return this->m_id;
}

HalfEdge::HalfEdge() {
    this->m_id = Mesh::HALFEDGE_ID++;
}

int HalfEdge::getId() {
    return this->m_id;
}

void HalfEdge::setVertex(Vertex* v) {
    this->m_vertex = v;
    v->m_halfEdge = this;

    // std::cout << "HE " << m_id << " - Setting vertex to " << v->m_pos.x << ", " << v->m_pos.y << ", " << v->m_pos.z << std::endl;
}

void HalfEdge::setSym(HalfEdge* halfEdge) {
    this->m_sym = halfEdge;
    halfEdge->m_sym = this;

    // std::cout << "HE " << m_id << " - Setting Sym to " << halfEdge->m_id << " at " << halfEdge << std::endl;
}

void HalfEdge::setFace(Face* face) {
    this->m_face = face;
    face->m_halfEdge = this;

    // std::cout << "HE " << m_id << " - Setting Face to Face " << face->m_id << " at " << face << std::endl;
}

glm::vec3 lerp_color(const glm::vec3& color1, const glm::vec3& color2, float t) {
    glm::vec3 lerped_color = (1 - t) * color1 + t * color2;
    return lerped_color;
}

glm::vec3 gen_rand_color() {
    std::vector<glm::vec3> color_scheme = {
        // glm::vec3(0.0f, 0.1f, 0.2f), // rly dark blue
        // glm::vec3(0.0f, 0.2f, 0.4f), // dark blue
        // glm::vec3(0.0f, 0.3f, 0.6f), // medium blue
        // glm::vec3(0.0f, 0.4f, 0.8f), // cerluean blue
        // glm::vec3(0.0f, 0.6f, 1.0f), // light blue
        // glm::vec3(0.2f, 0.4f, 0.4f), // blue-green
        // glm::vec3(0.2f, 0.6f, 0.6f), // teal
        // glm::vec3(0.2f, 0.8f, 0.8f), // lighter teal
        glm::vec3(0.1f, 0.3f, 0.2f), // dark green
        glm::vec3(0.2f, 0.5f, 0.4f), // medium green
        glm::vec3(0.3f, 0.7f, 0.5f), // normal green
        glm::vec3(0.4f, 0.9f, 0.6f), // light green
        // glm::vec3(0.3f, 0.2f, 0.5f), // dark purple
        // glm::vec3(0.4f, 0.4f, 0.7f), // medium purple
        // glm::vec3(0.5f, 0.6f, 0.9f), // normal purple
        // glm::vec3(0.6f, 0.8f, 1.0f)  // light purple
    };
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, color_scheme.size() - 1);

    int index1 = dis(gen);
    int index2 = dis(gen);
    while (index1 == index2) {
        index2 = dis(gen);
    }

    const glm::vec3& color1 = color_scheme[index1];
    const glm::vec3& color2 = color_scheme[index2];

    float t = static_cast<float>(rand()) / RAND_MAX;

    return lerp_color(color1, color2, t);
}
