//--------------------------------------------------------------------
// codex_pseudocode.c
//
// Combined file based on:
//  - pseudocode_Indicators.c (verbatim)
//  - pseudocode_warning.c   (verbatim)
// with an appended EEPROM-backed persistence module for LPC1768 projects.
//
// IMPORTANT: The original indicator and warning logic are pasted AS-IS.
// No changes were made to their behavior. The EEPROM section is appended
// and can be integrated without altering the above logic.
//--------------------------------------------------------------------

// ========================= BEGIN: pseudocode_Indicators.c =========================
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
// ========================== END: pseudocode_Indicators.c ==========================


// =========================== BEGIN: pseudocode_warning.c ==========================
//--------------------------------------------------------------------
// WARNING LIGHT CONTROLLER (Seatbelt, Service, Low Fuel)
// Assumptions:
//  - Runs from the same 5 ms scheduler as indicators.
//  - Inputs are debounced in read_*() helpers.
//  - Seatbelt: ON when unbuckled, OFF when buckled (no speed/occupancy gating).
//  - Service: ON when service due flag is set, else OFF.
//  - Low Fuel: ON when fuel level is low, with hysteresis to avoid flicker.
//--------------------------------------------------------------------

const uint16 TICK_MS = 5;

// ---- Low-fuel thresholds (choose one unit & be consistent) ----
// If you measure in %: use 15% ON, 18% OFF (hysteresis)
const float LOW_FUEL_ON_PERCENT  = 15.0;   // turns ON at or below this
const float LOW_FUEL_OFF_PERCENT = 18.0;   // turns OFF at or above this

// Optional smoothing (to reject sensor jitter on bumps)
// Use a lightweight low-pass filter ~250 ms time constant
// alpha �%^ TICK_MS / TAU  �+' 5/250 = 0.02
const float FUEL_FILTER_ALPHA = 0.02;

// ---- Inputs (after debouncing/ADC scaling) ----
bool  Seatbelt_Buckled;          // TRUE = buckled, FALSE = unbuckled
bool  Service_Due;               // TRUE = due, FALSE = ok
float FuelLevel_percent;         // 0..100 % (scaled)

// ---- Internal filtered state for fuel + hysteresis latch ----
float FuelLevel_filt = 100.0;    // initialize to full to avoid spurious ON at boot
bool  LowFuel_latched = false;   // hysteresis state (what drives the lamp)

// ---- Outputs ----
bool Seatbelt_LED = false;
bool Service_LED  = false;
bool LowFuel_LED  = false;

//--------------------------------------------------------------------
// Hardware abstraction (implement with GPIO/ADC/CAN as needed)
//--------------------------------------------------------------------
bool  read_seatbelt_buckled_debounced();
bool  read_service_due_debounced();
float read_fuel_percent_filtered_hw();  // if you already filter in HW, you can use that
float read_fuel_percent_raw_hw();       // else use raw and apply SW filter below

//--------------------------------------------------------------------
// Called once at startup
//--------------------------------------------------------------------
warning_init() {
  Seatbelt_LED = false;
  Service_LED  = false;
  LowFuel_LED  = false;

  LowFuel_latched = false;
  FuelLevel_filt  = 100.0;
}

//--------------------------------------------------------------------
// Called every 5 ms (from the same scheduler as indicators)
//--------------------------------------------------------------------
warning_on_5ms_tick() {

  // ---- 1) Read inputs (debounced) ----
  Seatbelt_Buckled = read_seatbelt_buckled_debounced();
  Service_Due      = read_service_due_debounced();

  // Fuel: choose ONE of the following:
  // (A) If you already have a reliable filtered percent from HW:
  // FuelLevel_percent = read_fuel_percent_filtered_hw();

  // (B) Or do a tiny SW low-pass filter here on raw reading:
  {
    float fuel_raw = read_fuel_percent_raw_hw();           // 0..100
    FuelLevel_filt = FuelLevel_filt
                   + FUEL_FILTER_ALPHA * (fuel_raw - FuelLevel_filt);
    FuelLevel_percent = FuelLevel_filt;
  }

  // ---- 2) Seatbelt LED ----
  // Simple rule: LED ON when UNBUCKLED; OFF when BUCKLED
  Seatbelt_LED = (Seatbelt_Buckled == false);

  // ---- 3) Service LED ----
  // Simple rule: LED ON when service is due; else OFF
  Service_LED = Service_Due;

  // ---- 4) Low Fuel LED with hysteresis ----
  // Hysteresis avoids chatter near the threshold due to bumps/noise.
  if (LowFuel_latched == false) {
    // Currently OFF �+' only turn ON if we cross the lower threshold
    if (FuelLevel_percent <= LOW_FUEL_ON_PERCENT) {
      LowFuel_latched = true;     // enter low-fuel state
    }
  } else {
    // Currently ON �+' only turn OFF after we recover past the higher threshold
    if (FuelLevel_percent >= LOW_FUEL_OFF_PERCENT) {
      LowFuel_latched = false;    // exit low-fuel state
    }
  }
  LowFuel_LED = LowFuel_latched;
}
// ============================ END: pseudocode_warning.c ===========================


// ======================= APPENDED: EEPROM Persistence (LPC1768) ==================
// Note: This section adds persistent storage for service-related data
// without modifying the original indicator/warning logic above.
//
// LPC1768 has no internal EEPROM. Common options:
//  - External I2C EEPROM (e.g., 24LCxx) on I2C0/I2C1
//  - Flash-based EEPROM emulation using IAP (requires wear leveling)
// Here we outline an external I2C EEPROM approach in pseudocode.

