# drone_referee_2017_18
TU/e PDEng Mechatronics Systems Design - Module 2

This project is documented in our Wiki: http://cstwiki.wtb.tue.nl/index.php?title=Drone_Referee_-_MSD_2017/18

## Accessing World Model of the robots:

Checkout the multicast svn repository: http://robocup.wtb.tue.nl/svn/techunited/trunk/src/Turtle2/Libs/multicast/ 

There is only one dependency: http://robocup.wtb.tue.nl/svn/techunited/trunk/src/Turtle2/Global_par/, check this out too.

To be able to build the multicast library and processes you need the following packages in ubuntu:

flex
bison

After downloading and checking out the multicast folder you should "make install" at top level to build and install the required libs and processes.

http://robocup.wtb.tue.nl/svn/techunited/trunk/src/Turtle2/Libs/multicast/example/ contains an example.c and a README_EXAMPLE and HOW-IT-WORKS. 

1) make sure that you are connected to the robocup network (MSL_FIELD_A_a) 
2) Make sure that the root user has the environment variable AGENT set to a number between 11 and 30
> export AGENT=11
3) Start the process "listen" on your laptop at the wireless interface (as root, in a terminal where the AGENT variable is set) 
> listen <your wireless interface>
4) Start the example (as root, in a terminal where the AGENT variable is set)
> ./example

If a robot is on the field and switched on, you should see something changing in the output of the process. (It should only be switched on, you don't need to connect to it or drive it around!)

## Collision detection:

If the above tutorial works, checkout the **multicast/example/example.c** from this repository. This contains the code for detection collisions between robots

