/**
 * \file utOctree.I
 *
 * \date 2016-06-07
 * \author consultit
 */

#ifndef UTOCTREE_I_
#define UTOCTREE_I_

///RNCrowdAgent inline definitions

INLINE ostream &operator <<(ostream &out, const UTOctree & octree)
{
	octree.output(out);
	return out;
}

#endif /* UTOCTREE_I_ */
