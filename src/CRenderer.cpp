#include <cstdio>
#include <cmath>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <limits>
#include <iostream>
#include <sstream>
#include <cstdarg>
#include "SOIL.h"

#include <GL/gl.h>
#include <GL/glu.h>

class CRenderer;
#include "CRenderer.h"

#include "CEngine.h"
#include "CEntity.h"
#include "CPlayer.h"
#include "CProp.h"

void CheckOpenGLError(const char* stmt, const char* fname, int line) {
    GLenum errnum;
    bool diderr = false;
    do {
        errnum = glGetError();
        if (errnum != GL_NO_ERROR) {
            const char * errstr = (const char*)gluErrorString(errnum);
            std::stringstream ss;
            ss << "OpenGL error " << errstr << " (" << errnum << ") at " << fname << ":" << line << ":"
               << std::endl << "   " << stmt
               << std::endl
            ;
            const char *msg = ss.str().c_str();
            printf("%s", msg);
            fflush(stdout);
            diderr = true;
        }
    } while(errnum != GL_NO_ERROR);
    if(diderr) {
        throw "OpenGL error";
    }
}
void ClearOpenGLError() {
    GLenum errnum;
    do {
        errnum = glGetError();
    } while(errnum != GL_NO_ERROR);
}


CRenderer::CRenderer(
    CEngine *pEngine
) : m_iTextureIndex(1),
    m_clTextureCache(),
    m_clLights(),
    m_bInitialised(false),
    m_bResized(true),
    m_bEnableSmoothShading(true)
{
    assert(pEngine);
    m_pEngine = pEngine;
    
    m_fRotationX = 0;
    m_fRotationY = 0;
}
CRenderer::~CRenderer() {
    
}

void CRenderer::_init() {
    int begintime = m_pEngine->getClock();
    GL_CHECK();
    
    GL_CHECK( glClearColor(0, 0, 0, 0.0) );
    
    //GL_CHECK( glEnable(GL_MULTISAMPLE) );
    GL_CHECK( glEnable(GL_MULTISAMPLE_ARB ) );
    // GL_CHECK( glEnable(GL_LINE_SMOOTH) );
    // GL_CHECK( glEnable(GL_POLYGON_SMOOTH) );
    // GL_CHECK( glHint(GL_LINE_SMOOTH_HINT, GL_NICEST) );
    // GL_CHECK( glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST) );
    //GL_CHECK( glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST) );
    
    GL_CHECK( glEnable(GL_DEPTH_TEST) );
    GL_CHECK( glDepthFunc(GL_LEQUAL) );
    
    // GL_CHECK( glEnable(GL_TEXTURE_2D) );
    
    m_pEngine->setWatchOutput("_init - LINE " STRINGIZE(__LINE__), m_pEngine->getClock()-begintime, "group='Profile'");
    
    TwDefine(" CG/Profile opened=false ");
    
    GL_CHECK();
}

void CRenderer::light(
    CLight &stLight,
    bool bIsSpecial
) {
    int maxlights;
    GL_CHECK( glGetIntegerv(GL_MAX_LIGHTS, &maxlights) );
    // Check we haven't hit the light limit
    // Reserve a special light for the cusor
    if(m_clLights.size() >= maxlights-(bIsSpecial ? 0 : 1)) return;
    
    m_clLights.push_back(stLight);
    int iLightId = m_clLights.size()-1;
    
    // Set up light parameters
    // W must be 1.0 for positional lights, and 0.0 for directional (like the sun)
    GLfloat aPosition[] = {(float)stLight.pos.x     , (float)stLight.pos.y     , (float)stLight.pos.z     , (float)(stLight.directional ? 0.0 : 1.0)};
    GLfloat aDiffuse [] = {(float)stLight.diffuse .x, (float)stLight.diffuse .y, (float)stLight.diffuse .z, (float)stLight.diffuse .w};
    GLfloat aAmbient [] = {(float)stLight.ambient .x, (float)stLight.ambient .y, (float)stLight.ambient .z, (float)stLight.ambient .w};
    GLfloat aSpecular[] = {(float)stLight.specular.x, (float)stLight.specular.y, (float)stLight.specular.z, (float)stLight.specular.w};
    // GL_CHECK( glEnable (GL_LIGHT0+iLightId) );
    GL_CHECK( glLightfv(GL_LIGHT0+iLightId, GL_POSITION, aPosition) );
    GL_CHECK( glLightfv(GL_LIGHT0+iLightId, GL_DIFFUSE , aDiffuse ) );
    GL_CHECK( glLightfv(GL_LIGHT0+iLightId, GL_AMBIENT , aAmbient ) );
    GL_CHECK( glLightfv(GL_LIGHT0+iLightId, GL_SPECULAR, aSpecular) );
    GL_CHECK( glLightf (GL_LIGHT0+iLightId, GL_CONSTANT_ATTENUATION , stLight.attConstant ) );
    GL_CHECK( glLightf (GL_LIGHT0+iLightId, GL_LINEAR_ATTENUATION   , stLight.attLinear   ) );
    GL_CHECK( glLightf (GL_LIGHT0+iLightId, GL_QUADRATIC_ATTENUATION, stLight.attQuadratic) );
    
    // If we're a spotlight, we need a direction
    if(stLight.spotEnabled) {
        GLfloat aSpotDirection[] = {(float)stLight.spotDirection.x, (float)stLight.spotDirection.y, (float)stLight.spotDirection.z};
        GL_CHECK( glLightf (GL_LIGHT0+iLightId, GL_SPOT_CUTOFF   , stLight.spotCutoff  ) );
        GL_CHECK( glLightfv(GL_LIGHT0+iLightId, GL_SPOT_DIRECTION, aSpotDirection      ) );
        GL_CHECK( glLightf (GL_LIGHT0+iLightId, GL_SPOT_EXPONENT , stLight.spotExponent) );
    } else {
       // A cutoff of 180 means we're not a spotlight
       GL_CHECK( glLightf(GL_LIGHT0+iLightId, GL_SPOT_CUTOFF     , 180) );
    }
}

