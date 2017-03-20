// pin states of the arduino
extern std::mutex m_pins;
extern int x_pinValue[50];

// led states for microbit simulator
extern std::mutex m_leds;
extern int x_leds[25]; 

// shutdown the simulator
extern std::atomic<bool> shutdown;

// stop the arduino code running
extern std::atomic<bool> running;

// run the simulator in fast_mode
extern std::atomic<bool> fast_mode;

// send updates back to server or not
extern std::atomic<bool> send_updates;

// how many microseconds have elapsed
extern uint64_t micros_elapsed;
extern std::mutex m_elapsed;

// suspend the arduino code
extern std::atomic<bool> suspend;
extern std::mutex m_suspend;
extern std::condition_variable cv_suspend;