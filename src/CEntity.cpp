#include <cassert>
#include <GL/freeglut.h>
#include <cstdio>

class CEntity;
#include "CEntity.h"

#include "CEngine.h"
#include "CPlayer.h"

CEntity::CEntity(
    CEngine *pEngine
) : m_bChangeMarked(true),
    m_vPosition(),
    m_qRotation(),
    m_bIsValid(true),
    m_sLabel("?!?"),
    m_eActiveTweenType(NONE),
    m_vTweenStartPos(),
    m_vTweenEndPos(),
    m_fTweenStartTime(0),
    m_fTweenEndTime(0),
    m_fLODDebug(0),
    m_fOpacity(1),
    m_bEnableTransparency(false),
    m_bRenderInitialised(false)
{
    m_pEngine = pEngine;
    assert(m_pEngine);
    m_pEngine->addEntity(this);
    
    m_vExtentsMin = vec3();
    m_vExtentsMax = vec3();
    
    dbgtext = "";
}
CEntity::~CEntity() {
    /*if(m_pParent) {
        m_pParent->removeChild(this);
    }*/
    m_pEngine->removeEntity(this);
}

void CEntity::_onChange() {
    calculateExtents(m_vExtentsMin, m_vExtentsMax);
}
void CEntity::_think(
    double fTime, double fDeltaTime
) {
    /*vec3 pos = getPosition();
    pos.y += sin(fTime)*fDeltaTime;
    setPosition(pos);*/
    //vec3 vMousePos = m_pEngine->getPlayer()->getPointerPosition();
    //setPosition(m_pEngine->getRenderer()->screenToWorld(vMousePos, 5.0));
}
void CEntity::_render(
    CRenderer *pRenderer,
    double fDeltaTime, double fDeltaClock,
    CEntity::ERenderMode eMode,
    int iDetailLevel
) {
    if(eMode == CEntity::ERenderMode::LIGHTS) {
        CRenderer::CLight light;
        light.directional = false;
        light.pos = vec3(0, 0, 0);
        light.diffuse = vec4(1, 1, 1, 1);
        light.ambient  = vec4(0, 0, 0, 0);
        light.specular = vec4(0, 0, 0, 0);
        light.attConstant = 0.1;
        light.attLinear = 0;
        light.attQuadratic = 0.0005;
        
        light.spotEnabled = false;
        //light.spotDirection = vec3(cos(m_pEngine->getTime()+fDeltaTime), 0.3*sin(m_pEngine->getTime()+fDeltaTime+2), sin(m_pEngine->getTime()+fDeltaTime));
        light.spotDirection = vec3(0, -1, 0);
        light.spotExponent = 2.0;
        light.spotCutoff = 45;
        
        m_pEngine->getOption("Diffuse  - Colour", (col4*)(&light.diffuse ), *((col4*)(&light.diffuse )), "group='Test Light'");
        m_pEngine->getOption("Ambient  - Colour", (col4*)(&light.ambient ), *((col4*)(&light.ambient )), "group='Test Light'");
        m_pEngine->getOption("Specular - Colour", (col4*)(&light.specular), *((col4*)(&light.specular)), "group='Test Light'");
        m_pEngine->getOption("Attenuation - Constant" , &light.attConstant , light.attConstant , "group='Test Light' min=0");
        m_pEngine->getOption("Attenuation - Linear"   , &light.attLinear   , light.attLinear   , "group='Test Light' min=0");
        m_pEngine->getOption("Attenuation - Quadratic", &light.attQuadratic, light.attQuadratic, "group='Test Light' min=0");
        m_pEngine->getOption("Spotlight - Enabled"  , &light.spotEnabled  , light.spotEnabled  , "group='Test Light'");
        //m_pEngine->getOption("Spotlight - Direction", &light.spotDirection, light.spotDirection, "group='Test Light'");
        m_pEngine->getOption("Spotlight - Exponent" , &light.spotExponent , light.spotExponent , "group='Test Light'");
        m_pEngine->getOption("Spotlight - Cutoff"   , &light.spotCutoff   , light.spotCutoff   , "group='Test Light' min=0 max=90");
        
        pRenderer->light(light);
        
        return;
    }
    
    if(eMode != CEntity::ERenderMode::NORMAL) return;
    GL_CHECK( glLineWidth(2) );
    glBegin(GL_LINES);
        glColor3f ( 1.0, 0.0, 0.0);
        glVertex3f(-0.0, 0.0, 0.0);
        glVertex3f( 1.0, 0.0, 0.0);
        glColor3f ( 0.0, 1.0, 0.0);
        glVertex3f( 0.0,-0.0, 0.0);
        glVertex3f( 0.0, 1.0, 0.0);
        glColor3f ( 0.0, 0.0, 1.0);
        glVertex3f( 0.0, 0.0,-0.0);
        glVertex3f( 0.0, 0.0, 1.0);
    GL_CHECK( glEnd() );
    
}

