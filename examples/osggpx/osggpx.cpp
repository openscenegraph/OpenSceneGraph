/* OpenSceneGraph example, osggpx.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#include <osg/Node>
#include <osg/Geometry>
#include <osg/Notify>
#include <osg/MatrixTransform>
#include <osg/Texture2D>
#include <osg/DrawPixels>
#include <osg/PolygonOffset>
#include <osg/Geode>
#include <osg/CoordinateSystemNode>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/XmlParser>

#include <osgText/Text>

#include <osgGA/TerrainManipulator>
#include <osgViewer/Viewer>

class TrackSegment : public osg::Object
{
public:
    TrackSegment() {}

    TrackSegment(const TrackSegment& ts, const osg::CopyOp=osg::CopyOp::SHALLOW_COPY) {}

    META_Object(osg, TrackSegment)

    struct TrackPoint
    {
        TrackPoint():
            latitude(0.0),
            longitude(0.0),
            elevation(0.0),
            time(0.0) {}

        double latitude;
        double longitude;
        double elevation;
        double time;
    };

    typedef std::vector< TrackPoint > TrackPoints;

    void addTrackPoint(const TrackPoint& trackPoint) { _trackPoints.push_back(trackPoint); }

    TrackPoints& getTrackPoints() { return _trackPoints; }
    const TrackPoints& getTrackPoints() const { return _trackPoints; }

protected:
    virtual ~TrackSegment() {}

    TrackPoints _trackPoints;
};

class Track : public osg::Object
{
public:
    Track() {}

    Track(const Track& track, const osg::CopyOp=osg::CopyOp::SHALLOW_COPY) {}

    META_Object(osg, Track)

    typedef std::vector< osg::ref_ptr<TrackSegment> > TrackSegments;

    void addTrackSegment(TrackSegment* trackSegment) { _trackSegments.push_back(trackSegment); }

    TrackSegments& getTrackSegments() { return _trackSegments; }
    const TrackSegments& getTrackSegments() const { return _trackSegments; }

protected:

    virtual ~Track() {}

    TrackSegments _trackSegments;
};

double convertTime(const std::string& timestr)
{
    osg::notify(osg::NOTICE)<<"       time = "<<timestr<<std::endl;
    return 0;
}

Track* readTrack(const std::string& filename)
{
    std::string foundFilename = osgDB::findDataFile(filename);
    if (foundFilename.empty()) return 0;

    std::string ext = osgDB::getFileExtension(foundFilename);
    if (ext!="gpx") return 0;

    osgDB::XmlNode::Input input;
    input.open(foundFilename);
    input.readAllDataIntoBuffer();

    osg::ref_ptr<osgDB::XmlNode> doc = new osgDB::XmlNode;
    doc->read(input);

    osgDB::XmlNode* root = 0;
    for(osgDB::XmlNode::Children::iterator itr = doc->children.begin();
        itr != doc->children.end() && !root;
        ++itr)
    {
        if ((*itr)->name=="gpx") root = itr->get();
    }

    if (!root) return 0;

    std::string latitude("lat");
    std::string longitude("lon");

    for(osgDB::XmlNode::Children::iterator itr = root->children.begin();
        itr != root->children.end();
        ++itr)
    {
        if ((*itr)->name=="rte")
        {
            osg::ref_ptr<Track> track = new Track;
            track->setName(filename);

            osg::ref_ptr<TrackSegment> trackSegment = new TrackSegment;
            for(osgDB::XmlNode::Children::iterator sitr = (*itr)->children.begin();
                sitr != (*itr)->children.end();
                ++sitr)
            {
                if ((*sitr)->name=="rtept" )
                {
                    osgDB::XmlNode* trkpt = sitr->get();
                    TrackSegment::TrackPoint point;
                    bool valid = false;
                    if (trkpt->properties.count(latitude)!=0)
                    {
                        valid = true;
                        point.latitude = osg::asciiToDouble(trkpt->properties[latitude].c_str());
                    }
                    if (trkpt->properties.count(longitude)!=0)
                    {
                        valid = true;
                        point.longitude = osg::asciiToDouble(trkpt->properties[longitude].c_str());
                    }

                    for(osgDB::XmlNode::Children::iterator pitr = trkpt->children.begin();
                        pitr != trkpt->children.end();
                        ++pitr)
                    {
                        if ((*pitr)->name=="ele") point.elevation = osg::asciiToDouble((*pitr)->contents.c_str());
                        else if ((*pitr)->name=="time") point.time = convertTime((*pitr)->contents);
                    }

                    if (valid)
                    {
                        osg::notify(osg::NOTICE)<<"  point.latitude="<<point.latitude<<", longitude="<<point.longitude<<", elev="<<point.elevation<<", time="<<point.time<<std::endl;
                        trackSegment->addTrackPoint(point);
                    }
                }

            }

            if (!trackSegment->getTrackPoints().empty())
            {
                track->addTrackSegment(trackSegment.get());
            }

            return track.release();
        }
        else  if ((*itr)->name=="trk")
        {
            osg::ref_ptr<Track> track = new Track;
            track->setName(filename);

            for(osgDB::XmlNode::Children::iterator citr = (*itr)->children.begin();
                citr != (*itr)->children.end();
                ++citr)
            {
                if ((*citr)->name=="trkseg")
                {
                    osg::ref_ptr<TrackSegment> trackSegment = new TrackSegment;
                    for(osgDB::XmlNode::Children::iterator sitr = (*citr)->children.begin();
                        sitr != (*citr)->children.end();
                        ++sitr)
                    {
                        if ((*sitr)->name=="trkpt" || (*sitr)->name=="rtept" )
                        {
                            osgDB::XmlNode* trkpt = sitr->get();
                            TrackSegment::TrackPoint point;
                            bool valid = false;
                            if (trkpt->properties.count(latitude)!=0)
                            {
                                valid = true;
                                point.latitude = osg::asciiToDouble(trkpt->properties[latitude].c_str());
                            }
                            if (trkpt->properties.count(longitude)!=0)
                            {
                                valid = true;
                                point.longitude = osg::asciiToDouble(trkpt->properties[longitude].c_str());
                            }

                            for(osgDB::XmlNode::Children::iterator pitr = trkpt->children.begin();
                                pitr != trkpt->children.end();
                                ++pitr)
                            {
                                if ((*pitr)->name=="ele") point.elevation = osg::asciiToDouble((*pitr)->contents.c_str());
                                else if ((*pitr)->name=="time") point.time = convertTime((*pitr)->contents);
                            }

                            if (valid)
                            {
                                // osg::notify(osg::NOTICE)<<"  point.latitude="<<point.latitude<<", longitude="<<point.longitude<<", elev="<<point.elevation<<", time="<<point.time<<std::endl;
                                trackSegment->addTrackPoint(point);
                            }
                        }
                    }
                    if (!trackSegment->getTrackPoints().empty())
                    {
                        track->addTrackSegment(trackSegment.get());
                    }
                }
            }
            return track.release();
        }
    }

    return 0;
}

TrackSegment* computeSmoothedTrackSegment(TrackSegment* ts)
{
    if (!ts) return 0;

    const TrackSegment::TrackPoints& orig_points = ts->getTrackPoints();

    if (orig_points.size()>2)
    {
        // only do smoothing if we have more than two points.
        osg::ref_ptr<TrackSegment> new_ts = new TrackSegment;

        TrackSegment::TrackPoints& new_points = new_ts->getTrackPoints();
        new_points.resize(orig_points.size());

        new_points[0] = orig_points[0];
        new_points[orig_points.size()-1] = orig_points[orig_points.size()-1];

        for(unsigned int i=1; i<orig_points.size()-1; ++i)
        {
            new_points[i].latitude = (orig_points[i-1].latitude+orig_points[i].latitude+orig_points[i+1].latitude)/3.0;
            new_points[i].longitude = (orig_points[i-1].longitude+orig_points[i].longitude+orig_points[i+1].longitude)/3.0;
            new_points[i].elevation = (orig_points[i-1].elevation+orig_points[i].elevation+orig_points[i+1].elevation)/3.0;
            new_points[i].time = (orig_points[i-1].time+orig_points[i].time+orig_points[i+1].time)/3.0;
        }
        return new_ts.release();
    }
    else
    {
        // we have two or less points and can't do smoothing, so will just return original TrackSegment
        return ts;
    }

}

TrackSegment* computeAveragedSpeedTrackSegment(TrackSegment* ts)
{
    if (!ts) return 0;

    osg::ref_ptr<osg::EllipsoidModel> em = new osg::EllipsoidModel;
    const TrackSegment::TrackPoints& orig_points = ts->getTrackPoints();

    if (orig_points.size()>2)
    {
        // only do smoothing if we have more than two points.
        osg::ref_ptr<TrackSegment> new_ts = new TrackSegment;


        // compute overall distance
        double total_distance = 0;
        for(unsigned int i=1; i<orig_points.size()-1; ++i)
        {
            osg::Vec3d point_a, point_b;
            em->convertLatLongHeightToXYZ(osg::DegreesToRadians(orig_points[i].latitude), osg::DegreesToRadians(orig_points[i].longitude), orig_points[i].elevation,
                                          point_a.x(), point_a.y(), point_a.z());
            em->convertLatLongHeightToXYZ(osg::DegreesToRadians(orig_points[i+1].latitude), osg::DegreesToRadians(orig_points[i+1].longitude), orig_points[i+1].elevation,
                                          point_b.x(), point_b.y(), point_b.z());
            total_distance += (point_b-point_a).length();
        }

        double total_time = orig_points[orig_points.size()-1].time - orig_points[0].time;
        double average_speed = total_distance/total_time;

        OSG_NOTICE<<"total_time = "<<total_time<<std::endl;
        OSG_NOTICE<<"total_distance = "<<total_distance<<std::endl;
        OSG_NOTICE<<"average_speed = "<<average_speed<<std::endl;

        TrackSegment::TrackPoints& new_points = new_ts->getTrackPoints();
        new_points.resize(orig_points.size());
        new_points[0] = orig_points[0];

        double accumulated_distance = 0.0;
        for(unsigned int i=0; i<orig_points.size()-1; ++i)
        {
            osg::Vec3d point_a, point_b;
            em->convertLatLongHeightToXYZ(osg::DegreesToRadians(orig_points[i].latitude), osg::DegreesToRadians(orig_points[i].longitude), orig_points[i].elevation,
                                          point_a.x(), point_a.y(), point_a.z());
            em->convertLatLongHeightToXYZ(osg::DegreesToRadians(orig_points[i+1].latitude), osg::DegreesToRadians(orig_points[i+1].longitude), orig_points[i+1].elevation,
                                          point_b.x(), point_b.y(), point_b.z());

            accumulated_distance += (point_b-point_a).length();

            new_points[i+1] = orig_points[i+1];
            new_points[i+1].time = accumulated_distance / average_speed;
        }
        return new_ts.release();
    }
    else
    {
        // we have two or less points and can't do smoothing, so will just return original TrackSegment
        return ts;
    }

}

Track* computeAveragedSpeedTrack(Track* track)
{
    osg::ref_ptr<Track> new_track = new Track;

    for(Track::TrackSegments::iterator itr = track->getTrackSegments().begin();
        itr != track->getTrackSegments().end();
        ++itr)
    {
        new_track->addTrackSegment(computeAveragedSpeedTrackSegment(itr->get()));
    }

    return new_track.release();
}


Track* computeSmoothedTrack(Track* track)
{
    osg::ref_ptr<Track> new_track = new Track;

    for(Track::TrackSegments::iterator itr = track->getTrackSegments().begin();
        itr != track->getTrackSegments().end();
        ++itr)
    {
        new_track->addTrackSegment(computeSmoothedTrackSegment(itr->get()));
    }

    return new_track.release();
}

osg::Node* createTrackModel(Track* track, const osg::Vec4& colour)
{
    osg::ref_ptr<osg::EllipsoidModel> em = new osg::EllipsoidModel;

    osg::ref_ptr<osg::Geode> geode = new osg::Geode;

    for(Track::TrackSegments::iterator itr = track->getTrackSegments().begin();
        itr != track->getTrackSegments().end();
        ++itr)
    {
        const TrackSegment::TrackPoints& points = (*itr)->getTrackPoints();
        if (points.size()<2) continue;

        osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
        osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
        geometry->setVertexArray(vertices.get());
        vertices->resize(points.size());
        for(unsigned int i=0; i<points.size(); ++i)
        {
            osg::Vec3d point;
            em->convertLatLongHeightToXYZ(osg::DegreesToRadians(points[i].latitude), osg::DegreesToRadians(points[i].longitude), points[i].elevation,
                                          point.x(), point.y(), point.z());

           (*vertices)[i] = point;
        }

        osg::ref_ptr<osg::Vec4Array> colours = new osg::Vec4Array;
        colours->push_back(colour);
        geometry->setColorArray(colours.get(), osg::Array::BIND_OVERALL);

        geometry->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP, 0, points.size()));

        geode->addDrawable(geometry.get());
    }

    geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    return geode.release();
}

int main(int argv, char **argc)
{
    osg::ArgumentParser arguments(&argv, argc);

    typedef std::list< osg::ref_ptr<Track> > Tracks;
    Tracks tracks;

    bool average = false;
    while (arguments.read("-a") || arguments.read("--average")) average = true;

    bool smooth = false;
    while (arguments.read("-s") || arguments.read("--smooth")) smooth = true;

    std::string outputFilename;
    while (arguments.read("-o",outputFilename)) {}

    std::string trackFilename;
    while (arguments.read("-t",trackFilename))
    {
        osg::ref_ptr<Track> track = readTrack(trackFilename);
        if (track.valid()) tracks.push_back(track.get());
    }

    osg::ref_ptr<osg::EllipsoidModel> em = new osg::EllipsoidModel;

    osg::ref_ptr<osg::Group> group = new osg::Group;

    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);
    if (loadedModel.valid()) group->addChild(loadedModel.get());

    for(Tracks::iterator itr = tracks.begin();
        itr != tracks.end();
        ++itr)
    {
        Track* track = itr->get();

        group->addChild(createTrackModel(track, osg::Vec4(1.0,1.0,1.0,1.0)));

        // smooth the track
        if (average)
        {
            for(Track::TrackSegments::iterator itr = track->getTrackSegments().begin();
                itr != track->getTrackSegments().end();
                ++itr)
            {
                *itr = computeAveragedSpeedTrackSegment(itr->get());
            }
        }

        // smooth the track
        if (smooth)
        {
            for(Track::TrackSegments::iterator itr = track->getTrackSegments().begin();
                itr != track->getTrackSegments().end();
                ++itr)
            {
                *itr = computeSmoothedTrackSegment(itr->get());
            }

            for(Track::TrackSegments::iterator itr = track->getTrackSegments().begin();
                itr != track->getTrackSegments().end();
                ++itr)
            {
                *itr = computeSmoothedTrackSegment(itr->get());
            }

            for(Track::TrackSegments::iterator itr = track->getTrackSegments().begin();
                itr != track->getTrackSegments().end();
                ++itr)
            {
                *itr = computeSmoothedTrackSegment(itr->get());
            }

            for(Track::TrackSegments::iterator itr = track->getTrackSegments().begin();
                itr != track->getTrackSegments().end();
                ++itr)
            {
                *itr = computeSmoothedTrackSegment(itr->get());
            }
        }

        double totalDistance = 0.0;
        double totalAscent = 0.0;
        double totalDescent = 0.0;

        osg::notify(osg::NOTICE)<<"Track read "<<track->getName()<<std::endl;
        for(Track::TrackSegments::iterator itr = track->getTrackSegments().begin();
            itr != track->getTrackSegments().end();
            ++itr)
        {
            TrackSegment* ts = itr->get();

            const TrackSegment::TrackPoints& points = ts->getTrackPoints();
            if (points.size()>1)
            {
                TrackSegment::TrackPoints::iterator pitr = ts->getTrackPoints().begin();
                osg::Vec3d previousPos;
                double previousElevation = pitr->elevation;
                em->convertLatLongHeightToXYZ(osg::DegreesToRadians(pitr->latitude), osg::DegreesToRadians(pitr->longitude), 0.0,
                                              previousPos.x(), previousPos.y(), previousPos.z());
                ++pitr;

                for(;
                    pitr != ts->getTrackPoints().end();
                    ++pitr)
                {
                    osg::Vec3d newPos;
                    double newElevation = pitr->elevation;
                    em->convertLatLongHeightToXYZ(osg::DegreesToRadians(pitr->latitude), osg::DegreesToRadians(pitr->longitude), 0.0,
                                                newPos.x(), newPos.y(), newPos.z());

                    double distance = (newPos-previousPos).length();

                    totalDistance += distance;
                    if (newElevation>previousElevation) totalAscent += (newElevation-previousElevation);
                    else totalDescent += (previousElevation-newElevation);

                    osg::notify(osg::NOTICE)<<"     distance="<<distance<<", "<<newElevation-previousElevation<<std::endl;

                    previousPos = newPos;
                    previousElevation = newElevation;
                }
            }
        }

        double metersToFeet = 1 / 0.3048;
        double metersToMiles = 1.0 / 1609.344;

        osg::notify(osg::NOTICE)<<"totalDistance = "<<totalDistance<<"m, "<<totalDistance*metersToMiles<<" miles"<<std::endl;
        osg::notify(osg::NOTICE)<<"totalAscent = "<<totalAscent<<"m, "<<totalAscent*metersToFeet<<"ft"<<std::endl;
        osg::notify(osg::NOTICE)<<"totalDescent = "<<totalDescent<<"m, "<<totalDescent*metersToFeet<<"ft"<<std::endl;

    }

    if (!outputFilename.empty())
    {
        std::ofstream fout(outputFilename.c_str());

        fout<<"<?xml version=\"1.0\" encoding=\"utf-8\"?><gpx version=\"1.0\" creator=\"osggpx\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\"http://www.topografix.com/GPX/1/0\" xsi:schemaLocation=\"http://www.topografix.com/GPX/1/0 http://www.topografix.com/GPX/1/0/gpx.xsd\">"<<std::endl;

        for(Tracks::iterator itr = tracks.begin();
            itr != tracks.end();
            ++itr)
        {
            Track* track = itr->get();

            fout<<"<trk>"<<std::endl;
            fout<<"<desc>The track description</desc>"<<std::endl;
            for(Track::TrackSegments::iterator itr = track->getTrackSegments().begin();
                itr != track->getTrackSegments().end();
                ++itr)
            {
                TrackSegment* ts = itr->get();
                fout<<"<trkseg>"<<std::endl;

                for(TrackSegment::TrackPoints::iterator pitr = ts->getTrackPoints().begin();
                    pitr != ts->getTrackPoints().end();
                    ++pitr)
                {
                    fout<<"<trkpt lat=\""<<pitr->latitude<<"\" lon=\""<<pitr->longitude<<"\">"<<std::endl;
                    fout<<"<ele>"<<pitr->elevation<<"</ele>"<<std::endl;
                    fout<<"<time>"<<pitr->time<<"</time>"<<std::endl;
                    fout<<"</trkpt>"<<std::endl;
                }

                fout<<"</trkseg>"<<std::endl;

            }
            fout<<"</trk>"<<std::endl;
        }
        fout<<"</gpx>"<<std::endl;
    }

    osgViewer::Viewer viewer(arguments);
    viewer.setCameraManipulator(new osgGA::TerrainManipulator);
    viewer.setSceneData(group.get());
    return viewer.run();

}
