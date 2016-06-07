/**
 * \file utOctree.h
 *
 * \date 2016-06-07
 * \author consultit
 */

#ifndef UTOCTREE_H_
#define UTOCTREE_H_

#include "utilities_includes.h"
#include "pandaNode.h"

/**
 * This class represents an octree.
 *
 * \see
 * 		- Ericson, Christer. Real-time Collision Detection. Amsterdam: Elsevier, 2005. Print.
 *
 * This is the UTOctree object.\n
 *
 * > **UTOctree text parameters**:
 * param | type | default | note
 * ------|------|---------|-----
 * | *param1*				|single| - | specified as
 * | *param2*				|single| - | -
 *
 * \note parts inside [] are optional.\n
 */
class EXPORT_CLASS UTOctree: public PandaNode
{
PUBLISHED:
	UTOctree(const string& name);
	virtual ~UTOctree();

	/**
	 * \name OUTPUT
	 */
	///@{
	void output(ostream &out) const;
	///@}

private:

public:
	/**
	 * \name TypedWritable API
	 */
	///@{
	static void register_with_read_factory();
	virtual void write_datagram(BamWriter *manager, Datagram &dg) override;
	virtual int complete_pointers(TypedWritable **plist, BamReader *manager) override;
	///@}

protected:
	static TypedWritable *make_from_bam(const FactoryParams &params);
	virtual void fillin(DatagramIterator &scan, BamReader *manager) override;

public:
	/**
	 * \name TypedObject API
	 */
	///@{
	static TypeHandle get_class_type()
	{
		return _type_handle;
	}
	static void init_type()
	{
		PandaNode::init_type();
		register_type(_type_handle, "UTOctree", PandaNode::get_class_type());
	}
	virtual TypeHandle get_type() const override
	{
		return get_class_type();
	}
	virtual TypeHandle force_init_type() override
	{
		init_type();
		return get_class_type();
	}
	///@}

private:
	static TypeHandle _type_handle;

};

INLINE ostream &operator << (ostream &out, const UTOctree & octree);

///inline
#include "utOctree.I"

#endif /* UTOCTREE_H_ */
