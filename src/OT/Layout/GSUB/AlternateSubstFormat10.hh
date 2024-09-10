#ifndef OT_LAYOUT_GSUB_ALTERNATESUBSTFORMAT10_HH
#define OT_LAYOUT_GSUB_ALTERNATESUBSTFORMAT10_HH

#include "AlternateSetWithTatweels.hh"
#include "Common.hh"


namespace OT {
namespace Layout {
namespace GSUB_impl {


struct AlternateSubstFormat10
{
  protected:
  HBUINT16      format;                 /* Format identifier--format = 10 */
  Offset16To<Coverage>
                coverage;               /* Offset to Coverage table--from
                                         * beginning of Substitution table */
  Array16Of<Offset16To<AlternateSetWithTatweels>>
                alternateSet;           /* Array of AlternateSet tables
                                         * ordered by Coverage Index */
  public:
  DEFINE_SIZE_ARRAY (2 + 2 * 2, alternateSet);

  bool sanitize (hb_sanitize_context_t *c) const
  {
    TRACE_SANITIZE (this);
    return_trace (coverage.sanitize (c, this) && alternateSet.sanitize (c, this));
  }

  bool intersects (const hb_set_t *glyphs) const
  { return (this+coverage).intersects (glyphs); }

  bool may_have_non_1to1 () const
  { return false; }

  void closure (hb_closure_context_t *c) const
  {    
  }

  void closure_lookups (hb_closure_lookups_context_t *c) const {}

  void collect_glyphs (hb_collect_glyphs_context_t *c) const
  {   
  }

  const Coverage &get_coverage () const { return this+coverage; }

  bool would_apply (hb_would_apply_context_t *c) const
  { return c->len == 1 && (this+coverage).get_coverage (c->glyphs[0]) != NOT_COVERED; }

  unsigned
  get_glyph_alternates (hb_codepoint_t  gid,
                        unsigned        start_offset,
                        unsigned       *alternate_count  /* IN/OUT.  May be NULL. */,
                        hb_codepoint_t *alternate_glyphs /* OUT.     May be NULL. */) const
  { return (this+alternateSet[(this+coverage).get_coverage (gid)])
           .get_alternates (start_offset, alternate_count, alternate_glyphs); }

  bool apply (hb_ot_apply_context_t *c) const
  {
    TRACE_APPLY (this);

    unsigned int index = (this+coverage).get_coverage (c->buffer->cur().codepoint);
    if (likely (index == NOT_COVERED)) return_trace (false);

    return_trace ((this+alternateSet[index]).apply (c));
  }

  bool serialize (hb_serialize_context_t *c,
                  hb_sorted_array_t<const HBGlyphID16> glyphs,
                  hb_array_t<const unsigned int> alternate_len_list,
                  hb_array_t<const HBGlyphID16> alternate_glyphs_list)
  {
    return false;
  }

  bool subset (hb_subset_context_t *c) const
  {
    return false;
  }
};

}
}
}

#endif  /* OT_LAYOUT_GSUB_ALTERNATESUBSTFORMAT10_HH */
