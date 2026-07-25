fn main() :int {

    // ── Greeting ───────────────────────────────────────────────────────────
    print("Hello, Kira!\n");

    // ── Integer types ──────────────────────────────────────────────────────
    def i16_val :short  = 32765;
    def u16_val :ushort = 65534;
    def i32_val :int    = 2147483640;
    def u32_val :uint   = 4294967290;
    def i64_val :long   = 9223372036854775800;
    def u64_val :ulong  = 18446744073709551615;

    print("-- integer types --\n");
    print("short  : ", i16_val, "\n");
    print("ushort : ", u16_val, "\n");
    print("int    : ", i32_val, "\n");
    print("uint   : ", u32_val, "\n");
    print("long   : ", i64_val, "\n");
    print("ulong  : ", u64_val, "\n");

    // ── Arithmetic ─────────────────────────────────────────────────────────
    var a :int = 10;
    var b :int = 3;

    var sum  :int = a + b;
    var diff :int = a - b;
    var prod :int = a * b;
    var quot :int = a / b;

    print("-- arithmetic (a=10, b=3) --\n");
    print("a + b = ", sum,  "\n");
    print("a - b = ", diff, "\n");
    print("a * b = ", prod, "\n");
    print("a / b = ", quot, "\n");

    // ── Compound assignment ────────────────────────────────────────────────
    a += 5;
    a -= 2;
    a *= 3;
    print("-- compound assign (+=5, -=2, *=3) --\n");
    print("a = ", a, "\n");

    // ── Boolean & comparison ───────────────────────────────────────────────
    var eq_check :bool = 5 == 5;
    var ne_check :bool = 5 == 6;

    print("-- booleans --\n");
    //print("5 == 5 : ", eq_check, "\n");
    //print("5 == 6 : ", ne_check, "\n");

    // ── Complex expression ─────────────────────────────────────────────────
    var x :int = 5 + 8 * 6;
    x += 81 - -5;
    print("-- complex expr --\n");
    print("x = ", x, "\n");

    // ── While loop ─────────────────────────────────────────────────────────
    print("-- counting 0..9 --\n");
    var i :int = 0;
    while i < 10 {
        print(i, " ");
        i += 1;
    }
    print("\n");

    // ── Nested while (multiplication table 1-3) ────────────────────────────
    print("-- multiplication table 1..3 --\n");
    var row :int = 1;
    while row < 4 {
        var col :int = 1;
        while col < 4 {
            var cell :int = row * col;
            print(cell, "\t");
            col += 1;
        }
        print("\n");
        row += 1;
    }

    return 0;
}