OPENQASM 2.0;
include "qelib1.inc";
qreg q[2];
rz(2.1708415200713618) q[1];
ry(2.9927845155637804) q[0];
cx q[1],q[0];
cx q[0],q[1];
rz(3.0000968394210035) q[0];
rz(0.37401085822834235) q[1];
z q[1];
h q[0];
