OPENQASM 2.0;
include "qelib1.inc";
qreg q[3];
ry(0.404800361860889) q[0];
h q[1];
h q[2];
rx(5.747076729061829) q[1];
cx q[2],q[0];
cx q[0],q[2];
z q[1];
cx q[2],q[0];
x q[1];
cx q[2],q[0];
ry(2.5417998980150722) q[1];
rz(2.916260447662212) q[2];
cx q[0],q[1];
cx q[0],q[1];
z q[2];
cx q[2],q[1];
x q[0];
cx q[1],q[0];
h q[2];
