//--------------------------------------------------------------------
// Indicator Controller - Last Press Wins, Toggle-to-Idle
// - Three push buttons: LEFT, RIGHT, HAZARD (debounced boolean levels)
// - Pressing any button (rising edge) takes exclusive control immediately
// - Pressing that same button again (falling edge) returns to IDLE
// - Other modes are forced OFF; no fallback even if another button stays ON
//--------------------------------------------------------------------

const uint16 TICK_MS          = 5;   // scheduler tick (periodic interrupt)
const uint16 LEFT_TOGGLE_MS   = 10;  // left LED toggles every 10 ms
const uint16 RIGHT_TOGGLE_MS  = 5;   // right LED toggles every 5 ms
const uint16 HAZARD_TOGGLE_MS = 5;   // both LEDs toggle every 5 ms

const uint16 LEFT_TICKS   = LEFT_TOGGLE_MS   / TICK_MS;   // = 2
const uint16 RIGHT_TICKS  = RIGHT_TOGGLE_MS  / TICK_MS;   // = 1
const uint16 HAZARD_TICKS = HAZARD_TOGGLE_MS / TICK_MS;   // = 1

// Inputs (after debouncing)
bool L, R, H;

// Edge tracking
bool L_prev=false, R_prev=false, H_prev=false;

// Suppression flags:
//  - When ownership moves to a new button, suppress the other buttons
//    (even if they remain physically ON). They must be RELEASED (falling edge)
//    before they can be eligible again.
bool L_suppressed=false, R_suppressed=false, H_suppressed=false;

enum Owner { IDLE_O, LEFT_O, RIGHT_O, HAZARD_O };
Owner owner = IDLE_O;

// LED outputs
bool Left_LED=false, Right_LED=false;

// Toggle counters
uint16 left_cnt=0, right_cnt=0, hazard_cnt=0;

//--------------------------------------------------------------------
// Called once at startup
//--------------------------------------------------------------------
init_controller() {
  owner = IDLE_O;
  Left_LED = Right_LED = false;
  left_cnt = right_cnt = hazard_cnt = 0;
  L_prev = R_prev = H_prev = false;
  L_suppressed = R_suppressed = H_suppressed = false;
  // Start a periodic timer interrupt to call on_tick() every 5 ms
}

//--------------------------------------------------------------------
// Sample & debounce hardware buttons here (GPIO / CAN / etc.)
// This function should update L, R, H with stable boolean levels.
//--------------------------------------------------------------------
sample_inputs() {
  L = read_left_button_debounced();
  R = read_right_button_debounced();
  H = read_hazard_button_debounced();
}

//--------------------------------------------------------------------
// Main 5 ms scheduler (ISR-safe or called from main loop)
//--------------------------------------------------------------------
on_tick() {
  sample_inputs();

  // Rising/falling edges
  bool L_rise =  (L && !L_prev);
  bool R_rise =  (R && !R_prev);
  bool H_rise =  (H && !H_prev);
  bool L_fall = (!L &&  L_prev);
  bool R_fall = (!R &&  R_prev);
  bool H_fall = (!H &&  H_prev);

  // Clear suppression when a button is released (makes it eligible again)
  if (L_fall) L_suppressed = false;
  if (R_fall) R_suppressed = false;
  if (H_fall) H_suppressed = false;

  // --------- Arbitration: Last Press Wins ----------
  // Any ON-press takes ownership immediately, if not suppressed
  if (L_rise && !L_suppressed) {
    owner = LEFT_O;
    // Cancel / suppress others until they are released
    R_suppressed = true; H_suppressed = true;
    // Reset blink counters for clean phase
    left_cnt = 0; Right_LED = false; // force other off
  }
  if (R_rise && !R_suppressed) {
    owner = RIGHT_O;
    L_suppressed = true; H_suppressed = true;
    right_cnt = 0; Left_LED = false;
  }
  if (H_rise && !H_suppressed) {
    owner = HAZARD_O;
    L_suppressed = true; R_suppressed = true;
    hazard_cnt = 0;
  }

  // --------- Toggle-to-Idle (press same again) ----------
  if (owner == LEFT_O  && L_fall)  { owner = IDLE_O; Left_LED=false; }
  if (owner == RIGHT_O && R_fall)  { owner = IDLE_O; Right_LED=false; }
  if (owner == HAZARD_O&& H_fall)  { owner = IDLE_O; Left_LED=false; Right_LED=false; }

  // --------- Outputs / blink timing ----------
  switch (owner) {
    case IDLE_O:
      // Ensure both OFF; reset counters to keep phases deterministic next time
      Left_LED = false; Right_LED = false;
      left_cnt = right_cnt = hazard_cnt = 0;
      break;

    case LEFT_O:
      Right_LED = false;                 // exclusivity
      left_cnt++;
      if (left_cnt >= LEFT_TICKS) {
        Left_LED = !Left_LED;            // toggle every 10 ms
        left_cnt = 0;
      }
      break;

    case RIGHT_O:
      Left_LED = false;                  // exclusivity
      right_cnt++;
      if (right_cnt >= RIGHT_TICKS) {
        Right_LED = !Right_LED;          // toggle every 5 ms
        right_cnt = 0;
      }
      break;

    case HAZARD_O:
      hazard_cnt++;
      if (hazard_cnt >= HAZARD_TICKS) {
        bool t = !Left_LED;              // toggled state shared to both
        Left_LED  = t;
        Right_LED = t;
        hazard_cnt = 0;
      }
      break;
  }

  // Remember levels for next tick
  L_prev = L; R_prev = R; H_prev = H;
}
