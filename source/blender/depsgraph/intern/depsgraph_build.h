/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2013 Blender Foundation.
 * All rights reserved.
 *
 * Original Author: Lukas Toenne
 * Contributor(s): None Yet
 *
 * ***** END GPL LICENSE BLOCK *****
 */

#ifndef __DEPSGRAPH_BUILD_H__
#define __DEPSGRAPH_BUILD_H__

#include "depsgraph_types.h"

#include "depsgraph_util_id.h"
#include "depsgraph_util_rna.h"
#include "depsgraph_util_string.h"

struct bConstraint;
struct ListBase;
struct ID;
struct FCurve;
struct Group;
struct Key;
struct Main;
struct Material;
struct MTex;
struct bNodeTree;
struct Object;
struct bPoseChannel;
struct Scene;
struct Tex;
struct World;

struct Depsgraph;
struct DepsNode;
struct DepsNodeHandle;
struct RootDepsNode;
struct SubgraphDepsNode;
struct IDDepsNode;
struct TimeSourceDepsNode;
struct ComponentDepsNode;
struct OperationDepsNode;

struct DepsgraphNodeBuilder {
	DepsgraphNodeBuilder(Main *bmain, Depsgraph *graph);
	~DepsgraphNodeBuilder();
	
	RootDepsNode *add_root_node();
	IDDepsNode *add_id_node(IDPtr id);
	TimeSourceDepsNode *add_time_source(IDPtr id);
	ComponentDepsNode *add_component_node(IDDepsNode *id_node, eDepsNode_Type comp_type, const string &comp_name = "");
	OperationDepsNode *add_operation_node(ComponentDepsNode *comp_node,
	                                      eDepsOperation_Type optype, DepsEvalOperationCb op, const string &description);
	OperationDepsNode *add_operation_node(IDDepsNode *id_node, eDepsNode_Type comp_type,
	                                      eDepsOperation_Type optype, DepsEvalOperationCb op, const string &description);
	
	void verify_entry_exit_operations();
	
	IDDepsNode *build_scene(Scene *scene);
	SubgraphDepsNode *build_subgraph(Group *group);
	void build_group(Group *group);
	IDDepsNode *build_object(Scene *scene, Object *ob);
	ComponentDepsNode *build_object_transform(Object *ob, IDDepsNode *ob_node);
	void build_object_constraints(Object *ob, IDDepsNode *ob_node);
	void build_pose_constraints(Object *ob, bPoseChannel *pchan, BoneComponentDepsNode *bone_node);
	void build_rigidbody(IDDepsNode *scene_node, Scene *scene);
	void build_particles(IDDepsNode *ob_node, Object *ob);
	void build_animdata(IDDepsNode *id_node);
	OperationDepsNode *build_driver(IDDepsNode *id_node, FCurve *fcurve);
	void build_ik_pose(ComponentDepsNode *bone_node, Object *ob, bPoseChannel *pchan, bConstraint *con);
	void build_splineik_pose(ComponentDepsNode *bone_node, Object *ob, bPoseChannel *pchan, bConstraint *con);
	void build_rig(IDDepsNode *ob_node, Object *ob);
	void build_shapekeys(Key *key);
	void build_obdata_geom(IDDepsNode *ob_node, IDDepsNode *obdata_node, Scene *scene, Object *ob);
	void build_camera(IDDepsNode *ob_node, IDDepsNode *obdata_node, Object *ob);
	void build_lamp(IDDepsNode *ob_node, IDDepsNode *obdata_node, Object *ob);
	void build_nodetree(DepsNode *owner_node, bNodeTree *ntree);
	void build_material(DepsNode *owner_node, Material *ma);
	void build_texture(DepsNode *owner_node, Tex *tex);
	void build_texture_stack(DepsNode *owner_node, MTex **texture_stack);
	void build_world(World *world);
	void build_compositor(IDDepsNode *scene_node, Scene *scene);
	
protected:
	void verify_entry_exit_operations(ComponentDepsNode *node);
	
private:
	Main *m_bmain;
	Depsgraph *m_graph;
};

struct RootKey
{
	RootKey() {}
};

struct TimeSourceKey
{
	TimeSourceKey() : id(NULL) {}
	TimeSourceKey(IDPtr id) : id(id) {}
	
	IDPtr id;
};

struct ComponentKey
{
	ComponentKey() : id(NULL), type(DEPSNODE_TYPE_UNDEFINED), name("") {}
	ComponentKey(IDPtr id, eDepsNode_Type type, const string &name = "") : id(id), type(type), name(name) {}
	
	IDPtr id;
	eDepsNode_Type type;
	string name;
};

struct OperationKey
{
	OperationKey() : id(NULL), component_type(DEPSNODE_TYPE_UNDEFINED), component_name(""), name("") {}
	OperationKey(IDPtr id, eDepsNode_Type component_type, const string &name) :
	    id(id), component_type(component_type), component_name(""), name(name)
	{}
	OperationKey(IDPtr id, eDepsNode_Type component_type, const string &component_name, const string &name) :
	    id(id), component_type(component_type), component_name(component_name), name(name)
	{}
	
	IDPtr id;
	eDepsNode_Type component_type;
	string component_name;
	string name;
};

