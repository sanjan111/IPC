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
// alpha ≈ TICK_MS / TAU  → 5/250 = 0.02
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
    // Currently OFF → only turn ON if we cross the lower threshold
    if (FuelLevel_percent <= LOW_FUEL_ON_PERCENT) {
      LowFuel_latched = true;     // enter low-fuel state
    }
  } else {
    // Currently ON → only turn OFF after we recover past the higher threshold
    if (FuelLevel_percent >= LOW_FUEL_OFF_PERCENT) {
      LowFuel_latched = false;    // exit low-fuel state
    }
  }
  LowFuel_LED = LowFuel_latched;
}
