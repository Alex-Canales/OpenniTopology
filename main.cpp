#include <iostream>
#include <algorithm>
#include <fstream>
#include <string>
#include <vector>
#include <stdio.h>
#include <OpenNI.h>

#include "displayer.h"
#include "geometry.h"

#define SAMPLE_READ_WAIT_TIMEOUT 2000 //2000ms

using namespace openni;
using namespace std;

//TODO: find better name
class Manager
{
public:
    Manager();
    bool initialize();
    void mainLoop();
    void destroy();
    Point origin;
private:
    Device device;
    VideoStream depth;
    Displayer displayer;
    vector<Point> points;
    bool drawFrame(VideoFrameRef frame);
    bool saveRealPointsToFiles(const VideoStream &depthStream,
            VideoFrameRef frame);
    bool savePointsToFiles(v_Point points, std::string namefile);

    //TODO: find a way to communicate with the tool
    bool setOriginFromMachine();  //Returns false if problem

    //Save points in the given vector
    bool addRealPoints(const VideoStream &depthStream, VideoFrameRef &frame,
            vector<Point> &vect);
    bool addPointUniq(vector<Point> &vect, Point point);  //True if added

    void sortAndUniq(vector<Point> &vect);

    //printing
    void printVectorPoints(vector<Point> &v);
    void printInstructions();
};

bool Manager::setOriginFromMachine()
{
    float x, y, z;
    ifstream file("coordinates.txt");
    if(!file.is_open())
        return false;

    file >> x;
    file >> y;
    file >> z;

    origin.x = x;
    origin.y = y;
    origin.z = z;

    std::cout << "(" << origin.x << "; " << origin.y << "; " <<
        origin.z << ")" << std::endl;

    file.close();
    return true;
}

bool Manager::savePointsToFiles(v_Point points, std::string fileName)
{
    std::ofstream file;

    file.open(fileName.c_str());
    if(!file.is_open())
        return false;

    for(unsigned int i=0; i < points.size(); i++)
    {
        file << points[i].x << " " << points[i].y << " " << points[i].z << "\n";
    }

    file.close();
    return true;
}

bool Manager::saveRealPointsToFiles(const VideoStream &depthStream,
            VideoFrameRef frame)
{
    std::string fileName("points.xyz");
    float worX = 0, worY = 0, worZ = 0;  //world point (the point in real world)
    int depX = 0, depY = 0;
    DepthPixel depZ = 0;  //depth point (the point on screen)
    std::ofstream file;

    file.open(fileName.c_str());
    if(!file.is_open())
        return false;

    DepthPixel* pDepth = (DepthPixel*)frame.getData();
    int width = frame.getWidth(), height = frame.getHeight();

    for(depY=0; depY < height; depY++)
    {
        for(depX=0; depX < width; depX++)
        {
            depZ = pDepth[depY * width + depX];

            CoordinateConverter::convertDepthToWorld(depthStream,
                    depX, depY, depZ, &worX, &worY, &worZ);

            file << worX << " " << worY << " " << worZ << "\n";
        }
    }

    file.close();
    return true;
}

bool Manager::drawFrame(VideoFrameRef frame)
{
    if(!displayer.isInitialized())
    {
        if(!displayer.initialize(frame.getWidth(), frame.getHeight(), "Depth"))
        {
            std::cout << "Initializing displayer" << std::endl;
            return false;
        }
    }

    DepthPixel* pDepth = (DepthPixel*)frame.getData();
    int x = 0, y = 0, width = frame.getWidth(), height = frame.getHeight();
    int grayscale = 0, division = 1000;
    float value = 0;
    DepthPixel depth;

    for(y=0; y < height; y++)
    {
        for(x=0; x < width; x++)
        {
            depth = pDepth[y * width + x];
            value = static_cast<float>(depth % division);
            value = value / static_cast<float>(division) * 255.0;
            grayscale = static_cast<int>(value);
            displayer.setColor(grayscale, grayscale, grayscale, x, y);
        }
    }
    displayer.refresh();

    return true;
}


