/**
 * \file octree_orig.h
 *
 * \date 2016-06-08
 * \author consultit
 */

#ifndef OCTREE_ORIG_H_
#define OCTREE_ORIG_H_

#include <nodePath.h>
#ifdef UT_DEBUG
#include <geomLines.h>
#include <geomVertexWriter.h>
#endif //UT_DEBUG

using namespace std;

double (*Abs)(double) = &abs;

/// OctreeNode data structure
template<typename Entity> class OctreeNode
{
public:
	OctreeNode(void (*callback)(Entity *, Entity *));
	~OctreeNode();

	// getters/setters
	LPoint3f getCenter();
	void setCenter(const LPoint3f&);
	float getHalfWidth();
	void setHalfWidth(float);
	OctreeNode *getParent();
	void setParent(OctreeNode *);
	int getIndex();
	void setIndex(int);

	void addGeometry(NodePath parent);
	NodePath getGeomParent();

	bool insertEntity(Entity *pEntity);
	bool deleteNode(OctreeNode* octree);

	void performAllTests();

	void setTestCallback(void (*)(Entity *, Entity *));

private:
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
	// Entity callback
	void (*testCallback)(Entity *, Entity *);

#ifdef UT_DEBUG
	// Drawing stuff
	NodePath geometry, geomParent;
	// check allocation
public:
	static int allocatedNum;
#endif //UT_DEBUG
};

///
template<typename Entity> OctreeNode<Entity>::OctreeNode(
		void (*callback)(Entity *, Entity *)) :
		testCallback(callback), pParent(NULL), halfWidth(0), index(0)
{
#ifdef UT_DEBUG
	cout << ++allocatedNum << endl;
#endif //UT_DEBUG
}

template<typename Entity> OctreeNode<Entity>::~OctreeNode()
{
	//deallocate children
	for (typename map<int, OctreeNode*>::const_iterator childrenIt =
			pChildren.begin(); childrenIt != pChildren.end(); ++childrenIt)
	{
		delete childrenIt->second;
	}

#ifdef UT_DEBUG
	if (not geometry.is_empty())
	{
		geometry.remove_node();
	}
	cout << --allocatedNum << endl;
#endif //UT_DEBUG
}

template<typename Entity> LPoint3f OctreeNode<Entity>::getCenter()
{
	return center;
}

template<typename Entity> void OctreeNode<Entity>::setCenter(
		const LPoint3f& value)
{
	center = value;
}

template<typename Entity> float OctreeNode<Entity>::getHalfWidth()
{
	return halfWidth;
}

template<typename Entity> void OctreeNode<Entity>::setHalfWidth(float value)
{
	halfWidth = value;
}

template<typename Entity> OctreeNode<Entity> *OctreeNode<Entity>::getParent()
{
	return pParent;
}

template<typename Entity> void OctreeNode<Entity>::setParent(OctreeNode *value)
{
	pParent = value;
}

template<typename Entity> int OctreeNode<Entity>::getIndex()
{
	return index;
}

template<typename Entity> void OctreeNode<Entity>::setIndex(int value)
{
	index = value;
}

template<typename Entity> void OctreeNode<Entity>::addGeometry(NodePath parent)
{
#ifdef UT_DEBUG
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
#endif //UT_DEBUG
}

template<typename Entity> NodePath OctreeNode<Entity>::getGeomParent()
{
#ifdef UT_DEBUG
	return geomParent;
#else
	return NodePath();
#endif //UT_DEBUG
}

template<typename Entity> bool OctreeNode<Entity>::insertEntity(Entity *pEntity)
{
	// Check if Entity is (partially) outside tree
	float delta = halfWidth - pEntity->getRadius();
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
			if (Abs(pEntity->getCenter()[i] - center[i]) >= delta)
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
		float delta = pEntity->getCenter()[i] - center[i];
		if (Abs(delta) <= pEntity->getRadius())
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
		if (pChildren.find(index) == pChildren.end())
		{
			// Children[index] OctreeNode doesn't exist: create one
			LVector3f offset;
			float step = halfWidth * 0.5f;
			offset.set_x((index & 1) ? step : -step);
			offset.set_y((index & 2) ? step : -step);
			offset.set_z((index & 4) ? step : -step);
			OctreeNode* node = new OctreeNode(this->testCallback);
			node->center = center + offset;
			node->halfWidth = step;
			node->index = index;
			node->pParent = this;
			node->addGeometry(getGeomParent());
			pChildren[index] = node;
		}
		// Fully contained in existing Children[index] OctreeNode; insert in
		// that subtree
		return pChildren[index]->insertEntity(/*pChildren[index],*/pEntity);
	}
	else
	{
		// Straddling so add entity into Children of this OctreeNode (if not
		// already present)
		pEntities.insert(pEntity);
		// Update the Entity's current container OctreeNode, if needed
		if (pEntity->getPNode() != this)
		{
			// pTree is the new container
			// Remove Entity from current container OctreeNode
			pEntity->getPNode()->pEntities.erase(pEntity);

			// Try to delete current container octree node and
			// its parents, where possible
			OctreeNode* toDeleteNode = pEntity->getPNode();
			while (toDeleteNode and deleteNode(toDeleteNode))
			{
				toDeleteNode = toDeleteNode->pParent;
			}

			// Update current container OctreeNode
			pEntity->setPNode(this);
		}
	}
	return true;
}

//
template<typename Entity> bool OctreeNode<Entity>::deleteNode(
		OctreeNode* octree)
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

// Tests all entities that could possibly overlap due to cell ancestry and
// coexistence in the same cell. Assumes entities exist in a single cell only,
// and fully inside it
template<typename Entity> void OctreeNode<Entity>::performAllTests()
{
	// Keep track of all ancestor entity lists in a stack
	const int MAX_DEPTH = 40;
	static OctreeNode *ancestorStack[MAX_DEPTH];
	static int depth = 0; // ’Depth == 0’ is invariant over calls
	// Check collision between all entities on this level and all
	// ancestor entities. The current level is included as its own
	// ancestor so all necessary pairwise tests are done
	ancestorStack[depth++] = this;
	for (int n = 0; n < depth; n++)
	{
		Entity *pA, *pB;
		typename set<Entity*>::const_iterator ancestorIt, treeIt;
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
				// Now perform the collision test between pA and pB in some
				// manner
				testCallback(pA, pB);
			}
		}
	}
	// Recursively visit all existing children
	for (typename map<int, OctreeNode*>::const_iterator childrenIt = pChildren.begin();
			childrenIt != pChildren.end(); ++childrenIt)
	{
		childrenIt->second->performAllTests();
	}

	// Remove current octree node from ancestor stack before returning
	depth--;
}

template<typename Entity> void OctreeNode<Entity>::setTestCallback(
		void (*callback)(Entity *, Entity *))
{
	testCallback = callback;
}

#ifdef UT_DEBUG
template<typename Entity> int OctreeNode<Entity>::allocatedNum = 0;
#endif //UT_DEBUG

#endif /* OCTREE_ORIG_H_ */
