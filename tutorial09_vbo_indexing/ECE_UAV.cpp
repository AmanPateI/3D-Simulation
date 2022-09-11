/*
Author: Aman Patel
Last Date Modified: 12/7/2021

Description: Thread member functions called in mail file. A member function start() 
             causes the thread member variable to run an external function called threadFunction
*/
#include "ECE_UAV.h"
#include <cmath>
#include <iostream>
#include <random>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

using namespace std;
// UAVs wait 5 seconds before moving
void threadFunction(ECE_UAV* pUAV)
{
    std::this_thread::sleep_for(std::chrono::seconds(5));
    do
    {
        pUAV->update();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    } while (!pUAV->m_bStop);
}
// begin movement
void ECE_UAV::start()
{
    processMovement = std::thread(threadFunction, this);
}
// stop movement
void ECE_UAV::stop()
{
    m_bStop = true;
    if (processMovement.joinable())
    {
        processMovement.join();
    }
}
// magnitude of velocity
double ECE_UAV::velMag() {
    return sqrt(pow(m_Velocity[0], 2) + pow(m_Velocity[1], 2) + pow(m_Velocity[2], 2));
}
// velocity unit vector representation
void ECE_UAV::UnitVecVelocity(double* input) {
    double mag = velMag();
    input[0] = m_Velocity[0] / mag;
    input[1] = m_Velocity[0] / mag;
    input[2] = m_Velocity[0] / mag;
}
// distance of UAV to virtual sphere
double ECE_UAV::distancetoSphere() {
    return sqrt(pow(sphereCenter[0] - m_Position[0], 2) + pow(sphereCenter[1] - m_Position[1], 2) + pow(sphereCenter[2] - m_Position[2], 2));
}
// force unit vector
void ECE_UAV::UnitVecForce(double* vecInput) {
    double distance = distancetoSphere();
    vecInput[0] = (sphereCenter[0] - m_Position[0]) / distance;
    vecInput[1] = (sphereCenter[1] - m_Position[1]) / distance;
    vecInput[2] = (sphereCenter[2] - m_Position[2]) / distance;
}
// magnitude of UAV force
double ECE_UAV::forceMagnitude() {
    double distance = distancetoSphere();
    double force = 0;
    if (initial)
    {
        // limit initial velocity
        if (velMag() < 2)
        {
            force = -springConstant* (10 - distancetoSphere());   // Hooke's law
        }
        else
        {
            force = 0;
        }

        // Condition to break out of the initial approach
        if (distance < 11)
        {
            initial = false;
        }
    }
    else
    {
        force = -springConstant * (10 - distancetoSphere());

        if (!slowingDown)
        {
            force *= .8;
        }
    }
    if (force > maxForce)
    {
        return maxForce;
    }
    else if (force < -maxForce)
    {
        return -maxForce;
    }
    else
    {
        return force;
    }
}

void ECE_UAV::getNormalVector(double* vec) {
    std::random_device rd; // random number generator
    std::mt19937 eng(rd()); // seed the generator
    std::uniform_int_distribution<> distr(1, 3); // range of seeded generator

    double x = distr(eng);
    double y = distr(eng);
    double z = distr(eng);
    double increment = distr(eng);
    bool set = false;

    double newVec[3];
    for (int i = 0; i < 3; i++)
    {
        int test = i + increment;
        switch (test % 3)
        {
        case 0:
            if (vec[0] != 0)
            {
                newVec[0] = (-y * vec[1] - z * vec[2]) / vec[0];
                newVec[1] = y;
                newVec[2] = z;
                set = true;
            }
            break;
        case 1:
            if (vec[1] != 0)
            {
                newVec[0] = (-x * vec[0] - z * vec[2]) / vec[1];
                newVec[1] = y;
                newVec[2] = z;
                set = true;
            }
            break;
        case 2:
            if (vec[2] != 0)
            {
                newVec[0] = (-x * vec[0] - y * vec[1]) / vec[2];
                newVec[1] = y;
                newVec[2] = z;
                set = true;
            }
            break;
        }
        if (set)
        {
            break;
        }
    }
    if (!set)
    {
        newVec[0] = 1;
        newVec[1] = 1;
        newVec[2] = 1;
    }

    double mag = sqrt(pow(newVec[0], 2) + pow(newVec[1], 2) + pow(newVec[2], 2));

    if (mag == 0) {
        newVec[0] = 1;
        newVec[1] = 1;
        newVec[2] = 1;
    }

    vec[0] = newVec[0] / mag;
    vec[1] = newVec[1] / mag;
    vec[2] = newVec[2] / mag;

}
// get UAV force value
void ECE_UAV::getForce(double* dir) {
    double mag = forceMagnitude();

    // limited force magnitude to account for gravity force
    if (mag > 8)
    {
        mag = 8;
    }
    else if (mag < -8)
    {
        mag = -8;
    }
    // calculted force direction
    UnitVecForce(dir);
    dir[0] *= mag;
    dir[1] *= mag;
    dir[2] *= mag;

    if (!initial)
    {
        double Force = 2;
        double orthog_vec[3];
        UnitVecForce(orthog_vec);
        getNormalVector(orthog_vec);
        dir[0] += Force * orthog_vec[0];
        dir[1] += Force * orthog_vec[1];
        dir[2] += Force * orthog_vec[2];
    }
    dir[2] += g;
    double forceMag = sqrt(pow(dir[0], 2) + pow(dir[1], 2) + pow(dir[2], 2));
    dir[2] -= g;
}
// update UAV positions
void ECE_UAV::update() {
    double force[3];
    getForce(force);
    // motion equation for UAVs
    posLock.lock();
    m_Position[0] += m_Velocity[0] * timeStep + .5 * (force[0] / m_mass) * pow(timeStep, 2);
    m_Position[1] += m_Velocity[1] * timeStep + .5 * (force[1] / m_mass) * pow(timeStep, 2);
    m_Position[2] += m_Velocity[2] * timeStep + .5 * (force[2] / m_mass) * pow(timeStep, 2);
    posLock.unlock();
    Velocitylock.lock();
    m_Velocity[0] += (force[0] / m_mass) * timeStep;
    m_Velocity[1] += (force[1] / m_mass) * timeStep;
    m_Velocity[2] += (force[2] / m_mass) * timeStep;
    Velocitylock.unlock();
}
// collision logic
bool ECE_UAV::checkCollision(double* position) {
    posLock.lock();
    double dist = sqrt(pow(position[0] - m_Position[0], 2) + pow(position[1] - m_Position[1], 2) + pow(position[2] - m_Position[2], 2));
    posLock.unlock();
    return dist < 6.1;
}
// simple cross product for use
void ECE_UAV::crossProduct(double* v_A, double* v_B, double* c_P) {
    c_P[0] = v_A[1] * v_B[2] - v_A[2] * v_B[1];
    c_P[1] = -(v_A[0] * v_B[2] - v_A[2] * v_B[0]);
    c_P[2] = v_A[0] * v_B[1] - v_A[1] * v_B[0];
}