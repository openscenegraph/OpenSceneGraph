#ifndef _CONVERTFROMINVENTOR_H_
#define _CONVERTFROMINVENTOR_H_

#include <osg/Group>
#include <osg/Geometry>
#include <osg/PrimitiveSet>
#include <osg/Texture2D>
#include <osg/Light>
#include <Inventor/actions/SoCallbackAction.h>
#include <Inventor/SbLinear.h>
#include <vector>
#include <stack>
#include <assert.h>

class ConvertFromInventor
{
    public:
        ConvertFromInventor();
        ~ConvertFromInventor();

        /** Initializes internal converting structures.
         *  The function is expected to be called after Inventor initialization. */
        static void init();

        /// Conversts from IV to OSG scene graph
        osg::Node* convert(SoNode* rootIVNode);

        /**
         * Preprocessing restructure the scene for the convertor
         * to be able to convert some peculiar scene constructions.
         * Resulting scene is geometrically equivalent to the source
         * scene. The preprocessing is related to grouping nodes only
         * (SoSeparator, SoGroup, SoLOD, SoSwitch,...) and their
         * treatment with traversal state.
         */
        void preprocess(SoNode *root);

    private:

        // Callback functions for converting inventor scene graph to osg
        // scene graph

        static SoCallbackAction::Response preNode(void* data,
                                 SoCallbackAction* action, const SoNode* node);
        static SoCallbackAction::Response postNode(void* data,
                                 SoCallbackAction* action, const SoNode* node);
        static SoCallbackAction::Response preTransformSeparator(void* data,
                                 SoCallbackAction* action, const SoNode* node);
        static SoCallbackAction::Response postTransformSeparator(void* data,
                                 SoCallbackAction* action, const SoNode* node);
        static SoCallbackAction::Response preLOD(void* data,
                                 SoCallbackAction* action, const SoNode* node);
        static SoCallbackAction::Response postLOD(void* data,
                                 SoCallbackAction* action, const SoNode* node);
        static SoCallbackAction::Response preShape(void* data,
                                 SoCallbackAction* action, const SoNode* node);
        static SoCallbackAction::Response postShape(void* data,
                                 SoCallbackAction* action, const SoNode* node);
        static SoCallbackAction::Response postTexture(void* data,
                                 SoCallbackAction* action, const SoNode* node);
        static SoCallbackAction::Response preLight(void* data,
                                 SoCallbackAction* action, const SoNode* node);
        static SoCallbackAction::Response preEnvironment(void* data,
                                 SoCallbackAction* action, const SoNode* node);
        static SoCallbackAction::Response preShaderProgram(void* data,
                                 SoCallbackAction* action, const SoNode* node);
        static SoCallbackAction::Response preRotor(void* data,
                                 SoCallbackAction* action, const SoNode* node);
        static SoCallbackAction::Response prePendulum(void* data,
                                 SoCallbackAction* action, const SoNode* node);
        static SoCallbackAction::Response preShuttle(void* data,
                                 SoCallbackAction* action, const SoNode* node);
        static SoCallbackAction::Response preInfo(void* data,
                                 SoCallbackAction* action, const SoNode* node);

        static void addTriangleCB(void* data, SoCallbackAction* action,
                              const SoPrimitiveVertex *v0,
                                  const SoPrimitiveVertex *v1,
                                  const SoPrimitiveVertex *v2);
        static void addLineSegmentCB(void* data, SoCallbackAction* action,
                                     const SoPrimitiveVertex *v0,
                                     const SoPrimitiveVertex *v1);
        static void addPointCB(void* data, SoCallbackAction* action,
                               const SoPrimitiveVertex *v0);

        static SoCallbackAction::Response restructure(void* data,
                                 SoCallbackAction* action, const SoNode* node);
        static SoCallbackAction::Response restructurePreNode(void* data,
                                 SoCallbackAction* action, const SoNode* node);
        static SoCallbackAction::Response restructurePostNode(void* data,
                                 SoCallbackAction* action, const SoNode* node);

    private:

        void addVertex(SoCallbackAction* action, const SoPrimitiveVertex* v,
                       int index);

        osg::ref_ptr<osg::StateSet> getStateSet(SoCallbackAction* action);

        osg::Texture2D* convertIVTexToOSGTex(const SoNode* soNode,
                                             SoCallbackAction* action);

        void transformLight(SoCallbackAction* action, const SbVec3f& vec,
                            osg::Vec3& transVec);

        // OSG doesn't seem to have a transpose function for matrices
        void transposeMatrix(osg::Matrix& mat);

    private:

