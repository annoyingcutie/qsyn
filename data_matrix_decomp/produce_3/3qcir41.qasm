OPENQASM 2.0;
include "qelib1.inc";
qreg q[3];
cx q[2],q[0];
rx(5.798057889213885) q[1];
y q[2];
cx q[0],q[1];
cx q[1],q[2];
rz(1.7442964525187972) q[0];
cx q[2],q[0];
z q[1];
rz(4.176328594955588) q[0];
h q[2];
x q[1];
cx q[2],q[1];
x q[0];
cx q[1],q[2];
y q[0];
h q[2];
x q[0];
z q[1];
cx q[0],q[2];
rz(0.07724477358116894) q[1];
x q[1];
cx q[2],q[0];
cx q[2],q[0];
h q[1];
y q[1];
cx q[0],q[2];
rz(5.970439690505128) q[2];
cx q[1],q[0];
ry(3.377322624794402) q[0];
cx q[2],q[1];
cx q[1],q[0];
rz(4.322010807845864) q[2];
cx q[0],q[1];
z q[2];
h q[2];
cx q[1],q[0];
x q[2];
cx q[1],q[0];
cx q[0],q[1];
rx(0.9249156276118133) q[2];
y q[1];
h q[2];
z q[0];
