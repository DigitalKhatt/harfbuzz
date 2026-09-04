#ifndef OT_LAYOUT_GSUB_ALTERNATESETWITHPARAMETERS_HH
#define OT_LAYOUT_GSUB_ALTERNATESETWITHPARAMETERS_HH

#include "Common.hh"
#include "GlyphParameterAdjustments.hh"

namespace OT {
namespace Layout {
namespace GSUB_impl {

/* VisualMetaFont AlternateSet: each alternate has a parallel parameter set.
 * Parameter-set offsets are relative to this set. */
struct AlternateSetWithParameters
{
  bool sanitize (hb_sanitize_context_t *c) const
  {
    TRACE_SANITIZE (this);
    if (unlikely (!alternates.sanitize (c))) return_trace (false);
    const auto &parameterSets =
	StructAfter<decltype (parameterSetsX)> (alternates);
    return_trace (parameterSets.sanitize (c, this));
  }

  bool intersects (const hb_set_t *glyphs) const
  { return hb_any (alternates, glyphs); }

  void closure (hb_closure_context_t *c) const
  { c->output->add_array (alternates.arrayZ, alternates.len); }

  void collect_glyphs (hb_collect_glyphs_context_t *c) const
  { c->output->add_array (alternates.arrayZ, alternates.len); }

  bool apply (hb_ot_apply_context_t *c) const
  {
    TRACE_APPLY (this);
    unsigned count = alternates.len;
    if (unlikely (!count)) return_trace (false);

    hb_mask_t glyph_mask = c->buffer->cur ().mask;
    hb_mask_t lookup_mask = c->lookup_mask;
    unsigned shift = hb_ctz (lookup_mask);
    unsigned alt_index = (lookup_mask & glyph_mask) >> shift;
    if (alt_index == HB_OT_MAP_MAX_VALUE && c->random)
    {
      c->buffer->unsafe_to_break (0, c->buffer->len);
      alt_index = c->random_number () % count + 1;
    }
    if (unlikely (alt_index > count || alt_index == 0))
      return_trace (false);

    const auto &parameterSets =
	StructAfter<decltype (parameterSetsX)> (alternates);
    if (unlikely (alt_index > parameterSets.len)) return_trace (false);

    auto &info = c->buffer->cur ();
    if (!(this + parameterSets[alt_index - 1]).apply (c->font, info))
      return_trace (false);
    c->replace_glyph (alternates[alt_index - 1]);
    return_trace (true);
  }

  unsigned get_alternates (unsigned start_offset,
			   unsigned *alternate_count,
			   hb_codepoint_t *alternate_glyphs) const
  {
    if (alternates.len && alternate_count)
      + alternates.as_array ().sub_array (start_offset, alternate_count)
      | hb_sink (hb_array (alternate_glyphs, *alternate_count));
    return alternates.len;
  }

  bool subset (hb_subset_context_t *) const { return false; }

  protected:
  Array16Of<HBGlyphID16> alternates;
  Array16Of<Offset16To<GlyphParameterAdjustmentSet>> parameterSetsX;
};

}
}
}

#endif /* OT_LAYOUT_GSUB_ALTERNATESETWITHPARAMETERS_HH */
