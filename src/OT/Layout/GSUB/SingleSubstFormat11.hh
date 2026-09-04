#ifndef OT_LAYOUT_GSUB_SINGLESUBSTFORMAT11_HH
#define OT_LAYOUT_GSUB_SINGLESUBSTFORMAT11_HH

#include "Common.hh"
#include "GlyphParameterAdjustments.hh"

namespace OT {
namespace Layout {
namespace GSUB_impl {

/* VisualMetaFont SingleSubst: parallel substitute and parameter-set arrays.
 * Parameter-set offsets are relative to this subtable. */
struct SingleSubstFormat11
{
  bool may_have_non_1to1 () const { return false; }
  void closure_lookups (hb_closure_lookups_context_t *) const {}

  bool intersects (const hb_set_t *glyphs) const
  { return (this + coverage).intersects (glyphs); }

  void closure (hb_closure_context_t *c) const
  {
    if (unlikely (!(this + coverage).intersects (c->glyphs))) return;
    c->output->add_array (substitutes.arrayZ, substitutes.len);
  }

  void collect_glyphs (hb_collect_glyphs_context_t *c) const
  {
    if (unlikely (!(this + coverage).collect_coverage (c->input))) return;
    c->output->add_array (substitutes.arrayZ, substitutes.len);
  }

  const Coverage &get_coverage () const { return this + coverage; }

  bool would_apply (hb_would_apply_context_t *c) const
  {
    return c->len == 1 &&
	   (this + coverage).get_coverage (c->glyphs[0]) != NOT_COVERED;
  }

  bool apply (hb_ot_apply_context_t *c) const
  {
    TRACE_APPLY (this);
    unsigned index =
	(this + coverage).get_coverage (c->buffer->cur ().codepoint);
    if (likely (index == NOT_COVERED)) return_trace (false);
    if (unlikely (index >= substitutes.len)) return_trace (false);

    const auto &parameterSets =
	StructAfter<decltype (parameterSetsX)> (substitutes);
    if (unlikely (index >= parameterSets.len)) return_trace (false);

    auto &info = c->buffer->cur ();
    if (!(this + parameterSets[index]).apply (c->font, info))
      return_trace (false);

    c->replace_glyph (substitutes[index]);
    return_trace (true);
  }

  template <typename Iterator>
  bool serialize (hb_serialize_context_t *, Iterator)
  { return false; }

  bool subset (hb_subset_context_t *) const { return false; }

  bool sanitize (hb_sanitize_context_t *c) const
  {
    TRACE_SANITIZE (this);
    if (unlikely (!coverage.sanitize (c, this) ||
		  !substitutes.sanitize (c)))
      return_trace (false);
    const auto &parameterSets =
	StructAfter<decltype (parameterSetsX)> (substitutes);
    return_trace (parameterSets.sanitize (c, this));
  }

  protected:
  HBUINT16 format;
  Offset16To<Coverage> coverage;
  Array16Of<HBGlyphID16> substitutes;
  Array16Of<Offset16To<GlyphParameterAdjustmentSet>> parameterSetsX;

  public:
  DEFINE_SIZE_MIN (6);
};

}
}
}

#endif /* OT_LAYOUT_GSUB_SINGLESUBSTFORMAT11_HH */
