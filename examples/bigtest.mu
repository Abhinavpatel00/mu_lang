// -----------------------------------------------------------------------------
// GLOBAL CONSTANTS AND TYPE ALIASES
// -----------------------------------------------------------------------------
PI :: 3.14159265359;
TAU :: 2.0 * PI;
MAX_ENTITIES :: 10000;
u32 :: uint;
f32 :: float;
string :: []u8;  // Alias (though built-in)

// Comptime constant evaluation
COMP_CALC :: comp (234 * 15 + MAX_ENTITIES);

// Distinct types for safety
EntityId :: distinct u32;
Seconds :: distinct f32;

// Enum for game states
GameState :: enum {
    Menu,
    Playing,
    Paused,
    GameOver,
};

// Enum with explicit values
LogLevel :: enum {
    Debug = 0,
    Info  = 1,
    Warn  = 2,
    Error = 3,
};

// -----------------------------------------------------------------------------
// STRUCT DEFINITIONS
// -----------------------------------------------------------------------------
Vec3 :: struct {
    x, y, z: f32;
}

Quaternion :: struct {
    x, y, z, w: f32 = 1.0;   // default w = 1
}

Transform :: struct {
    using pos: Vec3;          // inject x, y, z
    using rot: Quaternion;    // inject w, plus x,y,z conflict? Resolved by explicit override below
    scale: Vec3 = .{1.0, 1.0, 1.0};
    // The conflict: both Vec3 and Quaternion have x,y,z. Using 'except' on one.
    using,except(x, y, z) rot: Quaternion;  // Now only w is injected from Quaternion
}

Entity :: struct {
    id: EntityId;
    name: string;
    using transform: Transform;
    health: i32 = 100;
}

// Generic struct via comptime function
Array2D :: comp (T: type, rows: usize, cols: usize) -> type {
    return struct {
        data: [rows][cols]T;
        
        get :: (self: *@This, r: usize, c: usize) -> T {
            return self.data[r][c];
        }
        
        set :: (self: *mut @This, r: usize, c: usize, value: T) {
            self.data[r][c] = value;
        }
    };
}

// -----------------------------------------------------------------------------
// COMPTIME FUNCTIONS
// -----------------------------------------------------------------------------
print_fields :: comp (T: type, value: T) -> void {
    for field in T.fields {
        print(field.name, ": ", value.(field.name), "\n");
    }
}

make_vector :: comp (x: f32, y: f32, z: f32) -> Vec3 {
    return .{x, y, z};
}

// -----------------------------------------------------------------------------
// RUNTIME FUNCTIONS
// -----------------------------------------------------------------------------
normalize :: (v: *mut Vec3) {
    len := sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
    if len > 0.0 {
        v.x /= len;
        v.y /= len;
        v.z /= len;
    }
}

add :: (a, b: Vec3) -> Vec3 {
    return .{ a.x + b.x, a.y + b.y, a.z + b.z };
}

add :: (a, b: i32) -> i32 {
    return a + b;
}

// Generic function with $T
scale :: (v: *mut $T, factor: f32) -> void where T: struct { x, y, z: f32 } {
    v.x *= factor;
    v.y *= factor;
    v.z *= factor;
}

// UFCS example: trim, to_upper, print
trim :: (s: string) -> string {
    // Dummy implementation
    return s;
}

to_upper :: (s: string) -> string {
    // Dummy implementation
    return s;
}

// Function that never returns
panic :: (msg: string) -> never {
    print("PANIC: %\n", msg);
    exit(1);
}

// Function returning optional
find_entity :: (name: string) -> maybe EntityId {
    // search...
    if name == "player" {
        return EntityId(42);
    }
    return null;
}

// -----------------------------------------------------------------------------
// MAIN
// -----------------------------------------------------------------------------
main :: () {
    // Variable declarations
    count := 22;                 // inferred i64, mutable
    pi :: 3.14159;               // immutable
    health: i32 = 100;           // explicit type
    name: string = "Player";     // string literal
    
    // Array and slice
    fixed_arr: [5]i32 = .{1, 2, 3, 4, 5};
    slice: []i32 = fixed_arr[1..4];
    dynamic_arr: [..]i32;        // heap-allocated, growable
    append(*dynamic_arr, 42);
    
    // Struct literals
    v1 := Vec3.{1.0, 2.0, 3.0};
    v2 := .{ x = 4.0, y = 5.0, z = 6.0 };   // inferred Vec3
    v3 := .{ x = 1.0 };                      // y=0, z=0
    
    // Mutate via pointer
    pv := *mut v1;
    pv.x = 10.0;
    normalize(pv);
    
    // UFCS chain
    "  Hello, World!  ".trim().to_upper().print();
    
    // Match expression
    state: GameState = .Playing;
    match state {
        .Menu    => print("In menu\n"),
        .Playing => {
            print("Game running\n");
            // nested logic
        },
        .Paused  => print("Paused\n"),
        .GameOver(score) => print("Game over! Score: %\n", score), // with payload
        _ => {},
    }
    
    // Match on struct
    e: Entity;
    match e {
        {health = h, name = n} if h <= 0 => print("% is dead\n", n),
        {transform = t} => {
            t.x += 1.0;
        },
        _ => {},
    }
    
    // Match on tuple
    match (v1.x, v1.y) {
        (0.0, 0.0) => print("At origin\n"),
        (x, y) if x > y => print("x > y\n"),
        _ => {},
    }
    
    // For loops
    for i: 0..=10 step 2 {
        print("% ", i);
    }
    print("\n");
    
    for entity in world.entities {
        // process
    }
    
    // Defer
    defer print("Cleanup done\n");
    print("Doing work...\n");
    
    // Optional handling
    id := find_entity("enemy");
    if id {
        print("Found enemy id: %\n", id.?);
    } else {
        print("Enemy not found\n");
    }
    
    // Using comptime-generated type
    Mat2x2f := Array2D(f32, 2, 2);
    mat: Mat2x2f;
    mat.set(0, 0, 1.0);
    mat.set(0, 1, 0.0);
    
    // Casts
    x: i32 = 42;
    y: f32 = x as f32;      // safe
    ptr: *i32 = &x;
    mut_ptr: *mut i32 = ptr as! *mut i32;  // explicit unsafe
    
    // Distinct types cannot mix
    eid: EntityId = 1 as EntityId;
    // u32_val: u32 = eid;  // ERROR!
    
    // Here-string
    shader_code :: #string GLSL
#version 450
layout(location = 0) in vec3 position;
void main() {
    gl_Position = vec4(position, 1.0);
}
GLSL
    print("Shader:\n%\n", shader_code);
    
    // Comptime type reflection
    print_fields(Vec3, v1);
    
    // Nested block comment test
    /* 
        Outer comment
        /* Inner nested comment */
        Back to outer
    */
}

// -----------------------------------------------------------------------------
// TRAIT DEFINITION (low priority, but test lexer)
// -----------------------------------------------------------------------------
Printable :: trait {
    print :: (self: *Self) -> void;
}

impl Printable for Vec3 {
    print :: (self: *Vec3) -> void {
        print("Vec3(%, %, %)\n", self.x, self.y, self.z);
    }
}

// -----------------------------------------------------------------------------
// GENERIC STRUCT WITH TRAIT BOUNDS (test $ and where)
// -----------------------------------------------------------------------------
Container :: struct ($T: type) where T: Printable {
    value: T;
}

// -----------------------------------------------------------------------------
// LINEAR TYPE EXAMPLE (commented if not implemented)
// -----------------------------------------------------------------------------
// MutexGuard :: linear struct {
//     mutex: *mut Mutex;
// }