// ---- Persisted service data ----
struct PersistData {
  uint16 version;          // structure version
  uint16 crc;              // CRC over fields after 'crc'

  uint32 power_on_count;   // # of power cycles
  uint32 runtime_minutes;  // accumulated runtime in minutes
  uint32 service_count;    // # of completed service routines
  uint32 last_service_ts;  // timestamp of last service (RTC epoch or counter)
};

const uint16 PERSIST_VER = 1;
const uint16 EE_BASE_ADDR = 0x0000; // where the blob is stored in EEPROM

PersistData gPersist;
bool gPersistLoaded = false;
bool gPersistDirty  = false;

// ---- Timebase for persistence helpers ----
// Provide a millisecond tick (e.g., SysTick 1 ms) and RTC seconds.
uint32 millis();     // returns system ms (monotonic)
uint32 rtc_seconds(); // returns epoch or uptime seconds

// ---- EEPROM low-level helpers (replace with your HAL) ----
// i2c_write(addr7, bytes[], len) and i2c_write_read(addr7, wbuf[], wlen, rbuf[], rlen)
const uint8 I2C_ADDR_EE = 0x50; // 7-bit
const uint16 EE_PAGE = 32;      // device page size

bool ee_read(uint16 ee_addr, uint8* dst, uint16 len) {
  // write address high, low then read len bytes
  uint8 w[2] = { (uint8)(ee_addr>>8), (uint8)(ee_addr & 0xFF) };
  return i2c_write_read(I2C_ADDR_EE, w, 2, dst, len);
}

bool ee_write(uint16 ee_addr, const uint8* src, uint16 len) {
  // split into page-sized chunks; poll for write complete
  while (len>0) {
    uint16 page_off = ee_addr % EE_PAGE;
    uint16 chunk = EE_PAGE - page_off;
    if (chunk > len) chunk = len;
    uint8 buf[2 + EE_PAGE];
    buf[0] = (uint8)(ee_addr>>8); buf[1] = (uint8)(ee_addr & 0xFF);
    for (uint16 i=0;i<chunk;i++) buf[2+i]=src[i];
    if (!i2c_write(I2C_ADDR_EE, buf, 2+chunk)) return false;
    delay_ms(6); // tWR; in production use ACK polling
    ee_addr += chunk; src += chunk; len -= chunk;
  }
  return true;
}

// ---- CRC16-CCITT for data integrity ----
uint16 crc16_ccitt(uint8* data, uint32 len) {
  uint16 crc=0xFFFF; for (uint32 i=0;i<len;i++){ crc^=((uint16)data[i])<<8; for(int b=0;b<8;b++){ crc=(crc&0x8000)?((crc<<1)^0x1021):(crc<<1);} } return crc;
}

// ---- Defaults, load, save ----
defaults_persist(PersistData* p){
  p->version = PERSIST_VER; p->crc=0;
  p->power_on_count=0; p->runtime_minutes=0; p->service_count=0; p->last_service_ts=0;
}

bool persist_load(){
  PersistData tmp;
  if (!ee_read(EE_BASE_ADDR, (uint8*)&tmp, sizeof(tmp))) { defaults_persist(&gPersist); gPersistLoaded=false; return false; }
  if (tmp.version != PERSIST_VER) { defaults_persist(&gPersist); gPersistLoaded=false; return false; }
  uint16 calc = crc16_ccitt(((uint8*)&tmp)+offsetof(PersistData,crc)+2, sizeof(tmp)- (offsetof(PersistData,crc)+2));
  if (calc != tmp.crc) { defaults_persist(&gPersist); gPersistLoaded=false; return false; }
  gPersist = tmp; gPersistLoaded=true; return true;
}

bool persist_save_now(){
  gPersist.version = PERSIST_VER;
  gPersist.crc = crc16_ccitt(((uint8*)&gPersist)+offsetof(PersistData,crc)+2, sizeof(gPersist)- (offsetof(PersistData,crc)+2));
  if (!ee_write(EE_BASE_ADDR, (uint8*)&gPersist, sizeof(gPersist))) return false;
  gPersistDirty=false; return true;
}

// ---- Wear-aware periodic save ----
const uint32 PERSIST_MIN_SAVE_MS = 10000; // >=10s between writes
uint32 gLastPersistSaveMs = 0;

persist_save_maybe(){
  if (!gPersistDirty) return;
  if ((millis() - gLastPersistSaveMs) < PERSIST_MIN_SAVE_MS) return;
  if (persist_save_now()) gLastPersistSaveMs = millis();
}

// ---- Service data API (use these from your app) ----
persist_on_power_on(){ gPersist.power_on_count++; gPersistDirty=true; persist_save_now(); }
persist_on_minute(){   gPersist.runtime_minutes++; gPersistDirty=true; }
persist_service_completed(){ gPersist.service_count++; gPersist.last_service_ts = rtc_seconds(); gPersistDirty=true; }

// ---- Initialization snippet (call from system init) ----
persist_init(){
  // init I2C controller here
  i2c_init();
  if (!persist_load()) { defaults_persist(&gPersist); persist_save_now(); }
  persist_on_power_on();
}

// ---- Integration notes ----
// - Call persist_init() once at boot (after I2C init).
// - Ensure a 1 ms tick exists for millis() and schedule persist_save_maybe() periodically.
// - Aggregate 60,000 ticks to call persist_on_minute() every minute.
// - When a real service is performed, call persist_service_completed().
// - This module does not change indicator or warning logic; it only stores service data.