void CEntity::onThink(
    double fTime, double fDeltaTime
) {
    if(m_bChangeMarked) {
        _onChange();
        m_bChangeMarked = false;
    }
    if(m_eActiveTweenType != NONE) {
        double t = (fTime-m_fTweenStartTime)/(m_fTweenEndTime-m_fTweenStartTime);
        setPosition(m_vTweenStartPos.lerp(m_vTweenEndPos, t, true));
        if(t >= 1.0) {
            m_eActiveTweenType = NONE;
        }
    }
    
    _think(fTime, fDeltaTime);
    
    /*for(auto itChild = m_clChildren.begin(); itChild != m_clChildren.end(); itChild++) {
        if(auto pChild = itChild->lock()) {
            pChild->onThink(fTime, fDeltaTime);
        } else {
            // Entity has been deleted
            itChild = m_clChildren.erase(itChild);
        }
    }*/
}

void CEntity::renderInit() {
    
}

void CEntity::onRender(
    CRenderer *pRenderer,
    double fDeltaTime, double fDeltaClock,
    CEntity::ERenderMode eMode,
    int iDetailLevel
) {
    if(!m_bRenderInitialised) {
        renderInit();
        m_bRenderInitialised = true;
    }
    if(eMode != LIGHTS) {
        GLfloat aMatSpecular[] = { 1.0, 1.0, 1.0, 1.0 };
        GLfloat aMatShininess[] = { 100.0 };
        GL_CHECK( glMaterialfv(GL_FRONT, GL_SPECULAR, aMatSpecular) );
        GL_CHECK( glMaterialfv(GL_FRONT, GL_SHININESS, aMatShininess) );
    }
    //if(eMode == NORMAL) {
    if(true) {
        GL_CHECK( pRenderer->pushEntity(this) );
            _render(pRenderer, fDeltaTime, fDeltaClock, eMode, iDetailLevel);
            /*for(auto itChild = m_clChildren.begin(); itChild != m_clChildren.end(); itChild++) {
                if(auto pChild = itChild->lock()) {
                    pChild->onRender(pRenderer, fDeltaTime, fDeltaClock);
                }
            }*/
        GL_CHECK( pRenderer->popEntity(this) );
    }
    if(eMode == CEntity::ERenderMode::NORMAL || eMode == CEntity::ERenderMode::CULLED) {
        GL_CHECK( pRenderer->pushEntity(this, false) );
        
            // quat ang = getRotation();
            // mat4 m; ang.mat(m);
            
            box_aabb stTestBox;
            getWorldExtents(stTestBox.min, stTestBox.max);
            vec3 v_rub, v_ruf, v_rdb, v_rdf, v_lub, v_luf, v_ldb, v_ldf;
            stTestBox.getCorners(v_rub, v_ruf, v_rdb, v_rdf, v_lub, v_luf, v_ldb, v_ldf);
            
            v_rub = v_rub-getPosition();
            v_ruf = v_ruf-getPosition();
            v_rdb = v_rdb-getPosition();
            v_rdf = v_rdf-getPosition();
            v_lub = v_lub-getPosition();
            v_luf = v_luf-getPosition();
            v_ldb = v_ldb-getPosition();
            v_ldf = v_ldf-getPosition();
            
            if(m_vHighlightColor == vec3(0, 0, 0) && eMode == CULLED) {
                m_vHighlightColor = vec3(0.3, 0.3, 0.3);
            }
            
            if(m_vHighlightColor != vec3(0, 0, 0)) {
                double outset = 0.05;
                GL_CHECK( glColor3f(m_vHighlightColor.x, m_vHighlightColor.y, m_vHighlightColor.z) );
                GL_CHECK( glLineWidth(2) );
                glBegin(GL_LINE_STRIP);
                    glVertex3f(v_ldf.x-outset, v_ldf.y-outset, v_ldf.z-outset);
                    glVertex3f(v_rdf.x+outset, v_rdf.y-outset, v_rdf.z-outset);
                    glVertex3f(v_rdb.x+outset, v_rdb.y-outset, v_rdb.z+outset);
                    glVertex3f(v_ldb.x-outset, v_ldb.y-outset, v_ldb.z+outset);
                    glVertex3f(v_ldf.x-outset, v_ldf.y-outset, v_ldf.z-outset);
                    glVertex3f(v_luf.x-outset, v_luf.y+outset, v_luf.z-outset);
                    glVertex3f(v_ruf.x+outset, v_ruf.y+outset, v_ruf.z-outset);
                    glVertex3f(v_rub.x+outset, v_rub.y+outset, v_rub.z+outset);
                    glVertex3f(v_lub.x-outset, v_lub.y+outset, v_lub.z+outset);
                    glVertex3f(v_luf.x-outset, v_luf.y+outset, v_luf.z-outset);
                GL_CHECK( glEnd() );
                
                glBegin(GL_LINES);
                    glVertex3f(v_rdf.x+outset, v_rdf.y-outset, v_rdf.z-outset);
                    glVertex3f(v_ruf.x+outset, v_ruf.y+outset, v_ruf.z-outset);
                    glVertex3f(v_rdb.x+outset, v_rdb.y-outset, v_rdb.z+outset);
                    glVertex3f(v_rub.x+outset, v_rub.y+outset, v_rub.z+outset);
                    glVertex3f(v_ldb.x-outset, v_ldb.y-outset, v_ldb.z+outset);
                    glVertex3f(v_lub.x-outset, v_lub.y+outset, v_lub.z+outset);
                GL_CHECK( glEnd() );
            }
        GL_CHECK( pRenderer->popEntity(this) );
    }
}