bool Manager::addPointUniq(vector<Point> &vect, Point point)
{
    //NOTE: this is a quick and dirty solution. We could considerate of sorting
    // the points (according to x for example) before doing that.
    for(unsigned int i=0; i < vect.size(); i++)
    {
        if(Geometry::pointsEqual2D(vect[i], point))
            return false;
    }
    vect.push_back(point);
    return true;
}

void Manager::sortAndUniq(vector<Point> &vect)
{
    std::sort(vect.begin(), vect.end(), Geometry::compare);
    // std::sort(vect.begin(), vect.end(), Geometry::xCompare);
    // std::stable_sort(vect.begin(), vect.end(), Geometry::xyCompare);

    std::vector<Point>::iterator it = vect.end();
    std::vector<Point>::iterator begin = vect.begin();

    while(it > begin + 1)
    {
        //TODO: Should check if Z equals 0
        if(Geometry::pointsEqual2D(*it, *(it-1)))
            vect.erase(it);
        it--;
    }
}

bool Manager::addRealPoints(const VideoStream &depthStream,
        VideoFrameRef &frame, vector<Point>&vect)
{
    DepthPixel* pDepth = (DepthPixel*)frame.getData();
    int x = 0, y = 0, width = frame.getWidth(), height = frame.getHeight();
    DepthPixel depth;
    Point point;

    for(y=0; y < height; y++)
    {
        for(x=0; x < width; x++)
        {
            depth = pDepth[y * width + x];

            CoordinateConverter::convertDepthToWorld(depthStream, x, y, depth,
                    &point.x, &point.y, &point.z);

            if(point.x == 0 && point.y == 0 && point.z == 0)
                continue;

            point.x += origin.x;
            point.y += origin.y;
            point.z += origin.z;

            vect.push_back(point);
        }
    }

    return true;
}

Manager::Manager()
{
    origin.x = 0;
    origin.y = 0;
    origin.z = 0;
}

bool Manager::initialize()
{
    Status rc = OpenNI::initialize();
    if(rc != STATUS_OK)
    {
        std::cerr << "Initialize failed" << std::endl;
        std::cerr << OpenNI::getExtendedError() << std::endl;
        return false;
    }

    rc = device.open(ANY_DEVICE);
    if(rc != STATUS_OK)
    {
        std::cerr << "Couldn't open device" << std::endl;
        std::cerr << OpenNI::getExtendedError() << std::endl;
        return false;
    }

    if(device.getSensorInfo(SENSOR_DEPTH) != NULL)
    {
        rc = depth.create(device, SENSOR_DEPTH);
        if(rc != STATUS_OK)
        {
            std::cerr << "Couldn't create depth stream" << std::endl;
            std::cerr << OpenNI::getExtendedError() << std::endl;
            return false;
        }
    }

    rc = depth.start();
    if(rc != STATUS_OK)
    {
        std::cerr << "Couldn't start the depth stream" << std::endl;
        std::cerr << OpenNI::getExtendedError() << std::endl;
        return false;
    }

    return true;
}

