// -----------------------------------------------------------------------------
// STRUCT DEFINITION
// -----------------------------------------------------------------------------
Vec3 :: struct {
    x, y, z: f32;
}

// Struct with default field value
Player :: struct {
    name:   string = "Unnamed";
    health: i32    = 100;
    pos:    Vec3   = .{0.0, 0.0, 0.0};
}

// Struct with a method
Rect :: struct {
    x, y, w, h: f32;
// method is not stored in struct memory it should not increase size of struct 
    // Method taking pointer to self
    area :: (self: *Rect) -> f32 {
        return self.w * self.h;
    }

    // Method that mutates
    translate :: (self: *mut Rect, dx, dy: f32) {
        self.x += dx;
        self.y += dy;
    }
}

// -----------------------------------------------------------------------------
// USAGE
// -----------------------------------------------------------------------------
main :: () {
    // 1. Ordered initialization (all fields, in declaration order)
    v1 := Vec3.{1.0, 2.0, 3.0};

    // 2. Named initialization (any order, partial allowed)
    v2 := Vec3.{ y = 5.0, x = 4.0 };          // z defaults to 0.0
    v3 := .{ x = 1.0 };                       // inferred as Vec3 if context expects it

    v4 := .{ y = 5.0, x = 4.0 };          // z defaults to 0.0
    // 3. Using default values
    p1 := Player.{};                           // all defaults
    p2 := Player.{ name = "Mario", health = 3 };

    // 4. Calling methods
    r := Rect.{ x = 10, y = 20, w = 100, h = 50 };
    print("Area: %\n", r.area());              // 5000
    r.translate(5, 5);                         // mutates r

    // 5. Pointer to struct
    r_ptr := *mut r;
    r_ptr.w = 200;
    print("New area: %\n", r_ptr.area());      // 10000
}