struct RNAPathKey
{
	RNAPathKey(IDPtr id, const string &path);
	RNAPathKey(IDPtr id, const PointerRNA &ptr, PropertyRNA *prop);
	IDPtr id;
	PointerRNA ptr;
	PropertyRNA *prop;
};

struct DepsgraphRelationBuilder {
	DepsgraphRelationBuilder(Depsgraph *graph);
	
	template <typename KeyFrom, typename KeyTo>
	void add_relation(const KeyFrom &key_from, const KeyTo &key_to,
	                  eDepsRelation_Type type, const string &description);
	
	template <typename KeyType>
	void add_node_handle_relation(const KeyType &key_from, const DepsNodeHandle *handle,
	                              eDepsRelation_Type type, const string &description);
	
	void build_scene(Scene *scene);
	void build_object(Scene *scene, Object *ob);
	void build_object_parent(Object *ob);
	void build_constraints(Scene *scene, IDPtr id, eDepsNode_Type component_type, const string &component_subdata, ListBase *constraints);
	void build_animdata(IDPtr id);
	void build_driver(IDPtr id, FCurve *fcurve);
	void build_world(Scene *scene, World *world);
	void build_rigidbody(Scene *scene);
	void build_particles(Scene *scene, Object *ob);
	void build_ik_pose(Object *ob, bPoseChannel *pchan, bConstraint *con);
	void build_splineik_pose(Object *ob, bPoseChannel *pchan, bConstraint *con);
	void build_rig(Scene *scene, Object *ob);
	void build_shapekeys(IDPtr obdata, Key *key);
	void build_obdata_geom(Scene *scene, Object *ob);
	void build_camera(Object *ob);
	void build_lamp(Object *ob);
	void build_nodetree(IDPtr owner, bNodeTree *ntree);
	void build_material(IDPtr owner, Material *ma);
	void build_texture(IDPtr owner, Tex *tex);
	void build_texture_stack(IDPtr owner, MTex **texture_stack);
	void build_compositor(Scene *scene);
	
protected:
	RootDepsNode *find_node(const RootKey &key) const;
	TimeSourceDepsNode *find_node(const TimeSourceKey &key) const;
	ComponentDepsNode *find_node(const ComponentKey &key) const;
	OperationDepsNode *find_node(const OperationKey &key) const;
	DepsNode *find_node(const RNAPathKey &key) const;
	
	void add_operation_relation(OperationDepsNode *node_from, OperationDepsNode *node_to,
	                            eDepsRelation_Type type, const string &description);
	
	template <typename KeyType>
	DepsNodeHandle create_node_handle(const KeyType &key, const string &default_name = "");
	
private:
	Depsgraph *m_graph;
};

struct DepsNodeHandle {
	DepsNodeHandle(DepsgraphRelationBuilder *builder, OperationDepsNode *node, const string &default_name = "") :
	    builder(builder),
	    node(node),
	    default_name(default_name)
	{}
	
	DepsgraphRelationBuilder *builder;
	OperationDepsNode *node;
	const string &default_name;
};

/* Inline Function Templates -------------------------------------------------- */

#include "depsnode_component.h"

template <class NodeType>
static OperationDepsNode *get_entry_operation(NodeType *node)
{ return NULL; }

template <class NodeType>
static OperationDepsNode *get_exit_operation(NodeType *node)
{ return NULL; }

template <> OperationDepsNode *get_entry_operation(OperationDepsNode *node)
{ return node; }

template <> OperationDepsNode *get_exit_operation(OperationDepsNode *node)
{ return node; }

template <> OperationDepsNode *get_entry_operation(ComponentDepsNode *node)
{ return node ? node->entry_operation : NULL; }

template <> OperationDepsNode *get_exit_operation(ComponentDepsNode *node)
{ return node ? node->exit_operation : NULL; }

template <typename KeyFrom, typename KeyTo>
void DepsgraphRelationBuilder::add_relation(const KeyFrom &key_from, const KeyTo &key_to,
                                            eDepsRelation_Type type, const string &description)
{
	OperationDepsNode *op_from = get_exit_operation(find_node(key_from));
	OperationDepsNode *op_to = get_entry_operation(find_node(key_to));
	if (op_from && op_to) {
		add_operation_relation(op_from, op_to, type, description);
	}
	else {
		if (!op_from) {
			/* XXX TODO handle as error or report if needed */
		}
		if (!op_to) {
			/* XXX TODO handle as error or report if needed */
		}
	}
}

template <typename KeyType>
void DepsgraphRelationBuilder::add_node_handle_relation(const KeyType &key_from, const DepsNodeHandle *handle,
                                                        eDepsRelation_Type type, const string &description)
{
	OperationDepsNode *op_from = get_exit_operation(find_node(key_from));
	OperationDepsNode *op_to = get_entry_operation(handle->node);
	if (op_from && op_to) {
		add_operation_relation(op_from, op_to, type, description);
	}
	else {
		if (!op_from) {
			/* XXX TODO handle as error or report if needed */
		}
		if (!op_to) {
			/* XXX TODO handle as error or report if needed */
		}
	}
}

template <typename KeyType>
DepsNodeHandle DepsgraphRelationBuilder::create_node_handle(const KeyType &key, const string &default_name)
{
	return DepsNodeHandle(this, find_node(key), default_name);
}


#endif // __DEPSGRAPH_BUILD_H__