void Manager::mainLoop()
{
    Status rc;
    VideoFrameRef frame;
    bool toContinue(true), hasToSave(false), saveReference(true);
    vector<Point> reference, topologyRef, topology;
    SDL_Event event;

    printInstructions();

    while(toContinue)
    {
        int changedStreamDummy;
        VideoStream* pStream = &depth;
        rc = OpenNI::waitForAnyStream(&pStream, 1, &changedStreamDummy,
                SAMPLE_READ_WAIT_TIMEOUT);
        if(rc != STATUS_OK)
        {
            std::cerr << "Wait failed! (timeout is " << SAMPLE_READ_WAIT_TIMEOUT;
            std::cerr <<" ms)" << std::endl;
            std::cerr << OpenNI::getExtendedError() << std::endl;
            continue;
        }

        rc = depth.readFrame(&frame);
        if(rc != STATUS_OK)
        {
            std::cerr <<"Read failed!" << std::endl;
            std::cerr << OpenNI::getExtendedError() << std::endl;
            continue;
        }

        if(frame.getVideoMode().getPixelFormat() != PIXEL_FORMAT_DEPTH_1_MM &&
            frame.getVideoMode().getPixelFormat() != PIXEL_FORMAT_DEPTH_100_UM)
        {
            std::cerr <<"Unexpected frame format" << std::endl;
            continue;
        }

        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_QUIT:
                    toContinue = false;
                    break;
                case SDL_KEYDOWN:
                    if(event.key.keysym.sym == SDLK_ESCAPE)
                        toContinue = false;
                    else if(event.key.keysym.sym == SDLK_h)
                        printInstructions();
                    else if(event.key.keysym.sym == SDLK_r)
                    {
                        hasToSave = true;
                        saveReference = true;
                    }
                    else if(event.key.keysym.sym == SDLK_e)
                    {
                        hasToSave = true;
                        saveReference = false;
                    }
                    else if(event.key.keysym.sym == SDLK_t)
                    {
                        sortAndUniq(reference);
                        sortAndUniq(topologyRef);
                        std::cout << "Doing topology." << std::endl;
                        topology = Geometry::getTopology(reference, topologyRef);
                        std::cout << "Saving topology." << std::endl;
                        savePointsToFiles(topology, "topology.xyz");
                        std::cout << "Topology saved." << std::endl;
                    }
                    else if(event.key.keysym.sym == SDLK_o)
                        printVectorPoints(reference);
                    else if(event.key.keysym.sym == SDLK_p)
                        printVectorPoints(topologyRef);
                    else if(event.key.keysym.sym == SDLK_l)
                    {
                        sortAndUniq(reference);
                        savePointsToFiles(reference, "reference.xyz");
                    }
                    else if(event.key.keysym.sym == SDLK_m)
                    {
                        sortAndUniq(topologyRef);
                        savePointsToFiles(topologyRef, "topologyRef.xyz");
                    }
                    else if(event.key.keysym.sym == SDLK_g)
                    {
                        setOriginFromMachine();
                    }
                    break;
            }
        }

        if(hasToSave)
        {
            std::cout << "Saving sample." << std::endl;
            if(saveReference)
                addRealPoints(depth, frame, reference);
            else
                addRealPoints(depth, frame, topologyRef);
            std::cout << "Sample saved" << std::endl;
        }

        drawFrame(frame);
        hasToSave = false;
    }
    saveRealPointsToFiles(depth, frame);
}

void Manager::printVectorPoints(vector<Point> &v)
{
    std::cout << "Printing points" << std::endl;
    for(unsigned int i=0; i < v.size(); i++)
    {
        std::cout << "(" << v[i].x << "; " << v[i].y << "; ";
        std::cout << v[i].z << ")" << std::endl;
    }
}

void Manager::printInstructions()
{
    string msg;
    msg = "\nThis software is used to scan an object.\n";
    msg += "1) Take reference point (i.e. the table on which the object will ";
    msg += " be.\n";
    msg += "2) Take points of the object (which would be on the table).\n";
    msg += "3) Make the topology.\n";
    msg += "For setting the origin, modify the coordinates.txt\n";
    msg += "\tLine 1: x coordinate\n";
    msg += "\tLine 2: y coordinate\n";
    msg += "\tLine 3: z coordinate\n";
    msg += "Commands:\n";
    msg += "\tr - Take reference points\n";
    msg += "\te - Take topology reference points\n";
    msg += "\tt - Make the topology\n";
    msg += "\to - Print reference points\n";
    msg += "\tp - Print topology reference points\n";
    msg += "\tl - Save in file sorted and unique reference points\n";
    msg += "\tm - Save in file sorted and unique topology reference points\n";
    msg += "\tg - Set the origin\n";
    msg += "\th - Print instructions\n";
    msg += "\tEsc - Quit\n\n";

    std::cout << msg << std::endl;
}

void Manager::destroy()
{
    depth.stop();
    depth.destroy();
    device.close();
    OpenNI::shutdown();
    displayer.destroy();
}


int main()
{
    Manager manager;

    if(!manager.initialize())
        return 1;
    manager.mainLoop();
    manager.destroy();

    return 0;
}
