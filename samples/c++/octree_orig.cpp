/*
 *   This file is part of Ely.
 *
 *   Ely is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Ely is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Ely.  If not, see <http://www.gnu.org/licenses/>.
 */
/**
 * \file /Ely/training/octree.cpp
 *
 * \date 2013-03-10 
 * \author consultit
 */

#include <pandaFramework.h>
#include <pandaSystem.h>
#include <load_prc_file.h>
#include <geomLines.h>
#include <meshDrawer.h>
#include <lpoint3.h>
#include <iostream>
#include <cmath>
#include <set>
#include <map>
#include <list>
#include <random>

using namespace std;

static string baseDir("/REPOSITORY/KProjects/WORKSPACE/Ely/elygame/");
MeshDrawer* generator;
WindowFramework *window;

namespace ely
{

struct OctreeNode;
struct Entity
{
	Entity(const string& name)
	{
		this->name = name;
	}

	~Entity()
	{
		if (not geometry.is_empty())
		{
			geometry.remove_node();
		}
	}

	// Center point for entity
	LPoint3f center;
	// Radius of entity bounding sphere
	float radius;
	// Speed
	LVector3f speed;
	// name
	string name;

	// return name
	string get_name() const
	{
		return name;
	}

	// Update position
	void update(float dt, LPoint3f worldCenter, float maxWidth)
	{
		// update position and speed
		bool speedChanged = false;
		for (auto j = 0; j < 3; ++j)
		{
			center[j] += speed[j] * dt;
			float delta = center[j] - worldCenter[j];
			if (delta > maxWidth)
			{
				center[j] = worldCenter[j] + 2 * maxWidth - delta;
				speed[j] = -speed[j];
				speedChanged = true;
			}
			else if (delta < -maxWidth)
			{
				center[j] = worldCenter[j] - 2 * maxWidth - delta;
				speed[j] = -speed[j];
				speedChanged = true;
			}
		}
		// Update drawing
		if (speedChanged)
		{
			LVector3f dir = speed;
			dir.normalize();
			geometry.set_pos_quat(center, LOrientationf(dir, 0.0));
		}
		else
		{
			geometry.set_pos(center);
		}
	}
	// Current container OctreeNode
	OctreeNode* pNode;

	// Drawing stuff
	NodePath geometry;
	void addGeometry(NodePath parent, WindowFramework* window,
			PandaFramework& panda)
	{
		geometry = window->load_model(panda.get_models(), "smiley");
		geometry.reparent_to(parent);
		// rescale to match radius (scale = radius / geomRadius)
		//get "tight" dimensions of model
		LPoint3f minP, maxP;
		LVecBase3f geomDims, delta;
		float geomRadius;
		geometry.calc_tight_bounds(minP, maxP);
		delta = (maxP - minP);
		geomDims = LVector3f(abs(delta.get_x()), abs(delta.get_y()),
				abs(delta.get_z()));
		geomRadius = max(max(geomDims.get_x(), geomDims.get_y()),
				geomDims.get_z()) / 2.0;
		geometry.set_scale(radius / geomRadius);
		LVector3f dir = speed;
		dir.normalize();
		geometry.set_pos_quat(center, LOrientationf(dir, 0.0));
	}
};

ostream &operator <<(ostream &out, const Entity & entity)
{
	out << entity.get_name();
	return out;
}

// Octree node data structure
struct OctreeNode
{
	~OctreeNode()
	{
		if (not geometry.is_empty())
		{
			geometry.remove_node();
		}
	}

	// Center point of octree node
	LPoint3f center;
	// Half the width of the octree node volume
	float halfWidth;
	// Parent
	OctreeNode *pParent;
	// Index relative to its Parent (0-7)
	int index;

	// Child octree node table indexed by index (0-7)
	map<int, OctreeNode*> pChildren;
	// Entity set
	set<Entity*> pEntities;

