#include <iostream>
#include <ctime>
#include <fstream>
#include <string>
#include <vector>
#include <stdio.h>
#include <OpenNI.h>
#include <rapidjson/document.h>

#include "top_types.h"
#include "displayer.h"
#include "geometry.h"
#include "dashboard.h"

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
    static Point sensorPosition;
    static Point distanceBitSensor;
    static bool sensorSetted;  //Changing the name? More "callbackCalled"
// private:
    Device device;
    VideoStream depth;
    Displayer displayer;
    v_Point pathToScan;
    VideoFrameRef frame;
    v_Point topology, capturedPoints;

    std::string url;

    bool updateDepthAndFrame();

    bool processScanning();

    bool drawFrame(VideoFrameRef frame);

    bool savePointsToFile(v_Point points, std::string namefile);

    bool setSensorPosition();  //Returns false if problem
    void printSensorPosition();

    bool loadConfig();

    //Network communication:
    // Gives order to the machine to move the sensor
    bool moveSensorTo(float x, float y, float z);
    // Callback function when asking bit position
    static void getSensorPositionCb(bool result, float x, float y, float z);
    // Asks sensor position to the machine
    static bool getSensorPosition();

    //Save points in the given vector, convert them from the sensor point of
    // view to the machine point of view (by using the sensor position from
    // the machine)
    void addRealPoints(const VideoStream &depthStream, VideoFrameRef &frame,
            vector<Point> &vect);

    //printing
    void printInstructions();
};

Point Manager::sensorPosition;
Point Manager::distanceBitSensor;
bool Manager::sensorSetted = false;

void Manager::printSensorPosition()
{
    std::cout << "**** Current sensorPosition : (" << sensorPosition.x << "; ";
    cout << sensorPosition.y << "; " << sensorPosition.z << ") ****" << endl;
}

// Just set the sensor position from file, print nothing. Returns false if problem.
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

bool Manager::loadConfig()
{
    std::cout << "===== loadConfig =====" << std::endl;
    FILE* f = NULL;
    f = fopen("config.json", "r");

    if(!f)
        return false;

    std::cout << "Found file" << std::endl;

    // Determine file size
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);

    char* json = new char[size];

    rewind(f);
    fread(json, sizeof(char), size, f);

    rapidjson::Document document;
    document.Parse(json);

    Point point(0, 0, 0);

    if(document.IsObject())
    {
        if(document.HasMember("url") && document["url"].IsString())
            url = document["url"].GetString();
        else
        {
            std::cout << "No url found in config.json" << std::endl;
            url = "";
        }

        if(document.HasMember("distanceBitSensor") &&
                document["distanceBitSensor"].IsObject())
        {
            if(document["distanceBitSensor"].HasMember("x") &&
                document["distanceBitSensor"].HasMember("y") &&
                document["distanceBitSensor"].HasMember("z"))
            {
                point.x = document["distanceBitSensor"]["x"].GetDouble();
                point.y = document["distanceBitSensor"]["y"].GetDouble();
                point.z = document["distanceBitSensor"]["z"].GetDouble();
            }
        }
        else
            std::cout << "No distanceBitSensor found in config.json" << std::endl;

        distanceBitSensor = point;

        if(document.HasMember("pathToScan") && document["pathToScan"].IsArray())
        {
            for(unsigned int i=0; i < document["pathToScan"].Size(); i++)
            {
                if(document["pathToScan"][i].IsObject())
                {
                    if(document["pathToScan"][i].HasMember("x") &&
                        document["pathToScan"][i].HasMember("y") &&
                        document["pathToScan"][i].HasMember("z"))
                    {
                        point.x = document["pathToScan"][i]["x"].GetDouble();
                        point.y = document["pathToScan"][i]["y"].GetDouble();
                        point.z = document["pathToScan"][i]["z"].GetDouble();
                        pathToScan.push_back(point);
                    }
                }
            }
        }
        else
            std::cout << "No pathToScan found in config.json" << std::endl;
    }
    else
    {
        url = "";
        distanceBitSensor.set(0, 0, 0);
        std::cout << "No config.json found" << std::endl;
    }

    std::cout << "Load config is done:" << std::endl;
    std::cout << "url : " << url << std::endl;
    std::cout << "dist: " << "(" << distanceBitSensor.x << "; " << distanceBitSensor.y << "; " << distanceBitSensor.z << ")" << std::endl;
    std::cout << "Path size: " << pathToScan.size() << std::endl;

    fclose(f);
    delete[] json;

    return true;
}


