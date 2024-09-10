#ifndef OT_LAYOUT_GSUB_ALTERNATESETWITHTATWEELS_HH
#define OT_LAYOUT_GSUB_ALTERNATESETWITHTATWEELS_HH

#include "Common.hh"

namespace OT {
namespace Layout {
namespace GSUB_impl {

struct Tatweels
{
  bool sanitize (hb_sanitize_context_t *c) const
  {
    TRACE_SANITIZE (this);
    return_trace (likely (c->check_struct (this)));
  }
  
  F16DOT16 leftTatweel;
  F16DOT16 rightTatweel;

  public:
  DEFINE_SIZE_STATIC (8);
};

struct AlternateSetWithTatweels
{
  protected:
  Array16Of<HBGlyphID16>
                alternates;             /* Array of alternate GlyphIDs--in
                                         * arbitrary order */
  Array16Of<Tatweels>
                tatweelsX;		/* Array of alternate GlyphIDs--in
                                         * arbitrary order */
  public:
  //DEFINE_SIZE_ARRAY (4, alternates);

  bool sanitize (hb_sanitize_context_t *c) const
  {
    TRACE_SANITIZE (this);
    return_trace (alternates.sanitize (c));
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
    unsigned int count = alternates.len;

    if (unlikely (!count)) return_trace (false);

    hb_mask_t glyph_mask = c->buffer->cur().mask;
    hb_mask_t lookup_mask = c->lookup_mask;

    /* Note: This breaks badly if two features enabled this lookup together. */
    unsigned int shift = hb_ctz (lookup_mask);
    unsigned int alt_index = ((lookup_mask & glyph_mask) >> shift);

    /* If alt_index is MAX_VALUE, randomize feature if it is the rand feature. */
    if (alt_index == HB_OT_MAP_MAX_VALUE && c->random)
    {
      /* Maybe we can do better than unsafe-to-break all; but since we are
       * changing random state, it would be hard to track that.  Good 'nough. */
      c->buffer->unsafe_to_break (0, c->buffer->len);
      alt_index = c->random_number () % count + 1;
    }

    if (unlikely (alt_index > count || alt_index == 0)) return_trace (false);

    if (HB_BUFFER_MESSAGE_MORE && c->buffer->messaging ())
    {
      c->buffer->sync_so_far ();
      c->buffer->message (c->font,
			  "replacing glyph at %u (alternate substitution)",
			  c->buffer->idx);
    }

    auto subst = alternates[alt_index - 1];
    const auto &tatweels = StructAfter<decltype (tatweelsX)> (alternates);
    auto tats = tatweels[alt_index - 1];

    auto &info = c->buffer->cur ();

    info.lefttatweel += tats.leftTatweel.to_float ();
    info.righttatweel += tats.rightTatweel.to_float ();

    c->replace_glyph (subst);

    //c->replace_glyph (alternates[alt_index - 1]);

    if (HB_BUFFER_MESSAGE_MORE && c->buffer->messaging ())
    {
      c->buffer->message (c->font,
			  "replaced glyph at %u (alternate substitution)",
			  c->buffer->idx - 1u);
    }

    return_trace (true);
  }

  unsigned
  get_alternates (unsigned        start_offset,
                  unsigned       *alternate_count  /* IN/OUT.  May be NULL. */,
                  hb_codepoint_t *alternate_glyphs /* OUT.     May be NULL. */) const
  {
    if (alternates.len && alternate_count)
    {
      + alternates.as_array ().sub_array (start_offset, alternate_count)
      | hb_sink (hb_array (alternate_glyphs, *alternate_count))
      ;
    }
    return alternates.len;
  }

  template <typename Iterator,
            hb_requires (hb_is_source_of (Iterator, hb_codepoint_t))>
  bool serialize (hb_serialize_context_t *c,
                  Iterator alts)
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


#endif /* OT_LAYOUT_GSUB_ALTERNATESETWITHTATWEELS_HH */