	// Drawing stuff
	NodePath geometry, geomParent;
	void addGeometry(NodePath parent)
	{
		// Unique name postfix
		const unsigned long int addr =
				reinterpret_cast<const unsigned long int>(this);
		std::string postfix =
				dynamic_cast<std::ostringstream&>(std::ostringstream().operator <<(
						addr)).str();
		//Create the Drawing geom
		//Defining your own GeomVertexFormat: v3c4
		PT(GeomVertexArrayFormat)array = new GeomVertexArrayFormat();
		array->add_column(InternalName::make("vertex"), 3, Geom::NT_float32,
				Geom::C_point);
		array->add_column(InternalName::make("color"), 4, Geom::NT_float32,
				Geom::C_color);
		PT(GeomVertexFormat)unregistered_format = new GeomVertexFormat();
		unregistered_format->add_array(array);
		CPT(GeomVertexFormat)format =
		GeomVertexFormat::register_format(unregistered_format);

		//Creating and filling a GeomVertexData
		PT(GeomVertexData)vdata = new
		GeomVertexData("OctreeNodeGeomVertexData_" + postfix, GeomVertexFormat::get_v3c4(), Geom::UH_static);
		vdata->set_num_rows(8);
		GeomVertexWriter vertex, color;
		vertex = GeomVertexWriter(vdata, "vertex");
		color = GeomVertexWriter(vdata, "color");
		LColor lineColor(1, 0, 0, 0);
		//000
		vertex.add_data3f(LVector3f(-halfWidth, -halfWidth, -halfWidth));
		color.add_data4f(lineColor);
		//001
		vertex.add_data3f(LVector3f(halfWidth, -halfWidth, -halfWidth));
		color.add_data4f(lineColor);
		//010
		vertex.add_data3f(LVector3f(-halfWidth, halfWidth, -halfWidth));
		color.add_data4f(lineColor);
		//011
		vertex.add_data3f(LVector3f(halfWidth, halfWidth, -halfWidth));
		color.add_data4f(lineColor);
		//100
		vertex.add_data3f(LVector3f(-halfWidth, -halfWidth, halfWidth));
		color.add_data4f(lineColor);
		//101
		vertex.add_data3f(LVector3f(halfWidth, -halfWidth, halfWidth));
		color.add_data4f(lineColor);
		//110
		vertex.add_data3f(LVector3f(-halfWidth, halfWidth, halfWidth));
		color.add_data4f(lineColor);
		//111
		vertex.add_data3f(LVector3f(halfWidth, halfWidth, halfWidth));
		color.add_data4f(lineColor);

		// Creating the GeomPrimitive entities
		PT(GeomLines)prim = new GeomLines(Geom::UH_static);
		prim->add_vertices(0, 1);
		prim->add_vertices(1, 3);
		prim->add_vertices(3, 2);
		prim->add_vertices(2, 0);
		prim->add_vertices(4, 5);
		prim->add_vertices(5, 7);
		prim->add_vertices(7, 6);
		prim->add_vertices(6, 4);
		prim->add_vertices(4, 0);
		prim->add_vertices(5, 1);
		prim->add_vertices(7, 3);
		prim->add_vertices(6, 2);

		// Putting your new geometry in the scene graph
		PT(Geom)geom = new Geom(vdata);
		geom->add_primitive(prim);
		PT(GeomNode)node = new GeomNode("OctreeNodeGeomNode_" + postfix);
		node->add_geom(geom);
		geomParent = parent;
		geometry = geomParent.attach_new_node(node);
		geometry.set_pos(center);
		geometry.set_render_mode_thickness(1);
	}
};

//
double (*Abs)(double) = &abs;
// Dynamic
bool insertEntity(OctreeNode *pTree, Entity *pEntity);
bool deleteOctreeNode(OctreeNode* octree);
void deleteOctree(OctreeNode* octree);
//
void testAllCollisions(OctreeNode *pTree);
void testCollision(Entity *pA, Entity *pB);

bool insertEntity(OctreeNode *pTree, Entity *pEntity)
{
	// Check if Entity is (partially) outside tree
	float delta = pTree->halfWidth - pEntity->radius;
	if (delta <= 0.0)
	{
		// radius >= halfWidth: Entity cannot be contained in tree
		return false;
	}
	else
	{
		// delta > 0.0: radius < halfWidth
		for (int i = 0; i < 3; i++)
		{
			if (Abs(pEntity->center[i] - pTree->center[i]) >= delta)
			{
				// Entity (partially) outside tree
				return false;
			}
		}
	}
	// Entity insertion
	int index = 0;
	bool straddle = false;
	// Compute the octant number [0..7] the entity sphere center is in
	// If straddling any of the dividing x, y, or z planes, exit directly
	for (int i = 0; i < 3; i++)
	{
		float delta = pEntity->center[i] - pTree->center[i];
		if (Abs(delta) <= pEntity->radius)
		{
			straddle = true;
			break;
		}
		else if (delta > 0.0f)
		{
			index |= (1 << i); // ZYX
		}
	}
	if (!straddle)
	{
		if (pTree->pChildren.find(index) == pTree->pChildren.end())
		{
			// Children[index] OctreeNode doesn't exist: create one
			LVector3f offset;
			float step = pTree->halfWidth * 0.5f;
			offset.set_x((index & 1) ? step : -step);
			offset.set_y((index & 2) ? step : -step);
			offset.set_z((index & 4) ? step : -step);
			OctreeNode* node = new OctreeNode;
			node->center = pTree->center + offset;
			node->halfWidth = step;
			node->index = index;
			node->pParent = pTree;
			node->addGeometry(pTree->geomParent);
			pTree->pChildren[index] = node;
		}
		// Fully contained in existing Children[index] OctreeNode; insert in that subtree
		return insertEntity(pTree->pChildren[index], pEntity);
	}
	else
	{
		// Straddling so add entity into Children of this OctreeNode (if not already present)
		pTree->pEntities.insert(pEntity);
		// Update the Entity's current container OctreeNode, if needed
		if (pEntity->pNode != pTree)
		{
			// pTree is the new container
			// Remove Entity from current container OctreeNode
			pEntity->pNode->pEntities.erase(pEntity);

			// Try to delete current container octree node and
			// its parents, where possible
			OctreeNode* toDeleteNode = pEntity->pNode;
			while (toDeleteNode and deleteOctreeNode(toDeleteNode))
			{
				toDeleteNode = toDeleteNode->pParent;
			}

			// Update current container OctreeNode
			pEntity->pNode = pTree;
		}
	}
	return true;
}
//
inline bool deleteOctreeNode(OctreeNode* octree)
{
	bool result = false;
	// Delete when it has parent and has no entities and has no children
	if ((octree->pParent) and (octree->pEntities.size() == 0)
			and (octree->pChildren.size() == 0))
	{
		// Remove OctreeNode from its parent's children
		octree->pParent->pChildren.erase(octree->index);
		// delete OctreeNode actually
		delete octree;
		//
		result = true;
	}
	return result;
}
//
void deleteOctree(OctreeNode* octree)
{
	for (map<int, OctreeNode*>::const_iterator childrenIt =
			octree->pChildren.begin(); childrenIt != octree->pChildren.end();
			++childrenIt)
	{
		deleteOctree(childrenIt->second);
	}

	delete octree;
	octree = NULL;
}

// Tests all entities that could possibly overlap due to cell ancestry and coexistence
// in the same cell. Assumes entities exist in a single cell only, and fully inside it
void testAllCollisions(OctreeNode *pTree)
{
	// Keep track of all ancestor entity lists in a stack
	const int MAX_DEPTH = 40;
	static OctreeNode *ancestorStack[MAX_DEPTH];
	static int depth = 0; // ’Depth == 0’ is invariant over calls
	// Check collision between all entities on this level and all
	// ancestor entities. The current level is included as its own
	// ancestor so all necessary pairwise tests are done
	ancestorStack[depth++] = pTree;
	for (int n = 0; n < depth; n++)
	{
		Entity *pA, *pB;
		set<Entity*>::const_iterator ancestorIt, treeIt;
		for (ancestorIt = ancestorStack[n]->pEntities.begin();
				ancestorIt != ancestorStack[n]->pEntities.end(); ++ancestorIt)
		{
			pA = *ancestorIt;
			for (treeIt = ancestorStack[n]->pEntities.begin();
					treeIt != ancestorStack[n]->pEntities.end(); ++treeIt)
			{
				pB = *treeIt;
				// Avoid testing both A->B and B->A
				if (pA == pB)
					break;
				// Now perform the collision test between pA and pB in some manner
				testCollision(pA, pB);
			}
		}
	}
	// Recursively visit all existing children
	for (map<int, OctreeNode*>::const_iterator childrenIt =
			pTree->pChildren.begin(); childrenIt != pTree->pChildren.end();
			++childrenIt)
	{
		testAllCollisions(childrenIt->second);
	}

	// Remove current octree node from ancestor stack before returning
	depth--;
}

void testCollision(Entity *pA, Entity *pB)
{
//	cout << *pA << " <---> " << *pB << endl;
	nassertv_always(!pA->geometry.is_empty());
	nassertv_always(!pB->geometry.is_empty());

	generator->segment(pA->geometry.get_pos(), pB->geometry.get_pos(),
			LVector4f(0, 0, 1, 1), 0.5, LColorf(0, 1, 0, 1));
}

} // ely

