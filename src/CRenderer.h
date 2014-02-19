#pragma once
#ifndef id7F1E657E_4114_469E_966483D5C3B805E9
#define id7F1E657E_4114_469E_966483D5C3B805E9

#include <memory>
#include <map>
#include <sstream>
#include "matrix.h"

#include "GL/freeglut.h"

// http://stackoverflow.com/a/11258474/837856
void CheckOpenGLError(const char* stmt, const char* fname, int line);
void ClearOpenGLError();

#ifdef DEBUG
    #define CHECK_OPENGL_ERRORS 1
#endif

#if CHECK_OPENGL_ERRORS > 0
    #define GL_CHECK(stmt) do { \
            /* CheckOpenGLError("BEFORE", __FILE__, __LINE__); */ \
            stmt; \
            CheckOpenGLError(#stmt, __FILE__, __LINE__); \
        } while(0)
#else
    #define GL_CHECK(stmt) stmt
#endif

// http://stackoverflow.com/a/2670913/837856
#define STRINGIZE_DETAIL(tok) #tok // strange, but it solves the problem :)
#define STRINGIZE(tok) STRINGIZE_DETAIL(tok)

class CRenderer;
class CEngine;
class CEntity;
class CProp;
#include "CProp.h"

class CRenderer {
    
    public: // typedefs
        class CLight {
            public:
                CLight()
                  : pos(0, 0, 0),
                    diffuse(1, 1, 1, 1),
                    ambient(1, 1, 1, 1),
                    specular(1, 1, 1, 1),
                    attConstant(0),
                    attLinear(0),
                    attQuadratic(0),
                    spotEnabled(false)
                {
                    // 
                };
                bool directional;
                vec3 pos;
                vec4 diffuse;
                vec4 ambient;
                vec4 specular;
                double attConstant;
                double attLinear;
                double attQuadratic;
                
                bool spotEnabled;
                vec3 spotDirection;
                double spotExponent;
                double spotCutoff;
        };
        
    public: // constructors/destructors
        CRenderer(CEngine *pEngine);
        virtual ~CRenderer();
        
    public: // public methods
        CEngine *getEngine();
        
        void pushEntity(CEntity *pEntity, bool bWithRotation = true);
        void popEntity(CEntity *pEntity);
        
        void screenToWorld(const vec3 vScreen, vec3 &vWorldPos, vec3 &vWorldDir);
        vec3 worldToScreen(const vec3 &vWorld);
        
        void onResize(int iSizeW, int iSizeH);
        void onDisplay(double fDeltaTime, double fDeltaClock);
        
        unsigned int loadTexture(const std::string& sTextureFileName);
        unsigned int uploadTexture(const std::string &sLabel, int iWidth, int iHeight, unsigned char *pData);
        std::vector<std::string> findTexture(const std::string &sTextureFileName);
        
        void light(CLight &stLight, bool bIsSpecial = false);
        
        void text(const char *fmt, ...);
        
    protected: // internal methods
        void _init();
        void _perspective();
        void _orthographic();
        
    public: // private data
        CEngine *m_pEngine;
        
        unsigned int m_iTextureIndex; // first textures is 1, not 0
        std::map<std::string, unsigned int> m_clTextureCache;
        //std::map<std::string, std::shared_ptr<CProp::SMesh>> m_clModelCache;
        
        bool m_bInitialised;
        bool m_bResized;
        
        int m_iSizeW, m_iSizeH;
        double m_fFieldOfView;
        frustum m_stFrustum;
        
        std::vector<CLight> m_clLights;
        
        static double m_fDetailLevelMinDist;
        static double m_fDetailLevelMaxDist;
        static double m_fDetailLevelMin;
        static double m_fDetailLevelMax;
        
        bool m_bEnableSmoothShading;
        
        int m_iOverlayYPos;
        static void *m_pOverlayFont;
        static int m_pOverlayFontYInterval;
        
        double m_fRotationX;
        double m_fRotationY;
};

#endif // header
