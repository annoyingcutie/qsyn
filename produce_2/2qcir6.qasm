OPENQASM 2.0;
include "qelib1.inc";
qreg q[2];
cx q[1],q[0];
y q[1];
y q[0];
cx q[0],q[1];
ry(2.5307609453612336) q[0];
y q[1];
cx q[1],q[0];
y q[1];
x q[0];
z q[1];
x q[0];
x q[0];
x q[1];
h q[1];
z q[0];
cx q[1],q[0];
z q[1];
rx(3.4903487763520804) q[0];
rz(5.618182713087951) q[0];
ry(6.182025148616994) q[1];
h q[0];
ry(0.396946373149401) q[1];
x q[1];
y q[0];
y q[1];
x q[0];
