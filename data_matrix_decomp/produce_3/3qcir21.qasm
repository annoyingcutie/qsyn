OPENQASM 2.0;
include "qelib1.inc";
qreg q[3];
cx q[2],q[0];
y q[1];
cx q[0],q[1];
y q[2];
cx q[1],q[2];
rz(1.4844124716512748) q[0];
cx q[0],q[2];
h q[1];
cx q[0],q[1];
h q[2];
cx q[0],q[2];
h q[1];
ry(4.944599255040424) q[2];
h q[1];
z q[0];
cx q[2],q[0];
ry(4.363350698351218) q[1];
ry(3.8093137482882904) q[2];
cx q[1],q[0];
y q[2];
cx q[1],q[0];
cx q[2],q[0];
rz(2.849476005446511) q[1];
x q[2];
h q[1];
ry(2.386891620150038) q[0];
z q[0];
cx q[2],q[1];
cx q[1],q[0];
rz(2.4758580172167766) q[2];
cx q[1],q[2];
x q[0];
cx q[2],q[0];
x q[1];
