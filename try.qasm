OPENQASM 2.0;
include "qelib1.inc";
qreg q[2];
x q[0];
x q[0];
rz(1.2715970264530116) q[0];
cx q[1], q[0];
rz(0.8227980759401841) q[0];
ry(-1.2566370614359175) q[0];
cx q[1], q[0];
ry(1.2566370614359175) q[0];
rz(-2.0943951023931957) q[0];
rz(1.5707963267948966) q[1];
x q[0];
x q[0];
x q[1];
x q[1];
rz(-0.7479982508547125) q[1];
cx q[0], q[1];
ry(-0.054434205454682104) q[1];
cx q[0], q[1];
ry(0.054434205454682104) q[1];
rz(0.7479982508547124) q[1];
x q[1];
x q[1];
x q[1];
x q[1];
cx q[1], q[0];
rz(-0.7479982508547124) q[1];
cx q[0], q[1];
rz(-1.0471975511965974) q[1];
ry(-0.16590931798658476) q[1];
cx q[0], q[1];
ry(0.16590931798658476) q[1];
rz(1.7951958020513097) q[1];
cx q[1], q[0];
x q[1];
x q[1];
x q[0];
cx q[0], q[1];
x q[0];
rz(-0.2991993003418848) q[0];
cx q[1], q[0];
ry(-0.1659093179865841) q[0];
cx q[1], q[0];
ry(0.1659093179865841) q[0];
rz(0.2991993003418848) q[0];
x q[0];
cx q[0], q[1];
x q[0];
x q[1];
cx q[1], q[0];
x q[1];
cx q[1], q[0];
rz(2.8423933532479086) q[1];
cx q[0], q[1];
ry(-0.054434205454684144) q[1];
cx q[0], q[1];
ry(0.054434205454684144) q[1];
rz(-2.8423933532479086) q[1];
cx q[1], q[0];
x q[1];
cx q[1], q[0];
x q[1];
x q[0];
cx q[0], q[1];
x q[0];
cx q[0], q[1];
rz(0.7479982508547127) q[0];
cx q[1], q[0];
rz(-0.7479982508547125) q[0];
ry(-0.3141592653589791) q[0];
cx q[1], q[0];
ry(0.3141592653589791) q[0];
cx q[0], q[1];
x q[0];
cx q[0], q[1];
x q[0];
