import argparse
import os

parser = argparse.ArgumentParser("obj_2_c")
parser.add_argument("obj_path", help="path to obj", type=str)
args = parser.parse_args()

print("OBJ2C : OBJ Path : " + args.obj_path)

input = str()
with open(args.obj_path, 'r') as file:
    input = file.read()

vert_positions = []
vert_normals = []
vert_uv = []
faces = []

for line in input.splitlines():
    parts = line.split(' ')
    if parts[0] == 'v':
        vert_positions.append(str(parts[1] + ',' + parts[2] + ',' + parts[3]))
    if parts[0] == 'vn':
        vert_normals.append(str(parts[1] + ',' + parts[2] + ',' + parts[3]))
    if parts[0] == 'vt':
        vert_uv.append(str(parts[1] + ',' + parts[2]))
    if parts[0] == 'f':
        faces.append(parts[1] + ',' + parts[2] + ',' + parts[3])

vert_data = []

for face_txt in faces:
    faces = face_txt.split(',')
    for indices in faces:
        indexes = indices.split('/')
        vert_index = int(indexes[0]) - 1
        uv_index = int(indexes[1]) - 1
        normal_index = int(indexes[2]) - 1
        vert_data.append(vert_positions[vert_index])
        vert_data.append(vert_normals[normal_index])
        vert_data.append(vert_uv[uv_index])

num_verts = int(len(vert_data) / 3)

name = os.path.basename(args.obj_path)
name = name.replace(".obj", "")

print("--- C/C++ Code ---")
print("float " + name + "_vertex_data[" + str(int(8 * num_verts)) + "] = {")
for data in vert_data:
    print("            " + data + ",")
print("};")
print("const unsigned int " + name + "_num_verts = " + str(num_verts) + ";")