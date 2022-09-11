# 3D-Simulation
3D simulation using std::thread and OpenGL

The UAVs remain on the ground for 5 seconds after the beginning of the simulation. 
After the initial 5 seconds the UAVs then launch from the ground and go towards the 
point (0, 0, 50 m) above the ground with a maximum velocity of 2 m/s 

As they approach the point, (0, 0, 50 m),  they began to fly in random paths along the 
surface of a virtual sphere of radius 10 m while attempting to maintain a speed between 
2 to 10 m/s. 
 The simulation ends once all of the UAV have come within 10 m of the point, (0, 0, 50 
m), and the UAVs have flown along the surface for 60 seconds.  

Each UAV has the following  
a. Each UAV has a mass of 1 kg and is able to generate a single force vector with a 
total maximum magnitude of 20 N in any direction. 
b. Scale so it is just small enough to fix in a 20-cm cube bounding box. 

Multithreaded application using 16 threads.  The main thread is 
responsible for rendering the 3D scene with the 15 UAVs and a green (RGB=(0,255,0)) 
rectangle representing the football field in a 400 x 400 window. The other 15 threads are each 
responsible for controlling the motion of a single UAV. 

UAV that has member variables to contain the mass, (x, y, z) 
position, (vx, vy, yz) velocities, and (ax, ay, az) accelerations of the UAV. A member function start() 
causes the thread member variable to run an external function called threadFunction void threadFunction(UAV* pUAV);  
 
The threadFunction updates the kinematic information every 10 msec. 
The main thread polls the UAV once every 30 msec to determine their current locations 
in order to update the 3D scene. 
The coordinate system of the 3D simulation is defined as follows: The origin is located at 
the center of the football field at ground level.  The positive z axis points straight up, and 
the x axis is along the width of the field and the y axis is along the length. 

A camera location, orientation, and field of view should be used so that the whole 
football field and the virtual 10m sphere is in the view.

The flight of the UAV is controlled by the following kinematic rules 
a. The force vector created by the UAV plus the force of gravity (10 N in the 
negative z direction) determine the direction of acceleration for the UAV. 
b. Newton’s 2nd Law to determine the acceleration of the UAV in each 
direction. 
