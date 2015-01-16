#include "WPILib\WPILib.h"
#include "BooleanDebouncer.cpp"
#include "Constants.h"

extern void message(char *fmt, ...);

/*
 * Get rid of states for pickup
 * Make a pickup state function or make it public
 * (make toggle function)
 * do magic
 * profit!
 */

class BallSystem {
	public:
		typedef enum {
			deployed, // roller in deployed state but not rolling
			deploying, // roller moving to deployed state
			locked,    // what?
			passing,   // deployed and rolling 
			firing     // what?
		} BallState;
		
		typedef enum {
			fired,  // cat in fired state
			armed,  // cat in armed state
			arming, // cat in the process of transitioning to armed
			waiting // what?
		} CatapultState;
		
		BallSystem(CANJaguar *left, CANJaguar *right, Solenoid *deployA, Solenoid *deployB, Solenoid *engageA, Talon *winchMotor, AnalogChannel *potChan, DigitalInput *limitLeft, DigitalInput *limitRight, DigitalInput *pistonLeft, DigitalInput *pistonRight) {
			m_pscMotorLeft = left;
			m_pscMotorRight = right;
			m_pscWinchMotor = winchMotor;
			
			m_pValveDeployA = deployA;
			m_pValveDeployB = deployB;
			m_pValveWinchLockA = engageA;

			pot = potChan;
			limitSwitchLeft = limitLeft;
			limitSwitchRight = limitRight;
			leftDebounce = new BooleanDebouncer();
			rightDebounce = new BooleanDebouncer();
			pistonSwitchLeft = pistonLeft;
			pistonSwitchRight = pistonRight;			
			
			pickupState = locked;
			catapultState = armed;
			
			limitSwitchOverride = false;
			pickupOverride = false;
			
			catapultAutoArm = false;
			pickupDeployed = false;
			
			arm = false;
			fire = false;
			
			armTimer = new Timer();
			fireTimer = new Timer();
			armTimerStarted = false;
			fireTimerStarted = false;
		}
		
		~BallSystem() {
			delete m_pscMotorLeft;
			delete m_pscMotorRight;
			delete m_pscWinchMotor;

			delete m_pValveDeployA;
			delete m_pValveDeployB;
			delete m_pValveWinchLockA;

			delete pot;
			delete limitSwitchLeft;
			delete limitSwitchRight;
			delete leftDebounce;
			delete rightDebounce;
			delete pistonSwitchLeft;
			delete pistonSwitchRight;
			
			delete armTimer;
			delete fireTimer;
		}
		
		void SetPickupState(BallState state) {
			pickupState = state;
		}
		
		BallState GetPickupState() {
			return pickupState;
		}
		
		string GetPickupStateS() {
			switch(pickupState) {
			case deployed:
				return "deployed";
				break;
			case deploying:
				return "deploying";
				break;
			case firing:
				return "firing";
				break;
			case locked:
				return "locked";
				break;
			case passing:
				return "passing";
				break;
			}
			return "";
		}
		void SetCatapultState(CatapultState state) {
			catapultState = state;
		}
		
		CatapultState GetCatapultState() {
			return catapultState;
		}
		
		string GetCatapultStateS() {
			switch(catapultState) {
			case armed:
				return "armed";
				break;
			case arming:
				return "arming";
				break;
			case fired:
				return "fired";
				break;
			case waiting:
				return "waiting";
				break;
			}
			return "";
		}
		int GetCatapultPot() {
			return pot->GetValue();
		}
		
		bool GetLimitSwitchLeft() {
			return limitSwitchLeft->Get();
		}
		
		bool GetLimitSwitchRight() {
			return limitSwitchRight->Get();
		}
		
		bool GetPistonSwitchLeft() {
			return !pistonSwitchLeft->Get();
		}
		
		bool GetPistonSwitchRight() {
			return !pistonSwitchRight->Get();
		}
		
