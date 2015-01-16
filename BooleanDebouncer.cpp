class BooleanDebouncer {
	public:
		BooleanDebouncer() {
			cycleNum = 0;
			cycleInterval = 100;
			pressed = false;
		}
		bool Debounce(bool value) {			
			if (cycleNum < cycleInterval) {
				presses[cycleNum] = !value;
				cycleNum++;
			} else {
				cycleNum = 0;
				int numTrue = 0;
				for (int i = 0; i < cycleInterval; i++) {
					if (presses[i]) {
						numTrue++;
					}
				}
				if (numTrue > 50) {
					pressed = true;
				} else {
					pressed = false;
				}
			}
			return pressed;
		}
		
	private:
		int cycleNum;
		int cycleInterval;
		bool presses [100];
		bool pressed;
};
