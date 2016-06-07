/**
 * \file utOctree.cxx
 *
 * \date 2016-06-07
 * \author consultit
 */

#if !defined(CPPPARSER) && defined(_WIN32)
#include "support/pstdint.h"
#endif

#include "utOctree.h"

/**
 *
 */
UTOctree::UTOctree(const string& name):PandaNode(name)
{
}

/**
 *
 */
UTOctree::~UTOctree()
{
}


/**
 * Writes a sensible description of the RNCrowdAgent to the indicated output
 * stream.
 */
void UTOctree::output(ostream &out) const
{
	out << get_type() << " " << get_name();
}

//TypedWritable API
/**
 * Tells the BamReader how to create objects of type RNCrowdAgent.
 */
void UTOctree::register_with_read_factory()
{
	BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void UTOctree::write_datagram(BamWriter *manager, Datagram &dg)
{
	PandaNode::write_datagram(manager, dg);

	///Name of this RNCrowdAgent.
	dg.add_string(get_name());

}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int UTOctree::complete_pointers(TypedWritable **p_list, BamReader *manager)
{
	int pi = PandaNode::complete_pointers(p_list, manager);

	return pi;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type RNCrowdAgent is encountered in the Bam file.  It should create the
 * RNCrowdAgent and extract its information from the file.
 */
TypedWritable *UTOctree::make_from_bam(const FactoryParams &params)
{
	UTOctree *node;

	DatagramIterator scan;
	BamReader *manager;

	parse_params(params, scan, manager);
	node->fillin(scan, manager);

	return node;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new RNCrowdAgent.
 */
void UTOctree::fillin(DatagramIterator &scan, BamReader *manager)
{
	PandaNode::fillin(scan, manager);

	///Name of this RNCrowdAgent. string mName;
	set_name(scan.get_string());

}

//TypedObject semantics: hardcoded
TypeHandle UTOctree::_type_handle;
