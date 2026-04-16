
gravity :: 9.81;          // immutable inferred (f32)
count := 42;              // mutable inferred (i64)
temperature: f32 = 36.6;  // explicit type
integer_test: i32 = 36;  // explicit type

/*==============================
=           STRUCTS            =
==============================*/

Vec3 :: struct {
    x, y, z: f32;   // grouped fields
};

Player :: struct {
    id: i32;
    pos: Vec3;
    health: f32 = 100.0;   // default value
};

/*==============================
=        FUNCTIONS             =
==============================*/

length :: (v: Vec3) -> f32 {
    return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

move :: (p: *mut Player, delta: Vec3) -> void {
    p.pos.x += delta.x;
    p.pos.y += delta.y;
    p.pos.z += delta.z;
}
add_vec :: (a, b: Vec3) -> Vec3 {
    return .{ a.x + b.x, a.y + b.y, a.z + b.z };
}

move_2 :: (p: *mut Player, delta: Vec3) -> void {
    p.pos =add_vec(p.pos,delta);
}
/*==============================
=            MAIN              =
==============================*/

main :: () {
    v := .{ x = 1.0, y = 2.0, z = 3.0 };
    v2 := .{ 4.0, 5.0, 6.0 };
    p := .{
        id = 1,
        pos = v
    };
    p_ptr := *mut p;
    p_ptr.pos.x = 10.0;
    fixed: [3] i32 = .{1, 2, 3};
    dynamic: [..] i32;
    print_array :: (arr: [] i32) {
        for i : 0..arr.count-1 {
            print(arr[i]);
        }
    }
    print_array(fixed);
    len := length(v);
    print(len);
}
