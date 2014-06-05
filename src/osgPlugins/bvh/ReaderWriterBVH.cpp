#include <osg/Notify>
#include <osg/Geometry>
#include <osg/ShapeDrawable>

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>

#include <osgAnimation/Bone>
#include <osgAnimation/Skeleton>
#include <osgAnimation/UpdateBone>
#include <osgAnimation/StackedTransform>
#include <osgAnimation/StackedTranslateElement>
#include <osgAnimation/StackedQuaternionElement>
#include <osgAnimation/BasicAnimationManager>

class BvhMotionBuilder : public osg::Referenced
{
public:
    typedef std::pair<osg::ref_ptr<osgAnimation::Bone>, int> JointNode;
    typedef std::vector<JointNode> JointList;

    BvhMotionBuilder() : _drawingFlag(0) {}
    virtual ~BvhMotionBuilder() {}

    static BvhMotionBuilder* instance()
    {
        static osg::ref_ptr<BvhMotionBuilder> s_library = new BvhMotionBuilder;
        return s_library.get();
    }

    void buildHierarchy( osgDB::Input& fr, int level, osgAnimation::Bone* parent )
    {
        bool isRecognized = false;
        if ( !parent ) return;

        if ( fr.matchSequence("OFFSET %f %f %f") )
        {
            isRecognized = true;
            ++fr;

            osg::Vec3 offset;
            if ( fr.readSequence(offset) )
            {
                // Process OFFSET section
                parent->setMatrixInSkeletonSpace( osg::Matrix::translate(offset) *
                                                  parent->getMatrixInSkeletonSpace() );
                osgAnimation::UpdateBone* updateBone =
                    dynamic_cast<osgAnimation::UpdateBone*>( parent->getUpdateCallback() );
                if ( updateBone )
                {
                    osgAnimation::StackedTransform& stack = updateBone->getStackedTransforms();
                    stack.push_back( new osgAnimation::StackedTranslateElement("position", offset) );
                    stack.push_back( new osgAnimation::StackedQuaternionElement("quaternion", osg::Quat()) );
                }

                if ( _drawingFlag && parent->getNumParents() && level>0 )
                    parent->getParent(0)->addChild( createRefGeometry(offset, 0.5).get() );
            }
        }

        if ( fr.matchSequence("CHANNELS %i") )
        {
            isRecognized = true;

            // Process CHANNELS section
            int noChannels;
            fr[1].getInt( noChannels );
            fr += 2;

            for ( int i=0; i<noChannels; ++i )
            {
                // Process each channel
                std::string channelName;
                fr.readSequence( channelName );
                alterChannel( channelName, _joints.back().second );
            }
        }

        if ( fr.matchSequence("End Site {") )
        {
            isRecognized = true;
            fr += 3;

            if ( fr.matchSequence("OFFSET %f %f %f") )
            {
                ++fr;

                osg::Vec3 offsetEndSite;
                if ( fr.readSequence(offsetEndSite) )
                {
                    // Process End Site section
                    osg::ref_ptr<osgAnimation::Bone> bone = new osgAnimation::Bone( parent->getName()+"End" );
                    bone->setMatrixInSkeletonSpace( osg::Matrix::translate(offsetEndSite) *
                                                    bone->getMatrixInSkeletonSpace() );
                    bone->setDataVariance( osg::Object::DYNAMIC );
                    parent->insertChild( 0, bone.get() );

                    if ( _drawingFlag )
                        parent->addChild( createRefGeometry(offsetEndSite, 0.5).get() );
                }
            }
            fr.advanceOverCurrentFieldOrBlock();
        }

        if ( fr.matchSequence("ROOT %w {") || fr.matchSequence("JOINT %w {") )
        {
            isRecognized = true;

            // Process JOINT section
            osg::ref_ptr<osgAnimation::Bone> bone = new osgAnimation::Bone( fr[1].getStr() );
            bone->setDataVariance( osg::Object::DYNAMIC );
            bone->setDefaultUpdateCallback();
            parent->insertChild( 0, bone.get() );
            _joints.push_back( JointNode(bone, 0) );

            int entry = fr[1].getNoNestedBrackets();
            fr += 3;
            while ( !fr.eof() && fr[0].getNoNestedBrackets()>entry )
                buildHierarchy( fr, entry, bone.get() );
            fr.advanceOverCurrentFieldOrBlock();
        }

        if ( !isRecognized )
        {
            OSG_WARN << "BVH Reader: Unrecognized symbol " << fr[0].getStr()
                << ". Ignore current field or block." << std::endl;
            fr.advanceOverCurrentFieldOrBlock();
        }
    }

