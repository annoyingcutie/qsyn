OPENQASM 2.0;
include "qelib1.inc";
qreg q[3];
x q[0];
x q[0];
cx q[2], q[0];
rz(-0.37399912542735636) q[0];
cx q[2], q[0];
rz(0.37399912542735636) q[0];cx q[1], q[2];
cx q[2], q[0];
rz(0.37399912542735636) q[0];
cx q[2], q[0];
rz(-0.37399912542735636) q[0];cx q[1], q[2];
cx q[1], q[0];
rz(-0.37399912542735636) q[0];
cx q[1], q[0];
rz(0.37399912542735636) q[0];x q[0];
x q[0];
x q[1];
x q[1];
rz(-0.2991993003418851) q[1];
cx q[2], q[1];
ry(-0.6283185307179586) q[1];
cx q[2], q[1];
ry(0.6283185307179586) q[1];
rz(0.2991993003418851) q[1];
cx q[0], q[2];
rz(2.842393353247908) q[1];
cx q[2], q[1];
ry(-0.6283185307179586) q[1];
cx q[2], q[1];
ry(0.6283185307179586) q[1];
rz(-2.842393353247908) q[1];
cx q[0], q[2];
rz(-0.2991993003418851) q[1];
cx q[0], q[1];
ry(-0.6283185307179586) q[1];
cx q[0], q[1];
ry(0.6283185307179586) q[1];
rz(0.2991993003418851) q[1];
x q[1];
x q[1];
x q[1];
x q[1];
cx q[1], q[0];
cx q[2], q[1];
rz(-0.7479982508547128) q[1];
cx q[2], q[1];
rz(0.7479982508547128) q[1];cx q[0], q[2];
cx q[2], q[1];
rz(0.7479982508547128) q[1];
cx q[2], q[1];
rz(-0.7479982508547128) q[1];cx q[0], q[2];
cx q[0], q[1];
rz(-0.7479982508547128) q[1];
cx q[0], q[1];
rz(0.7479982508547128) q[1];cx q[1], q[0];
x q[1];
x q[1];
x q[1];
cx q[1], q[0];
x q[1];
cx q[1], q[0];
rz(-0.2991993003418851) q[1];
cx q[2], q[1];
ry(-0.6283185307179584) q[1];
cx q[2], q[1];
ry(0.6283185307179584) q[1];
rz(0.2991993003418851) q[1];
cx q[0], q[2];
rz(2.842393353247908) q[1];
cx q[2], q[1];
ry(-0.6283185307179584) q[1];
cx q[2], q[1];
ry(0.6283185307179584) q[1];
rz(-2.842393353247908) q[1];
cx q[0], q[2];
rz(-0.2991993003418851) q[1];
cx q[0], q[1];
ry(-0.6283185307179584) q[1];
cx q[0], q[1];
ry(0.6283185307179584) q[1];
rz(0.2991993003418851) q[1];
cx q[1], q[0];
x q[1];
cx q[1], q[0];
x q[1];
x q[0];
cx q[0], q[1];
x q[0];
cx q[0], q[1];
cx q[2], q[0];
rz(0.6731984257692414) q[0];
cx q[2], q[0];
rz(-0.6731984257692414) q[0];cx q[1], q[2];
cx q[2], q[0];
rz(-0.6731984257692414) q[0];
cx q[2], q[0];
rz(0.6731984257692414) q[0];cx q[1], q[2];
cx q[1], q[0];
rz(0.6731984257692414) q[0];
cx q[1], q[0];
rz(-0.6731984257692414) q[0];cx q[0], q[1];
x q[0];
cx q[0], q[1];
x q[0];
x q[2];
x q[2];
rz(-0.7479982508547125) q[2];
cx q[0], q[2];
ry(-0.02721710272734028) q[2];
cx q[0], q[2];
ry(0.02721710272734028) q[2];
rz(0.7479982508547125) q[2];
cx q[1], q[0];
rz(2.3935944027350806) q[2];
cx q[0], q[2];
ry(-0.02721710272734028) q[2];
cx q[0], q[2];
ry(0.02721710272734028) q[2];
rz(-2.3935944027350806) q[2];
cx q[1], q[0];
rz(-0.7479982508547125) q[2];
cx q[1], q[2];
ry(-0.02721710272734028) q[2];
cx q[1], q[2];
ry(0.02721710272734028) q[2];
rz(0.7479982508547125) q[2];
x q[2];
x q[2];
x q[2];
x q[2];
cx q[2], q[0];
cx q[1], q[0];
cx q[1], q[0];
cx q[2], q[0];
x q[2];
x q[2];
x q[2];
x q[2];
cx q[2], q[1];
rz(-0.7479982508547124) q[2];
cx q[0], q[2];
ry(-0.08295465899329255) q[2];
cx q[0], q[2];
ry(0.08295465899329255) q[2];
rz(0.7479982508547124) q[2];
cx q[1], q[0];
rz(2.3935944027350806) q[2];
cx q[0], q[2];
ry(-0.08295465899329255) q[2];
cx q[0], q[2];
ry(0.08295465899329255) q[2];
rz(-2.3935944027350806) q[2];
cx q[1], q[0];
rz(-0.7479982508547124) q[2];
cx q[1], q[2];
ry(-0.08295465899329255) q[2];
cx q[1], q[2];
ry(0.08295465899329255) q[2];
rz(0.7479982508547124) q[2];
cx q[2], q[1];
x q[2];
x q[2];
x q[2];
x q[2];
cx q[2], q[0];
cx q[2], q[1];
cx q[0], q[2];
rz(-1.0471975511965974) q[2];
cx q[0], q[2];
rz(1.0471975511965974) q[2];cx q[1], q[0];
cx q[0], q[2];
rz(1.0471975511965974) q[2];
cx q[0], q[2];
rz(-1.0471975511965974) q[2];cx q[1], q[0];
cx q[1], q[2];
rz(-1.0471975511965974) q[2];
cx q[1], q[2];
rz(1.0471975511965974) q[2];cx q[2], q[1];
cx q[2], q[0];
x q[2];
x q[2];
x q[2];
cx q[2], q[0];
x q[2];
cx q[2], q[0];
rz(-0.7479982508547125) q[2];
cx q[0], q[2];
ry(-0.02721710272734028) q[2];
cx q[0], q[2];
ry(0.02721710272734028) q[2];
rz(0.7479982508547125) q[2];
cx q[1], q[0];
rz(2.3935944027350806) q[2];
cx q[0], q[2];
ry(-0.02721710272734028) q[2];
cx q[0], q[2];
ry(0.02721710272734028) q[2];
rz(-2.3935944027350806) q[2];
cx q[1], q[0];
rz(-0.7479982508547125) q[2];
cx q[1], q[2];
ry(-0.02721710272734028) q[2];
cx q[1], q[2];
ry(0.02721710272734028) q[2];
rz(0.7479982508547125) q[2];
cx q[2], q[0];
x q[2];
cx q[2], q[0];
x q[2];
x q[0];
cx q[0], q[2];
x q[0];
cx q[0], q[1];
cx q[1], q[2];
cx q[1], q[2];
cx q[0], q[1];
x q[0];
cx q[0], q[2];
x q[0];
x q[2];
cx q[2], q[0];
x q[2];
cx q[2], q[0];
cx q[2], q[1];
rz(-0.7479982508547124) q[2];
cx q[0], q[2];
ry(-0.08295465899329255) q[2];
cx q[0], q[2];
ry(0.08295465899329255) q[2];
rz(0.7479982508547124) q[2];
cx q[1], q[0];
rz(2.3935944027350806) q[2];
cx q[0], q[2];
ry(-0.08295465899329255) q[2];
cx q[0], q[2];
ry(0.08295465899329255) q[2];
rz(-2.3935944027350806) q[2];
cx q[1], q[0];
rz(-0.7479982508547124) q[2];
cx q[1], q[2];
ry(-0.08295465899329255) q[2];
cx q[1], q[2];
ry(0.08295465899329255) q[2];
rz(0.7479982508547124) q[2];
cx q[2], q[1];
cx q[2], q[0];
x q[2];
cx q[2], q[0];
x q[2];
x q[0];
cx q[0], q[2];
x q[0];
cx q[0], q[2];
cx q[2], q[0];
rz(-0.8975979010256551) q[0];
cx q[2], q[0];
rz(0.8975979010256551) q[0];cx q[1], q[2];
cx q[2], q[0];
rz(0.8975979010256551) q[0];
cx q[2], q[0];
rz(-0.8975979010256551) q[0];cx q[1], q[2];
cx q[1], q[0];
rz(-0.8975979010256551) q[0];
cx q[1], q[0];
rz(0.8975979010256551) q[0];cx q[0], q[2];
x q[0];
cx q[0], q[2];
x q[0];
x q[1];
cx q[1], q[2];
x q[1];
rz(-0.29919930034188474) q[1];
cx q[2], q[1];
ry(-0.08295465899329255) q[1];
cx q[2], q[1];
ry(0.08295465899329255) q[1];
rz(0.29919930034188474) q[1];
cx q[0], q[2];
rz(2.8423933532479086) q[1];
cx q[2], q[1];
ry(-0.08295465899329255) q[1];
cx q[2], q[1];
ry(0.08295465899329255) q[1];
rz(-2.8423933532479086) q[1];
cx q[0], q[2];
rz(-0.29919930034188474) q[1];
cx q[0], q[1];
ry(-0.08295465899329255) q[1];
cx q[0], q[1];
ry(0.08295465899329255) q[1];
rz(0.29919930034188474) q[1];
x q[1];
cx q[1], q[2];
x q[1];
x q[1];
cx q[1], q[2];
x q[1];
cx q[1], q[0];
cx q[0], q[2];
cx q[0], q[2];
cx q[1], q[0];
x q[1];
cx q[1], q[2];
x q[1];
x q[2];
cx q[2], q[1];
x q[2];
cx q[2], q[1];
rz(2.8423933532479086) q[2];
cx q[0], q[2];
ry(-0.027217102727348442) q[2];
cx q[0], q[2];
ry(0.027217102727348442) q[2];
rz(-2.8423933532479086) q[2];
cx q[1], q[0];
rz(-0.2991993003418847) q[2];
cx q[0], q[2];
ry(-0.027217102727348442) q[2];
cx q[0], q[2];
ry(0.027217102727348442) q[2];
rz(0.2991993003418847) q[2];
cx q[1], q[0];
rz(2.8423933532479086) q[2];
cx q[1], q[2];
ry(-0.027217102727348442) q[2];
cx q[1], q[2];
ry(0.027217102727348442) q[2];
rz(-2.8423933532479086) q[2];
cx q[2], q[1];
x q[2];
cx q[2], q[1];
x q[2];
x q[2];
cx q[2], q[1];
x q[2];
cx q[2], q[0];
cx q[2], q[1];
cx q[1], q[0];
cx q[1], q[0];
cx q[2], q[1];
cx q[2], q[0];
x q[2];
cx q[2], q[1];
x q[2];
x q[1];
cx q[1], q[2];
x q[1];
cx q[1], q[2];
rz(0.7479982508547127) q[1];
cx q[2], q[1];
ry(-0.15707963267948938) q[1];
cx q[2], q[1];
ry(0.15707963267948938) q[1];
rz(-0.7479982508547127) q[1];
cx q[0], q[2];
rz(-2.3935944027350806) q[1];
cx q[2], q[1];
ry(-0.15707963267948938) q[1];
cx q[2], q[1];
ry(0.15707963267948938) q[1];
rz(2.3935944027350806) q[1];
cx q[0], q[2];
rz(0.7479982508547127) q[1];
cx q[0], q[1];
ry(-0.15707963267948938) q[1];
cx q[0], q[1];
ry(0.15707963267948938) q[1];
rz(-0.7479982508547127) q[1];
cx q[1], q[2];
x q[1];
cx q[1], q[2];
x q[1];
x q[1];
cx q[1], q[2];
x q[1];
cx q[1], q[0];
cx q[1], q[2];
cx q[2], q[1];
rz(-0.7479982508547125) q[1];
cx q[2], q[1];
rz(0.7479982508547125) q[1];cx q[0], q[2];
cx q[2], q[1];
rz(0.7479982508547125) q[1];
cx q[2], q[1];
rz(-0.7479982508547125) q[1];cx q[0], q[2];
cx q[0], q[1];
rz(-0.7479982508547125) q[1];
cx q[0], q[1];
rz(0.7479982508547125) q[1];cx q[1], q[2];
cx q[1], q[0];
x q[1];
cx q[1], q[2];
x q[1];
x q[1];
cx q[1], q[0];
cx q[1], q[2];
x q[1];
cx q[1], q[0];
rz(-0.29919930034188474) q[1];
cx q[2], q[1];
ry(-0.08295465899329255) q[1];
cx q[2], q[1];
ry(0.08295465899329255) q[1];
rz(0.29919930034188474) q[1];
cx q[0], q[2];
rz(2.8423933532479086) q[1];
cx q[2], q[1];
ry(-0.08295465899329255) q[1];
cx q[2], q[1];
ry(0.08295465899329255) q[1];
rz(-2.8423933532479086) q[1];
cx q[0], q[2];
rz(-0.29919930034188474) q[1];
cx q[0], q[1];
ry(-0.08295465899329255) q[1];
cx q[0], q[1];
ry(0.08295465899329255) q[1];
rz(0.29919930034188474) q[1];
cx q[1], q[0];
x q[1];
cx q[1], q[2];
cx q[1], q[0];
x q[1];
x q[0];
cx q[0], q[1];
cx q[0], q[2];
x q[0];
cx q[0], q[1];
cx q[1], q[2];
cx q[1], q[2];
cx q[0], q[1];
x q[0];
cx q[0], q[2];
cx q[0], q[1];
x q[0];
x q[2];
cx q[2], q[0];
cx q[2], q[1];
x q[2];
cx q[2], q[0];
cx q[2], q[1];
rz(2.8423933532479086) q[2];
cx q[0], q[2];
ry(-0.027217102727348442) q[2];
cx q[0], q[2];
ry(0.027217102727348442) q[2];
rz(-2.8423933532479086) q[2];
cx q[1], q[0];
rz(-0.2991993003418847) q[2];
cx q[0], q[2];
ry(-0.027217102727348442) q[2];
cx q[0], q[2];
ry(0.027217102727348442) q[2];
rz(0.2991993003418847) q[2];
cx q[1], q[0];
rz(2.8423933532479086) q[2];
cx q[1], q[2];
ry(-0.027217102727348442) q[2];
cx q[1], q[2];
ry(0.027217102727348442) q[2];
rz(-2.8423933532479086) q[2];
cx q[2], q[1];
cx q[2], q[0];
x q[2];
cx q[2], q[1];
cx q[2], q[0];
x q[2];
x q[0];
cx q[0], q[1];
cx q[0], q[2];
x q[0];
cx q[0], q[2];
cx q[1], q[2];
cx q[1], q[2];
cx q[0], q[2];
x q[0];
cx q[0], q[2];
cx q[0], q[1];
x q[0];
x q[1];
cx q[1], q[0];
cx q[1], q[2];
x q[1];
cx q[1], q[0];
cx q[1], q[2];
rz(0.7479982508547127) q[1];
cx q[2], q[1];
ry(-0.15707963267948938) q[1];
cx q[2], q[1];
ry(0.15707963267948938) q[1];
rz(-0.7479982508547127) q[1];
cx q[0], q[2];
rz(-2.3935944027350806) q[1];
cx q[2], q[1];
ry(-0.15707963267948938) q[1];
cx q[2], q[1];
ry(0.15707963267948938) q[1];
rz(2.3935944027350806) q[1];
cx q[0], q[2];
rz(0.7479982508547127) q[1];
cx q[0], q[1];
ry(-0.15707963267948938) q[1];
cx q[0], q[1];
ry(0.15707963267948938) q[1];
rz(-0.7479982508547127) q[1];
cx q[1], q[2];
cx q[1], q[0];
x q[1];
cx q[1], q[2];
cx q[1], q[0];
x q[1];
x q[0];
cx q[0], q[1];
cx q[0], q[2];
x q[0];
cx q[0], q[1];
cx q[0], q[2];
cx q[2], q[0];
rz(-0.3739991254273563) q[0];
cx q[2], q[0];
rz(0.3739991254273563) q[0];cx q[1], q[2];
cx q[2], q[0];
rz(0.3739991254273563) q[0];
cx q[2], q[0];
rz(-0.3739991254273563) q[0];cx q[1], q[2];
cx q[1], q[0];
rz(-0.3739991254273563) q[0];
cx q[1], q[0];
rz(0.3739991254273563) q[0];cx q[0], q[2];
cx q[0], q[1];
x q[0];
cx q[0], q[2];
cx q[0], q[1];
x q[0];
