OPENQASM 2.0;
include "qelib1.inc";
qreg q[2];
cx q[1],q[0];
cx q[0],q[1];
h q[0];
z q[1];
cx q[0],q[1];
x q[1];
z q[0];
cx q[0],q[1];
cx q[0],q[1];
cx q[0],q[1];
h q[1];
h q[0];
cx q[1],q[0];
cx q[0],q[1];
cx q[0],q[1];
cx q[0],q[1];
cx q[0],q[1];
ry(3.9744997991307547) q[1];
z q[0];
cx q[0],q[1];
ry(2.4189567783084356) q[1];
rx(3.2328682739805887) q[0];
cx q[1],q[0];
cx q[0],q[1];
cx q[1],q[0];
ry(3.223578008628288) q[1];
ry(1.069026587053876) q[0];
h q[0];
rx(6.200539809055617) q[1];
y q[0];
x q[1];
cx q[1],q[0];
cx q[0],q[1];
cx q[0],q[1];
h q[0];
z q[1];
rz(3.3589282559073372) q[0];
x q[1];
cx q[1],q[0];
h q[0];
ry(4.823390455431345) q[1];
x q[1];
h q[0];
cx q[1],q[0];
rx(5.996089426467483) q[1];
rz(4.312986589460665) q[0];
cx q[0],q[1];
z q[1];
ry(2.748937407029205) q[0];
y q[1];
h q[0];
cx q[0],q[1];
ry(2.660399953740003) q[1];
rz(0.5910032246589559) q[0];
h q[0];
z q[1];
cx q[0],q[1];
h q[1];
ry(4.229026678980416) q[0];
h q[1];
rz(3.8612997956115747) q[0];
cx q[0],q[1];
cx q[0],q[1];
y q[1];
h q[0];
y q[0];
z q[1];
cx q[1],q[0];
ry(5.463540112458704) q[1];
rx(3.3816480452702278) q[0];
cx q[0],q[1];
ry(2.26776696478622) q[1];
h q[0];
z q[1];
rz(4.216819296553356) q[0];
z q[0];
rx(0.013833907714362848) q[1];
h q[1];
y q[0];
y q[1];
y q[0];
cx q[0],q[1];
x q[0];
rz(0.5028682404161339) q[1];
rx(3.964280019973172) q[0];
h q[1];
z q[1];
rz(2.2903527671634443) q[0];
rz(0.16056358125058773) q[0];
rx(3.3726969322400704) q[1];
cx q[1],q[0];
cx q[1],q[0];
cx q[1],q[0];
x q[0];
rx(1.402205633976926) q[1];
z q[0];
h q[1];
rx(0.5793732262621191) q[1];
z q[0];
h q[1];
h q[0];
cx q[0],q[1];
h q[1];
ry(4.842248445375295) q[0];
cx q[1],q[0];
ry(1.613250698369882) q[0];
x q[1];
rz(4.658427407123963) q[0];
rz(4.659923858518324) q[1];
cx q[1],q[0];
cx q[0],q[1];
cx q[1],q[0];
cx q[0],q[1];
z q[0];
z q[1];
