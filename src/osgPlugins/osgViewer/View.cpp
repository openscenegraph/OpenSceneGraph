#include <osgViewer/View>

#include <iostream>
#include <string>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/ReadFile>
#include <osgDB/ParameterOutput>

bool View_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool View_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy View_Proxy
(
    new osgViewer::View,
    "View",
    "Object View",
    View_readLocalData,
    View_writeLocalData
);


static bool readMatrix(osg::Matrix& matrix, osgDB::Input& fr, const char* keyword)
{
    bool iteratorAdvanced = false;
    
    if (fr[0].matchWord(keyword) && fr[1].isOpenBracket())
    {
        int entry = fr[0].getNoNestedBrackets();

        fr += 2;

        int row=0;
        int col=0;
        double v;
        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            if (fr[0].getFloat(v))
            {
                matrix(row,col)=v;
                ++col;
                if (col>=4)
                {
                    col = 0;
                    ++row;
                }
                ++fr;
            }
            else fr.advanceOverCurrentFieldOrBlock();
        }
        iteratorAdvanced = true;
    }        
        
    return iteratorAdvanced;
}

#if 0
static bool writeMatrix(const osg::Matrix& matrix, osgDB::Output& fw, const char* keyword)
{
    fw.indent() << keyword <<" {" << std::endl;
    fw.moveIn();
    fw.indent() << matrix(0,0) << " " << matrix(0,1) << " " << matrix(0,2) << " " << matrix(0,3) << std::endl;
    fw.indent() << matrix(1,0) << " " << matrix(1,1) << " " << matrix(1,2) << " " << matrix(1,3) << std::endl;
    fw.indent() << matrix(2,0) << " " << matrix(2,1) << " " << matrix(2,2) << " " << matrix(2,3) << std::endl;
    fw.indent() << matrix(3,0) << " " << matrix(3,1) << " " << matrix(3,2) << " " << matrix(3,3) << std::endl;
    fw.moveOut();
    fw.indent() << "}"<< std::endl;
    return true;
}
#endif

osg::Image* readIntensityImage(osgDB::Input& fr, bool& itrAdvanced)
{
    if (fr.matchSequence("intensityMap {"))
    {
        int entry = fr[0].getNoNestedBrackets();
        fr += 2;

        typedef std::map<float,float> IntensityMap;
        IntensityMap intensityMap;
        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            float position, intensity;
            if (fr.read(position,intensity))
            {
                intensityMap[position] = intensity;
            }
            else
            {
                ++fr;
            }
        }
        ++fr;
        
        itrAdvanced = true;

        if (!intensityMap.empty())
        {
            unsigned int numPixels = 256;
        
            osg::Image* image = new osg::Image;
            image->allocateImage(1,numPixels,1,GL_LUMINANCE,GL_FLOAT);
        
            float intensityMultiplier = 0.01f;
            float* ptr = reinterpret_cast<float*>(image->data());
            for(unsigned int i=0; i<numPixels; ++i)
            {
                float position = (1.0f - float(i)/float(numPixels-1)) * 180.0f;
                float intensity = 1.0f;
                if (position <= intensityMap.begin()->first)
                {
                    intensity = intensityMap.begin()->second * intensityMultiplier;
                }
                else if (position>=intensityMap.rbegin()->first)
                {
                    intensity = intensityMap.rbegin()->second * intensityMultiplier;
                }
                else
                {
                    IntensityMap::iterator above_itr = intensityMap.lower_bound(position);
                    if (above_itr != intensityMap.begin())
                    {
                        IntensityMap::iterator below_itr = above_itr;
                        --below_itr;
                        float r = (position - below_itr->first) / (above_itr->first - below_itr->first);
                        intensity = (below_itr->second + (above_itr->second - below_itr->second) * r) * intensityMultiplier;
                    }
                    else
                    {
                        intensity = above_itr->second * intensityMultiplier;
                    }
                    
                }
                
                *ptr++ = intensity;
            }
            
            return image;
        }
        
    }
    return 0;
}

