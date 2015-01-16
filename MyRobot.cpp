#include "MyRobot.h"
#include "stdio.h"

#define RUN_ONCE_VAR(joystick,button,var)		\
    static bool var = false;				\
    if(!joystick->GetRawButton(button))			\
    {							\
    	var  = false;					\
    }							\
    else if(joystick->GetRawButton(button) &&		\
    	    !var && (var=true))

#define RUN_ONCE(joystick,button)			\
    RUN_ONCE_VAR(joystick,button,joystick##_##button##_pressed)

void message(char *fmt, ...)
{
    char message[256];

    va_list args;
    va_start(args, fmt);
    vsnprintf(message, 256, fmt, args);
    va_end(args);

    setErrorData(message, strlen(message), 100);
}

double limit(double val, double min = -1, double max = 1) 
{
    if(val > max)
    	return max;
    if(val < min)
    	return min;
    
    return val;
}

char *GetStringForBool(bool val) {
	if (val) {
		return "true";
	} else {
		return "false";
	}
}

MyRobot::MyRobot(void)    
{
	compressor = new Compressor(CHAN_COMP_AUTO_SHUTOFF, CHAN_RLY_COMPRESSOR);

	joystick = new Joystick(1);
#ifdef COMPETITION // competition
	steeringWheel = new Joystick(2);
#endif
	
	cameraSystem = new CameraSystem(AxisCamera::GetInstance());
//	camera = new CameraSystem();
	drive = new DriveSystem(new Encoder(CHAN_ENCODER_LEFT_A, CHAN_ENCODER_LEFT_B, false, Encoder::k1X),\
							new Encoder(CHAN_ENCODER_RIGHT_A, CHAN_ENCODER_RIGHT_B, false, Encoder::k1X),\
							new Solenoid(1, CHAN_SHIFT_SOLENOID_A),\
							new Solenoid(1, CHAN_SHIFT_SOLENOID_B),\
							new Talon(CHAN_LEFT_DRIVE_TALON),\
							new Talon(CHAN_RIGHT_DRIVE_TALON));
	ballSystem = new BallSystem(new CANJaguar(CHAN_JAG_PICKUP_LEFT),
								new CANJaguar(CHAN_JAG_PICKUP_RIGHT),
								new Solenoid(CHAN_PICKUP_SOLENOID_A),
								new Solenoid(CHAN_PICKUP_SOLENOID_B),
								new Solenoid(CHAN_WINCH_SOLENOID_A),
								new Talon(CHAN_WINCH_MOTOR),
								new AnalogChannel(CHAN_POT),
								new DigitalInput(CHAN_LIMIT_SWITCH_LEFT),
								new DigitalInput(CHAN_LIMIT_SWITCH_RIGHT),
								new DigitalInput(CHAN_PISTON_SWITCH_LEFT),
								new DigitalInput(CHAN_PISTON_SWITCH_RIGHT));
	usePid = box->GetPIDEnabled();  // if on, then use PID, otherwise override PID
	drive->SetPIDDrive(usePid);

#ifdef PARADE
	drive->SetShiftOverride(true); // parade
#else
	drive->SetShiftOverride(false);
#endif
	
	drive->SetWheelDiameter(4);
#ifdef COMPETITION
	box = new DSBox(DriverStation::GetInstance(), ballSystem);
#endif
}

MyRobot::~MyRobot() 
{
#ifdef COMPETITION
	delete box;
#endif
	delete drive;
	delete ballSystem;
	delete cameraSystem;
	delete joystick;
	delete steeringWheel;
	delete compressor;
}

void MyRobot::Autonomous(void) 
{	
//	CameraSystem cam = new CameraSystem();
	compressor->Start();
	if (AUTONOMOUS_MODE == 0) {
		drive->StartRecordingDistance();
		drive->SetDriveInstruction(3.0f, 0.0f);
		while(IsAutonomous()) {
			if (drive->GetDistanceTraveled() > 180) {
				drive->SetDriveInstruction(0.0f, 0.0f);
				ballSystem->SetPickupState(ballSystem->passing);
			}
			drive->Update();
			ballSystem->Update();
			Wait(0.02);
		}
		ballSystem->SetPickupState(ballSystem->locked);
	} else {
		drive->StartRecordingDistance();
		drive->SetDriveInstruction(5.4f, 0.0f);
		drive->SetShiftOverride(true);
		drive->SetShiftSuppressionMode(1);
		Timer *timer = new Timer();
		timer->Start();
		bool imageTaken = false;
		bool hasMoved = false;
		bool hasFired = false;
		while(IsAutonomous()) {
#if 0
			if (!hasMoved && drive->GetDistanceTraveled() > 150) {
				drive->SetDriveInstruction(0.0f, 0.0f);
				hasMoved = true;
			}
#endif
			
			if (drive->GetDistanceTraveled() > 186) {
				drive->SetDriveInstruction(0.0f, 0.0f);
				if (cameraSystem->HotOrNot() || timer->Get() > 6.0) {
					ballSystem->SetPickupState(ballSystem->passing);
				}
			}			
			if (!imageTaken && drive->GetDistanceTraveled() > 110){
				cameraSystem->Scan();
				imageTaken = true;
			}
			
			if (drive->GetDistanceTraveled() > 150 && drive->GetDistanceTraveled() < 186) {
				drive->SetDriveInstruction(2.0f, 0.0f);
			}
			
#if 0
			if (timer->Get() < 8.0f) {
				if (hasMoved) {
					cameraSystem->Scan();
					if (camera->HotOrNot()) {
						ballSystem->Fire();
						hasFired = true;
					} else {
						sleep(200);
					}
				}
			} else if (timer->Get() > 8.0f && !hasFired) {
				ballSystem->Fire();
				hasFired = true;
			}
#endif
			
			ballSystem->Update();
			drive->Update();
			Wait(0.02);
		}
	}
}

void MyRobot::OperatorControl(void)
{ 
	GetWatchdog().SetExpiration(100.0);
	GetWatchdog().SetEnabled(true);
	
	compressor->Start();
	drive->SetShiftOverride(false);
	int init_pot = ballSystem->GetCatapultPot();
	if(init_pot > 400) {
		ballSystem->SetCatapultState(ballSystem->fired);
	}
	
	bool pickupStateCanSwitch = true;  // what is this?
	
    while (IsOperatorControl() && IsEnabled()) {
		GetWatchdog().Feed();
		//camera->Scan();
		
#ifdef COMPETITION
		char *pickupButtonVal = GetStringForBool(box->GetPickupButton());
		char *pidVal = GetStringForBool(box->GetPIDEnabled());
		char *shootModeVal = GetStringForBool(box->GetShooterMode());
		char *passButtonVal = GetStringForBool(box->GetPassButton());
		char *fireButtonVal = GetStringForBool(box->GetShootButton());
		char *armButtonVal = GetStringForBool(box->GetArmButton());
		
		char *limSwitchLeftVal = GetStringForBool(ballSystem->GetLimitSwitchLeft());
		char *limSwitchRightVal = GetStringForBool(ballSystem->GetLimitSwitchRight());
		
		char *pistonSwitchLeftVal = GetStringForBool(ballSystem->GetPistonSwitchLeft());
		char *pistonSwitchRightVal = GetStringForBool(ballSystem->GetPistonSwitchRight());
		
//		message("Box Input Report: Pickup Button-%s, PID Switch-%s, Shoot Mode Switch-%s, Pass Button:-%s, Fire Button-%s, Arm Button-%s\nLimit Switch Report: Left Shooter Switch-%s, RightShooter Switch-%s", pickupButtonVal, pidVal, shootModeVal, passButtonVal, fireButtonVal, armButtonVal, limSwitchLeftVal, limSwitchRightVal);
//		message("Piston Switch Left-%s, Piston Switch Right-%s, Pot Value-%i", pistonSwitchLeftVal, pistonSwitchRightVal, ballSystem->GetCatapultPot());
#endif
		
		float x, y;
#ifdef COMPETITION
		x = steeringWheel->GetX();
#else // parade mode
		x = joystick->GetX();
#endif
		y = -joystick->GetY();
		
		
		if(x > -0.1 && x < 0.1) {
			x = 0;
		}
		if(y > -0.1 && y < 0.1) {
			y = 0;
		}

		drive->SetDriveInstruction(y * MAX_RPS, x * MAX_RPS);
// check to see if pid override has changed
		if (usePid != box->GetPIDEnabled())
		{
			usePid = box->GetPIDEnabled();
			drive->SetDriveInstruction(y * MAX_RPS, x * MAX_RPS);
//			drive->SetPIDDrive(!box->GetPIDEnabled());
			drive->SetPIDDrive(usePid);
		}

		drive->Update();

		if (!ballSystem->GetPickupOverride()) {
			if (ballSystem->GetPickupState() == ballSystem->locked || ballSystem->GetPickupState() == ballSystem->passing) {
#ifdef COMPETITION
				if (box->GetPassButton()) { 
#else // parade
				if (joystick->GetRawButton(4)) {
#endif
//					message("pass button pressed in locked or passing state");
					ballSystem->SetPickupState(ballSystem->passing);
				} else {
					ballSystem->SetPickupState(ballSystem->locked);
//					message("set state to locked");
				}
#ifdef COMPETITION
				if (box->GetPickupButton() && pickupStateCanSwitch) { // CanSwitch true
#else // parade
				if (joystick->GetRawButton(5) && pickupStateCanSwitch) {
#endif
//					message("pickup button pressed & CanSwitch true");
					pickupStateCanSwitch = false;
					ballSystem->SetPickupState(ballSystem->deploying);
//					message("setting state to deploying and CanSwitch false");
				}
			} else if (ballSystem->GetPickupState() == ballSystem->deployed) {
#ifdef COMPETITION
				if (box->GetPickupButton() && pickupStateCanSwitch) {
#else // parade
				if (joystick->GetRawButton(5) && pickupStateCanSwitch) {
#endif
//					message("pickup button pressed again in deployed state");
					pickupStateCanSwitch = false;
					ballSystem->SetPickupState(ballSystem->locked);
//					message("in locked state and CanSwitch false");
				}
			}
		}
    
#ifdef COMPETITION
		if (!box->GetPickupButton()) {
#else // parade
		if (!joystick->GetRawButton(5)) {
#endif
			pickupStateCanSwitch = true;
//			message("CanSwitch now set to true - pickup buttom pressed");
		}

#ifdef PARADE		
		if (joystick->GetRawButton(8)) {
			drive->SetShiftOverride(true);
		}
		if (joystick->GetRawButton(9)) {
			drive->SetShiftOverride(false);
		}
#endif
		
#ifdef COMPETITION
		if (box->GetArmButton() && ballSystem->GetCatapultState() == ballSystem->fired) {
			ballSystem->Arm();
		}
		if (box->GetShootButton() && ballSystem->GetCatapultState() == ballSystem->armed) {
			ballSystem->Fire();
		}
#else // parade		
		if (joystick->GetRawButton(2) && ballSystem->GetCatapultState() == ballSystem->fired) {
			ballSystem->Arm();
		}
		if (joystick->GetRawButton(1) && ballSystem->GetCatapultState() == ballSystem->armed) {
			ballSystem->Fire();
		}		
#endif
	
#ifdef COMPETITION		
		ballSystem->SetCatapultAutoArm(box->GetShooterMode());
#endif
		
		ballSystem->Update();
#ifdef COMPETITION
		box->Update();
#endif
		
		SmartDashboard::PutNumber("Encoder Right", drive->GetRightEncoder());
		SmartDashboard::PutNumber("Encoder Left", drive->GetLeftEncoder());
		
		SmartDashboard::PutNumber("Pot", ballSystem->GetCatapultPot());
		
		SmartDashboard::PutString("Catapult State", ballSystem->GetCatapultStateS());
		SmartDashboard::PutString("Pickup State", ballSystem->GetPickupStateS());
		SmartDashboard::PutNumber("PID State", usePid);
		
/*		message("Pot value = %d, cat state = %d,  pick state = %d", 
				ballSystem->GetCatapultPot(), ballSystem->GetCatapultState(),
				ballSystem->GetPickupState()); */
		
		Wait(0.02);
	}
	// why is this commented out?
    //ballSystem->SetPickupState(ballSystem->locked);
    //ballSystem->Update();

    compressor->Stop();
}

START_ROBOT_CLASS(MyRobot) //This tells WPILib to start this robot class when code loads
			   //Do not remove or nothing will run