// This seems backwards...
double CRenderer::m_fDetailLevelMinDist = 200;
double CRenderer::m_fDetailLevelMaxDist = 40;
double CRenderer::m_fDetailLevelMin = 0;
double CRenderer::m_fDetailLevelMax = 11;

void CRenderer::onDisplay(
    double fDeltaTime, double fDeltaClock
) {
    GL_CHECK();
    
    if(!m_bInitialised) {
        // Initialise OpenGL stuff
        _init();
        m_bInitialised = true;
    }
    
    if(m_bResized) {
        // Window has changed size, reset viewport coordinates
        GL_CHECK( glViewport(0, 0, (GLsizei)m_iSizeW, (GLsizei)m_iSizeH) );
        m_bResized = false;
    }
    
    // Ignore these lines, they're for debugging
    #ifdef DEBUG
        int begintime = m_pEngine->getClock();
        m_pEngine->setWatchOutput("onDisplay - LINE " STRINGIZE(__LINE__), m_pEngine->getClock()-begintime, "group='Profile'");
    #endif
    
    // Do we have fog?
    bool bEnableFog = true;
    m_pEngine->getOption("Fog - Enabled", &bEnableFog, bEnableFog, "group='Fog'");
    if(bEnableFog) {
        GLuint fogMode = GL_LINEAR;
        GLfloat fogCol[4] = {0.44, 0.68, 0.77, 1.0};
        GLfloat fogDensity = 0.005;
        GLfloat fogStart = 120;
        GLfloat fogEnd = 520.0;
        
        m_pEngine->getOption("Fog - Mode"       , &fogMode      , fogMode          , "group='Fog'");
        m_pEngine->getOption("Fog - Colour"     , (col4*)&fogCol, *((col4*)&fogCol), "group='Fog'");
        m_pEngine->getOption("Fog - Density"    , &fogDensity   , fogDensity       , "group='Fog' min=0");
        m_pEngine->getOption("Fog - Base Start Depth", &fogStart     , fogStart         , "group='Fog'");
        fogStart += m_pEngine->getPlayer()->m_iTopDownViewZoomLevel*4;
        m_pEngine->setWatchOutput("Fog - Start Depth", fogStart, "group='Fog'");
        m_pEngine->getOption("Fog - Base End Depth"  , &fogEnd       , fogEnd           , "group='Fog'");
        fogEnd += m_pEngine->getPlayer()->m_iTopDownViewZoomLevel*4;
        m_pEngine->setWatchOutput("Fog - End Depth", fogEnd, "group='Fog'");
        
        // Clear color should be equal to fog, otherwise you just get blue buildings on a black background
        GL_CHECK( glClearColor(fogCol[0], fogCol[1], fogCol[2], 1) );
        
        // Setup fog parameters
        GL_CHECK( glEnable(GL_FOG) );
        GL_CHECK( glFogi (GL_FOG_MODE   , fogMode     ) );
        GL_CHECK( glFogfv(GL_FOG_COLOR  , fogCol      ) );
        GL_CHECK( glFogf (GL_FOG_DENSITY, fogDensity  ) );
        //GL_CHECK( glHint (GL_FOG_HINT   , GL_DONT_CARE) );
        GL_CHECK( glFogf (GL_FOG_START  , fogStart    ) );
        GL_CHECK( glFogf (GL_FOG_END    , fogEnd      ) );
    } else {
        // Reset clear color in case we turned off fog
        GL_CHECK( glClearColor(0, 0, 0, 1) );
    }
    
    // Clear screen
    GL_CHECK( glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) );
    
    #ifdef DEBUG
        m_pEngine->setWatchOutput("onDisplay - LINE " STRINGIZE(__LINE__), m_pEngine->getClock()-begintime, "group='Profile'");
    #endif
    // Get camera data
    CPlayer::SCameraData stCameraData;
    m_pEngine->getPlayer()->getCameraData(stCameraData);
    // Are we showing the frustum?
    bool bShowFrustum = false;
    m_pEngine->getOption("Frustum - Show", &bShowFrustum, bShowFrustum, "group='LOD'");
    
    
    // Apply our perspective projection for rendering the scene (and also update the frustum)
    m_fFieldOfView = stCameraData.fFieldOfView;
    _perspective();
    
    // Strange bug with VirtualBox where the color matrix is uninitialised... fix it!
    GL_CHECK( glMatrixMode(GL_TEXTURE) );
    GL_CHECK( glLoadIdentity() );
    
    #ifdef DEBUG
        m_pEngine->setWatchOutput("onDisplay - LINE " STRINGIZE(__LINE__), m_pEngine->getClock()-begintime, "group='Profile'");
    #endif
    
    // Should we cull faces pointing away from us?
    bool bCullingEnabled = true;
    m_pEngine->getOption("Backface Culling - Enabled", &bCullingEnabled, bCullingEnabled, "group='Renderer'");
    if(bCullingEnabled) {
        GL_CHECK( glEnable(GL_CULL_FACE) );
        GL_CHECK( glCullFace(GL_BACK) );
    } else {
        GL_CHECK( glDisable(GL_CULL_FACE) );
    }
    
    // Should we cull entities not in the frustum?
    bool bFrustumClippingEnabled = true;
    m_pEngine->getOption("Frustum - Enabled", &bFrustumClippingEnabled, bFrustumClippingEnabled, "group='LOD'");
    
    // Make sure we're using the Model/View matrix
    GL_CHECK( glMatrixMode(GL_MODELVIEW) );
    
    // mat4 assertmat;
    // GL_CHECK( glGetDoublev(GL_MODELVIEW_MATRIX, assertmat) );
    
    // Give us a stack to render our scene
    GL_CHECK( glPushMatrix() );
        GL_CHECK( glLoadIdentity() );
        
        vec3 camerapos = -stCameraData.vPosition;
        quat cameraang = stCameraData.qRotation;
        vec3 cameraforward = cameraang.forward(), cameraup = cameraang.up();
        // Update frustum
        if(bFrustumClippingEnabled) {
            m_stFrustum.setCoordinates(camerapos, cameraforward, cameraup);
        }
        // mat4 m; cameraang.mat(m);
        // GL_CHECK( glMultMatrixd(m.inverse()) );
        
        // Apply camera translation and rotation
        // GL_CHECK( glTranslated(camerapos.x, camerapos.y, camerapos.z) );
        gluLookAt(
            camerapos.x, camerapos.y, camerapos.z,
            camerapos.x+cameraforward.x, camerapos.y+cameraforward.y, camerapos.z+cameraforward.z,
            cameraup.x, cameraup.y, cameraup.z
        );
        
        #ifdef DEBUG
            m_pEngine->setWatchOutput("onDisplay - LINE " STRINGIZE(__LINE__), m_pEngine->getClock()-begintime, "group='Profile'");
        #endif
        int starttime = m_pEngine->getClock();
        
        // Should the scene be illuminated?
        bool bEnableLighting = false;
        // Get maxlights, just to check
        int maxlights;
        GL_CHECK( glGetIntegerv(GL_MAX_LIGHTS, &maxlights) );
        int iLightId = -1;
        m_pEngine->getOption("Lighting - Enabled", &bEnableLighting, bEnableLighting, "group='Renderer' key=l");
        if(bEnableLighting) {
        
            // Should we enable the sun?
            bool bEnableSunlight = true;
            m_pEngine->getOption("Enabled", &bEnableSunlight, bEnableSunlight, "group='Sunlight'");
            if(bEnableSunlight) {
                CRenderer::CLight sunlight;
                sunlight.directional = true;
                sunlight.pos = vec3(-0.3, -1, -0.3).unit();
                sunlight.diffuse = vec3(0, 0, 0);//vec4(251.0/255.0, 231.0/255.0, 128.0/255.0, 1)*1.3;
                sunlight.ambient  = vec4(204.0/255.0, 198.0/255.0, 198.0/255.0, 1)*0.3;
                sunlight.specular = vec4(204.0/255.0, 198.0/255.0, 200.0/255.0, 1)*0.3;
                sunlight.attConstant = 0;
                sunlight.attLinear = 0;
                sunlight.attQuadratic = 0.0;
                
                sunlight.spotEnabled = false;
                sunlight.spotDirection = vec3(0, -1, 0);
                sunlight.spotExponent = 2.0;
                sunlight.spotCutoff = 45;
                m_pEngine->getOption(" Direction", (vec3*)(&sunlight.pos ), *((vec3*)(&sunlight.pos )), "group='Sunlight'");
                m_pEngine->getOption(" Diffuse  - Colour", (col4*)(&sunlight.diffuse ), *((col4*)(&sunlight.diffuse )), "group='Sunlight'");
                m_pEngine->getOption(" Ambient  - Colour", (col4*)(&sunlight.ambient ), *((col4*)(&sunlight.ambient )), "group='Sunlight'");
                m_pEngine->getOption(" Specular - Colour", (col4*)(&sunlight.specular), *((col4*)(&sunlight.specular)), "group='Sunlight'");
                m_pEngine->getOption(" Attenuation - Constant" , &sunlight.attConstant , sunlight.attConstant , "group='Sunlight'");
                m_pEngine->getOption(" Attenuation - Linear"   , &sunlight.attLinear   , sunlight.attLinear   , "group='Sunlight'");
                m_pEngine->getOption(" Attenuation - Quadratic", &sunlight.attQuadratic, sunlight.attQuadratic, "group='Sunlight'");
                m_pEngine->getOption(" Spotlight - Enabled"    , &sunlight.spotEnabled  , sunlight.spotEnabled  , "group='Sunlight'");
                //m_pEngine->getOption(" Spotlight - Direction", &sunlight.spotDirection, sunlight.spotDirection, "group='Sunlight'");
                m_pEngine->getOption(" Spotlight - Exponent"   , &sunlight.spotExponent , sunlight.spotExponent , "group='Sunlight'");
                m_pEngine->getOption(" Spotlight - Cutoff"     , &sunlight.spotCutoff   , sunlight.spotCutoff   , "group='Sunlight' min=0 max=90");
                
                light(sunlight, true);
            }
            
            auto clEntities = m_pEngine->getEntities();
            for(auto itEntity = clEntities.begin(); itEntity != clEntities.end(); itEntity++) {
                CEntity *pEntity = itEntity->second;
                if(!pEntity->isValid()) continue;
                // What lights are there in the scene?
                pEntity->onRender(this, fDeltaTime, fDeltaClock, CEntity::ERenderMode::LIGHTS, 0);
            }
            for(auto itLight = m_clLights.begin(); itLight != m_clLights.end(); itLight++) {
                CLight &stLight = *itLight;
                iLightId = (itLight-m_clLights.begin());
                
                if(iLightId >= maxlights) {
                    throw "Too many lights!";
                }
                // Enable this light!
                GL_CHECK( glEnable (GL_LIGHT0+iLightId) );
            }
            
            GL_CHECK( glEnable(GL_LIGHTING) );
            GL_CHECK( glEnable(GL_COLOR_MATERIAL) );
        }
        // Display some light statistics to the user
        m_pEngine->setWatchOutput("Lighting - Count", iLightId+1, "group='Renderer'");
        m_pEngine->setWatchOutput("Lighting - Max Count", maxlights, "group='Renderer'");
        
        #ifdef DEBUG
            m_pEngine->setWatchOutput("onDisplay - Lights", m_pEngine->getClock()-starttime, "group='Profile'");
        #endif
        
        // Should OpenGL normalize all the normals for us?
        bool bEnableNormalize = false;
        m_pEngine->getOption("Normalisation - Enabled", &bEnableNormalize, bEnableNormalize, "group='Renderer'");
        if(bEnableNormalize) {
            GL_CHECK( glEnable(GL_NORMALIZE) );
        } else {
            GL_CHECK( glDisable(GL_NORMALIZE) );
        }
        
        // Smooth or flat shading?
        if(m_pEngine->getButtonState("Set Smooth Shading", "group='Renderer' key=P")) {
            m_bEnableSmoothShading = true;
        } else if(m_pEngine->getButtonState("Set Flat Shading", "group='Renderer' key=p")) {
            m_bEnableSmoothShading = false;
        }
        m_pEngine->setWatchOutput("Smooth Shading - Enabled", m_bEnableSmoothShading, "group='Renderer'");
        if(m_bEnableSmoothShading) {
            GL_CHECK( glShadeModel(GL_SMOOTH) );
        } else {
            GL_CHECK( glShadeModel(GL_FLAT  ) );
        }
        
        #ifdef DEBUG
            m_pEngine->setWatchOutput("onDisplay - LINE " STRINGIZE(__LINE__), m_pEngine->getClock()-begintime, "group='Profile'");
        #endif
        
        // Should we show the frustum with lines?
        if(bShowFrustum) {
            // m_stFrustum.setCoordinates(vec3(0, 50, 0), vec3(0, -1, 0), vec3(1, 0, 0));
            vec3 o = cameraforward*0.0001;
            vec3 v_rub = m_stFrustum.ntr+o; vec3 v_lub = m_stFrustum.ntl+o;
            vec3 v_ruf = m_stFrustum.ftr  ; vec3 v_luf = m_stFrustum.ftl  ;
            vec3 v_rdb = m_stFrustum.nbr+o; vec3 v_ldb = m_stFrustum.nbl+o;
            vec3 v_rdf = m_stFrustum.fbr  ; vec3 v_ldf = m_stFrustum.fbl  ;
            
            GL_CHECK( glColor3f(1, 1, 1) );
            GL_CHECK( glLineWidth(2) );
            glBegin(GL_LINE_STRIP);
                glVertex3f(v_ldf.x, v_ldf.y, v_ldf.z);
                glVertex3f(v_rdf.x, v_rdf.y, v_rdf.z);
                glVertex3f(v_rdb.x, v_rdb.y, v_rdb.z);
                glVertex3f(v_ldb.x, v_ldb.y, v_ldb.z);
                glVertex3f(v_ldf.x, v_ldf.y, v_ldf.z);
                glVertex3f(v_luf.x, v_luf.y, v_luf.z);
                glVertex3f(v_ruf.x, v_ruf.y, v_ruf.z);
                glVertex3f(v_rub.x, v_rub.y, v_rub.z);
                glVertex3f(v_lub.x, v_lub.y, v_lub.z);
                glVertex3f(v_luf.x, v_luf.y, v_luf.z);
            GL_CHECK( glEnd() );
            
            glBegin(GL_LINES);
                glVertex3f(v_rdf.x, v_rdf.y, v_rdf.z);
                glVertex3f(v_ruf.x, v_ruf.y, v_ruf.z);
                glVertex3f(v_rdb.x, v_rdb.y, v_rdb.z);
                glVertex3f(v_rub.x, v_rub.y, v_rub.z);
                glVertex3f(v_ldb.x, v_ldb.y, v_ldb.z);
                glVertex3f(v_lub.x, v_lub.y, v_lub.z);
            GL_CHECK( glEnd() );
        }
        
        // Should we draw polygons with filling or just outlines?
        bool bEnableWireframe = false;
        m_pEngine->getOption("WireFrame - Enabled", &bEnableWireframe, bEnableWireframe, "group='Renderer'");
        if(bEnableWireframe) {
            GL_CHECK( glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) );
        } else {
            GL_CHECK( glPolygonMode(GL_FRONT_AND_BACK, GL_FILL) );
        }
        
        // Should we accumulate two render buffers for transparency, insteaed of blending transparent objects?
        bool bEnableAccumulation = false;
        m_pEngine->getOption("Accumulation - Enabled", &bEnableAccumulation, bEnableAccumulation, "group='Renderer'");
        
        #ifdef DEBUG
            m_pEngine->setWatchOutput("onDisplay - LINE " STRINGIZE(__LINE__), m_pEngine->getClock()-begintime, "group='Profile'");
        #endif
        
        // Rotate the scene
        m_pEngine->watchVariable("X Rotation", &m_fRotationX, "group='Camera'", false);
        m_pEngine->watchVariable("Y Rotation", &m_fRotationY, "group='Camera'", false);
        glRotated(m_fRotationX, 1.0, 0.0, 0.0);
        glRotated(m_fRotationY, 0.0, 1.0, 0.0);
        
        // Reset entity debug info
        auto clEntities = m_pEngine->getEntities();
        for(auto itEntity = clEntities.begin(); itEntity != clEntities.end(); itEntity++) {
            CEntity *pEntity = itEntity->second;
            pEntity->m_bWasRendered = false;
            pEntity->m_iDebugPassCount = 0;
        }
        
        // Find closest entity in first pass, then make it transparent in the second (if it's a building)
        CEntity *pClosestEntity = NULL;
        double fClosestDist = 9999;
        double fClosestMax = 30;
        
        int normalpasstime = 0;
        int otherpasstime = 0;
        int culltime = 0;
        // Pass 0 = normal, pass 1 = trasparent
        for(int iPass = 0; iPass < 2; iPass++) {
            int passstarttime = m_pEngine->getClock();
            
            glColor4f(1, 1, 1, 1);
            if(iPass == 0) {
                // Disable blending
                GL_CHECK( glBlendColor(1, 1, 1, 1) );
                GL_CHECK( glDisable(GL_BLEND) );
                // GL_CHECK( glEnable(GL_DEPTH_TEST) );
                if(bEnableAccumulation) {
                    GL_CHECK( glAccum(GL_LOAD, 0.5) );
                }
            } else {
                // Enable blending
                GL_CHECK( glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA) );
                GL_CHECK( glEnable(GL_BLEND) );
                // GL_CHECK( glDisable(GL_DEPTH_TEST) );
                if(bEnableAccumulation) {
                    GL_CHECK( glAccum(GL_ACCUM, 0.5) );
                }
            }
            // Render each entity
            int entityid = 0;
            for(auto itEntity = clEntities.begin(); itEntity != clEntities.end(); itEntity++) {
                int starttime = m_pEngine->getClock();
                CEntity *pEntity = itEntity->second;
                if(!pEntity->isValid()) continue;
                //printf("R: %s\n", pEntity->getLabel().c_str());
                bool bShouldRender = true;
                int cullstarttime = m_pEngine->getClock();
                box_aabb stTestBox;
                pEntity->getWorldExtents(stTestBox.min, stTestBox.max);
                
                // How far is this entity away?
                double fDist = (camerapos-stTestBox.clamp(camerapos)).length();
                //m_pEngine->getOption("Far Distance", &m_fDetailLevelMinDist, m_fDetailLevelMinDist, "group='LOD'");
                
                // calculate LOD level from that
                double fLevel = 1-(fDist-m_fDetailLevelMaxDist)/(m_fDetailLevelMinDist-m_fDetailLevelMaxDist);
                int iDetailLevel = std::max(0, (int)(m_fDetailLevelMin+m_fDetailLevelMin+m_fDetailLevelMax*fLevel));
                pEntity->m_fLODDebug = iDetailLevel;
                //m_pEngine->getOption("Detail Level", &iDetailLevel, iDetailLevel, "group='LOD'");
                double fBoxDist = (camerapos-stTestBox.clamp(camerapos)).length();
                if(iPass == 0) {
                    // if(fBoxDist < fClosestDist) {
                        // pClosestEntity = pEntity;
                        // fClosestDist = fBoxDist;
                    // }
                    // First pass, no transparency
                    GL_CHECK( glBlendColor(0, 0, 0, 0) );
                } else if(iPass == 1) {
                    // if(pEntity == pClosestEntity) {
                        // pEntity->m_fOpacity = 0.3;
                    // }
                    // Second pass, make it transparent if required
                    if(pEntity->m_bEnableTransparency && m_pEngine->getPlayer()->m_eViewMode == CPlayer::EViewMode::TOPDOWN) {
                        pEntity->m_fOpacity = std::min(1.0, fBoxDist/fClosestMax);
                    }
                    GL_CHECK( glBlendColor(0, 0, 0, pEntity->m_fOpacity) );
                }
                
                // Should we cull this entity if it's out of the frustum?
                if(bFrustumClippingEnabled) {
                    frustum::EResult result = m_stFrustum.intersect(stTestBox);
                    if(result == frustum::EResult::OUTSIDE) {
                        bShouldRender = false;
                    }
                }
                culltime += m_pEngine->getClock()-cullstarttime;
                
                // Reset materials, incase we forgot to do so in each entity's render (... which I did)
                GLfloat aMatAmbient [] = { 0.0, 0.0, 0.0, 1.0 };
                GLfloat aMatDiffuse [] = { 0.0, 0.0, 0.0, 1.0 };
                GLfloat aMatSpecular[] = { 0.0, 0.0, 0.0, 1.0 };
                GLfloat aMatEmission[] = { 0.0, 0.0, 0.0, 1.0 };
                GLfloat aMatShininess[] = { (float)number_rand()*128 };
                
                GL_CHECK( glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT , aMatAmbient ) );
                GL_CHECK( glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE , aMatDiffuse ) );
                GL_CHECK( glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, aMatSpecular) );
                GL_CHECK( glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, aMatEmission) );
                GL_CHECK( glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, aMatShininess) );
                
                // Should we render?
                if(bShouldRender) {
                    // yay :D
                    if(iPass == 0) {
                        pEntity->onRender(
                            this, fDeltaTime, fDeltaClock,
                            CEntity::ERenderMode::NORMAL,
                            iDetailLevel
                        );
                    } else if(iPass == 1) {
                        if(bEnableAccumulation) {
                            pEntity->onRender(
                                this, fDeltaTime, fDeltaClock,
                                CEntity::ERenderMode::NORMAL,
                                iDetailLevel
                            );
                        }
                        pEntity->onRender(
                            this, fDeltaTime, fDeltaClock,
                            CEntity::ERenderMode::TRANSPARENT,
                            iDetailLevel
                        );
                    }
                    pEntity->m_bWasRendered = true;
                } else if(iPass == 0) {
                    // We were culled, draw our AABB to help debug (in case we were incorrectly culled)
                    pEntity->onRender(this, fDeltaTime, fDeltaClock, CEntity::ERenderMode::CULLED, iDetailLevel);
                }
                
                if(iPass == 1) {
                    // Reset opacity, if we changed it
                    if(pEntity == pClosestEntity) {
                        pEntity->m_fOpacity = 1;
                    }
                }
                int endtime = m_pEngine->getClock();
                pEntity->m_iRenderTimeMs += endtime-starttime;
                #ifdef DEBUG
                    if(endtime-starttime > 8) {
                        std::stringstream ss;
                        ss << "onDisplay - Entity " << entityid;
                        m_pEngine->setWatchOutput(ss.str(), endtime-starttime, "group='Profile'");
                    }
                #endif
                entityid++;
            }
            if(iPass == 0) {
                normalpasstime = m_pEngine->getClock()-passstarttime;
            } else {
                otherpasstime = m_pEngine->getClock()-passstarttime;
            }
        }
        // Display debug profiling information to user
        #ifdef DEBUG
            m_pEngine->setWatchOutput("onDisplay - Pass 1", normalpasstime, "group='Profile'");
            m_pEngine->setWatchOutput("onDisplay - Pass 2", otherpasstime, "group='Profile'");
            m_pEngine->setWatchOutput("onDisplay - Cull", culltime, "group='Profile'");
            m_pEngine->setWatchOutput("onDisplay - LINE " STRINGIZE(__LINE__), m_pEngine->getClock()-begintime, "group='Profile'");
        #endif
        if(bEnableAccumulation) {
            GL_CHECK( glAccum(GL_RETURN, 1) );
        }
        
        // Reset OpenGL state
        GL_CHECK( glDisable(GL_NORMALIZE) );
        GL_CHECK( glDisable(GL_COLOR_MATERIAL) );
        GL_CHECK( glDisable(GL_LIGHTING) );
        GL_CHECK( glDisable(GL_FOG) );
        
        for(auto itLight = m_clLights.begin(); itLight != m_clLights.end(); itLight++) {
            CLight &stLight = *itLight;
            int iLightId = (itLight-m_clLights.begin());
            GL_CHECK( glDisable(GL_LIGHT0+iLightId) );
        }
        m_clLights.clear();
        
        #ifdef DEBUG
            m_pEngine->setWatchOutput("onDisplay - LINE " STRINGIZE(__LINE__), m_pEngine->getClock()-begintime, "group='Profile'");
        #endif
    GL_CHECK( glPopMatrix() );
    
    #ifdef DEBUG
        m_pEngine->setWatchOutput("onDisplay - LINE " STRINGIZE(__LINE__), m_pEngine->getClock()-begintime, "group='Profile'");
    #endif
    
    // Load orthographic projection for rendering on the screen in 2D
    _orthographic();
    GL_CHECK( glMatrixMode(GL_MODELVIEW) );
    GL_CHECK( glLoadIdentity() );
    
    m_iOverlayYPos = m_pOverlayFontYInterval;
    
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glColor4f(1, 1, 1, 1);
    
    //text("blah!");
    
    for(auto itEntity = clEntities.begin(); itEntity != clEntities.end(); itEntity++) {
        int starttime = m_pEngine->getClock();
        CEntity *pEntity = itEntity->second;
        if(!pEntity->isValid()) continue;
        // Let each entity render on the screen
        pEntity->onRender(
            this, fDeltaTime, fDeltaClock,
            CEntity::ERenderMode::OVERLAY,
            0
        );
    }
    
    glEnable(GL_DEPTH_TEST);
    
    _perspective();
    
    #ifdef DEBUG
        m_pEngine->setWatchOutput("onDisplay - LINE " STRINGIZE(__LINE__), m_pEngine->getClock()-begintime, "group='Profile'");
    #endif
    
    // mat4 assertmat2;
    // glGetDoublev(GL_MODELVIEW_MATRIX, assertmat2);
    // #ifdef DEBUG
        // assert(assertmat == assertmat2);
    // #endif
    
    GL_CHECK();
}

