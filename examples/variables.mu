

PI :: 225;
MAX_COUNT :: 22O.44;


main::(){

count : = 33;
age : i32 = 22;
name :: "player";



}
/*
// -----------------------------------------------------------------------------
// VARIABLES & CONSTANTS
// -----------------------------------------------------------------------------

// Immutable constants (compile‑time known)
PI :: 3.14159265359;
MAX_HEALTH :: 100;
APP_NAME :: "Mu Game Engine";

// Mutable variables (type inferred)
score := 0;
player_name := "Link";
is_running := true;
gravity := 9.81;

// Explicit type annotations
frame_count: u64 = 0;
delta_time: f32 = 0.016;
initial: char = 'A';

// Zero‑initialized (all types default to zero)
uninitialized_int: i32;           // 0
uninitialized_float: f32;         // 0.0
uninitialized_bool: bool;         // false
uninitialized_string: string;     // "" (empty slice)

// -----------------------------------------------------------------------------
// ARRAYS & SLICES
// -----------------------------------------------------------------------------

// Fixed‑size array
position_history: [60]Vec3;       // 60 zero‑initialized Vec3s

matrix: [4][4]f32 = .{
    .{1, 0, 0, 0},
    .{0, 1, 0, 0},
    .{0, 0, 1, 0},
    .{0, 0, 0, 1},
};

// Slice (view into existing array)
recent_positions: []Vec3 = position_history[0..10];

// Dynamic array (heap‑allocated, growable)
dynamic_ids: [..]EntityId;
append(*dynamic_ids, EntityId(1));
append(*dynamic_ids, EntityId(2));

// -----------------------------------------------------------------------------
// POINTERS
// -----------------------------------------------------------------------------

value := 42;
ptr_to_value := *value;           // immutable pointer to immutable data
mut_ptr := *mut value;            // mutable pointer to mutable data

// Dereferencing
mut_ptr.* = 100;
print("%\n", value);              // prints 100

// Pointer to array element
first_element := *mut matrix[0][0];
first_element.* = 2.0;

// -----------------------------------------------------------------------------
// DISTINCT TYPES
// -----------------------------------------------------------------------------

EntityId :: distinct u32;
Seconds :: distinct f32;
Meters :: distinct f32;

entity_id: EntityId = EntityId(1000);
elapsed: Seconds = Seconds(2.5);
distance: Meters = Meters(10.0);

// Cannot mix distinct types without cast
// distance = elapsed;             // ERROR: Meters != Seconds
distance = Meters(elapsed as f32 * 2.0);   // Explicit conversion

// -----------------------------------------------------------------------------
// OPTIONAL TYPES
// -----------------------------------------------------------------------------

maybe_int: maybe i32 = 42;
no_value: maybe i32 = null;

// Alternative syntax with `?`
optional_float: f32? = 3.14;
empty_float: f32? = null;

// Checking and unwrapping
if maybe_int {
    print("Value: %\n", maybe_int.?);
}

// -----------------------------------------------------------------------------
// CASTS
// -----------------------------------------------------------------------------

x: i32 = 42;
y: f32 = x as f32;                // Safe numeric widening
z: u8 = x as u8;                  // Safe narrowing (runtime bounds check)

// Pointer casts
imm_ptr: *i32 = mut_ptr as *i32;  // mutable → immutable (safe)
unsafe_ptr: *mut i32 = imm_ptr as! *mut i32;  // immutable → mutable (unsafe)

// -----------------------------------------------------------------------------
// CONSTANTS WITH COMPTIME EVALUATION
// -----------------------------------------------------------------------------

TAU :: 2.0 * PI;
DEFAULT_CAPACITY :: comp(1024 * 8);
HALF_HEALTH :: MAX_HEALTH / 2;

// -----------------------------------------------------------------------------
// ENUM VALUES (can be assigned later)
// -----------------------------------------------------------------------------

PipelineState :: enum {
    Uninitialized,
    Created,
    Bound,
};

current_state := PipelineState.Uninitialized;
current_state = .Created;          // shorthand after initial use


*/
