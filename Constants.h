#define CHAN_ENCODER_LEFT_A 		10
#define CHAN_ENCODER_LEFT_B			11
#define CHAN_ENCODER_RIGHT_A		3
#define CHAN_ENCODER_RIGHT_B		4
#define CHAN_GYRO					0
#define CHAN_SHIFT_SOLENOID_A		3
#define CHAN_SHIFT_SOLENOID_B		4
#define CHAN_PICKUP_SOLENOID_A		5
#define CHAN_PICKUP_SOLENOID_B		6
#define CHAN_LEFT_DRIVE_TALON		1
#define CHAN_RIGHT_DRIVE_TALON		2
#define CHAN_JAG_PICKUP_LEFT		12
#define CHAN_JAG_PICKUP_RIGHT		11
#define CHAN_WINCH_SOLENOID_A		8
#define CHAN_WINCH_SOLENOID_B		7
#define CHAN_WINCH_MOTOR			3
#define CHAN_POT					1
#define CHAN_LIMIT_SWITCH_LEFT		12
#define CHAN_LIMIT_SWITCH_RIGHT		13
#define CHAN_PISTON_SWITCH_LEFT		9
#define CHAN_PISTON_SWITCH_RIGHT	8

#define CHAN_COMP_AUTO_SHUTOFF		5
#define CHAN_RLY_COMPRESSOR         1

#define MAX_RPS						8
#define SHIFT_POINT                 200.0 // was 200.0


// competition settings
#define BOX_PID_SWITCH				4
#define BOX_SHOOT_MODE_SWITCH		6
#define BOX_PASS_BUTTON				1
#define BOX_PICKUP_BUTTON			2
#define BOX_SHOOT_BUTTON			5
#define BOX_ARM_BUTTON				3
#define BOX_PICKUP_STATE_LED		6
#define BOX_SHOOTER_STATE_LED		8
#define BOX_PID_STATE_LED			5
#define BOX_SHOETUR_FALLT_LED		0



#define Y_IMAGE_RES 				240
#define VIEW_ANGLE 					37.4
#define PI 							3.141592653
#define RECTANGULARITY_LIMIT 		40
#define ASPECT_RATIO_LIMIT 			55
#define TAPE_WIDTH_LIMIT 			50
#define VERTICAL_SCORE_LIMIT 		50
#define LR_SCORE_LIMIT 				50
#define AREA_MINIMUM 				150
#define MAX_PARTICLES 				8

#define CATAPULT_UP					380
#define CATAPULT_LOADED				500
#define CATAPULT_DOWN				210 //240 .. 200 is good but hits limit switches

// AUTONOMOUS_MODE  0=no vision, 1=vision
#define AUTONOMOUS_MODE				1
#define VIRTUAL_MODE				0

#define COMPETITION
//#define PARADE

