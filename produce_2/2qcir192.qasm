OPENQASM 2.0;
include "qelib1.inc";
qreg q[2];
x q[1];
x q[0];
x q[0];
h q[1];
cx q[1],q[0];
cx q[1],q[0];
cx q[1],q[0];
cx q[1],q[0];
z q[1];
ry(0.9332506860996644) q[0];
cx q[0],q[1];
cx q[0],q[1];
z q[0];
z q[1];
ry(1.0320328908938177) q[1];
ry(0.6308624948310182) q[0];
cx q[0],q[1];
x q[1];
rz(2.7981463998937097) q[0];
cx q[0],q[1];
cx q[1],q[0];
cx q[1],q[0];
cx q[1],q[0];
cx q[1],q[0];
cx q[0],q[1];
rz(4.689432676432187) q[0];
y q[1];
cx q[1],q[0];
cx q[0],q[1];
cx q[0],q[1];
cx q[0],q[1];
cx q[1],q[0];
ry(5.11429873988973) q[1];
ry(2.5579044385872343) q[0];
cx q[0],q[1];
cx q[0],q[1];
cx q[1],q[0];
cx q[1],q[0];
cx q[1],q[0];
y q[1];
z q[0];
cx q[1],q[0];
rx(3.7854936078139354) q[1];
h q[0];
rz(4.109620883713477) q[0];
x q[1];
rz(4.251940306840761) q[0];
rz(1.9365473078208635) q[1];
rx(4.175372724116003) q[0];
y q[1];
x q[1];
z q[0];
cx q[0],q[1];
ry(3.813095242346962) q[1];
ry(4.318448760978926) q[0];
x q[0];
h q[1];
cx q[0],q[1];
rx(4.022991210403047) q[1];
rx(4.6500060978243) q[0];
h q[1];
rz(5.655223695028748) q[0];
cx q[0],q[1];
y q[1];
y q[0];
rz(4.386886201503549) q[0];
x q[1];
cx q[0],q[1];
cx q[1],q[0];
cx q[0],q[1];
cx q[0],q[1];
cx q[0],q[1];
cx q[0],q[1];
cx q[1],q[0];
z q[0];
ry(5.711840422085846) q[1];
rx(3.6855037248048754) q[0];
y q[1];
cx q[0],q[1];
cx q[0],q[1];
ry(1.4416595329432498) q[1];
x q[0];
cx q[1],q[0];
cx q[1],q[0];
cx q[0],q[1];
cx q[1],q[0];
y q[1];
rz(2.126743196675387) q[0];
rz(3.1036637912772536) q[1];
rz(5.302680165630841) q[0];
cx q[1],q[0];
cx q[0],q[1];
cx q[1],q[0];
z q[1];
h q[0];
cx q[0],q[1];
z q[0];
x q[1];
cx q[0],q[1];
rx(5.405537860772761) q[1];
x q[0];
cx q[1],q[0];
cx q[1],q[0];
x q[0];
z q[1];
cx q[1],q[0];
y q[1];
rz(1.1136445039362157) q[0];
cx q[1],q[0];
cx q[0],q[1];
ry(0.813289007755062) q[0];
y q[1];
cx q[1],q[0];
x q[1];
rz(3.9847763271868635) q[0];
ry(5.320968985019486) q[1];
z q[0];
z q[0];
x q[1];
cx q[1],q[0];
cx q[1],q[0];
x q[0];
rz(1.1932885602898422) q[1];
cx q[1],q[0];
rx(4.539329591841239) q[1];
rx(4.367655214912689) q[0];
cx q[1],q[0];
y q[1];
y q[0];
cx q[1],q[0];
x q[1];
x q[0];
cx q[1],q[0];
cx q[0],q[1];
cx q[1],q[0];
rx(0.06847013461586408) q[1];
x q[0];
cx q[1],q[0];
z q[1];
ry(5.4504764542097695) q[0];
rz(1.840639830138884) q[0];
x q[1];
ry(5.314311652290858) q[0];
y q[1];
rz(1.692609646790401) q[1];
x q[0];
cx q[0],q[1];