    void buildMotion( osgDB::Input& fr, osgAnimation::Animation* anim )
    {
        int i=0, frames=0;
        float frameTime=0.033f;

        if ( !fr.readSequence("Frames:", frames) )
        {
            OSG_WARN << "BVH Reader: Frame number setting not found, but an unexpected "
                << fr[0].getStr() << ". Set to 1." << std::endl;
        }

        ++fr;
        if ( !fr.readSequence("Time:", frameTime) )
        {
            OSG_WARN << "BVH Reader: Frame time setting not found, but an unexpected "
                << fr[0].getStr() << ". Set to 0.033 (30FPS)." << std::endl;
        }

        // Each joint has a position animating channel and an euler animating channel
        std::vector< osg::ref_ptr<osgAnimation::Vec3LinearChannel> > posChannels;
        std::vector< osg::ref_ptr<osgAnimation::QuatSphericalLinearChannel> > rotChannels;
        for ( JointList::iterator itr=_joints.begin(); itr!=_joints.end(); ++itr )
        {
            std::string name = itr->first->getName();

            osg::ref_ptr<osgAnimation::Vec3KeyframeContainer> posKey = new osgAnimation::Vec3KeyframeContainer;
            osg::ref_ptr<osgAnimation::Vec3LinearSampler> posSampler = new osgAnimation::Vec3LinearSampler;
            osg::ref_ptr<osgAnimation::Vec3LinearChannel> posChannel = new osgAnimation::Vec3LinearChannel( posSampler.get() );
            posSampler->setKeyframeContainer( posKey.get() );
            posChannel->setName( "position" );
            posChannel->setTargetName( name );

            osg::ref_ptr<osgAnimation::QuatKeyframeContainer> rotKey = new osgAnimation::QuatKeyframeContainer;
            osg::ref_ptr<osgAnimation::QuatSphericalLinearSampler> rotSampler = new osgAnimation::QuatSphericalLinearSampler;
            osg::ref_ptr<osgAnimation::QuatSphericalLinearChannel> rotChannel = new osgAnimation::QuatSphericalLinearChannel( rotSampler.get() );
            rotSampler->setKeyframeContainer( rotKey.get() );
            rotChannel->setName( "quaternion" );
            rotChannel->setTargetName( name );

            posChannels.push_back( posChannel );
            rotChannels.push_back( rotChannel );
        }

        // Obtain motion data from the stream
        while ( !fr.eof() && i<frames )
        {
            for ( unsigned int n=0; n<_joints.size(); ++n )
            {
                osgAnimation::Vec3LinearChannel* posChannel = posChannels[n].get();
                osgAnimation::QuatSphericalLinearChannel* rotChannel = rotChannels[n].get();

                setKeyframe( fr, _joints[n].second, frameTime*(double)i,
                    dynamic_cast<osgAnimation::Vec3KeyframeContainer*>(posChannel->getSampler()->getKeyframeContainer()),
                    dynamic_cast<osgAnimation::QuatKeyframeContainer*>(rotChannel->getSampler()->getKeyframeContainer()) );
            }

            i++;
        }

        // Add valid channels to the animate object
        for ( unsigned int n=0; n<_joints.size(); ++n )
        {
            if ( posChannels[n]->getOrCreateSampler()->getOrCreateKeyframeContainer()->size()>0 )
                anim->addChannel( posChannels[n].get() );
            if ( rotChannels[n]->getOrCreateSampler()->getOrCreateKeyframeContainer()->size()>0 )
                anim->addChannel( rotChannels[n].get() );
        }
    }

