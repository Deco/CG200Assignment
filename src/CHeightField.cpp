
#include "matrix.h"
#include "GL/freeglut.h"

#include "CHeightField.h"

CHeightField::CHeightField(
    CEngine *pEngine,
    int iGridW, int iGridH
) : CEntity(pEngine),
    m_iGridW(iGridW), m_iGridH(iGridH)
{
    m_vExtentsMin = vec3(-0.5, -0.5, -0.5);
    m_vExtentsMax = vec3( 0.5,  0.5,  0.5);
    
    m_aafGridValues = new double[m_iGridW*m_iGridH];
    
    for(int i = 0; i < m_iGridW*m_iGridH; i++) {
        m_aafGridValues[i] = number_rand()*0.0;
    }
}
CHeightField::~CHeightField() {
    
}

void CHeightField::_think(
    double fTime, double fDeltaTime
) {
    /*vec3 pos = getPosition();
    pos.y += sin(fTime)*fDeltaTime;
    setPosition(pos);*/
    /*vec3 vMousePos = m_pEngine->getPlayer()->getPointerPosition();
    setPosition(m_pEngine->getRenderer()->screenToWorld(vMousePos, 5.0));*/
}
void CHeightField::_render(
    CRenderer *pRenderer,
    double fDeltaTime, double fDeltaClock,
    CEntity::ERenderMode eMode,
    int iDetailLevel
) {
    if(eMode != CEntity::ERenderMode::NORMAL) return;
    
    double xmin = m_vExtentsMin.x; double xmax = m_vExtentsMax.x;
    double ymin = m_vExtentsMin.y; double ymax = m_vExtentsMax.y;
    double cw = (xmax-xmin)/(double)m_iGridW;
    double ch = (ymax-ymin)/(double)m_iGridH;
    xmin += cw/2.0; ymin += ch/2.0;
    
    glColor3f(0.0, 1.0, 0.0);
    glLineWidth(2);
    glBegin(GL_LINES);
        for(int y = 0; y < m_iGridH; y++) {
            for(int x = 0; x < m_iGridW; x++) {
                double v = m_aafGridValues[y*m_iGridH+x];
                if(x < m_iGridW-1) {
                    double ov = m_aafGridValues[y*m_iGridH+(x+1)];
                    glVertex3d(xmin+cw*(x  ),  v, ymin+ch*(y  ));
                    glVertex3d(xmin+cw*(x+1), ov, ymin+ch*(y+0));
                }
                if(y < m_iGridH-1) {
                    double ov = m_aafGridValues[(y+1)*m_iGridH+x];
                    glVertex3d(xmin+cw*(x  ),  v, ymin+ch*(y  ));
                    glVertex3d(xmin+cw*(x+0), ov, ymin+ch*(y+1));
                }
            }
        }
    glEnd();
    
    glColor3f(0.0, 0.0, 1.0);
    glBegin(GL_LINES);
        glVertex3f(-1.0, 0.0, 0.0);
        glVertex3f( 1.0, 0.0, 0.0);
        glVertex3f( 0.0,-1.0, 0.0);
        glVertex3f( 0.0, 1.0, 0.0);
        glVertex3f( 0.0, 0.0,-1.0);
        glVertex3f( 0.0, 0.0, 1.0);
    glEnd();
}

bool CHeightField::calculateExtents(vec3 &vExtentsMin, vec3 &vExtentsMax) const {
    return false;
}
