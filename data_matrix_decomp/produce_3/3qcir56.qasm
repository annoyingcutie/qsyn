OPENQASM 2.0;
include "qelib1.inc";
qreg q[3];
x q[1];
rz(1.5405815420883469) q[0];
h q[2];
cx q[0],q[2];
h q[1];
cx q[2],q[1];
h q[0];
ry(6.2569376772390015) q[2];
h q[0];
y q[1];
cx q[2],q[0];
y q[1];
z q[0];
cx q[2],q[1];
h q[0];
cx q[2],q[1];
cx q[0],q[1];
z q[2];
cx q[1],q[0];
ry(2.614928225257759) q[2];
rz(1.3598860566172397) q[2];
h q[1];
rz(1.750600419373891) q[0];
cx q[1],q[0];
rx(5.263589797823521) q[2];
rz(1.1621832835568044) q[2];
y q[1];
h q[0];
ry(3.521260278033187) q[2];
cx q[1],q[0];
x q[2];
cx q[0],q[1];
rx(4.986743552703014) q[0];
cx q[2],q[1];
cx q[0],q[1];
y q[2];
x q[2];
h q[0];
y q[1];
cx q[2],q[1];
ry(4.453438988358591) q[0];
rx(2.438730371744713) q[0];
z q[1];
y q[2];
cx q[2],q[0];
h q[1];
