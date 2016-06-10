/**
 * \file octree_orig.cpp
 *
 * \date 2016-06-08
 * \author consultit
 */

#include <pandaFramework.h>
#include <load_prc_file.h>
#include <geomLines.h>
#include <meshDrawer.h>
#include "octree_orig.h"

using namespace std;

#include "data.h"

///forward declaration
class Collider;
///functions' declarations
void cleanOnExit(const Event* e, void* data);
void testCollision(Collider *pA, Collider *pB);

///global data
PandaFramework framework;
WindowFramework *window;
MeshDrawer* generator;
//some values and declaration
OctreeNode<Collider>* octree;
LPoint3f WORLDCENTER(0.0, 0.0, 0.0);
float WORLDWIDTH = 200.0, WORLDHALFWIDTH;
int OBJECTSNUM = 3;
list<Collider> entities;
float OBJECTMAXRADIUS = 5.0;
float OBJECTMAXSPEED = 10.0;
AsyncTask::DoneStatus updateAndTestCollisions(GenericAsyncTask* task,
		void* data);

class Collider
{
public:
	Collider(const string& name);
	~Collider();

	// getters/setters
	string get_name() const;
	void setName(const string&);
	LPoint3f getCenter() const;
	void setCenter(const LPoint3f&);
	float getRadius() const;
	void setRadius(float);
	LVector3f getSpeed() const;
	void setSpeed(const LVector3f&);
	OctreeNode<Collider>* getPNode() const;
	void setPNode(OctreeNode<Collider>*);

	// Update position
	void update(float dt, LPoint3f worldCenter, float maxWidth);

	// Drawing stuff
	void addGeometry(NodePath parent, WindowFramework* window,
			PandaFramework& panda);
	NodePath getGeometry() const;

private:
	// Center point for entity
	LPoint3f center;
	// Radius of entity bounding sphere
	float radius;
	// Speed
	LVector3f speed;
	// name
	string name;
	// Current container OctreeNode
	OctreeNode<Collider>* pNode;
	// Drawing stuff
	NodePath geometry;
};

ostream &operator <<(ostream &out, const Collider & entity);

int main(int argc, char *argv[])
{
	// Load your configuration
	load_prc_file_data("", "model-path " + dataDir);
	load_prc_file_data("", "win-size 1024 768");
	load_prc_file_data("", "show-frame-rate-meter #t");
	load_prc_file_data("", "sync-video #t");
	load_prc_file_data("", "lock-to-one-cpu 0");
	load_prc_file_data("", "support-threads 1");

	//open a new window framework
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
	octree = new OctreeNode<Collider>(&testCollision);
	octree->setCenter(WORLDCENTER);
	octree->setHalfWidth(WORLDHALFWIDTH);
	octree->setIndex(-1);
	octree->setParent(NULL);
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
				Collider(
						string("entity")
								+ static_cast<std::ostringstream&>(std::ostringstream().operator <<(
										i)).str()));
		// Entity starts by being (possibly) contained into octree root
		entities.back().setPNode(octree);
		// set random position (between +/-(WORLDWIDTH/2.0 - radius))
		// set random speed (between +/- OBJECTMAXSPEED)
		LPoint3f center;
		LVector3f speed;
		for (auto j = 0; j < 3; ++j)
		{
			// position (-1.0 <= rnd <= 1.0)
			rnd = (1.0 - 2.0 * (float) rd() / (float) rangeRND);
			center[j] = rnd * (octree->getHalfWidth() - OBJECTMAXRADIUS * 1.1);
			// speed (-0.5 <= rnd <= 0.5)
			rnd = 0.5 - (float) rd() / (float) rangeRND;
			rnd >= 0.0 ?
					speed[j] = (1.0 - rnd) * OBJECTMAXSPEED :
					speed[j] = (-1.0 - rnd) * OBJECTMAXSPEED;
		}
		entities.back().setCenter(center);
		entities.back().setSpeed(speed);
		// set radius (0.5 <= rnd <= 1.0)
		rnd = (1.0 - 0.5 * (float) rd() / (float) rangeRND);
		entities.back().setRadius(rnd * OBJECTMAXRADIUS);
		// add geometry
		entities.back().addGeometry(window->get_render(), window, framework);
	}

	// Entities' updates and test all collisions
	AsyncTask* task = new GenericAsyncTask("updateAndTestCollisions",
			&updateAndTestCollisions, reinterpret_cast<void*>(octree));
	framework.get_task_mgr().add(task);
	// clean on exit
	window->get_graphics_window()->set_close_request_event(
			"close_request_event");
	framework.define_key("close_request_event", "cleanOnExit", &cleanOnExit,
			(void*) task);

	//do the main loop, equal to run() in python
	framework.main_loop();

	return (0);
}

