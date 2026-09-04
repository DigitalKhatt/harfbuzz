#ifndef OT_LAYOUT_GSUB_ALTERNATESUBSTFORMAT10_HH
#define OT_LAYOUT_GSUB_ALTERNATESUBSTFORMAT10_HH

#include "AlternateSetWithParameters.hh"
#include "Common.hh"

namespace OT {
namespace Layout {
namespace GSUB_impl {

struct AlternateSubstFormat10
{
  bool sanitize (hb_sanitize_context_t *c) const
  {
    TRACE_SANITIZE (this);
    return_trace (coverage.sanitize (c, this) &&
		  alternateSet.sanitize (c, this));
  }

  bool intersects (const hb_set_t *glyphs) const
  { return (this + coverage).intersects (glyphs); }
  bool may_have_non_1to1 () const { return false; }
  void closure (hb_closure_context_t *) const {}
  void closure_lookups (hb_closure_lookups_context_t *) const {}
  void collect_glyphs (hb_collect_glyphs_context_t *) const {}
  const Coverage &get_coverage () const { return this + coverage; }

  bool would_apply (hb_would_apply_context_t *c) const
  {
    return c->len == 1 &&
	   (this + coverage).get_coverage (c->glyphs[0]) != NOT_COVERED;
  }

  unsigned get_glyph_alternates (hb_codepoint_t gid, unsigned start_offset,
				 unsigned *alternate_count,
				 hb_codepoint_t *alternate_glyphs) const
  {
    unsigned index = (this + coverage).get_coverage (gid);
    if (unlikely (index == NOT_COVERED || index >= alternateSet.len)) return 0;
    return (this + alternateSet[index])
	.get_alternates (start_offset, alternate_count, alternate_glyphs);
  }

  bool apply (hb_ot_apply_context_t *c) const
  {
    TRACE_APPLY (this);
    unsigned index =
	(this + coverage).get_coverage (c->buffer->cur ().codepoint);
    if (likely (index == NOT_COVERED) || unlikely (index >= alternateSet.len))
      return_trace (false);
    return_trace ((this + alternateSet[index]).apply (c));
  }

  bool subset (hb_subset_context_t *) const { return false; }

  protected:
  HBUINT16 format;
  Offset16To<Coverage> coverage;
  Array16Of<Offset16To<AlternateSetWithParameters>> alternateSet;

  public:
  DEFINE_SIZE_ARRAY (6, alternateSet);
};

}
}
}

#endif /* OT_LAYOUT_GSUB_ALTERNATESUBSTFORMAT10_HH */