// Wait until at the position. Return false if problem (no connexion or not moving)
bool Manager::moveSensorTo(float x, float y, float z)
{
    Point currentPosition = sensorPosition;
    bool commandSent = false;

        std::cout << "[[[[ Move sensor to ======" << std::endl;
        cout << "Move to: " << "(" << x << "; " << y << "; " << z << ")" << endl;

        x -= distanceBitSensor.x;
        y -= distanceBitSensor.y;
        z -= distanceBitSensor.z;

        cout << "Really move to: " << "(" << x << "; ";
        cout << y << "; " << z << ")" << endl;
        std::cout << "====== Move sensor to ]]]]" << std::endl;

    for(int i=0; i < 3; i++)
    {
        if(Dashboard::setPosition(x, y, z))
        {
            commandSent = true;
            break;
        }
    }

    std::cout << "commandSent:" << commandSent << std::endl;

    if(!commandSent)
        return false;

    return true;
    // //Wait until the end of the job
    // //TODO: test if connection problem
    // while(Dashboard::isRunning() != DASHBOARD_TRUE);
    //
    // getSensorPosition();
    //
    // return (sensorPosition.x == x && sensorPosition.y == y &&
    //         sensorPosition.z == z);

    // return true;
}

//Set sensor position (this function is used as callback for dashboard)
void Manager::getSensorPositionCb(bool result, float x, float y, float z)
{
    //Receiving bit position (not sensor position)
    if(result)
    {
        std::cout << "[[[[ Sensor position cb ======" << std::endl;
        cout << "Bit:" << "(" << x << "; " << y << "; " << z << ")" << endl;
        sensorPosition.x = x + distanceBitSensor.x;
        sensorPosition.y = y + distanceBitSensor.y;
        sensorPosition.z = z + distanceBitSensor.z;
        sensorSetted = true;
        cout << "sensorPosition: " << "(" << sensorPosition.x << "; ";
        cout << sensorPosition.y << "; " << sensorPosition.z << ")" << endl;
        std::cout << "====== Sensor position cb ]]]]" << std::endl;
    }

    return;
}

bool Manager::getSensorPosition()
{
    int i = 3;
    sensorSetted = false;
    while(i > 0)
    {
        //Test if manage to send a request and the given answer is correct
        if(Dashboard::getPosition() && sensorSetted)
            return true;
        i--;
    }

    return false;
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

//Scan the body (do not do the topology)
bool Manager::processScanning()
{
    std::cout << "Processing scanning" << std::endl;
    for(size_t i=0; i < pathToScan.size(); i++)
    {
        cout << "Processing scan for: " << "(" << pathToScan[i].x << "; ";
        cout  << pathToScan[i].y << "; " << pathToScan[i].z << ")" << endl;

        moveSensorTo(pathToScan[i].x, pathToScan[i].y, pathToScan[i].z);
        if(!getSensorPosition())
        {
            std::cout << "Impossible to process scanning (impossible to have ";
            std::cout << "the sensor position)" << std::endl;
            return false;
        }
        cout << "Position: " << "(" << sensorPosition.x << "; ";
        cout  << sensorPosition.y << "; " << sensorPosition.z << ")" << endl;
        updateDepthAndFrame();
        addRealPoints(depth, frame, capturedPoints);
    }
    std::cout << "End processing scanning" << std::endl;
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

    loadConfig();

    std::cout << "Initialize with url: " << url << std::endl;
    Dashboard::initialize(url, &(Manager::getSensorPositionCb));
    std::cout << "Dashboard::baseURL: " << Dashboard::baseURL  << std::endl;

    getSensorPosition();

    return true;
}

bool Manager::updateDepthAndFrame()
{
    Status rc;
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
        return false;
    }

    rc = depth.readFrame(&frame);
    if(rc != STATUS_OK)
    {
        std::cerr <<"Read failed!" << std::endl;
        std::cerr << OpenNI::getExtendedError() << std::endl;
        return false;
    }

    if(frame.getVideoMode().getPixelFormat() != PIXEL_FORMAT_DEPTH_1_MM &&
            frame.getVideoMode().getPixelFormat() != PIXEL_FORMAT_DEPTH_100_UM)
    {
        std::cerr <<"Unexpected frame format" << std::endl;
        return false;
    }

    return true;
}

void Manager::mainLoop()
{
    Status rc;
    bool toContinue(true);
    SDL_Event event;

    setSensorPosition();

    printInstructions();
    processScanning();

    while(toContinue)
    {
        if(!updateDepthAndFrame())
            continue;

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
                    else if(event.key.keysym.sym == SDLK_n)
                    {
                        if(!getSensorPosition())
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
    msg += "\tp - Set the sensor [p]osition from file\n";
    msg += "\tn - Set the sensor position from [n]etwork\n";
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
    Dashboard::cleanAll();
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