    osg::Group* buildBVH( std::istream& stream, const osgDB::ReaderWriter::Options* options )
    {
        if ( options )
        {
            if ( options->getOptionString().find("contours")!=std::string::npos ) _drawingFlag = 1;
            else if ( options->getOptionString().find("solids")!=std::string::npos ) _drawingFlag = 2;
        }

        osgDB::Input fr;
        fr.attach( &stream );

        osg::ref_ptr<osgAnimation::Bone> boneroot = new osgAnimation::Bone( "Root" );
        boneroot->setDefaultUpdateCallback();

        osg::ref_ptr<osgAnimation::Skeleton> skelroot = new osgAnimation::Skeleton;
        skelroot->setDefaultUpdateCallback();
        skelroot->insertChild( 0, boneroot.get() );

        osg::ref_ptr<osgAnimation::Animation> anim = new osgAnimation::Animation;

        while( !fr.eof() )
        {
            if ( fr.matchSequence("HIERARCHY") )
            {
                ++fr;
                buildHierarchy( fr, 0, boneroot.get() );
            }
            else if ( fr.matchSequence("MOTION") )
            {
                ++fr;
                buildMotion( fr, anim.get() );
            }
            else
            {
                if ( fr[0].getStr()==NULL ) continue;

                OSG_WARN << "BVH Reader: Unexpected beginning " << fr[0].getStr()
                    <<  ", neither HIERARCHY nor MOTION. Stopped." << std::endl;
                break;
            }
        }

#if 0
        std::cout << "Bone number: " << _joints.size() << "\n";
        for ( unsigned int i=0; i<_joints.size(); ++i )
        {
            JointNode node = _joints[i];
            std::cout << "Bone" << i << " " << node.first->getName() << ": ";
            std::cout << std::hex << node.second << std::dec << "; ";
            if ( node.first->getNumParents() )
                std::cout << "Parent: " << node.first->getParent(0)->getName();
            std::cout << "\n";
        }
#endif

        osg::Group* root = new osg::Group;
        osgAnimation::BasicAnimationManager* manager = new osgAnimation::BasicAnimationManager;
        root->addChild( skelroot.get() );
        root->setUpdateCallback(manager);
        manager->registerAnimation( anim.get() );
        manager->buildTargetReference();
        manager->playAnimation( anim.get() );

        _joints.clear();
        return root;
    }

protected:
    void alterChannel( std::string name, int& value )
    {
        if      ( name=="Xposition" ) value |= 0x01;
        else if ( name=="Yposition" ) value |= 0x02;
        else if ( name=="Zposition" ) value |= 0x04;
        else if ( name=="Zrotation" ) value |= 0x08;
        else if ( name=="Xrotation" ) value |= 0x10;
        else if ( name=="Yrotation" ) value |= 0x20;
    }