void *CRenderer::m_pOverlayFont = GLUT_BITMAP_9_BY_15;
int CRenderer::m_pOverlayFontYInterval = 17;

void CRenderer::text(const char *fmt, ...) {
    va_list args;
    
    // Usable like printf with fmt and args
    char *str;
    va_start(args, fmt);
    int res = vasprintf(&str, fmt, args);
    va_end(args);
    
    if(res >= 0) {
        // Find out how wide the string is...
        int strwidth = 5;
        for(char *ch = str; *ch != '\0'; ch++) {
            strwidth += glutBitmapWidth(m_pOverlayFont, *ch);
        }
        
        double rgb[3];
        hsvToRgb(fmod((double)m_iOverlayYPos/5.0, 1), 1, 0.45, rgb);
        glColor4f(rgb[0], rgb[1], rgb[2], 1);
        // ... and shift it to the left by that much, it's right-justified
        glRasterPos2f(m_iSizeW-strwidth+1, m_iOverlayYPos+1);
        for(char *ch = str; *ch != '\0'; ch++) {
            glutBitmapCharacter(m_pOverlayFont, *ch);
        }
        // Draw the text! :)
        glColor4f(1, 1, 1, 1);
        glRasterPos2f(m_iSizeW-strwidth, m_iOverlayYPos);
        for(char *ch = str; *ch != '\0'; ch++) {
            glutBitmapCharacter(m_pOverlayFont, *ch);
        }
        
        //printf("%s\n", str);
        // Move the next bit of text downwards
        m_iOverlayYPos += m_pOverlayFontYInterval;
    } else {
        throw "vasprintf failed";
    }
}

