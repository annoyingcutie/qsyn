OPENQASM 2.0;
include "qelib1.inc";
qreg q[3];
cx q[2],q[1];
h q[0];
cx q[2],q[0];
y q[1];
cx q[0],q[1];
ry(3.0268027501351296) q[2];
cx q[2],q[1];
h q[0];
rz(5.361070175309019) q[2];
cx q[1],q[0];
cx q[0],q[2];
x q[1];
cx q[1],q[0];
y q[2];
y q[1];
ry(5.151794333067199) q[0];
y q[2];
h q[0];
cx q[2],q[1];
z q[2];
h q[1];
z q[0];
y q[0];
cx q[1],q[2];
cx q[2],q[1];
rx(1.0673069270553819) q[0];
cx q[2],q[0];
rx(0.9983183341956724) q[1];
cx q[2],q[0];
ry(4.355636170470418) q[1];
cx q[1],q[2];
ry(4.258686315260023) q[0];
cx q[0],q[1];
h q[2];
cx q[2],q[1];
z q[0];
x q[1];
h q[0];
rx(0.6844407282588888) q[2];
h q[0];
cx q[2],q[1];
z q[1];
ry(3.9534377361027357) q[2];
rx(5.256659989902872) q[0];
y q[0];
rx(1.1155143188159709) q[2];
ry(4.3900489997475285) q[1];
rz(5.994416974133758) q[1];
cx q[0],q[2];