    void setKeyframe( osgDB::Input& fr, int ch, double time,
                      osgAnimation::Vec3KeyframeContainer* posKey,
                      osgAnimation::QuatKeyframeContainer* rotKey )
    {
        if ( ch&0x07 && posKey )  // Position keyframe
        {
            float keyValue[3] = { 0.0f };
            if ( ch&0x01 ) fr.readSequence( keyValue[0] );
            if ( ch&0x02 ) fr.readSequence( keyValue[1] );
            if ( ch&0x04 ) fr.readSequence( keyValue[2] );

            osg::Vec3 vec( keyValue[0], keyValue[1], keyValue[2] );
            posKey->push_back( osgAnimation::Vec3Keyframe(time, vec) );
        }

        if ( ch&0x38 && rotKey )  // Rotation keyframe
        {
            float keyValue[3] = { 0.0f };
            if ( ch&0x08 ) fr.readSequence( keyValue[2] );
            if ( ch&0x10 ) fr.readSequence( keyValue[0] );
            if ( ch&0x20 ) fr.readSequence( keyValue[1] );

            // Note that BVH data need to concatenate the rotating matrices as Y*X*Z
            // So we should not create Quat directly from input values, which makes a wrong X*Y*Z
            osg::Matrix rotMat = osg::Matrix::rotate(osg::DegreesToRadians(keyValue[1]), osg::Vec3(0.0,1.0,0.0))
                               * osg::Matrix::rotate(osg::DegreesToRadians(keyValue[0]), osg::Vec3(1.0,0.0,0.0))
                               * osg::Matrix::rotate(osg::DegreesToRadians(keyValue[2]), osg::Vec3(0.0,0.0,1.0));
            osg::Quat quat = rotMat.getRotate();
            rotKey->push_back( osgAnimation::QuatKeyframe(time, quat) );
        }
    }

    osg::ref_ptr<osg::Geode> createRefGeometry( osg::Vec3 p, double len )
    {
        osg::ref_ptr<osg::Geode> geode = new osg::Geode;

        if ( _drawingFlag==1 )
        {
            osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
            osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;

            // Joint
            vertices->push_back( osg::Vec3(-len, 0.0, 0.0) );
            vertices->push_back( osg::Vec3( len, 0.0, 0.0) );
            vertices->push_back( osg::Vec3( 0.0,-len, 0.0) );
            vertices->push_back( osg::Vec3( 0.0, len, 0.0) );
            vertices->push_back( osg::Vec3( 0.0, 0.0,-len) );
            vertices->push_back( osg::Vec3( 0.0, 0.0, len) );

            // Bone
            vertices->push_back( osg::Vec3( 0.0, 0.0, 0.0) );
            vertices->push_back( p );

            geometry->addPrimitiveSet( new osg::DrawArrays(osg::PrimitiveSet::LINES, 0, 8) );
            geometry->setVertexArray( vertices.get() );

            geode->addDrawable( geometry.get() );
        }
        else if ( _drawingFlag==2 )
        {
            osg::Quat quat;
            osg::ref_ptr<osg::Box> box = new osg::Box( p*0.5, p.length(), len, len );
            quat.makeRotate( osg::Vec3(1.0,0.0,0.0), p );
            box->setRotation( quat );

            geode->addDrawable( new osg::ShapeDrawable(box.get()) );
        }

        return geode;
    }

    int _drawingFlag;
    JointList _joints;
};

class ReaderWriterBVH : public osgDB::ReaderWriter
{
public:
    ReaderWriterBVH()
    {
        supportsExtension( "bvh", "Biovision motion hierarchical file" );

        supportsOption( "contours","Show the skeleton with lines." );
        supportsOption( "solids","Show the skeleton with solid boxes." );
    }

    virtual const char* className() const
    { return "BVH Motion Reader"; }

    virtual ReadResult readNode(std::istream& stream, const osgDB::ReaderWriter::Options* options) const
    {
        ReadResult rr = BvhMotionBuilder::instance()->buildBVH( stream, options );
        return rr;
    }

    virtual ReadResult readNode(const std::string& file, const osgDB::ReaderWriter::Options* options) const
    {
        std::string ext = osgDB::getLowerCaseFileExtension( file );
        if ( !acceptsExtension(ext) ) return ReadResult::FILE_NOT_HANDLED;

        std::string fileName = osgDB::findDataFile( file, options );
        if ( fileName.empty() ) return ReadResult::FILE_NOT_FOUND;

        osgDB::ifstream stream( fileName.c_str(), std::ios::in|std::ios::binary );
        if( !stream ) return ReadResult::ERROR_IN_READING_FILE;
        return readNode( stream, options );
    }
};

// Now register with Registry to instantiate the above reader/writer.
REGISTER_OSGPLUGIN( bvh, ReaderWriterBVH )
