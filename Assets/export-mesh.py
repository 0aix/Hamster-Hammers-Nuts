#!/usr/bin/env python

import sys

import bpy
import struct
import mathutils
import bmesh

args = None
for arg in sys.argv:
	if arg == '--':
		args = []
	elif args != None:
		args.append(arg)

if args == None:
	args = []

if len(args) == 0:
	print("Usage:\n\tblender --background --python export-mesh.py -- <in.blend> ...")
	exit(1)

mesh_names = dict()
mesh_action_names = dict()

bone_data = b''
bone_name_to_idx = dict()

#write bind pose info for bone, return packed index:
def write_bone(bone):
	global bone_data

	if bone == None: return struct.pack('i', -1)
	if bone.name in bone_name_to_idx: return bone_name_to_idx[bone.name]
	#bone will be stored as:
	bone_data += write_bone(bone.parent) #parent (index, or -1)

	#bind matrix inverse as 3-row, 4-column matrix:
	transform = bone.matrix_local.copy()
	transform.invert()
	#Note: store *column-major*:
	bone_data += struct.pack('3f', transform[0].x, transform[1].x, transform[2].x)
	bone_data += struct.pack('3f', transform[0].y, transform[1].y, transform[2].y)
	bone_data += struct.pack('3f', transform[0].z, transform[1].z, transform[2].z)
	bone_data += struct.pack('3f', transform[0].w, transform[1].w, transform[2].w)

	#finally, record bone index:
	bone_name_to_idx[bone.name] = struct.pack('i', len(bone_name_to_idx))

