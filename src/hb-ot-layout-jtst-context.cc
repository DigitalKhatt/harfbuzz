#include "hb-ot-layout-jtst-context.hh"
#include "hb-font.hh"
#include "hb-ot-layout-gsubgpos.hh"

namespace OT {

JustificationContext::JustificationContext (hb_font_t *font) : font{font}
{
  minSpace = font->em_scale_x (MINSPACEWIDTH);
  defaultSpace = font->em_scale_x (SPACEWIDTH);
}

int
JustificationContext::getWidth (hb_buffer_t *buffer)
{
  int currentlineWidth = 0;
  unsigned int glyph_count;

  hb_glyph_position_t *glyph_pos =
      hb_buffer_get_glyph_positions (buffer, &glyph_count);
  hb_glyph_info_t *glyph_info =
      hb_buffer_get_glyph_infos (buffer, &glyph_count);

  std::vector<int> spaces;

  for (int i = 0; i < glyph_count; i++)
  {
    if (glyph_info[i].codepoint == 32)
    {
      glyph_pos[i].x_advance = minSpace;
      spaces.push_back (i);
    }
    else
    {
      currentlineWidth += glyph_pos[i].x_advance;
    }
  }

  currentlineWidth += spaces.size () * (double) defaultSpace;

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

  int totalWeight = this->totalWeight;

  bool remaining = true;

  double remainingWidth = -1;

  bool stretch = diff > 0;

  while (totalWeight != 0 && remaining && remainingWidth != 0.0)
  {

    double expaUnit = diff / totalWeight;
    if (expaUnit == 0.0)
    {
      diff = 0.0;
      break;
      ;
    }

    totalWeight = 0;

    std::map<int, GlyphExpansion> affectedIndexes;

    bool insideGroup = false;
    hb_position_t oldWidth = 0;
    hb_position_t newWidth = 0;
    GlyphExpansion groupExpa{};
    groupExpa.weight = 0;
    std::vector<int> group;
    remaining = false;
    remainingWidth = 0.0;

    for (int i = 0; i < this->GlyphsToExtend.size (); i++)
    {

      int index = this->GlyphsToExtend[i];

      GlyphExpansion &expa = this->Expansions[index];

      if (expa.stretchIsAbsolute)
      {
	expa.MaxLeftTatweel =
	    expa.MaxLeftTatweel - glyph_info[index].lefttatweel;
	expa.MaxRightTatweel =
	    expa.MaxRightTatweel - glyph_info[index].righttatweel;
	expa.stretchIsAbsolute = false;
      }

      if (expa.shrinkIsAbsolute)
      {
	expa.MinLeftTatweel =
	    expa.MinLeftTatweel - glyph_info[index].lefttatweel;
	expa.MinRightTatweel =
	    expa.MinRightTatweel - glyph_info[index].righttatweel;
	expa.shrinkIsAbsolute = false;
      }

      group.push_back (i);
      oldWidth += glyph_pos[index].x_advance;
      if (glyph_info[index].codepoint == this->Substitutes[i])
      { newWidth += glyph_pos[index].x_advance; }
      else
      {
	hb_position_t advance = 0;
	hb_glyph_info_t info;
	info.codepoint = this->Substitutes[i];
	info.lefttatweel = glyph_pos[index].lefttatweel;
	info.righttatweel = glyph_pos[index].righttatweel;

	font->get_glyph_h_advances (1, &info.codepoint, sizeof (info), &advance,
				    0);

	newWidth += advance;
      }

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

      if (groupExpa.weight == 0) goto next;

      int widthDiff = newWidth - oldWidth;

      auto tatweel = expaUnit * groupExpa.weight + remainingWidth;

      if ((stretch && widthDiff > tatweel) || (!stretch && widthDiff < tatweel))
      {
	totalWeight += groupExpa.weight;
	remainingWidth = tatweel;
      }
      else
      {

	diff -= widthDiff;
	tatweel -= widthDiff;

	for (int i : group)
	{

	  int index = this->GlyphsToExtend[i];
	  GlyphExpansion &expa = this->Expansions[index];

	  glyph_info[index].codepoint = this->Substitutes[i];

	  if (stretch)
	  {
	    expa.MaxLeftTatweel =
		expa.MaxLeftTatweel > 0 ? expa.MaxLeftTatweel : 0;
	    expa.MaxRightTatweel =
		expa.MaxRightTatweel > 0 ? expa.MaxRightTatweel : 0;

	    auto maxTatweel = expa.MaxLeftTatweel + expa.MaxRightTatweel;
	    hb_position_t maxStretch = 0;
	    if (maxTatweel != 0)
	    {

	      hb_position_t actualWidth = 0;
	      hb_position_t maxWidth = 0;
	      hb_glyph_info_t info;
	      info.codepoint = glyph_info[index].codepoint;
	      info.lefttatweel = glyph_info[index].lefttatweel;
	      info.righttatweel = glyph_info[index].righttatweel;

	      font->get_glyph_h_advances (1, &info.codepoint, sizeof (info),
					  &actualWidth, 0);

	      info.lefttatweel =
		  glyph_info[index].lefttatweel + expa.MaxLeftTatweel;
	      info.righttatweel =
		  glyph_info[index].righttatweel + expa.MaxRightTatweel;

	      font->get_glyph_h_advances (1, &info.codepoint, sizeof (info),
					  &maxWidth, 0);

	      maxStretch = maxWidth - actualWidth;
	    }
	    if (maxStretch != 0 ){

	      double ratio = maxTatweel / maxStretch;

	      // auto maxStretch = maxTatweel * nuqta;

	      if (tatweel > maxStretch)
	      {
		remainingWidth = tatweel - maxStretch;
		tatweel = maxStretch;
	      }
	      else
	      {
		remainingWidth = 0;
	      }

	      double leftTatweel =
		  (tatweel * (expa.MaxLeftTatweel / maxTatweel)) * ratio;
	      // / nuqta;
	      double rightTatweel =
		  (tatweel * (expa.MaxRightTatweel / maxTatweel)) * ratio;
	      // / nuqta;

	      glyph_info[index].lefttatweel += leftTatweel;
	      glyph_info[index].righttatweel += rightTatweel;
	      /*
	      if (std::isnan (glyph_info[index].lefttatweel) ||
		  std::isnan (glyph_info[index].righttatweel))
	      { int stop = 5; }*/

	      diff -= tatweel;

	      if (maxStretch > tatweel)
	      {
		expa.MaxLeftTatweel -= leftTatweel;
		expa.MaxRightTatweel -= rightTatweel;
		remaining = true;
		totalWeight += expa.weight;
	      }
	      else
	      {
		expa.weight = 0;
	      }
	    }
	    else
	    {
	      expa.weight = 0;
	    }
	  }
	  else
	  {
	    expa.MinLeftTatweel =
		expa.MinLeftTatweel < 0 ? expa.MinLeftTatweel : 0;
	    expa.MinRightTatweel =
		expa.MinRightTatweel < 0 ? expa.MinRightTatweel : 0;

	    auto MinTatweel = expa.MinLeftTatweel + expa.MinRightTatweel;
	    hb_position_t maxShrink = 0;
	    if (MinTatweel != 0)
	    {
	      hb_position_t actualWidth = 0;
	      hb_position_t maxWidth = 0;
	      hb_glyph_info_t info;
	      info.codepoint = glyph_info[index].codepoint;
	      info.lefttatweel = glyph_info[index].lefttatweel;
	      info.righttatweel = glyph_info[index].righttatweel;

	      font->get_glyph_h_advances (1, &info.codepoint, sizeof (info),
					  &actualWidth, 0);

	      info.lefttatweel =
		  glyph_info[index].lefttatweel + expa.MinLeftTatweel;
	      info.righttatweel =
		  glyph_info[index].righttatweel + expa.MinRightTatweel;

	      font->get_glyph_h_advances (1, &info.codepoint, sizeof (info),
					  &maxWidth, 0);

	      maxShrink = maxWidth - actualWidth;
	    }
	    if (maxShrink != 0)
	    {

	      // auto maxShrink = MinTatweel * nuqta;
	      double ratio = std::abs (MinTatweel / maxShrink);

	      if (tatweel < maxShrink)
	      {
		remainingWidth = tatweel - maxShrink;
		tatweel = maxShrink;
	      }
	      else
	      {
		remainingWidth = 0;
	      }

	      double leftTatweel =
		  (tatweel * (expa.MinLeftTatweel / MinTatweel)) * ratio;
	      double rightTatweel =
		  (tatweel * (expa.MinRightTatweel / MinTatweel)) * ratio;

	      /*
	      if (std::isnan (leftTatweel) || std::isnan (rightTatweel))
	      { int stop = 5; }*/

	      glyph_info[index].lefttatweel += leftTatweel;
	      glyph_info[index].righttatweel += rightTatweel;

	      diff -= tatweel;

	      if (tatweel > maxShrink)
	      {
		expa.MinLeftTatweel -= leftTatweel;
		expa.MinRightTatweel -= rightTatweel;
		remaining = true;
		totalWeight += expa.weight;
	      }
	      else
	      {
		expa.weight = 0;
	      }
	    }
	    else
	    {
	      expa.weight = 0;
	    }
	  }
	}
      }

    next:
      insideGroup = false;
      oldWidth = 0.0;
      newWidth = 0.0;
      groupExpa = {};
      groupExpa.weight = 0;
      group.clear ();
    }
  }
}

} /* namespace OT */
