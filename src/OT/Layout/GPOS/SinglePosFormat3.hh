#ifndef OT_LAYOUT_GPOS_SINGLEPOSFORMAT3_HH
#define OT_LAYOUT_GPOS_SINGLEPOSFORMAT3_HH

#include "Common.hh"

namespace OT {
namespace Layout {
namespace GPOS_impl {

// Added for VisualMetaFont for coloring glyph

struct SinglePosFormat3
{
  bool intersects (const hb_set_t *glyphs) const
  {
    return (this + coverage).intersects (glyphs);
  }

  void closure_lookups (hb_closure_lookups_context_t *c) const {}
  void
  collect_variation_indices (hb_collect_variation_indices_context_t *c) const
  {
    if (!valueFormat.has_device ()) return;

    auto it = +hb_iter (this + coverage) | hb_filter (c->glyph_set);

    if (!it) return;
    valueFormat.collect_variation_indices (
	c, this, values.as_array (valueFormat.get_len ()));
  }

  void collect_glyphs (hb_collect_glyphs_context_t *c) const
  {
    if (unlikely (!(this + coverage).collect_coverage (c->input))) return;
  }

  const Coverage &get_coverage () const { return this + coverage; }

  bool apply (hb_ot_apply_context_t *c) const
  {
    TRACE_APPLY (this);
    hb_buffer_t *buffer = c->buffer;
    unsigned int index =
	(this + coverage).get_coverage (buffer->cur ().codepoint);
    if (likely (index == NOT_COVERED)) return_trace (false);

    if (likely (index >= valueCount)) return_trace (false);

    /*valueFormat.apply_value (c, this, &values[index * valueFormat.get_len ()],
			     buffer->cur_pos ());*/
    const Value *mvalues = &values[index * valueFormat.get_len ()];

    auto p1 = *reinterpret_cast<const HBINT16 *> (mvalues++);
    auto p2 = *reinterpret_cast<const HBINT16 *> (mvalues++);
    auto p3 = *reinterpret_cast<const HBINT16 *> (mvalues++);
    auto p4 = *reinterpret_cast<const HBINT16 *> (mvalues);

    c->font->record_glyph_positioning (buffer->cur (), c->lookup_index, c->subtable_index,
                                      (p1 << 24) + (p2 << 16) + (p3 << 8) + p4);

    buffer->idx++;
    return_trace (true);
  }

  template <typename Iterator,
	    typename SrcLookup,
	    hb_requires (hb_is_iterator (Iterator))>
  void serialize (hb_serialize_context_t *c,
		  const SrcLookup *src,
		  Iterator it,
		  ValueFormat newFormat,
		  const hb_hashmap_t<unsigned, hb_pair_t<unsigned, int>>
		      *layout_variation_idx_delta_map)
  {
    auto out = c->extend_min (this);
    if (unlikely (!out)) return;
    if (unlikely (!c->check_assign (valueFormat, newFormat,
				    HB_SERIALIZE_ERROR_INT_OVERFLOW)))
      return;
    if (unlikely (!c->check_assign (valueCount, it.len (),
				    HB_SERIALIZE_ERROR_ARRAY_OVERFLOW)))
      return;

    +it | hb_map (hb_second) | hb_apply ([&] (hb_array_t<const Value> _) {
      src->get_value_format ().copy_values (c, newFormat, src, &_,
					    layout_variation_idx_delta_map);
    });

    auto glyphs = +it | hb_map_retains_sorting (hb_first);

    coverage.serialize_serialize (c, glyphs);
  }

  bool subset (hb_subset_context_t *c) const
  {
    TRACE_SUBSET (this);
    const hb_set_t &glyphset = *c->plan->glyphset_gsub ();
    const hb_map_t &glyph_map = *c->plan->glyph_map;

    hb_set_t intersection;
    (this + coverage).intersect_set (glyphset, intersection);

    auto it = +hb_iter (intersection) | hb_map_retains_sorting (glyph_map) |
	      hb_zip (hb_repeat (values.as_array (valueFormat.get_len ())));

    bool ret = bool (it);
    SinglePos_serialize (c->serializer, this, it,
			 &c->plan->layout_variation_idx_delta_map,
			 c->plan->all_axes_pinned);
    return_trace (ret);
  }

  bool sanitize (hb_sanitize_context_t *c) const
  {
    TRACE_SANITIZE (this);
    return_trace (c->check_struct (this) && coverage.sanitize (c, this) &&
		  valueFormat.sanitize_values (c, this, values, valueCount));
  }

  ValueFormat get_value_format () const { return valueFormat; }

  protected:
  HBUINT16 format;	       /* Format identifier--format = 2 */
  Offset16To<Coverage> coverage; /* Offset to Coverage table--from
				* beginning of subtable */
  ValueFormat valueFormat;     /* Defines the types of data in the
				* ValueRecord */
  HBUINT16 valueCount;	       /* Number of ValueRecords */
  ValueRecord values;	       /* Array of ValueRecords--positioning
				* values applied to glyphs */
  public:
  DEFINE_SIZE_ARRAY (8, values);
};


}
}
}

#endif /* OT_LAYOUT_GPOS_SINGLEPOSFORMAT3_HH */
