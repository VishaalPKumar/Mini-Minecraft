#include "mygl.h"
#include <glm_includes.h>
// #include <iostream>
#include <QApplication>
#include <QKeyEvent>
#include "utils/utils.h"
#include <thread>
#include <unistd.h>

// #include <chrono>

constexpr float MAX_DT_MS = 500.f;
static QString TEXTURE_ATLAS_PATH = "/textures/minecraft_textures_all.png";
static QString CREEPER_OBJ_PATH = "/objs/creeper.obj";

MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
    m_worldAxes(this),m_pointer(this),
    m_progLambert(this), m_progLambertNPC(this), m_progFlat(this), m_progInstanced(this), m_postProcessShader(this),
    m_terrain(this), m_player(glm::vec3(48.f, 1.5 + m_terrain.maxHeight(48, 48), 48.f), m_terrain),
    m_blockTexture(this, GL_TEXTURE5),
    quadDrawable(this), postProcessFrameBuffer(this, width(), height(), 1),
    inventory_flag(false),
    prevFrameTime(QDateTime::currentMSecsSinceEpoch()),
    m_creeperMesh(this),
    m_showCreeper(false),
    m_creeper(glm::vec3(-10.f, 1.5f + m_terrain.maxHeight(48, 48), -10.f), m_terrain, m_creeperMesh)

{
    // Connect the timer to a function so that when the timer ticks the function is executed
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(tick()));
    // Tell the timer to redraw 60 times per second
    m_timer.start(16);
    setFocusPolicy(Qt::ClickFocus);

    setMouseTracking(true); // MyGL will track the mouse's movements even if a mouse button is not pressed
    setCursor(Qt::BlankCursor); // Make the cursor invisible

    std::srand(std::time(nullptr));
}

MyGL::~MyGL() {
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
}

void MyGL::moveMouseToCenter() {
    QCursor::setPos(this->mapToGlobal(QPoint(width() / 2, height() / 2)));
}

void MyGL::initializeGL()
{
    // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
    // If you were programming in a non-Qt context you might use GLEW (GL Extension Wrangler)instead
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);
    glEnable (GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthFunc(GL_LEQUAL);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.37f, 0.74f, 1.0f, 1);

    // printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    postProcessFrameBuffer.create();
    // Create and set texture atlas
    QString path = getCurrentPath() + TEXTURE_ATLAS_PATH;
    m_blockTexture.create(path.toStdString().c_str(), GL_RGBA, GL_BGRA);

    //Create the instance of the world axes
    m_worldAxes.createVBOdata();
    m_pointer.createVBOdata();
    quadDrawable.createVBOdata();

    // Create the creepa
    QString creeperPath = getCurrentPath() + CREEPER_OBJ_PATH;
    m_creeperMesh.loadMeshFromObjFile(creeperPath);

    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        std::cout << "Current working directory: " << cwd << std::endl;
    }

    m_creeperMesh.createVBOdata();

    // Create and set up the diffuse shader
    m_progLambert.create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    m_progLambertNPC.create(":/glsl/lambert_untext.vert.glsl", ":/glsl/lambert_untext.frag.glsl");
    // Create and set up the flat lighting shader
    m_progFlat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");
    m_progInstanced.create(":/glsl/instanced.vert.glsl", ":/glsl/lambert.frag.glsl");
    m_postProcessShader.create(":/glsl/passthrough.vert.glsl", ":/glsl/postOp.frag.glsl");

    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    glBindVertexArray(vao);
    m_progLambert.setUnifFloat("u_RenderDistanceBlocks", Terrain::RENDER_DISTANCE * Terrain::CHUNK_LENGTH);
    m_progLambertNPC.setUnifFloat("u_RenderDistanceBlocks", Terrain::RENDER_DISTANCE * Terrain::CHUNK_LENGTH);
    m_terrain.updateLoadedChunks(m_player.mcr_position.x, m_player.mcr_position.z);
}


void MyGL::resizeGL(int w, int h) {
    postProcessFrameBuffer.resize(w, h, 1);
    postProcessFrameBuffer.destroy();
    postProcessFrameBuffer.create();
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    m_player.setCameraWidthHeight(static_cast<unsigned int>(w), static_cast<unsigned int>(h));
    glm::mat4 viewproj = m_player.mcr_camera.getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)
    m_progLambert.setUnifMat4("u_ViewProj", viewproj);
    m_progLambertNPC.setUnifMat4("u_ViewProj", viewproj);
    m_progFlat.setUnifMat4("u_ViewProj", viewproj);
    m_progInstanced.setUnifMat4("u_ViewProj", viewproj);

    // printGLErrorLog();
}


