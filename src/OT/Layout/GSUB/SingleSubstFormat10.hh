#ifndef OT_LAYOUT_GSUB_SINGLESUBSTFORMAT10_HH
#define OT_LAYOUT_GSUB_SINGLESUBSTFORMAT10_HH

#include "Common.hh"

namespace OT {
namespace Layout {
namespace GSUB_impl {

// Added for VisualMetaFont
struct JustParams
{
  bool sanitize (hb_sanitize_context_t *c) const
  {
    TRACE_SANITIZE (this);
    return_trace (likely (c->check_struct (this)));
  }

  HBGlyphID16 substitute;
  F16DOT16 minLeftTatweel;
  F16DOT16 maxLeftTatweel;
  F16DOT16 minRightTatweel;
  F16DOT16 maxRightTatweel;
  HBUINT16 weight;
  HBUINT32 flags;

  public:
  DEFINE_SIZE_STATIC (24);
};

// Added for VisualMetaFont
struct SingleSubstFormat10
{
  bool may_have_non_1to1 () const { return false; }

  bool intersects (const hb_set_t *glyphs) const
  {
    return (this + coverage).intersects (glyphs);
  }

  void closure_lookups (hb_closure_lookups_context_t *c) const {}

  void closure (hb_closure_context_t *c) const
  {
    /*
    +hb_zip (this + coverage, substitute) | hb_filter (*c->glyphs, hb_first) |
	hb_map (hb_second) | hb_sink (c->output);*/
  }

  void collect_glyphs (hb_collect_glyphs_context_t *c) const
  {
    /*
    if (unlikely (!(this + coverage).add_coverage (c->input))) return;
    +hb_zip (this + coverage, substitute) | hb_map (hb_second) |
	hb_sink (c->output);*/
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
    unsigned int index =
	(this + coverage).get_coverage (c->buffer->cur ().codepoint);
    if (likely (index == NOT_COVERED)) return_trace (false);

    if (unlikely (index >= substitute.len)) return_trace (false);

      // VisualMetaFont
      // c->replace_glyph (substitute[index]);

      // return_trace (true);
#ifndef HB_NO_JUSTIFICATION
    if (c->buffer->justContext != nullptr)
    {
      OT::JustificationContext &justContext = *c->buffer->justContext;
      // justContext.GlyphsToExtend.push_back (c->buffer->idx);
      justContext.Substitutes.push_back (substitute[index].substitute);

      OT::GlyphExpansion expa;

      expa.MinLeftTatweel = substitute[index].minLeftTatweel.to_float ();
      expa.MaxLeftTatweel = substitute[index].maxLeftTatweel.to_float ();
      expa.MinRightTatweel = substitute[index].minRightTatweel.to_float ();
      expa.MaxRightTatweel = substitute[index].maxRightTatweel.to_float ();
      expa.weight = substitute[index].weight;
      expa.startEndLig = (StartEndLig) (substitute[index].flags & 7);
      expa.stretchIsAbsolute = substitute[index].flags & 8;
      expa.shrinkIsAbsolute = substitute[index].flags & 16;
      expa.index = c->buffer->idx;

      // justContext.Expansions.insert ({c->buffer->idx, expa});
      justContext.GlyphsToExtend.push_back (expa);
      justContext.totalWeight += expa.weight;
    }
#endif

    hb_substitution_context_t substitution_context;

    // substitution_context.ot_context = c;
    substitution_context.lookup_index = c->lookup_index;
    substitution_context.subtable_index = c->subtable_index;
    substitution_context.buffer = c->buffer;
    substitution_context.substitute = substitute[index].substitute;
    substitution_context.curr = c->buffer->idx;

    auto result = c->font->get_substitution (&substitution_context);

    if (result) {
      c->replace_glyph (substitute[index].substitute);
    }

    return_trace (result);
  }

  template <typename Iterator,
	    hb_requires (hb_is_sorted_source_of (Iterator,
						 hb_codepoint_pair_t))>
  bool serialize (hb_serialize_context_t *c, Iterator it)
  {
    /*
    TRACE_SERIALIZE (this);
    auto substitutes = +it | hb_map (hb_second);
    auto glyphs = +it | hb_map_retains_sorting (hb_first);
    if (unlikely (!c->extend_min (*this))) return_trace (false);
    if (unlikely (!substitute.serialize (c, substitutes))) return_trace (false);
    if (unlikely (!coverage.serialize (c, this).serialize (c, glyphs)))
      return_trace (false);*/
    return true;
  }

  bool subset (hb_subset_context_t *c) const
  {
    /*
    TRACE_SUBSET (this);
    const hb_set_t &glyphset = *c->plan->glyphset_gsub ();
    const hb_map_t &glyph_map = *c->plan->glyph_map;

    auto it = +hb_zip (this + coverage, substitute) |
	      hb_filter (glyphset, hb_first) | hb_filter (glyphset, hb_second) |
	      hb_map_retains_sorting (
		  [&] (hb_pair_t<hb_codepoint_t, const HBGlyphID &> p)
		      -> hb_codepoint_pair_t {
		    return hb_pair (glyph_map[p.first], glyph_map[p.second]);
		  });

    bool ret = bool (it);
    SingleSubst_serialize (c->serializer, it);
    return_trace (ret);*/
    return true;
  }

  bool sanitize (hb_sanitize_context_t *c) const
  {
    TRACE_SANITIZE (this);
    return_trace (coverage.sanitize (c, this) && substitute.sanitize (c));
  }

  protected:
  HBUINT16 format;		  /* Format identifier--format = 2 */
  Offset16To<Coverage> coverage;	  /* Offset to Coverage table--from
				   * beginning of Substitution table */

  Array16Of<JustParams> substitute; /* Array of substitute
				   * GlyphIDs--ordered by Coverage Index */
  public:
  DEFINE_SIZE_ARRAY (6, substitute);
};

}
}
}


#endif /* OT_LAYOUT_GSUB_SINGLESUBSTFORMAT10_HH */