        // Normal and color binding
        deprecated_osg::Geometry::AttributeBinding normalBinding;
        deprecated_osg::Geometry::AttributeBinding colorBinding;

        // List of vertices, normals, colors and texture coordinates
        std::vector<osg::Vec3> vertices;
        std::vector<osg::Vec3> normals;
        std::vector<osg::Vec4> colors;
        std::vector<osg::Vec2> textureCoords;

        // Num of primitive and primitive type
        int numPrimitives;
        osg::PrimitiveSet::Mode primitiveType;

        // Vertex ordering
        enum VertexOrder { CLOCKWISE, COUNTER_CLOCKWISE };
        VertexOrder vertexOrder;

        // Mapping from SoTexture2 and SoVRMLImageTexture to already converted
        // osg::Texture2D - avoids duplication of same texture objects
        std::map<const SoNode*, osg::Texture2D*> ivToOsgTexMap;

        osg::ref_ptr<osg::MatrixTransform> _root;///<The root node;

        /**
         * IvStateItem aids lack of some state retrieval methods
         * of SoCallbackAction. State is maintained in stack
         * manner separately from Open Inventor.
         */
        class IvStateItem {
        public:

            // Pop flags and node caused the push
            enum Flags {
                DEFAULT_FLAGS = 0,
                MULTI_POP = 1,
                KEEP_CHILDREN_ORDER = 2,
                APPEND_AT_PUSH = 4,
                UPDATE_STATE = 8,
                UPDATE_STATE_EXCEPT_TRANSFORM = 0x10 // this has the same
                          // effect as UPDATE_STATE at the present time
            };
            int flags;
            const SoNode *pushInitiator;

            // Tracking of model transformation
            SbMatrix inheritedTransformation;
            SbMatrix lastUsedTransformation;

            // Active texture node (used for attaching the right texture to the
            // geosets). Supported types are SoTexture2 and SoVRMLImageTexture for now.
            // No multitexturing yet.
            const SoNode* inheritedTexture;
            const SoNode* currentTexture;

            // List of active lights
            std::vector<osg::ref_ptr<osg::Light> > inheritedLights;
            std::vector<osg::ref_ptr<osg::Light> > currentLights;

            // Active OpenGL glProgram and associated shaders
            osg::ref_ptr<osg::Program> inheritedGLProgram;
            osg::ref_ptr<osg::Program> currentGLProgram;

            // Ambient light (of SoEnvironment)
            SbColor inheritedAmbientLight;
            SbColor currentAmbientLight;

            // Generated OSG graph
            osg::ref_ptr<osg::Group> osgStateRoot;

            // Extra variables
            const SoNode *keepChildrenOrderParent;

            IvStateItem(const SoNode *initiator, osg::Group *root = NULL) :
                flags(IvStateItem::DEFAULT_FLAGS),
                pushInitiator(initiator),
                inheritedTransformation(SbMatrix::identity()),
                lastUsedTransformation(SbMatrix::identity()),
                inheritedTexture(NULL),
                currentTexture(NULL),
                inheritedLights(),
                currentLights(),
                inheritedGLProgram(NULL),
                currentGLProgram(NULL),
                inheritedAmbientLight(SbColor(0.2f,0.2f,0.2f)),
                currentAmbientLight(SbColor(0.2f,0.2f,0.2f)),
                osgStateRoot(root ? root : new osg::Group) {}

            IvStateItem(const IvStateItem& i, const SoCallbackAction *action,
                        const SoNode *initiator, const int f,
                        osg::Group *root) :
                flags(f),
                pushInitiator(initiator),
                inheritedTransformation(action->getModelMatrix()),
                lastUsedTransformation(action->getModelMatrix()),
                inheritedTexture(i.currentTexture),
                currentTexture(i.currentTexture),
                inheritedLights(i.currentLights),
                currentLights(i.currentLights),
                inheritedGLProgram(i.currentGLProgram),
                currentGLProgram(i.currentGLProgram),
                inheritedAmbientLight(i.inheritedAmbientLight),
                currentAmbientLight(i.currentAmbientLight),
                osgStateRoot(root) {}
        };

        /// State stack for Inventor scene traversal
        std::stack<IvStateItem> ivStateStack;

        void ivPushState(const SoCallbackAction *action,
                         const SoNode *initiator, const int flags = IvStateItem::DEFAULT_FLAGS,
                         osg::Group *root = new osg::Group);
        void ivPopState(const SoCallbackAction *action, const SoNode *initator);

        void appendNode(osg::Node *n, const SoCallbackAction *action);

};

#endif