void CRenderer::_perspective() {
    // Don't ask me why there's an underscore.. I forgot
    
    // Load projection matrix
    GL_CHECK( glMatrixMode(GL_PROJECTION) );
    // Reset it
    GL_CHECK( glLoadIdentity() );
    double aspect = (double)m_iSizeW/(double)m_iSizeH;
    double dnear = 1;
    double dfar = 550.0; // Too far?
    // Modify projection matrix for perspective
    GL_CHECK( gluPerspective(m_fFieldOfView, aspect, dnear, dfar) );
    if(m_fFieldOfView > 0) {
        // If we're debugging the frustum, it may have a different FOV
        double fFrustomFOV = m_fFieldOfView;
        m_pEngine->getOption("Frustum - FOV", &fFrustomFOV, fFrustomFOV, "group='LOD'");
        
        bool bShowFrustum = false;
        m_pEngine->getOption("Frustum - Show", &bShowFrustum, bShowFrustum, "group='LOD'");
        if(!bShowFrustum) {
            fFrustomFOV = m_fFieldOfView;
            m_pEngine->setOption("Frustum - FOV", fFrustomFOV);
        }
        // Update our frustum
        m_stFrustum.setPerspective(fFrustomFOV, aspect, dnear, dfar);
    }
}

void CRenderer::_orthographic() {
    GL_CHECK( glMatrixMode(GL_PROJECTION) );
    GL_CHECK( glLoadIdentity() );
    
    // Pixel-coordinates are nicer than 0-1
    // Y is flipped to make it top-to-bottom
    GL_CHECK( gluOrtho2D(0, m_iSizeW, m_iSizeH, 0) );
    //GL_CHECK( gluOrtho2D(0, 1, 1, 0) );
    
}


