#ifndef OT_LAYOUT_GSUB_GLYPHPARAMETERADJUSTMENTS_HH
#define OT_LAYOUT_GSUB_GLYPHPARAMETERADJUSTMENTS_HH

#include "Common.hh"

namespace OT {
namespace Layout {
namespace GSUB_impl {

enum GlyphParameterOperation : uint16_t
{
  GLYPH_PARAMETER_ADD = 0,
  GLYPH_PARAMETER_SET = 1
};

/* Shared payload used by the custom SingleSubst and AlternateSubst formats.
 * `parameter` is the stable paramsN suffix, rather than a client-specific
 * dense slot, so the table remains meaningful when axes are registered in a
 * different order. */
struct GlyphParameterAdjustment
{
  bool sanitize (hb_sanitize_context_t *c) const
  {
    TRACE_SANITIZE (this);
    return_trace (likely (c->check_struct (this)) &&
		  operation <= GLYPH_PARAMETER_SET);
  }

  HBUINT16 parameter; /* Stable MetaPost paramsN suffix. */
  HBUINT16 operation;
  F16DOT16 value;

  public:
  DEFINE_SIZE_STATIC (8);
};

struct GlyphParameterAdjustmentSet
{
  bool sanitize (hb_sanitize_context_t *c) const
  {
    TRACE_SANITIZE (this);
    return_trace (adjustments.sanitize (c));
  }

  bool apply (hb_font_t *font, hb_glyph_info_t &info) const
  {
    if (!adjustments.len) return true;

    constexpr unsigned stack_capacity = 8;
    hb_glyph_parameter_adjustment_t stack_values[stack_capacity];
    if (adjustments.len <= stack_capacity)
    {
      for (unsigned i = 0; i < adjustments.len; ++i)
      {
	const auto &adjustment = adjustments[i];
	stack_values[i] = {
	  adjustment.parameter,
	  adjustment.operation == GLYPH_PARAMETER_SET
	      ? HB_GLYPH_PARAMETER_SET
	      : HB_GLYPH_PARAMETER_ADD,
	  static_cast<double> (adjustment.value.to_float ())};
      }
      return font->adjust_glyph_parameters (info, stack_values,
					    adjustments.len);
    }

    hb_vector_t<hb_glyph_parameter_adjustment_t> values;
    values.alloc (adjustments.len, true);
    for (const auto &adjustment : adjustments)
    {
      hb_glyph_parameter_adjustment_t value = {
	adjustment.parameter,
	adjustment.operation == GLYPH_PARAMETER_SET
	    ? HB_GLYPH_PARAMETER_SET
	    : HB_GLYPH_PARAMETER_ADD,
	static_cast<double> (adjustment.value.to_float ())};
      values.push (value);
    }
    if (unlikely (values.in_error ())) return false;
    return font->adjust_glyph_parameters (info, values.arrayZ, values.length);
  }

  protected:
  Array16Of<GlyphParameterAdjustment> adjustments;

  public:
  DEFINE_SIZE_ARRAY (2, adjustments);
};

}
}
}

#endif /* OT_LAYOUT_GSUB_GLYPHPARAMETERADJUSTMENTS_HH */