// MyGL's constructor links tick() to a timer that fires 60 times per second.
// We're treating MyGL as our game engine class, so we're going to perform
// all per-frame actions here, such as performing physics updates on all
// entities in the scene.
void MyGL::tick() {
    m_terrain.updateLoadedChunks(m_player.mcr_position.x, m_player.mcr_position.z);

    qint64 currFrameTime = QDateTime::currentMSecsSinceEpoch();
    float dT = currFrameTime - prevFrameTime;
    prevFrameTime = currFrameTime;

    m_progLambert.setUnifInt("u_Time", currFrameTime);
    m_postProcessShader.setUnifInt("u_Time", currFrameTime);

    m_progLambert.setUnifVec3("u_PlayerPos", m_player.getPos());
    m_progLambertNPC.setUnifVec3("u_PlayerPos", m_player.getPos());

    m_player.tick(glm::min(dT, MAX_DT_MS), m_inputs);
    m_creeper.tick(glm::min(dT, MAX_DT_MS), m_inputsNPC);

    m_progLambertNPC.setUnifMat4("u_Model", glm::translate(glm::mat4(), m_creeper.getPos()));
    m_progLambertNPC.setUnifMat4("u_ModelInvTr", glm::mat4());

    update(); // Calls paintGL() as part of a larger QOpenGLWidget pipeline

    sendPlayerDataToGUI(); // Updates the info in the secondary window displaying player data
}

void MyGL::sendPlayerDataToGUI() const {
    emit sig_sendPlayerPos(m_player.posAsQString());
    emit sig_sendPlayerVel(m_player.velAsQString());
    emit sig_sendPlayerAcc(m_player.accAsQString());
    emit sig_sendPlayerLook(m_player.lookAsQString());
    glm::vec2 pPos(m_player.mcr_position.x, m_player.mcr_position.z);
    glm::ivec2 chunk(16 * glm::ivec2(glm::floor(pPos / 16.f)));
    glm::ivec2 zone(64 * glm::ivec2(glm::floor(pPos / 64.f)));
    emit sig_sendPlayerChunk(QString::fromStdString("( " + std::to_string(chunk.x) + ", " + std::to_string(chunk.y) + " )"));
    emit sig_sendPlayerTerrainZone(QString::fromStdString("( " + std::to_string(zone.x) + ", " + std::to_string(zone.y) + " )"));
    emit sig_sendPlayerState(m_player.stateAsQString());
    emit sig_sendPlayerInventoryUpdate(m_player.inventoryAsQString());
}

// This function is called whenever update() is called.
// MyGL's constructor links update() to a timer that fires 60 times per second,
// so paintGL() called at a rate of 60 frames per second.www
void MyGL::paintGL() {
    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    assignSurfaceShaderUniforms();
    assignPostProcessShaderUniforms();

    renderTerrain();

    if (!m_showCreeper) {
        m_progLambertNPC.draw(m_creeper.m_mesh);
    }

    renderPostProcessing();

    glDisable(GL_DEPTH_TEST);
    m_progFlat.setUnifMat4("u_Model", glm::mat4());
    m_progFlat.draw(m_worldAxes);
    m_progFlat.setUnifMat4("u_ViewProj", glm::mat4());
    m_progFlat.draw(m_pointer);
    if (m_showCreeper) {
        m_progLambertNPC.draw(m_creeper.m_mesh);
    }
    glEnable(GL_DEPTH_TEST);
}

void MyGL::assignSurfaceShaderUniforms() {

    glm::mat4 viewproj = m_player.mcr_camera.getViewProj();
    m_progFlat.setUnifMat4("u_ViewProj", viewproj);

    m_progInstanced.setUnifMat4("u_ViewProj", viewproj);

    m_progLambert.setUnifMat4("u_ViewProj", viewproj);
    m_progLambert.setUnifMat4("u_Model", glm::mat4());
    m_progLambert.setUnifMat4("u_ModelInvTr", glm::mat4());

    m_progLambertNPC.setUnifMat4("u_ViewProj", viewproj);

    m_progLambert.useMe();
    m_blockTexture.bind(1);
    glUniform1i(m_progLambert.m_unifs.at("u_Texture"),
                          1);
}

void MyGL::assignPostProcessShaderUniforms() {
    if (m_player.getPlayerState().inLava) {
        m_postProcessShader.setUnifInt("u_WaterLava", 2);
    } else if (m_player.getPlayerState().inWater) {
        m_postProcessShader.setUnifInt("u_WaterLava", 1);
    } else {
        m_postProcessShader.setUnifInt("u_WaterLava", 0);
    }
}

void MyGL::renderTerrain() {

    postProcessFrameBuffer.bindFrameBuffer();
    glViewport(0, 0, width(), height());

    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::vec2 player_pos = glm::vec2(floor(m_player.mcr_position.x / 16.) * 16.,
                                 floor(m_player.mcr_position.z / 16.) * 16.);

    m_terrain.draw(player_pos.x - 16 * Terrain::RENDER_DISTANCE,
                   player_pos.x + 16 * Terrain::RENDER_DISTANCE,
                   player_pos.y - 16 * Terrain::RENDER_DISTANCE,
                   player_pos.y + 16 * Terrain::RENDER_DISTANCE,
                   &m_progLambert);

}