std::vector<std::string> CRenderer::findTexture(const std::string &sTextureFileName) {
    int max = 9; // *sigh* too lazy :P
    std::vector<std::string> result;
    FILE *file;
    for(int i = 0; i <= max; i++) {
        std::string name = sTextureFileName;
        std::replace(name.begin(), name.end(), '*', (char)('0'+i));
        file = fopen(name.c_str(), "r");
        if(file != NULL) {
            //printf("?!? %s\n", name.c_str());
            result.push_back(name);
            fclose(file);
        }
    }
    return result;
}

// http://stackoverflow.com/a/15813530/837856
bool notisprint(char c) {
    return !isprint(c);
}
void stripUnicode(std::string &str) {
    str.erase(std::remove_if(str.begin(), str.end(), notisprint), str.end());
}

unsigned int CRenderer::loadTexture(const std::string &sTextureFileName) {
    std::string texname = sTextureFileName;
    stripUnicode(texname);
    if(texname == "") {
        return 0;
    }
    
    if(texname.find("*")) {
        std::vector<std::string> choices = findTexture(texname);
        if(choices.size() > 0) {
            texname = choices[randrange(0, choices.size()-1)];
        }
    }
    auto it = m_clTextureCache.find(texname);
    if(it != m_clTextureCache.end()) {
        return it->second;
    }
    std::cout << "Texture loaded: " << m_iTextureIndex << " -> " << texname << std::endl;
    unsigned int iTexIdx = SOIL_load_OGL_texture(
        texname.c_str(),
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,//m_iTextureIndex,
        SOIL_FLAG_INVERT_Y | SOIL_FLAG_MIPMAPS // generate MIP maps, please!
    );
    if(iTexIdx != 0) {
        GL_CHECK( glEnable(GL_TEXTURE_2D) );
        GL_CHECK( glBindTexture(GL_TEXTURE_2D, iTexIdx) );
        
        // Repeat texture if too small (yay seamless textures)
        GL_CHECK( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT) );
        GL_CHECK( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT) );
        // Tell OpenGL to use MIP maps when filtering
        GL_CHECK( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR) );
        GL_CHECK( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR) );
        
        GL_CHECK( glBindTexture(GL_TEXTURE_2D, 0) );
        
        m_clTextureCache.insert(std::pair<std::string, unsigned int>(texname, iTexIdx));
        m_iTextureIndex++;
        return iTexIdx;
    }
    std::cout << "Failed to load texture " << texname << ": " << SOIL_last_result() << std::endl;
    return 0;
}

