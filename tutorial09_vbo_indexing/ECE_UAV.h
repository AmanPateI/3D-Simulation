/*
Author: Aman Patel
Last Date Modified: 12/7/2021

Description: Header file used to outline ECE_UAV.cpp. Contains member variables for mass, (x,y,z), 
             position, (vx, vy, yz) velocities, and (ax, ay, az) accelerations of the UAV
*/
#include <thread>
#include <string>
#include <atomic>
#include <mutex>

using namespace std;

class ECE_UAV {

public:
    void start();
    void stop();
    double distancetoSphere();
    void UnitVecForce(double* input);
    void UnitVecVelocity(double* input);
    double forceMagnitude();
    double velMag();
    void getForce(double* input);
    void getNormalVector(double* vec);
    void update();
    bool checkCollision(double* input);

    void position(double* inPos)
    {
        posLock.lock();
        memcpy(m_Position, inPos, 3 * sizeof(double));
        posLock.unlock();
    }

    void getPosition(double* inPos)
    {
        posLock.lock();
        memcpy(inPos, m_Position, 3 * sizeof(double));
        posLock.unlock();
    }

    void velocity(double* inVel)
    {
        Velocitylock.lock();
        memcpy(m_Velocity, inVel, 3 * sizeof(double));
        Velocitylock.unlock();
    }

    void getVelocity(double* inVel)
    {
        Velocitylock.lock();
        memcpy(inVel, m_Velocity, 3 * sizeof(double));
        Velocitylock.unlock();
    }

    void acceleration(double* inAcc)
    {
        Velocitylock.lock();
        memcpy(m_Acceleration, inAcc, 3 * sizeof(double));
        Velocitylock.unlock();
    }

    void getAcceleration(double* inAcc)
    {
        Velocitylock.lock();
        memcpy(inAcc, m_Acceleration, 3 * sizeof(double));
        Velocitylock.unlock();
    }
    friend void threadFunction(ECE_UAV* pUAV);
    void crossProduct(double* v_A, double* v_B, double* c_P);
private:
    std::atomic<bool> m_bStop = { false };
    double m_mass = 1.00;
    double m_Position[3] = {};
    double m_Velocity[3] = {};
    double m_Acceleration[3] = {};

    std::thread processMovement;

    std::mutex Velocitylock, posLock;

    double springConstant = 1;
    double maxForce = 20.0;
    double sphereCenter[3] = { 0, 0, 70 };
    bool initial = true;
    double g = -10.0;
    bool slowingDown = false;
    double timeStep = 0.05;
};