//some values and declaration
LPoint3f WORLDCENTER(0.0, 0.0, 0.0);
float WORLDWIDTH = 200.0, WORLDHALFWIDTH;
int OBJECTSNUM = 3;
list<ely::Entity> entities;
float OBJECTMAXRADIUS = 5.0;
float OBJECTMAXSPEED = 10.0;
AsyncTask::DoneStatus updateAndTestCollisions(GenericAsyncTask* task,
		void* data);

int main(int argc, char *argv[])
{
	// Load your configuration
	load_prc_file_data("", "model-path " + baseDir + "data/models");
	load_prc_file_data("", "model-path " + baseDir + "data/shaders");
	load_prc_file_data("", "model-path " + baseDir + "data/sounds");
	load_prc_file_data("", "model-path " + baseDir + "data/textures");
	load_prc_file_data("", "show-frame-rate-meter #t");
	load_prc_file_data("", "lock-to-one-cpu 0");
	load_prc_file_data("", "support-threads 1");
	load_prc_file_data("", "audio-buffering-seconds 5");
	load_prc_file_data("", "audio-preload-threshold 2000000");
	load_prc_file_data("", "sync-video #t");
	//open a new window framework
	PandaFramework framework;
	framework.open_framework(argc, argv);
	//set the window title to My Panda3D Window
	framework.set_window_title("My Panda3D Window");
	//open the window

	window = framework.open_window();
	if (window != (WindowFramework *) NULL)
	{
		std::cout << "Opened the window successfully!\n";
		// common setup
		window->enable_keyboard(); // Enable keyboard detection
		window->setup_trackball(); // Enable default camera movement
	}
	//setup camera trackball (local coordinate)
	NodePath tballnp = window->get_mouse().find("**/+Trackball");
	PT(Trackball)trackball = DCAST(Trackball, tballnp.node());
	trackball->set_pos(0, 500, 0);
	trackball->set_hpr(0, 15, 0);

	///here is room for your own code
	// get parameters
	int c;
	opterr = 0;
	while ((c = getopt(argc, argv, "w:o:r:s:")) != -1)
	{
		switch (c)
		{
		case 'w':
			WORLDWIDTH = atof(optarg);
			break;
		case 'o':
			OBJECTSNUM = atoi(optarg);
			break;
		case 'r':
			OBJECTMAXRADIUS = atof(optarg);
			break;
		case 's':
			OBJECTMAXSPEED = atof(optarg);
			break;
		case '?':
			if ((optopt == 'w') or (optopt == 'o') or (optopt == 'r')
					or (optopt == 's'))
				std::cerr << "Option " << optopt << " requires an argument.\n"
						<< std::endl;
			else if (isprint(optopt))
				std::cerr << "Unknown option " << optopt << std::endl;
			else
				std::cerr << "Unknown option character " << optopt << std::endl;
			return 1;
		default:
			abort();
		}
	}
	// set drawer
	generator = new MeshDrawer();
	generator->set_budget(1000);
	NodePath generatorNode = generator->get_root();
	generatorNode.reparent_to(window->get_render());
	generatorNode.set_depth_write(false);
	generatorNode.set_transparency(TransparencyAttrib::M_alpha);
	generatorNode.set_two_sided(true);
	generatorNode.set_bin("fixed", 0);
	generatorNode.set_light_off();

	//
	WORLDHALFWIDTH = WORLDWIDTH / 2.0;
	// Create the octree root node
	ely::OctreeNode* octree = new ely::OctreeNode;
	octree->center = WORLDCENTER;
	octree->halfWidth = WORLDHALFWIDTH;
	octree->index = -1;
	octree->pParent = NULL;
	octree->addGeometry(window->get_render());

	// Initialize randomness
	std::random_device rd("/dev/urandom");
	size_t rangeRND = rd.max() - rd.min();
	// Add Entities and initialize them
	for (auto i = 0; i < OBJECTSNUM; ++i)
	{
		float rnd;
		// Add entities
		entities.push_back(
				ely::Entity(
						string("entity")
								+ static_cast<std::ostringstream&>(std::ostringstream().operator <<(
										i)).str()));
		// Entity starts by being (possibly) contained into octree root
		entities.back().pNode = octree;
		// set random position (between +/-(WORLDWIDTH/2.0 - radius))
		// set random speed (between +/- OBJECTMAXSPEED)
		for (auto j = 0; j < 3; ++j)
		{
			// position (-1.0 <= rnd <= 1.0)
			rnd = (1.0 - 2.0 * (float) rd() / (float) rangeRND);
			entities.back().center[j] = rnd
					* (octree->halfWidth - OBJECTMAXRADIUS * 1.1);
			// speed (-0.5 <= rnd <= 0.5)
			rnd = 0.5 - (float) rd() / (float) rangeRND;
			rnd >= 0.0 ?
					entities.back().speed[j] = (1.0 - rnd) * OBJECTMAXSPEED :
					entities.back().speed[j] = (-1.0 - rnd) * OBJECTMAXSPEED;
		}
		// set radius (0.5 <= rnd <= 1.0)
		rnd = (1.0 - 0.5 * (float) rd() / (float) rangeRND);
		entities.back().radius = rnd * OBJECTMAXRADIUS;
		// add geometry
		entities.back().addGeometry(window->get_render(), window, framework);
	}

	// Entities' updates and test all collisions
	AsyncTask* task = new GenericAsyncTask("updateAndTestCollisions",
			&updateAndTestCollisions, reinterpret_cast<void*>(octree));
	framework.get_task_mgr().add(task);

	//do the main loop, equal to run() in python
	framework.main_loop();

	// Remove the octree
	deleteOctree(octree);

	// delete drawer
	delete generator;

	//close the window framework
	framework.close_framework();
	return (0);
}