unsigned int CRenderer::uploadTexture(const std::string &sLabel, int iWidth, int iHeight, unsigned char *pData) {
    unsigned int iTexIdx;
    GL_CHECK( glGenTextures(1, &iTexIdx) );
    if(iTexIdx != 0) {
        GL_CHECK( glActiveTexture(GL_TEXTURE0) );
        GL_CHECK( glBindTexture(GL_TEXTURE_2D, iTexIdx) );
        
        GL_CHECK( glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, iWidth, iHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, pData) );
        
        GL_CHECK( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE) );
        GL_CHECK( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE) );
        GL_CHECK( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR) );
        GL_CHECK( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR) );
        
        // GL_CHECK( glGenerateMipmap(GL_TEXTURE_2D) );
        // Mipmaps are... difficult :S
        
        GL_CHECK( glBindTexture(GL_TEXTURE_2D, 0) );
        
        m_clTextureCache.insert(std::pair<std::string, unsigned int>(sLabel, iTexIdx));
        m_iTextureIndex++;
        
        return iTexIdx;
    }
    std::cerr << "Failed to upload texture " << sLabel << ": " << gluErrorString(glGetError()) << std::endl;
    return 0;
}

void CRenderer::screenToWorld(const vec3 vScreen, vec3 &vWorldPos, vec3 &vWorldDir) {
    if(!m_bInitialised) {
        return;
    }
    /*
        What's happening in this function is:
            - Get projection matrix (it'll be in perspective, unless you call it during the Overlay pass)
            - Set the model view matrix to camera-orientation only
            - Get the model view matrix
            - Calculate their inverses
            - Take a point, put it through that, and get the starting point on the near-plane
            - Take the same point, but it through with w = 0, and get the direction from the near-plane
            
            The two together give you a ray representing everything the user could have their mouse over.
            Use it wisely.
    */
    
    
    // Get camera data
    CPlayer::SCameraData stCameraData;
    m_pEngine->getPlayer()->getCameraData(stCameraData);
    
    // if(!number_eq(m_fFieldOfView, stCameraData.fFieldOfView)) {
    /* if(true) {
        // Field of view has changed, redo projection matrix
        m_fFieldOfView = stCameraData.fFieldOfView;
        _perspective();
    } */
    // Get the projection matrix
    GL_CHECK();
    
    mat4 mProjectionMatrix; {
        GL_CHECK( glGetDoublev(GL_PROJECTION_MATRIX , mProjectionMatrix) );
    };
    // Get the model/view matrix
    mat4 mModelViewMatrix; {
        GL_CHECK( glMatrixMode(GL_MODELVIEW) );
        GL_CHECK( glPushMatrix() );
            GL_CHECK( glLoadIdentity() );
            vec3 camerapos = -stCameraData.vPosition;
            quat cameraang = stCameraData.qRotation;
            
            // mat4 m; cameraang.mat(m);
            // glMultMatrixd(m.inverse());
            // glTranslated(camerapos.x, camerapos.y, camerapos.z);
            vec3 forward = cameraang.forward(), up = cameraang.up();
            gluLookAt(
                camerapos.x, camerapos.y, camerapos.z,
                camerapos.x+forward.x, camerapos.y+forward.y, camerapos.z+forward.z,
                up.x, up.y, up.z
            );
            
            GL_CHECK( glGetDoublev(GL_MODELVIEW_MATRIX, mModelViewMatrix) );
        GL_CHECK( glPopMatrix() );
    };
    double fX = 2.0*vScreen.x/(double)m_iSizeW-1.0;
    double fY = 1.0-2.0*vScreen.y/(double)m_iSizeH;
    vec4 vTempPosScreen(fX, fY,  0.0,  1.0);
    vec4 vTempDirScreen(fX, fY, -0.0,  1.0);
    
    vec4 vTempPosEye = vTempPosScreen*mProjectionMatrix.inverse();
    vec4 vTempDirEye = vTempDirScreen*mProjectionMatrix.inverse();
    vTempDirEye.z = -1.0;
    vTempDirEye.w =  0.0;
    
    vec4 vTempPosWorld = vTempPosEye*mModelViewMatrix.inverse();
    vec4 vTempDirWorld = vTempDirEye*mModelViewMatrix.inverse();
    
    vTempPosWorld.x /= vTempPosWorld.w;
    vTempPosWorld.y /= vTempPosWorld.w;
    vTempPosWorld.z /= vTempPosWorld.w;
    vTempPosWorld.w /= vTempPosWorld.w;
    
    vWorldPos.x = vTempPosWorld.x;
    vWorldPos.y = vTempPosWorld.y;
    vWorldPos.z = vTempPosWorld.z;
    vWorldDir = vec3(vTempDirWorld.x, vTempDirWorld.y, vTempDirWorld.z).unit();
}