		void SetCatapultAutoArm(bool ov) {
			catapultAutoArm = ov;
		}
		void SetPickupOverride(bool ov) {
			pickupOverride = ov;
		}
		bool GetPickupOverride() {
			return pickupOverride;
		}
		void Arm() {
			arm = true;
			pickupOverride = true;
			catapultState = waiting;
		}
		void Fire() {
			fire = true;
			pickupOverride = true;
			catapultState = waiting;
		}
		void SetRollerSpeed(float val) {
			m_pscMotorLeft->Set(-val);
			m_pscMotorRight->Set(val);
		}
		void SetDeployed(bool val) {
			if (val) {
				m_pValveDeployA->Set(false);
				m_pValveDeployB->Set(true);
			} else {
				m_pValveDeployA->Set(true);
				m_pValveDeployB->Set(false);
			}
		}
		void Update() {
			bool leftSwitch = leftDebounce->Debounce(limitSwitchLeft->Get());
			bool rightSwitch = rightDebounce->Debounce(limitSwitchRight->Get());
			if (leftSwitch || rightSwitch) {
				limitSwitchOverride = true;
				SetWinchSpeed(0.0f);
				SetCatapultEngaged(true);  // sets piston to out position
			}

			if (GetPistonSwitchLeft() && GetPistonSwitchLeft()) { // pickup physically in deployed position
				pickupDeployed = true;	
			} else {
				pickupDeployed = false;
			}

			if (pickupDeployed) { // pickup physically in deployed position
				if (pickupOverride) { // cat state machine is controlling pickup - cat is busy
					if(pickupState == deploying) {
						pickupState = firing;
					}
				} else {  // cat NOT controlling - user commanded deploy
					if(pickupState == deploying) {
						pickupState = deployed;
					}
				}
			}

			if (arm) {  // received arm command
				if (pickupDeployed) {  // pickup already in deployed position
					catapultState = arming;
					if (!armTimerStarted) {
						armTimer->Reset();
						armTimer->Start();
						armTimerStarted = true;
					}
				} else {  // pickup NOT YET in deployed state
					pickupState = deploying;  // sets pickup "deploying" state - but doesn't command it?
				}
			} else if (fire) { // received fire command
				if (pickupDeployed) {  // pickup already in deployed position
					catapultState = fired;  // sets cat "fired" state - but doesn't command it?
					if(!fireTimerStarted) {
						fireTimer->Reset();
						fireTimer->Start();
						fireTimerStarted = true;
					}
				} else { // pickup NOT YET in deployed state
					pickupState = deploying; // sets pickup "deploying" state - but doesn't command it?
				}
			}

			if (catapultState == arming) {  // if in "arming state", 
				if (pot->GetValue() < CATAPULT_DOWN) { // test to see if it has reached the arm point
					catapultState = armed;  // if so, set cat state to "armed"
				}
			}

			switch(pickupState) {
				case deployed:
					SetDeployed(true);
					SetRollerSpeed(1.0f);
					break;
				case deploying:
					SetDeployed(true);
					SetRollerSpeed(1.0f);
					break;
				case locked:
					SetDeployed(false);
					SetRollerSpeed(0.0f);
					break;
				case passing:
					SetRollerSpeed(-1.0f);
					break;
				case firing:
					SetDeployed(true);
					SetRollerSpeed(0.0f);
					break;
			}
			
			if (!limitSwitchOverride) {
				switch (catapultState) {
					case fired:
						 if (catapultAutoArm) {
							if(fireTimer->Get() > 1.0f) {
								fireTimer->Stop();
								fireTimerStarted = false;
								Arm();
							}
						} else {
							pickupOverride = false;
						}
						SetCatapultEngaged(false);
						fire = false;
						if(pickupState == firing) {
							pickupState = deployed;
						}
						break;
					case armed:
						SetCatapultEngaged(true);
						//Let it pull it down to point then after mechanics loosen pull past 
						if(pot->GetValue() > CATAPULT_DOWN - 5 && pot->GetValue() < CATAPULT_DOWN + 50) {
							SetWinchSpeed(-0.5);
						} else {
							SetWinchSpeed(0.0f);
						}
						pickupOverride = false;
						arm = false;
						armTimer->Stop();
						armTimerStarted = false;
						if(pickupState == firing) {
							pickupState = deployed;
						}
						break;
					case arming:
						SetCatapultEngaged(true);
						if (armTimer->Get() < 1.0f) {
							SetWinchSpeed(-0.5f);
						} else {
							if(GetCatapultPot() < CATAPULT_DOWN + 100) {
								SetWinchSpeed(-0.75f);
							} else {
								SetWinchSpeed(-1.0f);
							}
						}
						break;
					case waiting:
						break;
				}
			}
		}
	private:
		CANJaguar *m_pscMotorLeft;
		CANJaguar *m_pscMotorRight;
		Talon *m_pscWinchMotor;

		Solenoid *m_pValveDeployA;
		Solenoid *m_pValveDeployB;
		Solenoid *m_pValveWinchLockA;

		AnalogChannel *pot;
		DigitalInput *limitSwitchLeft;
		DigitalInput *limitSwitchRight;
		BooleanDebouncer *leftDebounce;
		BooleanDebouncer *rightDebounce;
		DigitalInput *pistonSwitchLeft;
		DigitalInput *pistonSwitchRight;

		BallState pickupState;
		CatapultState catapultState;

		bool limitSwitchOverride;
		bool pickupOverride;

		bool catapultAutoArm;
		bool pickupDeployed;

		bool arm;
		bool fire;

		Timer *armTimer;
		Timer *fireTimer;
		bool armTimerStarted;
		bool fireTimerStarted;

		void SetCatapultEngaged(bool val) {
			if (val) {
				m_pValveWinchLockA->Set(false);
			}
			else {
				m_pValveWinchLockA->Set(true);
			}
			//message("setCatapult: %i", val);
		}

		void SetWinchSpeed(float speed) {
			m_pscWinchMotor->Set(speed);
		}
};
