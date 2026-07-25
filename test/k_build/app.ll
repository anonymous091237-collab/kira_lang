@long_print = private unnamed_addr constant [5 x i8] c"%lld\00", align 1
@int_print = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@ulong_print = private unnamed_addr constant [5 x i8] c"%llu\00", align 1
@uint_print = private unnamed_addr constant [3 x i8] c"%u\00", align 1
@flat_block = private unnamed_addr constant [306 x i8] c"Hello, Kira!\0A\00-- integer types --\0A\00short  : \00\0A\00ushort : \00int    : \00uint   : \00long   : \00ulong  : \00-- arithmetic (a=10, b=3) --\0A\00a + b = \00a - b = \00a * b = \00a / b = \00-- compound assign (+=5, -=2, *=3) --\0A\00a = \00-- booleans --\0A\00-- complex expr --\0A\00x = \00-- counting 0..9 --\0A\00 \00-- multiplication table 1..3 --\0A\00\09\00", align 1
declare i32 @printf(ptr noundef, ...)
define i32 @main() {
entry:
%v84 = getelementptr inbounds [306 x i8], ptr @flat_block, i32 0, i32 0
call i32 (ptr, ...) @printf(ptr %v84)
%v1 = add i16 32765, 0
%v2 = add i16 65534, 0
%v3 = add i32 2147483640, 0
%v4 = add i32 4294967290, 0
%v5 = add i64 9223372036854775800, 0
%v6 = add i64 18446744073709551615, 0
%v85 = getelementptr inbounds [306 x i8], ptr @flat_block, i32 0, i32 14
call i32 (ptr, ...) @printf(ptr %v85)
%v86 = getelementptr inbounds [306 x i8], ptr @flat_block, i32 0, i32 35
call i32 (ptr, ...) @printf(ptr %v86)
%v87  = zext i16 %v1 to i32
call i32 (ptr, ...) @printf(ptr @int_print, i32 %v87)
%v88 = getelementptr inbounds [306 x i8], ptr @flat_block, i32 0, i32 45
call i32 (ptr, ...) @printf(ptr %v88)
%v89 = getelementptr inbounds [306 x i8], ptr @flat_block, i32 0, i32 47
call i32 (ptr, ...) @printf(ptr %v89)
%v90  = zext i16 %v2 to i32
call i32 (ptr, ...) @printf(ptr @int_print, i32 %v90)
%v91 = getelementptr inbounds [306 x i8], ptr @flat_block, i32 0, i32 45
call i32 (ptr, ...) @printf(ptr %v91)
%v92 = getelementptr inbounds [306 x i8], ptr @flat_block, i32 0, i32 57
call i32 (ptr, ...) @printf(ptr %v92)
call i32 (ptr, ...) @printf(ptr @int_print, i32 %v3)
%v93 = getelementptr inbounds [306 x i8], ptr @flat_block, i32 0, i32 45
call i32 (ptr, ...) @printf(ptr %v93)
%v94 = getelementptr inbounds [306 x i8], ptr @flat_block, i32 0, i32 67
call i32 (ptr, ...) @printf(ptr %v94)
call i32 (ptr, ...) @printf(ptr @uint_print, i32 %v4)
%v95 = getelementptr inbounds [306 x i8], ptr @flat_block, i32 0, i32 45
call i32 (ptr, ...) @printf(ptr %v95)
%v96 = getelementptr inbounds [306 x i8], ptr @flat_block, i32 0, i32 77
call i32 (ptr, ...) @printf(ptr %v96)
call i32 (ptr, ...) @printf(ptr @long_print, i64 %v5)
%v97 = getelementptr inbounds [306 x i8], ptr @flat_block, i32 0, i32 45
call i32 (ptr, ...) @printf(ptr %v97)
%v98 = getelementptr inbounds [306 x i8], ptr @flat_block, i32 0, i32 87
call i32 (ptr, ...) @printf(ptr %v98)
call i32 (ptr, ...) @printf(ptr @ulong_print, i64 %v6)
%v99 = getelementptr inbounds [306 x i8], ptr @flat_block, i32 0, i32 45
call i32 (ptr, ...) @printf(ptr %v99)
%v13 = alloca i32, align 4
store i32 10, ptr %v13 , align 4
%v14 = alloca i32, align 4
store i32 3, ptr %v14 , align 4
%v15 = alloca i32, align 4
%v100 = load i32 , ptr %v13, align 4
%v101 = load i32 , ptr %v14, align 4
%v18 = add i32 %v100, %v101
store i32 %v18, ptr %v15, align 4
%v19 = alloca i32, align 4
%v102 = load i32 , ptr %v13, align 4
%v103 = load i32 , ptr %v14, align 4
%v22 = sub i32 %v102, %v103
store i32 %v22, ptr %v19, align 4
%v23 = alloca i32, align 4
%v104 = load i32 , ptr %v13, align 4
%v105 = load i32 , ptr %v14, align 4
%v26 = mul i32 %v104, %v105
store i32 %v26, ptr %v23, align 4
%v27 = alloca i32, align 4
%v106 = load i32 , ptr %v13, align 4
%v107 = load i32 , ptr %v14, align 4
%v30 = sdiv i32 %v106, %v107
store i32 %v30, ptr %v27, align 4
%v108 = getelementptr inbounds [306 x i8], ptr @flat_block, i32 0, i32 97
call i32 (ptr, ...) @printf(ptr %v108)
%v109 = getelementptr inbounds [306 x i8], ptr @flat_block, i32 0, i32 127
call i32 (ptr, ...) @printf(ptr %v109)
%v110 = load i32 , ptr %v15, align 4
call i32 (ptr, ...) @printf(ptr @int_print, i32 %v110)
%v111 = getelementptr inbounds [306 x i8], ptr @flat_block, i32 0, i32 45
call i32 (ptr, ...) @printf(ptr %v111)
%v112 = getelementptr inbounds [306 x i8], ptr @flat_block, i32 0, i32 136
call i32 (ptr, ...) @printf(ptr %v112)
%v113 = load i32 , ptr %v19, align 4
call i32 (ptr, ...) @printf(ptr @int_print, i32 %v113)
%v114 = getelementptr inbounds [306 x i8], ptr @flat_block, i32 0, i32 45
call i32 (ptr, ...) @printf(ptr %v114)
%v115 = getelementptr inbounds [306 x i8], ptr @flat_block, i32 0, i32 145
call i32 (ptr, ...) @printf(ptr %v115)
%v116 = load i32 , ptr %v23, align 4
call i32 (ptr, ...) @printf(ptr @int_print, i32 %v116)
%v117 = getelementptr inbounds [306 x i8], ptr @flat_block, i32 0, i32 45
call i32 (ptr, ...) @printf(ptr %v117)
%v118 = getelementptr inbounds [306 x i8], ptr @flat_block, i32 0, i32 154
call i32 (ptr, ...) @printf(ptr %v118)
%v119 = load i32 , ptr %v27, align 4
call i32 (ptr, ...) @printf(ptr @int_print, i32 %v119)
%v120 = getelementptr inbounds [306 x i8], ptr @flat_block, i32 0, i32 45
call i32 (ptr, ...) @printf(ptr %v120)
%v121 = load i32 , ptr %v13, align 4
%v36 = add i32 %v121, 5
store i32 %v36, ptr %v13, align 4
%v122 = load i32 , ptr %v13, align 4
%v38 = sub i32 %v122, 2
store i32 %v38, ptr %v13, align 4
%v123 = load i32 , ptr %v13, align 4
%v40 = mul i32 %v123, 3
store i32 %v40, ptr %v13, align 4
%v124 = getelementptr inbounds [306 x i8], ptr @flat_block, i32 0, i32 163
call i32 (ptr, ...) @printf(ptr %v124)
%v125 = getelementptr inbounds [306 x i8], ptr @flat_block, i32 0, i32 202
call i32 (ptr, ...) @printf(ptr %v125)
%v126 = load i32 , ptr %v13, align 4
call i32 (ptr, ...) @printf(ptr @int_print, i32 %v126)
%v127 = getelementptr inbounds [306 x i8], ptr @flat_block, i32 0, i32 45
call i32 (ptr, ...) @printf(ptr %v127)
%v42 = alloca i1, align 1
%v43 = icmp eq i32 5, 5
store i1 %v43, ptr %v42, align 1
%v44 = alloca i1, align 1
%v45 = icmp eq i32 5, 6
store i1 %v45, ptr %v44, align 1
%v128 = getelementptr inbounds [306 x i8], ptr @flat_block, i32 0, i32 207
call i32 (ptr, ...) @printf(ptr %v128)
%v46 = alloca i32, align 4
%v47 = mul i32 8, 6
%v48 = add i32 5, %v47
store i32 %v48, ptr %v46, align 4
%v50 = sub i32 0, 5
%v51 = sub i32 81, %v50
%v129 = load i32 , ptr %v46, align 4
%v52 = add i32 %v129, %v51
store i32 %v52, ptr %v46, align 4
%v130 = getelementptr inbounds [306 x i8], ptr @flat_block, i32 0, i32 223
call i32 (ptr, ...) @printf(ptr %v130)
%v131 = getelementptr inbounds [306 x i8], ptr @flat_block, i32 0, i32 243
call i32 (ptr, ...) @printf(ptr %v131)
%v132 = load i32 , ptr %v46, align 4
call i32 (ptr, ...) @printf(ptr @int_print, i32 %v132)
%v133 = getelementptr inbounds [306 x i8], ptr @flat_block, i32 0, i32 45
call i32 (ptr, ...) @printf(ptr %v133)
%v134 = getelementptr inbounds [306 x i8], ptr @flat_block, i32 0, i32 248
call i32 (ptr, ...) @printf(ptr %v134)
%v54 = alloca i32, align 4
store i32 0, ptr %v54 , align 4
br label %l55
l55:
%v135 = load i32 , ptr %v54, align 4
%v59 = icmp slt i32 %v135, 10
br i1 %v59, label %l56, label %l57
l56:
%v136 = load i32 , ptr %v54, align 4
call i32 (ptr, ...) @printf(ptr @int_print, i32 %v136)
%v137 = getelementptr inbounds [306 x i8], ptr @flat_block, i32 0, i32 269
call i32 (ptr, ...) @printf(ptr %v137)
%v138 = load i32 , ptr %v54, align 4
%v62 = add i32 %v138, 1
store i32 %v62, ptr %v54, align 4
br label %l55
l57:
%v139 = getelementptr inbounds [306 x i8], ptr @flat_block, i32 0, i32 45
call i32 (ptr, ...) @printf(ptr %v139)
%v140 = getelementptr inbounds [306 x i8], ptr @flat_block, i32 0, i32 271
call i32 (ptr, ...) @printf(ptr %v140)
%v63 = alloca i32, align 4
store i32 1, ptr %v63 , align 4
br label %l64
l64:
%v141 = load i32 , ptr %v63, align 4
%v68 = icmp slt i32 %v141, 4
br i1 %v68, label %l65, label %l66
l65:
%v69 = alloca i32, align 4
store i32 1, ptr %v69 , align 4
br label %l70
l70:
%v142 = load i32 , ptr %v69, align 4
%v74 = icmp slt i32 %v142, 4
br i1 %v74, label %l71, label %l72
l71:
%v75 = alloca i32, align 4
%v143 = load i32 , ptr %v63, align 4
%v144 = load i32 , ptr %v69, align 4
%v78 = mul i32 %v143, %v144
store i32 %v78, ptr %v75, align 4
%v145 = load i32 , ptr %v75, align 4
call i32 (ptr, ...) @printf(ptr @int_print, i32 %v145)
%v146 = getelementptr inbounds [306 x i8], ptr @flat_block, i32 0, i32 304
call i32 (ptr, ...) @printf(ptr %v146)
%v147 = load i32 , ptr %v69, align 4
%v81 = add i32 %v147, 1
store i32 %v81, ptr %v69, align 4
br label %l70
l72:
%v148 = getelementptr inbounds [306 x i8], ptr @flat_block, i32 0, i32 45
call i32 (ptr, ...) @printf(ptr %v148)
%v149 = load i32 , ptr %v63, align 4
%v83 = add i32 %v149, 1
store i32 %v83, ptr %v63, align 4
br label %l64
l66:
ret i32 0
}

