#ifndef OT_LAYOUT_GSUB_GLYPHWITHTATWEEL_HH
#define OT_LAYOUT_GSUB_GLYPHWITHTATWEEL_HH

#include "Common.hh"

namespace OT {
namespace Layout {
namespace GSUB_impl {

// Added for VisualMetaFont
struct GlyphWithTatweels
{
  bool sanitize (hb_sanitize_context_t *c) const
  {
    TRACE_SANITIZE (this);
    return_trace (likely (c->check_struct (this)));
  }

  HBGlyphID16 substitute;
  F16DOT16 leftTatweel;
  F16DOT16 rightTatweel;

  public:
  DEFINE_SIZE_STATIC (10);
};

}
}
}
#endif /* OT_LAYOUT_GSUB_GLYPHWITHTATWEEL_HH */
