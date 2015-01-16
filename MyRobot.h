#ifndef MyRobot_h_
#define MyRobot_h_

#include "WPILib.h"
#include "DriveSystem.cpp"
#include "Constants.h"
#include "CameraSystem.cpp"
#include "DSBox.cpp"

class MyRobot : public SimpleRobot
{
private:
	DSBox *box;
	DriveSystem *drive;
	CameraSystem *cameraSystem;
	BallSystem *ballSystem;
	Joystick *joystick;
	Joystick *steeringWheel;
	Compressor *compressor;
	int usePid;
	
public:
    MyRobot(void);
    ~MyRobot(void);
    void Autonomous(void);
    void OperatorControl(void);
    void Drive(float, float);
};

#endif