CEntity& CEntity::tween(vec3 vStartPos, vec3 vEndPos, double fDuration, ETweenType eType) {
    m_eActiveTweenType = eType;
    m_vTweenStartPos = vStartPos;
    m_vTweenEndPos = vEndPos;
    m_fTweenStartTime = m_pEngine->getTime();
    m_fTweenEndTime = m_fTweenStartTime+fDuration;
    
    return *this;
}
void CEntity::getWorldExtents(vec3 &vExtentsMin, vec3 &vExtentsMax) {
  /*   mat4 mModelViewMatrix; {
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
            glLoadIdentity();
            
            vec3 pos = getPosition();
            glTranslated(pos.x, pos.y, pos.z);
            
            quat ang = getRotation();
            mat4 m; ang.mat(m);
            glMultMatrixd(m);
            
            glGetDoublev(GL_MODELVIEW_MATRIX , mModelViewMatrix);
            
        glPopMatrix();
    };
    box_aabb stTestBox;
    getExtents(stTestBox.min, stTestBox.max);
    vec3 v_rub, v_ruf, v_rdb, v_rdf, v_lub, v_luf, v_ldb, v_ldf;
    stTestBox.getCorners(v_rub, v_ruf, v_rdb, v_rdf, v_lub, v_luf, v_ldb, v_ldf);
    
    //mModelViewMatrix.print();
    
    v_rub = v_rub*mModelViewMatrix; v_lub = v_lub*mModelViewMatrix;
    v_ruf = v_ruf*mModelViewMatrix; v_luf = v_luf*mModelViewMatrix;
    v_rdb = v_rdb*mModelViewMatrix; v_ldb = v_ldb*mModelViewMatrix;
    v_rdf = v_rdf*mModelViewMatrix; v_ldf = v_ldf*mModelViewMatrix;
    
    stTestBox.setFromCloud({v_rub, v_ruf, v_rdb, v_rdf, v_lub, v_luf, v_ldb, v_ldf});
    
    vExtentsMin = stTestBox.min;
    vExtentsMax = stTestBox.max; */
    
    getLocalExtents(vExtentsMin, vExtentsMax);
    vExtentsMin = vExtentsMin+getPosition();
    vExtentsMax = vExtentsMax+getPosition();
}
void CEntity::getLocalExtents(vec3 &vExtentsMin, vec3 &vExtentsMax) {
    quat ang = getRotation();
    mat4 m; ang.mat(m);
    
    box_aabb stTestBox;
    getExtents(stTestBox.min, stTestBox.max);
    vec3 v_rub, v_ruf, v_rdb, v_rdf, v_lub, v_luf, v_ldb, v_ldf;
    stTestBox.getCorners(v_rub, v_ruf, v_rdb, v_rdf, v_lub, v_luf, v_ldb, v_ldf);
    
    //mModelViewMatrix.print();
    
    v_rub = v_rub*m; v_lub = v_lub*m;
    v_ruf = v_ruf*m; v_luf = v_luf*m;
    v_rdb = v_rdb*m; v_ldb = v_ldb*m;
    v_rdf = v_rdf*m; v_ldf = v_ldf*m;
    
    stTestBox.setFromCloud({v_rub, v_ruf, v_rdb, v_rdf, v_lub, v_luf, v_ldb, v_ldf});
    
    vExtentsMin = stTestBox.min;
    vExtentsMax = stTestBox.max;
}

/*
CEntity& CEntity::SetParent(CEntity *pEntity) {
    if(pEntity != NULL) {
        pEntity->addChild(this); // will remove us from previous parent for us
    } else if(m_pParent != NULL) {
        m_pParent->removeChild(this);
    }
    mark();
    return *this;
}
CEntity& CEntity::addChild(CEntity *pEntity) {
    assert(pEntity);
    if(pEntity->m_pParent != NULL) {
        pEntity->m_pParent->removeChild(pEntity);
    }
    pEntity->m_pParent = NULL;
    
    bool bSuccess = m_clChildren.insert(std::shared_ptr<CEntity>(pEntity)).second;
    if(bSuccess) {
        onChildAttached(pEntity);
    }
    return *this;
}
bool CEntity::removeChild(CEntity *pEntity) {
    assert(pEntity);
    if(pEntity->m_pParent == this) {
        onChildDetached(pEntity);
        pEntity->m_pParent = NULL;
    }
    std::shared_ptr<CEntity> sptr(pEntity);
    bool bSuccess = (m_clChildren.erase(std::weak_ptr<CEntity>(sptr)) > 0);
    return bSuccess;
}
*/