AsyncTask::DoneStatus updateAndTestCollisions(GenericAsyncTask* task,
		void* data)
{
	float dt = ClockObject::get_global_clock()->get_dt();

	//reset generator
	generator->begin(window->get_camera_group().get_child(0),
			window->get_render());

	// First: update Entities' positions
	list<Collider>::iterator entityIt;
	for (entityIt = entities.begin(); entityIt != entities.end(); ++entityIt)
	{
		entityIt->update(dt, WORLDCENTER, WORLDHALFWIDTH);
	}

	///Tests for collisions
	// Update octree content: cycle over Entities
	for (entityIt = entities.begin(); entityIt != entities.end(); ++entityIt)
	{
		// Try first to insert Entity into its current container octree node
		OctreeNode<Collider> *containerNode = entityIt->getPNode();
		while (not containerNode->insertEntity(/*containerNode,*/&(*entityIt)))
		{
			// Try current container octree node's parent, if any
			if (containerNode->getParent())
			{
				containerNode = containerNode->getParent();
			}
			else
			{
				// Entity cannot be inserted into this octree
				break;
			}
		}
	}
	// Test collisions
	OctreeNode<Collider>* octree =
			reinterpret_cast<OctreeNode<Collider>*>(data);

	// draw check collision's feedback if any
	generator->begin(window->get_camera_group().get_child(0),
			window->get_render());
	octree->performAllTests();
	generator->end();

	return AsyncTask::DS_again;
}

void cleanOnExit(const Event* e, void* data)
{
	AsyncTask* task = reinterpret_cast<AsyncTask*>(data);
	framework.get_task_mgr().remove(task);

	// Remove the octree
	delete octree;
#ifdef UT_DEBUG
	assert(OctreeNode<Collider>::allocatedNum == 0);
#endif //UT_DEBUG

	// delete drawer
	delete generator;

	//close the window framework
	framework.close_framework();
	//
	exit(0);

}

void testCollision(Collider *pA, Collider *pB)
{
//	cout << *pA << " <---> " << *pB << endl;
	nassertv_always(!pA->getGeometry().is_empty());
	nassertv_always(!pB->getGeometry().is_empty());

	generator->segment(pA->getGeometry().get_pos(), pB->getGeometry().get_pos(),
			LVector4f(0, 0, 1, 1), 0.5, LColorf(0, 1, 0, 1));
}

//Entity
Collider::Collider(const string& name) :
		radius(0), pNode(NULL)
{
	this->name = name;
}

Collider::~Collider()
{
	if (not geometry.is_empty())
	{
		geometry.remove_node();
	}
}

// getters
string Collider::get_name() const
{
	return name;
}

LPoint3f Collider::getCenter() const
{
	return center;
}

void Collider::setCenter(const LPoint3f& value)
{
	center = value;
}

float Collider::getRadius() const
{
	return radius;
}

void Collider::setRadius(float value)
{
	radius = value;
}

LVector3f Collider::getSpeed() const
{
	return speed;
}

void Collider::setSpeed(const LVector3f& value)
{
	speed = value;
}

OctreeNode<Collider>* Collider::getPNode() const
{
	return pNode;
}

void Collider::setPNode(OctreeNode<Collider>* value)
{
	pNode = value;
}

// Update position
void Collider::update(float dt, LPoint3f worldCenter, float maxWidth)
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

void Collider::addGeometry(NodePath parent, WindowFramework* window,
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
	geomRadius = max(max(geomDims.get_x(), geomDims.get_y()), geomDims.get_z())
			/ 2.0;
	geometry.set_scale(radius / geomRadius);
	LVector3f dir = speed;
	dir.normalize();
	geometry.set_pos_quat(center, LOrientationf(dir, 0.0));
}

NodePath Collider::getGeometry() const
{
	return geometry;
}

ostream &operator <<(ostream &out, const Collider & entity)
{
	out << entity.get_name();
	return out;
}