AsyncTask::DoneStatus updateAndTestCollisions(GenericAsyncTask* task,
		void* data)
{
	float dt = ClockObject::get_global_clock()->get_dt();

	//reset generator
	generator->begin(window->get_camera_group().get_child(0), window->get_render());

	// First: update Entities' positions
	list<ely::Entity>::iterator entityIt;
	for (entityIt = entities.begin(); entityIt != entities.end(); ++entityIt)
	{
		entityIt->update(dt, WORLDCENTER, WORLDHALFWIDTH);
	}

	///Tests for collisions
	// Update octree content: cycle over Entities
	for (entityIt = entities.begin(); entityIt != entities.end(); ++entityIt)
	{
		// Try first to insert Entity into its current container octree node
		ely::OctreeNode *containerNode = entityIt->pNode;
		while (not insertEntity(containerNode, &(*entityIt)))
		{
			// Try current container octree node's parent, if any
			if (containerNode->pParent)
			{
				containerNode = containerNode->pParent;
			}
			else
			{
				// Entity cannot be inserted into this octree
				break;
			}
		}
	}
	// Test collisions
	ely::OctreeNode* octree = reinterpret_cast<ely::OctreeNode*>(data);

	// draw check collision's feedback if any
	generator->begin(window->get_camera_group().get_child(0), window->get_render());
	ely::testAllCollisions(octree);
	generator->end();

	return AsyncTask::DS_again;
}
