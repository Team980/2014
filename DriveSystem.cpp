#include "WPILib.h"
#include "Math.h"
#include "Constants.h"

extern void message(char *fmt, ...);

class DriveSystem {
	public:
		DriveSystem(Encoder *left, Encoder *right, Gyro *g, Solenoid *shiftA, Solenoid *shiftB, SpeedController *ml, SpeedController *mr) {
			m_pGyro = g;
			DriveSystem(left, right, shiftA, shiftB, ml, mr);
		}

		DriveSystem(Encoder *left, Encoder *right, Solenoid *shiftA, Solenoid *shiftB, SpeedController *ml, SpeedController *mr) {
			m_pLeftEncoder = left;
			m_pRightEncoder = right;
			m_pscMotorLeft = ml;
			m_pscMotorRight = mr;
			m_pValveShiftA = shiftA;
			m_pValveShiftB = shiftB;
			distanceMultiplier = 0;
			shiftSuppressMode = 0;
			pidDrive = true;  // Why this commented out - check

			SetHighGear(false);
	
			m_pLeftEncoder->SetPIDSourceParameter(m_pLeftEncoder->kRate);
			m_pRightEncoder->SetPIDSourceParameter(m_pRightEncoder->kRate);
			double distPerPulse  = 1 / 1024.0f;
			m_pLeftEncoder->SetDistancePerPulse(distPerPulse);
			m_pRightEncoder->SetDistancePerPulse(distPerPulse);
			m_pLeftEncoder->SetSamplesToAverage(100);
			m_pRightEncoder->SetSamplesToAverage(100);
			controlLeft = new PIDController(0.1f, 0.05f, 0.0f, m_pLeftEncoder, m_pscMotorLeft);
			controlRight = new PIDController(0.1f, 0.05f, 0.0f, m_pRightEncoder, m_pscMotorRight);
			SetDriveDirectionForward(true);
		}
		~DriveSystem() {
			delete m_pLeftEncoder;
			delete m_pRightEncoder;
			delete m_pGyro;
			delete m_pscMotorLeft;
			delete m_pscMotorRight;
			delete m_pValveShiftA;
			delete m_pValveShiftB;
			delete controlLeft;
			delete controlRight;
		}

		void SetDriveInstruction(float velocity, float angle) {
			this->velocity = velocity * driveDirection;
			this->angle = angle / 2.0f;
		}
		
		float GetRobotSpeedInRPM() {
			return ((-m_pLeftEncoder->GetRate() + m_pRightEncoder->GetRate()) / 2) * 60;
		}

		void SetWheelDiameter(float diameter) {
			float distPerRev = diameter * PI;
			distanceMultiplier = distPerRev / 1024;
		}
	
		float GetDistanceTraveled() {
			return abs(m_pLeftEncoder->Get() - startingTick) * distanceMultiplier;
		}
		
		void StartRecordingDistance() {
			startingTick = m_pLeftEncoder->Get();
		}

		void SetShiftOverride (bool shift) {
			if (shift == shiftOverride)
				return;
			shiftOverride = shift;
		}
		
		
		void SetPIDDrive(bool pid) {
/*			
			if(pid == pidDrive)
			{
				message("not using PID");
				return;
			}
			message("using PID"); */
			
			if (pid) // override PID 
				message("using PID");
			else
				message("not using PID");
			// When in PID mode, send speeds for instructions, else send motor power values
			pidDrive = pid;
			if (pid && !controlLeft->IsEnabled()) {
				controlLeft->Enable();
				controlRight->Enable();
			} else if (controlLeft->IsEnabled()) {
				controlLeft->Disable();
				controlRight->Disable();
			}
			m_pLeftEncoder->Start();
			m_pRightEncoder->Start();
		}

		void SetDriveDirectionForward(bool val) {
			if (val) {
				driveDirection = 1;
			} else {
				driveDirection = -1;
			}
		}
		
		bool GetDriveDirectionForward() {
			return (driveDirection == 1);
		}
		
		void SetHighGear(bool val) {
			if (val) {
				m_pValveShiftA->Set(false);
				m_pValveShiftB->Set(true);
				robotGearState = high;
			} else {
				m_pValveShiftA->Set(true);
				m_pValveShiftB->Set(false);
				robotGearState = low;
			}
		}
		
		void SetShiftSuppressionMode(int mode) { // change to enum pls senpai
			shiftSuppressMode = mode;
		}

		void Update() {
			if (fabs(angle) < 1) {
				if (!shiftOverride)
				{
					if (robotGearState == high) {
						if (fabs(GetRobotSpeedInRPM()) < (SHIFT_POINT - 50.0)) {
							SetHighGear(false);
						}
					} else {
						if (fabs(GetRobotSpeedInRPM()) > SHIFT_POINT) {
							SetHighGear(true);
						}
					}
				} else {
					if (shiftSuppressMode == 0) { // yolo 0 means low gear
						SetHighGear(false);
					} else {
						SetHighGear(true);
					}
				}
			}
	//		message("Left Speed: %f, Right Speed: %f, LE: %f, RE: %f, Speed: %f", GetLeftDriveMotorSpeed(), GetRightDriveMotorSpeed(), m_pLeftEncoder->GetRate(), m_pRightEncoder->GetRate(), GetRobotSpeedInRPM());
	//		message("Is PID Enabled??? %s", (controlLeft->IsEnabled()) ? "Yes!" : "No :(");
			SetMotorSpeedLeft(GetLeftDriveMotorSpeed());
			SetMotorSpeedRight(GetRightDriveMotorSpeed());
		}
		
		float GetRightEncoder() {
			return m_pRightEncoder->GetRate();
		}
		
		float GetLeftEncoder() {
			return m_pLeftEncoder->GetRate();
		}

	private:
		Encoder *m_pLeftEncoder;
		Encoder *m_pRightEncoder;
		Gyro *m_pGyro;
		SpeedController *m_pscMotorLeft;
		SpeedController *m_pscMotorRight;
		Solenoid *m_pValveShiftA;
		Solenoid *m_pValveShiftB;
		PIDController *controlLeft;
		PIDController *controlRight;
		float distanceMultiplier;
		float velocity;
		float angle;
		int startingTick;
		bool pidDrive;
		bool shiftOverride;
		typedef enum {low, high} GearState;
		GearState robotGearState;
		int driveDirection;
		bool shiftSuppress;
		int shiftSuppressMode;
		
		float GetLeftDriveMotorSpeed() {
			float speed = velocity;
			if (angle < -.05f || angle > .05f) {
				speed += angle * .7f;
			}
			return speed;
		}

		float GetRightDriveMotorSpeed() {
			float speed = velocity;
			if (angle < -.05f || angle > .05f) {
				speed -= angle * .7f;
			}
			return speed;
		}

		void SetMotorSpeedLeft(float speed) {
			if (pidDrive) {
				if (fabs(speed) < 0.5) {
					speed = 0;
				}
				controlLeft->SetSetpoint(-speed);
			} else {
				m_pscMotorLeft->Set(-speed);
			}
		}

		void SetMotorSpeedRight(float speed) {
			if (pidDrive) {
				if (fabs(speed) < 0.5) {
					speed = 0;
				}
				controlRight->SetSetpoint(speed);
			} else {
				m_pscMotorRight->Set(speed);
			}
		}
};