bool View_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgViewer::View& view = dynamic_cast<osgViewer::View&>(obj);
    bool iteratorAdvanced = false;

    bool matchedFirst = false;
    if ((matchedFirst = fr.matchSequence("setUpViewFor3DSphericalDisplay {")) ||
        fr.matchSequence("setUpViewForPanoramicSphericalDisplay {"))
    {
        double radius = 1.0;
        double collar = 0.45;
        unsigned int screenNum = 0;
        unsigned int intensityFormat = 8;
        osg::Matrix matrix; 
        std::string filename;
        osg::ref_ptr<osg::Image> intensityMap;
        int entry = fr[0].getNoNestedBrackets();

        fr += 2;

        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            bool local_itrAdvanced = false;
            if (fr.read("radius",radius)) local_itrAdvanced = true;
            if (fr.read("collar",collar)) local_itrAdvanced = true;
            if (fr.read("screenNum",screenNum)) local_itrAdvanced = true;
            if (fr.read("intensityFile",filename)) local_itrAdvanced = true;
            if (fr.matchSequence("intensityMap {")) intensityMap = readIntensityImage(fr,local_itrAdvanced);
            if (fr.read("intensityFormat",intensityFormat)) local_itrAdvanced = true;
            if (readMatrix(matrix,fr,"projectorMatrix")) local_itrAdvanced = true;
            
            if (!local_itrAdvanced) ++fr;
        }
        
        // skip trailing '}'
        ++fr;
        
        iteratorAdvanced = true;
        
        if (!filename.empty())
        {
            intensityMap = osgDB::readRefImageFile(filename);
        }

        if (intensityMap.valid())
        {
            if (intensityFormat==16) intensityMap->setInternalTextureFormat(GL_LUMINANCE16F_ARB);
            else if (intensityFormat==32) intensityMap->setInternalTextureFormat(GL_LUMINANCE32F_ARB);
            // else intensityMap->setInternalTextureFormat(image->getPixelFormat());
        }


        if (matchedFirst) view.setUpViewFor3DSphericalDisplay(radius, collar, screenNum, intensityMap.get(), matrix);
        else view.setUpViewForPanoramicSphericalDisplay(radius, collar, screenNum, intensityMap.get(), matrix);
    }

    int x = 0;
    int y = 0;
    int width = 128;
    int height = 1024;
    unsigned int screenNum = 0;
    
    if (fr.read("setUpViewOnSingleScreen",screenNum))
    {
        view.setUpViewOnSingleScreen(screenNum);
        iteratorAdvanced = true;
    }
    
    if (fr.read("setUpViewAcrossAllScreens"))
    {
        view.setUpViewAcrossAllScreens();
        iteratorAdvanced = true;
    }
    
    if (fr.read("setUpViewInWindow",x,y,width,height,screenNum))
    {
        view.setUpViewInWindow(x, y, width, height, screenNum);
    }
    
    if (fr.read("setUpViewInWindow",x,y,width,height))
    {
        view.setUpViewInWindow(x, y, width, height);
    }


    osg::ref_ptr<osg::Object> readObject;
    while((readObject=fr.readObjectOfType(osgDB::type_wrapper<osg::Camera>())).valid())
    {
        view.setCamera(static_cast<osg::Camera*>(readObject.get()));
        iteratorAdvanced = true;
    }
    
    if (fr.matchSequence("Slaves {"))
    {
        int entry = fr[0].getNoNestedBrackets();

        fr += 2;

        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            readObject = fr.readObjectOfType(osgDB::type_wrapper<osg::Camera>());
            if (readObject.valid()) view.addSlave(static_cast<osg::Camera*>(readObject.get()));
            else ++fr;
        }
        
        // skip trailing '}'
        ++fr;
        
        iteratorAdvanced = true;

    }
    
    return iteratorAdvanced;
}

bool View_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgViewer::View& view = dynamic_cast<const osgViewer::View&>(obj);

    osg::notify(osg::NOTICE)<<"View_writeLocalData"<<std::endl;

    if (view.getCamera())
    {
        fw.writeObject(*view.getCamera());
    }
    
    if (view.getNumSlaves() != 0)
    {
        fw.indent()<<"Slaves {"<<std::endl;
        fw.moveIn();
    
        for(unsigned int i=0; i<view.getNumSlaves(); ++i)
        {
            const osg::Camera* camera = view.getSlave(i)._camera.get();
            if (camera)
            {
                fw.writeObject(*camera);
            }
        }
        
        fw.moveOut();
        fw.indent()<<"}"<<std::endl;
    }
    
    return true;
}
