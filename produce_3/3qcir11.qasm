OPENQASM 2.0;
include "qelib1.inc";
qreg q[3];
rz(0.7214848868229929) q[2];
z q[1];
y q[0];
h q[1];
cx q[2],q[0];
cx q[1],q[0];
y q[2];
rx(2.3180779604784365) q[2];
h q[1];
rz(4.292743252543086) q[0];
rx(2.8362493237253417) q[1];
z q[2];
z q[0];
z q[2];
cx q[1],q[0];
cx q[2],q[0];
rz(4.4305948133581055) q[1];