for arg in args:
	bpy.ops.wm.open_mainfile(filepath=arg)
	for obj in bpy.data.objects:
		#check if obj has a mesh
		if obj.data.name not in bpy.data.meshes:
			continue

		#data contains vertex and normal data from the meshes:
		data = b''
		obj_name = obj.name.lower()

		#check if name was used already
		if obj_name in mesh_names:
			input('Error:' + obj_name + ' appears in ' + mesh_names[obj_name] + ' and ' + arg)
			exit(1)
		mesh_names[obj_name] = arg

		obj.data = obj.data.copy() #make mesh single user, just in case it is shared with another object the script needs to write later.

		#make sure object is on a visible layer:
		bpy.context.scene.layers = obj.layers
		#select the object and make it the active object:
		bpy.ops.object.select_all(action='DESELECT')
		obj.select = True
		bpy.context.scene.objects.active = obj

		armature = obj.find_armature()
		if armature != None:
			armature.data = armature.data.copy()

			bone_data = b''
			bone_name_to_idx = dict()
			for bone in armature.data.bones:
				write_bone(bone)

			assert(len(bone_name_to_idx) * (4 + 3 * 4 + 3 * 4 + 3 * 4 + 3 * 4) == len(bone_data))
			outfile = open('skeletons/' + obj_name + '.skn', 'wb')
			outfile.write(struct.pack('I', len(bone_name_to_idx)))
			outfile.write(bone_data)
			outfile.close()

			idx_to_bone_name = [-1] * len(bone_name_to_idx)
			for kv in bone_name_to_idx.items():
				idx = struct.unpack('i', kv[1])[0]
				assert(idx_to_bone_name[idx] == -1)
				idx_to_bone_name[idx] = kv[0]

			for action in bpy.data.actions:
				action_data = b''
				action_name = obj_name + '_' + action.name.lower()

				if action_name in mesh_action_names:
					input('Error:' + action_name + ' appears in ' + mesh_action_names[action_name] + ' and ' + arg)
					exit(1)
				mesh_action_names[action_name] = arg

				armature.animation_data.action = action
				first = round(action.frame_range[0])
				last = round(action.frame_range[1])

				frame_data = b''
				frame_count = 0
				for frame in range(first, last + 1):
					bpy.context.scene.frame_set(frame, 0.0) #note: second param is sub-frame
					frame_count += 1
					for name in idx_to_bone_name:
						pose_bone = armature.pose.bones[name]
						if pose_bone.parent != None:
							to_parent = pose_bone.parent.matrix.copy()
							to_parent.invert()
							local_to_parent = to_parent * pose_bone.matrix
						else:
							local_to_parent = armature.matrix_world.copy() * pose_bone.matrix

						trs = local_to_parent.decompose()
						frame_data += struct.pack('fff', trs[0].x, trs[0].y, trs[0].z)
						frame_data += struct.pack('ffff', trs[1].x, trs[1].y, trs[1].z, trs[1].w)
						frame_data += struct.pack('fff', trs[2].x, trs[2].y, trs[2].z)

				assert(frame_count * (3 * 4 + 4 * 4 + 3 * 4) * len(idx_to_bone_name) == len(frame_data))
				outfile = open('animations/' + action_name + '.anim', 'wb')
				outfile.write(struct.pack('I', frame_count))
				outfile.write(struct.pack('I', len(idx_to_bone_name)))
				outfile.write(frame_data)
				outfile.close()

		if armature != None:
			armature.data.pose_position = 'REST'
			bpy.context.scene.update()
			mesh = obj.to_mesh(bpy.context.scene, True, 'RENDER')
			bm = bmesh.new()
			bm.from_mesh(mesh)
			bmesh.ops.triangulate(bm, faces=bm.faces[:], quad_method=3, ngon_method=1)
			bm.to_mesh(mesh)
			bm.free()
			armature.data.pose_position = 'POSE'
		else:
			#subdivide object's mesh into triangles:
			bpy.ops.object.mode_set(mode='EDIT')
			bpy.ops.mesh.select_all(action='SELECT')
			bpy.ops.mesh.quads_convert_to_tris(quad_method='BEAUTY', ngon_method='BEAUTY')
			bpy.ops.object.mode_set(mode='OBJECT')
			mesh = obj.data

		#compute normals (respecting face smoothing):
		#if armature == None:
		#	mesh = obj.data
		#if armature != None:
		#	mesh = obj.to_mesh(bpy.context.scene, True, 'RENDER')

		mesh.calc_normals_split()

		colors = None
		if len(mesh.vertex_colors) != 0:
			colors = mesh.vertex_colors.active.data

		uvs = None
		if len(mesh.uv_layers) != 0:
			uvs = mesh.uv_layers.active.data

		xf = mathutils.Matrix()
		if armature != None:
			obj_to_arm = armature.matrix_world.copy()
			obj_to_arm.invert()
			obj_to_arm = obj_to_arm * obj.matrix_world
			xf = obj_to_arm

		itxf = xf.copy()
		itxf.transpose()
		itxf.invert()

		#write the mesh:
		vertex_count = len(mesh.polygons) * 3
		outfile = open('meshes/' + obj_name + '.mesh', 'wb')
		outfile.write(struct.pack('I', vertex_count))

		print(len(mesh.polygons))
		j = 0
		for poly in mesh.polygons:
			j += 1
			if j % 1000 == 0:
				print(j)
			assert(len(poly.loop_indices) == 3)
			data = b''
			for i in range(0, 3):
				assert(mesh.loops[poly.loop_indices[i]].vertex_index == poly.vertices[i])
				loop = mesh.loops[poly.loop_indices[i]]
				vertex = mesh.vertices[loop.vertex_index]
				position = xf * vertex.co
				normal = itxf * loop.normal
				normal.normalize()
				data += struct.pack('fff', *position)
				data += struct.pack('fff', *normal)
				if colors != None:
					data += struct.pack('fff', *(colors[poly.loop_indices[i]].color))
				else:
					data += struct.pack('fff', 0, 0, 0)
				if uvs != None:
					data += struct.pack('ff', *(uvs[poly.loop_indices[i]].uv))
				else:
					data += struct.pack('ff', 0, 0)
				if armature != None:
					bone_weights = []
					for g in vertex.groups:
						group_name = obj.vertex_groups[g.group].name
						assert(group_name in bone_name_to_idx)
						bone_weights.append([g.weight, group_name])
					bone_weights.sort()
					bone_weights.reverse()
					if len(bone_weights) > 4:
						print("WARNING: clamping vertex with weights:")
						for bw in bone_weights:
							print("  " + str(bw[0]) + " for '" + bw[1] + "'")
						#trim and normalize remaining weights:
						bone_weights = bone_weights[0:4]
						total = 0.0
						for bw in bone_weights:
							total += bw[0]
						for bw in bone_weights:
							bw[0] /= total
					if len(bone_weights) == 0:
						print("WARNING: vertex with no bone weights.")

					while len(bone_weights) < 4:
						bone_weights.append([0, idx_to_bone_name[0]])

					for bw in bone_weights:
						data += struct.pack('f', bw[0])

					for bw in bone_weights:
						data += bone_name_to_idx[bw[1]]
					'''
					bone_weights = []
					for g in vertex.groups:
						bone_weights.append([g.weight, g.group])

					bone_weights.sort()
					bone_weights.reverse()
					
					if len(bone_weights) == 0:
						input("ERROR: vertex with no bone weights.")
						exit(1)
					if len(bone_weights) > 4:
						#trim and normalize remaining weights:
						print(bone_weights[0])
						print(bone_weights[1])
						print(bone_weights[2])
						print(bone_weights[3])
						print(' ')
						bone_weights = bone_weights[0:4]
						total = 0.0
						for bw in bone_weights:
							total += bw[0]
						for bw in bone_weights:
							bw[0] /= total
					while len(bone_weights) < 4:
						bone_weights.append([0, 0])
					for bw in bone_weights:
						data += struct.pack('f', bw[0])
					for bw in bone_weights:
						data += struct.pack('I', bw[1])
					'''
				else:
					data += struct.pack('ffff', 0, 0, 0, 0)
					data += struct.pack('IIII', 0, 0, 0, 0)
			#assert(3 * 4 + 3 * 4 + 3 * 4 + 2 * 4 + 4 * 4 + 4 * 4 == len(data))
			outfile.write(data)		

		outfile.close()

input('done')