void MyGL::renderPostProcessing() {
    glBindFramebuffer(GL_FRAMEBUFFER, this->defaultFramebufferObject());
    glViewport(0, 0, width() * this->devicePixelRatio(), height() * this->devicePixelRatio());

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // printGLErrorLog();
    postProcessFrameBuffer.bindToTextureSlot(0);
    // printGLErrorLog();

    m_postProcessShader.useMe();
    glUniform1i(m_postProcessShader.m_unifs.at("u_Texture"),
                          postProcessFrameBuffer.getTextureSlot());
    // draw quad with post shader
    m_postProcessShader.draw(quadDrawable);
}


void MyGL::keyPressEvent(QKeyEvent *e) {
    float amount = 2.0f;
    if(e->modifiers() & Qt::ShiftModifier){
        amount = 10.0f;
    }
    switch (e->key()) {
        case Qt::Key_Escape:
            QApplication::quit();
            break;
        case Qt::Key_Right:
            m_player.rotateOnUpGlobal(-amount);
            break;
        case Qt::Key_Left:
            m_player.rotateOnUpGlobal(amount);
            break;
        case Qt::Key_Up:
            m_player.rotateOnRightLocal(-amount);
            break;
        case Qt::Key_Down:
            m_player.rotateOnRightLocal(amount);
            break;
        case Qt::Key_W:
            m_inputs.wPressed = true;
            break;
        case Qt::Key_S:
            m_inputs.sPressed = true;
            break;
        case Qt::Key_D:
            m_inputs.dPressed = true;
            break;
        case Qt::Key_A:
            m_inputs.aPressed = true;
            break;
        case Qt::Key_Q:
            m_inputs.qPressed = true;
            break;
        case Qt::Key_E:
            m_inputs.ePressed = true;
            break;
        case Qt::Key_F:
            m_inputs.fPressed = true;
            break;
        case Qt::Key_Space:
            m_inputs.spacePressed = true;
            break;
        case Qt::Key_V:
            m_creeper.startRandomMotion();
            break;
        case Qt::Key_B:
            m_creeper.stopRandomMotion();
            m_creeper.setPos(m_player.getPos() + glm::vec3(-5.f, 0.f, -5.f));
            break;
        case Qt::Key_I:
            emit sig_showInventory();
            break;
        case Qt::Key_1:
            m_player.getPlayerInventory().setSelectedBlock(BlockType::GRASS);
            emit sig_sendSelectedBlock(BlockType::GRASS);
            break;
        case Qt::Key_2:
            m_player.getPlayerInventory().setSelectedBlock(BlockType::DIRT);
            emit sig_sendSelectedBlock(BlockType::DIRT);
            break;
        case Qt::Key_3:
            m_player.getPlayerInventory().setSelectedBlock(BlockType::STONE);
            emit sig_sendSelectedBlock(BlockType::STONE);
            break;
        case Qt::Key_4:
            m_player.getPlayerInventory().setSelectedBlock(BlockType::WATER);
            emit sig_sendSelectedBlock(BlockType::WATER);
            break;
        case Qt::Key_5:
            m_player.getPlayerInventory().setSelectedBlock(BlockType::SNOW);
            emit sig_sendSelectedBlock(BlockType::SNOW);
            break;
        case Qt::Key_6:
            m_player.getPlayerInventory().setSelectedBlock(BlockType::SAND);
            emit sig_sendSelectedBlock(BlockType::SAND);
            break;
        case Qt::Key_7:
            m_player.getPlayerInventory().setSelectedBlock(BlockType::LAVA);
            emit sig_sendSelectedBlock(BlockType::LAVA);
            break;
        case Qt::Key_C:
            m_showCreeper = !m_showCreeper;
            break;
        default:
            break;
    }

}

void MyGL::keyReleaseEvent(QKeyEvent *e) {
    switch (e->key()) {
    case Qt::Key_W:
        m_inputs.wPressed = false;
        break;
    case Qt::Key_S:
        m_inputs.sPressed = false;
        break;
    case Qt::Key_D:
        m_inputs.dPressed = false;
        break;
    case Qt::Key_A:
        m_inputs.aPressed = false;
        break;
    case Qt::Key_Q:
        m_inputs.qPressed = false;
        break;
    case Qt::Key_E:
        m_inputs.ePressed = false;
        break;
    case Qt::Key_Space:
        m_inputs.spacePressed = false;
        break;
    default:
        break;
    }
}

void MyGL::mouseMoveEvent(QMouseEvent *e) {
    float dx = e->position().x() - this->width() / 2;
    float dy = e->position().y() - this->height() / 2;
    this->m_player.moveCamera(dx, dy);
    this->m_creeper.moveCamera(dx, dy);

    moveMouseToCenter();
}


void MyGL::mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::RightButton) {
        // Create Block
        m_player.createBlock();
    } else if (e->buttons() == Qt::LeftButton) {
        // Destroy Block
        m_player.destroyBlock();
    }
}
