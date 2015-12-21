#include <iostream>
#include <ctime>
#include <fstream>
#include <string>
#include <vector>
#include <stdio.h>
#include <OpenNI.h>

#include "top_types.h"
#include "displayer.h"
#include "geometry.h"

#define SAMPLE_READ_WAIT_TIMEOUT 2000 //2000ms

using namespace openni;
using namespace std;

/**
 * Class setting the parameters and doing the interactions/interface.
 */
class Manager
{
public:
    Manager();
    bool initialize();
    void mainLoop();
    void destroy();
    Point sensorPosition;
private:
    Device device;
    VideoStream depth;
    Displayer displayer;

    bool drawFrame(VideoFrameRef frame);

    bool savePointsToFile(v_Point points, std::string namefile);

    //TODO: find a way to communicate with the tool
    bool setSensorPosition();  //Returns false if problem
    void printSensorPosition();

    //Save points in the given vector, convert them from the sensor point of
    // view to the machine point of view (by using the sensor position from
    // the machine)
    void addRealPoints(const VideoStream &depthStream, VideoFrameRef &frame,
            vector<Point> &vect);

    //printing
    void printInstructions();
};

void Manager::printSensorPosition()
{
    std::cout << "**** Current sensorPosition : (" << sensorPosition.x << "; ";
    cout << sensorPosition.y << "; " << sensorPosition.z << ") ****" << endl;
}

// Just set the sensor position, print nothing. Returns false if problem.
bool Manager::setSensorPosition()
{
    float x, y, z;
    ifstream file("coordinates.txt");
    if(!file.is_open())
        return false;

    file >> x;
    file >> y;
    file >> z;

    sensorPosition.x = x;
    sensorPosition.y = y;
    sensorPosition.z = z;

    file.close();
    return true;
}

bool Manager::savePointsToFile(v_Point points, std::string fileName)
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

// Not added if the point has a depth equal to zero
// Sorts and checks if points are unique
void Manager::addRealPoints(const VideoStream &depthStream,
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
            if(depth == 0)
                continue;

            // Get the point from the sensor point of view (z = depth)
            CoordinateConverter::convertDepthToWorld(depthStream, x, y, depth,
                    &point.x, &point.y, &point.z);

            // Convert the point into machine point of view (z = height)
            point.x = sensorPosition.x + point.x;
            point.y = sensorPosition.y + point.y;
            point.z = sensorPosition.z - point.z;

            vect.push_back(point);
        }
    }

    Geometry::sortAndUnique(vect);
}

Manager::Manager()
{
    sensorPosition.x = 0;
    sensorPosition.y = 0;
    sensorPosition.z = 0;
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
    bool toContinue(true);
    vector<Point> topology, capturedPoints;
    SDL_Event event;

    clock_t c_start, c_end;

    setSensorPosition();

    printInstructions();

    while(toContinue)
    {
        int changedStreamDummy;
        VideoStream* pStream = &depth;

        if(pStream->getMirroringEnabled())
            pStream->setMirroringEnabled(false);

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
                    else if(event.key.keysym.sym == SDLK_c)
                    {
                        addRealPoints(depth, frame, capturedPoints);
                        cout << "Point added" << endl;
                    }
                    else if(event.key.keysym.sym == SDLK_s)
                    {
                        savePointsToFile(capturedPoints, "topology.xyz");
                        cout << "Topology saved in topology.xyz" << endl;
                    }
                    else if(event.key.keysym.sym == SDLK_p)
                    {
                        if(setSensorPosition())
                            cout << "New sensor position set" << endl;
                        else
                            cout << "Failed to set new sensor position" << endl;
                        printSensorPosition();
                    }
                    break;
            }
        }

        drawFrame(frame);
    }
}

void Manager::printInstructions()
{
    string msg;
    msg = "\nThis software is used to scan an object using a 3D sensor.\n";
    msg += "It captures the points and converts there positions to ";
    msg += "the machine coordinate.\n";
    msg += "For setting the sensor position, modify the file coordinates.txt\n";
    msg += "\tLine 1: x coordinate\n";
    msg += "\tLine 2: y coordinate\n";
    msg += "\tLine 3: z coordinate\n";
    msg += "Commands:\n";
    msg += "\tc - [C]apture points\n";
    msg += "\ts - [S]ave points\n";
    msg += "\tp - Set the sensor [p]osition\n";
    msg += "\th - Print these instructions\n";
    msg += "\tEsc - Quit\n\n";

    std::cout << msg << std::endl;
    printSensorPosition();
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
