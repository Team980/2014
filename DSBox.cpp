#include "BallSystem.cpp"
#include "Constants.h"

class DSBox {
	public:
		DSBox(DriverStation *io, BallSystem *bs) {
			dsIO = io;
			ballSystem = bs;
			pickupStateLED = false;
			shooterStateLED = false;
			pidStateLED = false;
			shoeturFalltLED = false;
		}
		~DSBox() {
			delete dsIO;
		}
		void Update() {
			SetPIDStateLEDOn(GetPIDEnabled());
			SetShooterStateLEDOn(GetShooterMode());
			SetPickupStateLEDOn(ballSystem->GetPickupState() == ballSystem->locked);
		}
		bool GetPIDEnabled() {
			return !dsIO->GetDigitalIn(BOX_PID_SWITCH);
		}
		bool GetShooterMode() {
			return !dsIO->GetDigitalIn(BOX_SHOOT_MODE_SWITCH);
		}
		bool GetArmButton() {
			return !dsIO->GetDigitalIn(BOX_ARM_BUTTON);
		}
		bool GetPassButton() {
			return !dsIO->GetDigitalIn(BOX_PASS_BUTTON);
		}
		bool GetPickupButton() {
			return !dsIO->GetDigitalIn(BOX_PICKUP_BUTTON);
		}
		bool GetShootButton() {
			return !dsIO->GetDigitalIn(BOX_SHOOT_BUTTON);
		}
		void SetShoeturFalltLEDOn(bool val) {
			shoeturFalltLED = val;
			dsIO->SetDigitalOut(BOX_SHOETUR_FALLT_LED, val);
		}
	private:
		DriverStation *dsIO;
		BallSystem *ballSystem;
		bool pickupStateLED;
		bool shooterStateLED;
		bool pidStateLED;
		bool shoeturFalltLED;
		
		void SetPickupStateLEDOn(bool val) {
			pickupStateLED = val;
			dsIO->SetDigitalOut(BOX_PICKUP_STATE_LED, val);
		}
		void SetShooterStateLEDOn(bool val) {
			shooterStateLED = val;
			dsIO->SetDigitalOut(BOX_SHOOTER_STATE_LED, val);
		}
		void SetPIDStateLEDOn(bool val) {
			pidStateLED = val;
			dsIO->SetDigitalOut(BOX_PID_STATE_LED, val);
		}
};
