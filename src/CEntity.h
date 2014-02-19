#pragma once
#ifndef id7B51F08C_1923_4FF4_A3B87F21964440FE
#define id7B51F08C_1923_4FF4_A3B87F21964440FE

#include <memory>
#include <string>
#include "matrix.h"

class CEntity;
class CEngine;
class CRenderer;

class CEntity {
    
    public: // typedefs
        //typedef std::unordered_set<std::weak_ptr<CEntity>> CEntityCollection;
        typedef enum {
            NONE = 0,
            LINEAR,
        } ETweenType;
        typedef enum {
            CULLED = 0,
            NORMAL,
            TRANSPARENT,
            LIGHTS,
            OVERLAY
        } ERenderMode;
        
    public: // constructors/destructors
        CEntity(CEngine *pEngine);
        virtual ~CEntity();
        
        virtual void renderInit();
        
    public: // public methods
        const std::string &getLabel() const { return m_sLabel; }
        CEntity& setLabel(const char *sLabel) { m_sLabel = std::string(sLabel); return *this; }
        
        bool isValid() const { return m_bIsValid; }
        bool remove() { m_bIsValid = false; }
        
        vec3 getPosition() const { return m_vPosition; }
        CEntity& setPosition(vec3 vPosition) { m_vPosition = vPosition; return *this; }
        
        quat getRotation() const { return m_qRotation; }
        CEntity& setRotation(quat qRotation) { m_qRotation = qRotation; return *this; }
        
        CEntity& mark() { m_bChangeMarked = true; return *this; }
        
        void onThink(double fTime, double fDeltaTime);
        void onRender(
            CRenderer *pRenderer, double fDeltaTime, double fDeltaClock,
            CEntity::ERenderMode eMode, int iDetailLevel
        );
        
        void getExtents(vec3 &vExtentsMin, vec3 &vExtentsMax) const {
            vExtentsMin = m_vExtentsMin; vExtentsMax = m_vExtentsMax;
        }
        CEntity& setExtents(const vec3 &vExtentsMin, const vec3 &vExtentsMax) {
            m_vExtentsMin = vExtentsMin; m_vExtentsMax = vExtentsMax;
            return mark();
        }
        virtual bool calculateExtents(vec3 &vExtentsMin, vec3 &vExtentsMax) const {
            vExtentsMin = vec3(-0.5, -0.5, -0.5);
            vExtentsMax = vec3( 0.5,  0.5,  0.5);
            return true;
        }
        
        virtual void getWorldExtents(vec3 &vExtentsMin, vec3 &vExtentsMax);
        virtual void getLocalExtents(vec3 &vExtentsMin, vec3 &vExtentsMax);
        
        virtual CEntity& tween(vec3 vStartPos, vec3 vEndPos, double fDuration, ETweenType eType);
        
        
        
    protected: // internal methods
        //CEntity& SetParent(CEntity *pEntity);
        //const CEntityCollection& getChildren() const;
        //CEntity& addChild(CEntity *pEntity);
        //bool removeChild(CEntity *pEntity);
        
        virtual void onChildAttached(CEntity* pEntity) { }
        virtual void onChildDetached(CEntity* pEntity) { }
        
        virtual void _think(double fTime, double fDeltaTime);
        virtual void _render(
            CRenderer *pRenderer, double fDeltaTime, double fDeltaClock,
            CEntity::ERenderMode eMode, int iDetailLevel
        );
        virtual void _onChange();
        
    protected: // private data
        bool m_bIsValid; // will be removed if false
        CEngine *m_pEngine;
        
        //CEntity *m_pParent;
        //CEntityCollection m_clChildren;
        
        std::string m_sLabel;
        
        bool m_bChangeMarked;
        vec3 m_vPosition;
        quat m_qRotation;
        vec3 m_vExtentsMin;
        vec3 m_vExtentsMax;
        
        ETweenType m_eActiveTweenType;
        vec3 m_vTweenStartPos;
        vec3 m_vTweenEndPos;
        double m_fTweenStartTime;
        double m_fTweenEndTime;
        
    public:
        vec3 m_vHighlightColor;
        
        double m_fLODDebug;
        double m_fOpacity;
        bool m_bEnableTransparency;
        long long m_iRenderTimeMs;
        bool m_bWasRendered;
        int m_iDebugPassCount;
        
        mat4 debugmat;
        
        bool m_bRenderInitialised;
        
        std::string dbgtext;
};

// If only school computers supported C++11 :'(
/* typedef struct {
    size_t operator()(const std::shared_ptr<CEntity> &key) const {
        //return std::hash<void*>()((void*)key.get());
        return (size_t)(key.get());
    }
} CEntitySharedPtrHash;
typedef struct {
    bool operator()(const std::shared_ptr<CEntity> &x, const std::shared_ptr<CEntity> &y) const {
        return (x.get() == y.get());
    }
} CEntitySharedPtrEqualTo;
 */
#endif // header