void CRenderer::pushEntity(CEntity *pEntity, bool bWithRotation) {
    
    // glGetDoublev(GL_MODELVIEW_MATRIX, pEntity->debugmat);
    
    // Push the modelview matrix so we can render the entity in it's local space
    GL_CHECK( glPushMatrix() );
    
    vec3 pos = pEntity->getPosition();
    GL_CHECK( glTranslated(pos.x, pos.y, pos.z) );
    
    if(bWithRotation) {
        quat ang = pEntity->getRotation();
        mat4 m; ang.mat(m);
        GL_CHECK( glMultMatrixd(m) );
    }
    /*glRotated(M_DEG(ang.y), 0.0, 1.0, 0.0);
    glRotated(M_DEG(ang.p), 1.0, 0.0, 0.0);
    glRotated(M_DEG(ang.r), 0.0, 0.0, 1.0);*/
}
void CRenderer::popEntity(CEntity *pEntity) {
    // Pop goes the weasel!
    GL_CHECK( glPopMatrix() );
    // #ifdef DEBUG
        // mat4 wat2;
        // glGetDoublev(GL_MODELVIEW_MATRIX, wat2);
        // assert(pEntity->debugmat == wat2);
    // #endif
}

void CRenderer::onResize(
    int iSizeW, int iSizeH
) {
    m_iSizeW = iSizeW; m_iSizeH = iSizeH;
    m_bResized = true;
}
