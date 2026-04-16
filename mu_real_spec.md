
language for high performance games
i want full comptime execution similar to zig and jai

i want to make a data oriented language , a language that expands the way to write easy to read code 

i want lower level language 
we want awesome errors like rust 


nested comments are allowed with this /* */
/ global scope:
MASS_EARTH0 : f32 : 5.97219e24;  // (1) in kg
MASS_EARTH :: 5.97219e24;          // (2)
COMP_CALC :: comp (234 * 15);      // (2B)

main :: () {
    MASS_MARS :: MASS_EARTH * 0.15;  // (3)
    print("The earth mass is %\n", MASS_EARTH);
    // => The earth mass is 5972189887017193070000000


Vec3 :: struct {
    x: f32;
    y: f32;
    z: f32;
};
also this
Vec3 :: struct {
    x, y, z: f32;
}
Quaternion :: struct {
    x, y, z, w: float;

    w = 1;
}


normalize :: (v: *mut Vec3) {
    len := sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
    if len > 0.0 {
        v.x /= len;
        v.y /= len;
        v.z /= len;
    }
}


struct can also have default  values initialised



⚙️ Compile-time function
print_struct :: comp (T: type, value: T) ->void {
    for field in T.fields {
        print(field.name, ": ", value.(field.name), "\n");
    }
};
🧪 Usage
main :: () {

    v := .{ x = 1.0, y = 2.0, z = 3.0 };
    this is optional we have this below option for cleaner approach but we have to put values in same order as type was declared but in above u can also do this    v := .{ x = 1.0, z =3.0, y = 2.0, };
//v := .{ 1.0, 2.0, 3.0 };
v1 := .{ x = 1.0 } // y and z default to 0.0
    print_struct(Vec3, v);
}
💥 Output
x: 1.0
y: 2.0
z: 3.0

No format strings. No %. No suffering.

🧠 How this actually works (don’t skip this)
/*
Compile-time execution:

1. T = Vec3
2. T.fields = [
    {name="x", type=f32},
    {name="y", type=f32},
    {name="z", type=f32}
]

3. Loop runs at compile time
4. Generates runtime print calls

Resulting generated code:

print("x: ", v.x, "\n");
print("y: ", v.y, "\n");
print("z: ", v.z, "\n");
*/



we will also have type inference by default any value declared like 

count := 22;
will get a type i64
and floats will get f32




we will by default initialize everything to zero

but initialized option will also be there for optimization in some cases 



a: [50] int; // An array of 50 integers
b: [..] int; // A dynamic array of integers
Arrays do not automatically cast to pointers as in C. Rather, they are “wide pointers” that contain array size information. Functions can take array types and query for the size of the array.

print_int_array :: (a: [] int) {
    n := a.count;
    for i : 0..n-1 {
        print("array[%] = %\n", i, a[i]);
    }
}



Retaining the array size information can help developers avoid the pattern of passing array lengths as additional parameters and assist in automatic bounds checking (see Walter Bright – C’s Biggest Mistake)


enum values can also asigned later



like we can have enum of pipelines or then asign or mutate when we allocate pipeline id


we willl also have defer


value.trim().toUpper().print();
is also possile
Instead of:

print(toUpper(trim(value)));

👉 Much cleaner ✨

📌 Languages that support UFCS
Zig
D language
Rust (similar concept via traits/methods)
C++ (partially with ADL / free functions)
📌 Exam-style definition 📝

UFCS (Universal Function Call Syntax) allows functions to be called using method-call syntax, where the first argument becomes the receiver of the call.

⚡ Key takeaway

👉 UFCS = “Turn normal functions into method-style calls”





. vs -> 


everything sholud be either . or some other cool syntax with only one keystroke . vs -> is a refactoring nightmare and is bad for semantic compression oriented prog lang 

we can also have optinal types

we will also support inlining

we will also have local function



function synta

get_to_add :: (allocated: s64, load_factor: s64) -> s64
{


}
add :: (a, b: i32) -> i32 {
        return a + b;
}
add :: (a, b: Vec3) -> Vec3 {
        return .{ a.x + b.x, a.y + b.y, a.z + b.z };
        }




Variables can be made immutable by using :: instead of :=. This prevents other code from changing the variable in any way.



// just like before, the compiler will figure out the type if you leave it out

age := 21        // inferred, mutable
age :: 21        // inferred, immutable
age = 22; // ERROR! age is immutable
age: i32 = 21    // explicit type

Or use as for all casts with a notion of as! for unchecked cast


Pointers are created with *value or *mut value

 pointers are either mutable or immutable, like Rust.

foo := 5;
bar := *mut foo;

bar.* = 10;

core.println(foo);  // prints "10"
Unlike Rust however, there are currently no borrow checking rules like "either one mutable reference or multiple immutable references".

I say currently but to be honest I'm not sure if they even should be added down the road.

Mutable pointers greatly improve the readability of code, and allow one to see at a glance the side-effects of a function
~~~
a := 5;
        b := *a;
        c := b.*;
~~~
A pointer to a variable contains the memory address of that variable, it _points to_ the variable. So it is a  reference to a memory location, which can be on the stack or on the heap . If var is the variable,  then a pointer ptr to var is written with a * as follows: 

```mu
ptr = *var
```

* is sometimes called the **address-of** operator.  


Types can be created with the distinct keyword, which creates a new type that has the same underlying semantics of its sub type

Seconds :: distinct i32;

foo : Seconds = 42;
bar : i32 = 12;

bar = foo; // ERROR! Seconds != i32
This can be useful for making sure one doesn't mix up Seconds for Minutes for e.g.
wait = duration as Minutes; // Safe numeric cast, but still distinct types?
Since distinct creates a new type with same representation, as should be allowed for numeric conversions but perhaps require as! for cross-type casting

Dessert :: enum {
    Ice_Cream,
    Chocolate_Cake,
    Apple_Pie,
    Milkshake,
};

order_list := Dessert.[
    Dessert.Chocolate_Cake,
    Dessert.Ice_Cream,
    Dessert.Milkshake,
];


// Safe: mutable to immutable
mut_ptr: *mut i32 = ...;
imm_ptr := mut_ptr as *i32;      // ok

// Unsafe: immutable to mutable
imm_ptr: *i32 = ...;
mut_ptr := imm_ptr as! *mut i32; // explicit hazard

// Numeric casts (safe widening)
x: i32 = 42;
y := x as f32;   // ok
z := y as i32;   // may truncate; require as! or allow as with runtime check?



[N]T – fixed-size array.

[]T – slice (pointer + length).

Dynamic arrays (heap-allocated, growable) could be [..]T as you have


optinal aro either  T? or maybe Keyword

value: maybe i32 = 42;
result: maybe i32 = parse_int("42");

parse_int :: (s: string) -> maybe i32 { ... }
maybe_value: i32? = 42;
no_value: i32? = null;
low priority features {
using – Field & Namespace Injection
using brings fields from a struct (or names from a module) directly into the current scope. This reduces verbosity when working with deeply nested data – especially useful in game ECS (entity‑component‑systems) where you often want to treat component fields as if they were part of the entity.

1. Basic using – Inject All Fields
mu
Vec3 :: struct { x, y, z: f32 }

Entity :: struct {
    using pos: Vec3;        // injects x, y, z into Entity
    name: string;
}

e: Entity;
e.x = 10.0;   // same as e.pos.x
e.y = 20.0;
e.z = 30.0;
The fields x, y, z become directly accessible on Entity.

2. Conflict Resolution: except and only
When two injected fields have the same name, you get a redeclaration error. Use except to skip specific names, or only to bring in only a few.

using,except(name1, name2, …)
mu
VectorWithLength :: struct { x, y, z: f32; length: f32 }

Monolith :: struct {
    using,except(length) pos: VectorWithLength;   // inject x, y, z but NOT length
    depth  := 1.0;
    width  := 4.0;
    length := 9.0;        // now no conflict
}
using,only(name1, name2, …)
mu
Quaternion :: struct { x, y, z, w: f32; }

Thing :: struct {
    using,only(w) rot: Quaternion;   // injects only rot.w
}
t: Thing;
t.w = 1.0;   // works
// t.x = 2.0;  // error – not injected


}








match input {

    .Key(W) => player.move_forward(),
    .Key(S) => player.move_backward(),

    .Mouse(.Left) => shoot(),

    _ => {}
}


match game.state {

    .Menu => render_menu(),
AT
    .Playing => update_game(),

    .Paused => show_pause(),

}

match game.state {

    .Playing(level) => update_level(level),

    .GameOver(score) => show_score(score),

}

match (a, b) {

    (.Player, .Enemy) => damage_player(),

    (.Bullet, .Enemy) => destroy_enemy(),

    (.Bullet, .Player) => {}, // ignore

    _ => {}
}

for entity in world {

    match entity {

        {Position p, Velocity v} => {
            p.x += v.x;
            p.y += v.y;
        },

        _ => {}
    }
}

match (velocity, grounded) {

    (v, true) if v > 0 => .Run,
    (_, false)         => .Jump,
    _                  => .Idle,
}

match tile {

    '#' => wall(),
    '.' => floor(),
    'E' => spawn_enemy(),

}

match tile {

    Tile.{type = Wall} => ...,
    Tile.{type = Water} => ...,

}

A function like make_array(comptime T: type, comptime n: usize) -> [n]T creates true dependent array types at compile time.

This gives you type‑level computation without a full dependent type checker – you just run ordinary code at compile time.

mu
// Example: compile‑time sized matrix
Matrix :: comp (T: type, rows: usize, cols: usize) -> type {
    return struct {
        data: [rows][cols]T;
        // + generated methods like .transpose(), .mul()
    };
}
Mat4f := Matrix(f32, 4, 4);

// Structural constraint (implicit)
translate :: (obj: struct { x, y, z: f32 }, dx, dy, dz: f32) -> void {
    obj.x += dx; obj.y += dy; obj.z += dz;
}
// Works with any type that has x,y,z: f32 – no interface vtable.

TRAITS      → attach behavior to a type
TYPECLASSES → constrain functions by capability

we can also have traits on types
A generic function just uses .x, .y, .z – the compiler verifies at compile time that the argument type has those fields. Zero manual predicates.
 your comp keyword generates code, transforms data layouts (AoS ↔ SoA), and reflects over types.
This is more powerful than traits because it can change how data is laid out based on usage, not just which methods exist.

statically typed, structurally typed system with full compile-time execution (comptime), dependent typing via comptime functions, and support for data-oriented programming through metaprogramming (e.g., automatic SoA conversion, compile-time introspection

soa conversion can handled viaa module in compile time
not imp{
compile-time duck typing with reflection and code generation

}

may 

linear MutexGuard {
    mutex: *mut Mutex;
}
lock :: (m: *mut Mutex) -> linear MutexGuard;
unlock :: (guard: linear MutexGuard) -> void;
// You cannot forget to call unlock – compiler error.
translate :: (obj: *mut $T, dx, dy, dz: f32) -> void {
    obj.x += dx;  // compiler notes: T must have .x: ? (supports += f32)
    obj.y += dy;
    obj.z += dz;
}
// The requirement set is automatically the "concept" – no extra syntax.








for frame in 0..=360 step 30 {
    rotation = frame;
}


never vs void – Final Recommendation
Use both, but with distinct, non‑overlapping meanings.

void – no value, but control returns normally
Functions that perform an action and finish, returning nothing useful.

You can call a void function and continue execution.

Variables cannot have type void (zero‑sized type).

Consistent with C, Zig, Rust’s ().

mu
print_hello :: () -> void {
    println("hello");
}
never – the function never returns (bottom type)
Infinite loops, panic(), exit(), @unreachable().

never can coerce to any type – useful for expression‑level unreachable code.

A function returning never must either loop forever, panic, or call another never function.

Variables cannot be instantiated with never (no value exists).

mu
panic :: (msg: string) -> never {
    // print and abort
}

main :: () -> void {
    x := if condition 10 else panic("oops");  // OK: panic returns never → coerces to i32
}
Why not just one?
void alone – cannot express “this function never returns” for compiler optimizations (e.g., eliminating dead code after call).

never alone – functions that return normally but have no value would need a dummy type like unit – that’s void






exit :: (code: i32) -> never;  // provided by runtime

assert :: (cond: bool) -> void {
    if !cond {
        panic("assertion failed");  // panic returns never, so no return needed after
    }
}

main :: () -> void {
    let x := compute();
    if x == 0 {
        exit(1);  // never returns, but function is void – fine
    }
    // code after exit is unreachable, compiler warns
}

string and array are bounds  checked only when we pass the compiler a flag to bound check it mucc --enable-bounds-checked 

TextureSource :: union {
    File      { filename: string }
    Files     { filenames: []string }
    Image     { image: Image, debug_name: string? }
}

create_texture :: (device: Device, source: TextureSource, mipmaps: u32) -> Texture
match source {
    .File(f)   => ...
    .Files(fs) => ...
    .Image(i)  => ...
}

🧩 Option 3 (your style: compile-time specialization)
create_texture :: ($S: Type, device: Device, source: S) -> Texture

Where:

S = FileSource
S = MultiFileSource
S = ImageSource

we will also have traits
type system more advanced than rust which have higher kinded types and much more 
insp from this https://www.reddit.com/r/rust/comments/n0367k/does_any_language_have_a_typesystem_that_provides/
and something like refinement types , dependent types with comp time checks and runtime checks in specific builds but no runtime checks in realease builds if wanted


{


not real goal the keywords may be changed at comptime by simply replacing strings and what keywords to change may be defined by a file our philosophy for compiling the language is options will be exposed via a big struct in the main file so that compiler can just do mucc compileoptfile.mu and then it comiles based on struct defined in that file 

}


we will use combination of pratt parsing and recursive descent to parse language 


