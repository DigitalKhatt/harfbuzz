#include "hb-ot-layout-jtst-context.hh"
#include "hb-font.hh"
#include "hb-ot-layout-gsubgpos.hh"

namespace OT {

JustificationContext::JustificationContext (hb_font_t *font) : font{font}
{
  // minSpace = font->em_scale_x (MINSPACEWIDTH);
  // defaultSpace = font->em_scale_x (SPACEWIDTH);
}

int
JustificationContext::getWidth (hb_buffer_t *buffer, int *minLineWidth)
{
  // int maxCurrentlineWidth = 0;
  int currentlineWidth = 0;
  unsigned int glyph_count;

  hb_glyph_position_t *glyph_pos =
      hb_buffer_get_glyph_positions (buffer, &glyph_count);
  /* hb_glyph_info_t *glyph_info =
      hb_buffer_get_glyph_infos (buffer, &glyph_count);*/

  std::vector<int> spaces;

  for (unsigned int i = 0; i < glyph_count; i++)
  {
    currentlineWidth += glyph_pos[i].x_advance;
    /*
    if (glyph_info[i].codepoint == 32) { spaces.push_back (i); }
    else
    {
      maxCurrentlineWidth += glyph_pos[i].x_advance;
    }*/
  }
  /*
  if (minLineWidth != nullptr)
  {
    *minLineWidth = maxCurrentlineWidth + spaces.size () * (double) minSpace;
  }

  maxCurrentlineWidth += spaces.size () * (double) defaultSpace;*/

  return currentlineWidth;
}
void
JustificationContext::justify (int &diff,
			       hb_buffer_t *buffer,
			       hb_glyph_position_t *glyph_pos)
{
  unsigned int glyph_count;

  hb_glyph_info_t *glyph_info =
      hb_buffer_get_glyph_infos (buffer, &glyph_count);

  // int totalWeight = this->totalWeight;

  bool remaining = true;

  double remainingWidth = -1;

  bool stretch = diff > 0;

  std::map<int, GlyphExpansion> affectedIndexes;

  bool insideGroup = false;
  hb_position_t totalCurrentWidth = 0;
  hb_position_t totalNextWidth = 0;
  hb_position_t maxExpansion = 0;
  GlyphExpansion groupExpa{};
  groupExpa.weight = 0;
  std::vector<int> group;
  remaining = false;
  remainingWidth = 0.0;

  std::vector<unsigned int> NewGlyphsToExtend;

  double totalExpansion = 0.0;

  for (unsigned int i = 0; i < this->GlyphsToExtend.size (); i++)
  {

    int index = this->GlyphsToExtend[i];

    GlyphExpansion &expa = this->Expansions[index];

    // TODO: Need only IsAbsolute
    //   All values ares used for stretching if strech or shrinking if shrink
    //   => separate lookups for stretch and shrink

    if (stretch && expa.stretchIsAbsolute || !stretch && expa.shrinkIsAbsolute)
    {
      expa.MaxLeftTatweel = expa.MaxLeftTatweel - glyph_info[index].lefttatweel;
      expa.MaxRightTatweel =
	  expa.MaxRightTatweel - glyph_info[index].righttatweel;
      expa.stretchIsAbsolute = false;
      expa.MinLeftTatweel = expa.MinLeftTatweel - glyph_info[index].lefttatweel;
      expa.MinRightTatweel =
	  expa.MinRightTatweel - glyph_info[index].righttatweel;
      expa.shrinkIsAbsolute = false;

      if (stretch)
      {
	if (expa.MinLeftTatweel < 0) { expa.MinLeftTatweel = 0; }
	if (expa.MinRightTatweel < 0) { expa.MinRightTatweel = 0; }
      }
      else
      {
	if (expa.MinLeftTatweel > 0) { expa.MinLeftTatweel = 0; }
	if (expa.MinRightTatweel > 0) { expa.MinRightTatweel = 0; }
      }

      expa.MaxLeftTatweel -= expa.MinLeftTatweel;
      expa.MaxRightTatweel -= expa.MinRightTatweel;
    }

    group.push_back (i);

    hb_glyph_info_t info;
    info.codepoint = glyph_info[index].codepoint;

    info.lefttatweel = glyph_info[index].lefttatweel;
    info.righttatweel = glyph_info[index].righttatweel;

    hb_position_t currentWidth = 0;
    hb_position_t nextWidth = 0;
    hb_position_t maxWidth = 0;

    font->get_glyph_h_advances (1, &info.codepoint, sizeof (info),
				&currentWidth, 0);

    // oldWidth += glyph_pos[index].x_advance;

    info.codepoint = this->Substitutes[i];
    auto minLeft = expa.MinLeftTatweel;
    auto minRight = expa.MinRightTatweel;
    if (glyph_info[index].codepoint == info.codepoint && minLeft == 0.0 &&
	minRight == 0.0)
    {
      nextWidth = currentWidth;
    }
    else
    {
      info.lefttatweel = glyph_info[index].lefttatweel + minLeft;
      info.righttatweel = glyph_info[index].righttatweel + minRight;

      font->get_glyph_h_advances (1, &info.codepoint, sizeof (info), &nextWidth,
				  0);
    }
    auto expaLeft = expa.MaxLeftTatweel;
    auto expaRight = expa.MaxRightTatweel;
    info.lefttatweel += expaLeft;
    info.righttatweel += expaRight;

    font->get_glyph_h_advances (1, &info.codepoint, sizeof (info), &maxWidth,
				0);

    totalCurrentWidth += currentWidth;
    totalNextWidth += nextWidth;
    maxExpansion += maxWidth - nextWidth;

    groupExpa.weight += expa.weight;
    groupExpa.MinLeftTatweel += expa.MinLeftTatweel;
    groupExpa.MaxLeftTatweel += expa.MaxLeftTatweel;
    groupExpa.MinRightTatweel += expa.MinRightTatweel;
    groupExpa.MaxRightTatweel += expa.MaxRightTatweel;

    if (expa.startEndLig == StartEndLig::Start)
    {
      insideGroup = true;
      continue;
    }
    else if (insideGroup && expa.startEndLig != StartEndLig::End &&
	     expa.startEndLig != StartEndLig::EndKashida)
    {
      continue;
    }

    int widthDiff = totalNextWidth - totalCurrentWidth;

    if ((stretch && widthDiff <= diff && (widthDiff + maxExpansion) >= 0) ||
	(!stretch && widthDiff >= diff && (widthDiff + maxExpansion) <= 0))
    {
      diff = diff - widthDiff;
      totalExpansion += maxExpansion;
      for (int i : group)
      {
	int index = this->GlyphsToExtend[i];
	GlyphExpansion &expa = this->Expansions[index];

	glyph_info[index].codepoint = this->Substitutes[i];

	glyph_info[index].lefttatweel += expa.MinLeftTatweel;
	glyph_info[index].righttatweel += expa.MinRightTatweel;

	NewGlyphsToExtend.push_back (index);
      }
    }

    insideGroup = false;
    totalCurrentWidth = 0.0;
    totalNextWidth = 0.0;
    maxExpansion = 0;
    groupExpa = {};
    groupExpa.weight = 0;
    group.clear ();
  }

  double ratio = 0.0;

  if (stretch && diff > 0 && totalExpansion > 0)
  {
    if (totalExpansion > diff)
    {
      ratio = diff / totalExpansion;
      diff = 0.0;
    }
    else
    {
      ratio = 1.0;
      diff = diff - totalExpansion;
    }
  }
  else if (!stretch && diff < 0 && totalExpansion < 0)
  {
    if (totalExpansion < diff)
    {
      ratio = diff / totalExpansion;
      diff = 0.0;
    }
    else
    {
      ratio = 1;
      diff = diff - totalExpansion;
    }
  }

  if (ratio == 0.0) return;

  for (unsigned int i = 0; i < NewGlyphsToExtend.size (); i++)
  {

    int index = NewGlyphsToExtend[i];

    GlyphExpansion &expa = this->Expansions[index];

    glyph_info[index].lefttatweel += expa.MaxLeftTatweel * ratio;
    glyph_info[index].righttatweel += expa.MaxRightTatweel * ratio;
  }
}

} /* namespace OT */
